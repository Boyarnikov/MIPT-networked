#include <enet/enet.h>
#include <iostream>
#include "entity.h"
#include "protocol.h"
#include "bitstream.h"
#include <stdlib.h>
#include <vector>
#include <map>

static std::vector<Entity> entities;
static std::map<uint16_t, ENetPeer*> controlledMap;

static uint16_t create_random_entity()
{
  uint16_t newEid = entities.size();
  uint32_t color = 0x44000000 * (1 + rand() % 4) +
                   0x00440000 * (1 + rand() % 4) +
                   0x00004400 * (1 + rand() % 4) + 
                   0x000000ff;
  float x = (rand() % 100 - 50) * 5.f;
  float y = (rand() % 100 - 50) * 5.f;
  float size = (rand() % 5 + 5.f);
  Entity ent = {color, x, y, newEid, false, 0.f, 0.f, size};
  entities.push_back(ent);
  return newEid;
}

void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
  // send all entities
  for (const Entity &ent : entities)
    send_new_entity(peer, ent);

  // find max eid
  uint16_t newEid = create_random_entity();
  const Entity& ent = entities[newEid];

  controlledMap[newEid] = peer;

  // send info about new entity to everyone
  for (size_t i = 0; i < host->connectedPeers; ++i)
    send_new_entity(&host->peers[i], ent);
  // send info about controlled entity
  send_set_controlled_entity(peer, newEid);
}

void on_state(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  float x = 0.f; float y = 0.f; float size = 1.f;
  deserialize_entity_state(packet, eid, x, y, size);
  for (Entity &e : entities)
    if (e.eid == eid)
    {
      e.x = x;
      e.y = y;
      e.size = size;
    } 
}


void teleport_to_random_position(Entity &e) {
  e.x = (rand() % 200 - 100) * 5.f;
  e.y = (rand() % 200 - 100) * 5.f;
}


void on_collision(Entity &e1, Entity &e2) {
  if (e1.size > e2.size) {
    e1.size += e2.size / 2;
    e2.size /= 2;
    teleport_to_random_position(e2);
  }
  else if (e1.size < e2.size) {
    e2.size += e1.size / 2;
    e1.size /= 2;
    teleport_to_random_position(e1);
  }
  else {
    teleport_to_random_position(e1);
    teleport_to_random_position(e2);
  }
  if (controlledMap[e1.eid] != nullptr) {
    send_entity_update(controlledMap[e1.eid], e1.eid, e1.x, e1.y, e1.size);
  }
  if (controlledMap[e2.eid] != nullptr) {
    send_entity_update(controlledMap[e2.eid], e2.eid, e2.x, e2.y, e2.size);
  }
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10131;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  constexpr int numAi = 10;

  for (int i = 0; i < numAi; ++i)
  {
    uint16_t eid = create_random_entity();
    entities[eid].serverControlled = true;
    controlledMap[eid] = nullptr;
  }

  uint32_t lastTime = enet_time_get();
  while (true)
  {
    uint32_t curTime = enet_time_get();
    float dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
          case E_CLIENT_TO_SERVER_JOIN:
            on_join(event.packet, event.peer, server);
            break;
          case E_CLIENT_TO_SERVER_STATE:
            on_state(event.packet);
            break;
        };
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    for (Entity &e : entities)
    {
      if (e.serverControlled)
      {
        const float diffX = e.targetX - e.x;
        const float diffY = e.targetY - e.y;
        const float dirX = diffX > 0.f ? 1.f : -1.f;
        const float dirY = diffY > 0.f ? 1.f : -1.f;
        constexpr float spd = 50.f;
        e.x += dirX * spd * dt;
        e.y += dirY * spd * dt;
        if (fabsf(diffX) < 10.f && fabsf(diffY) < 10.f)
        {
          e.targetX = (rand() % 40 - 20) * 15.f;
          e.targetY = (rand() % 40 - 20) * 15.f;
        }
      }
    }
    for (const Entity &e : entities)
    {
      for (size_t i = 0; i < server->connectedPeers; ++i)
      {
        ENetPeer *peer = &server->peers[i];
        if (controlledMap[e.eid] != peer)
          send_snapshot(peer, e.eid, e.x, e.y, e.size);
      }
    }
    for (Entity &e1 : entities)
    {
      for (Entity &e2 : entities)
      {
        if (e1.eid != e2.eid && ((e1.x - e2.x) * (e1.x - e2.x) + (e1.y - e2.y) * (e1.y - e2.y)) < ((e1.size - e2.size) * (e1.size - e2.size)) + 2.f) {
          on_collision(e1, e2);
        }
      }
    }
    //usleep(400000);
  }
  printf("Done");

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}


