#include "protocol.h"
#include "bitstream.h"
#include <cstring> // memcpy

void send_join(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);
  Bitstream bs(packet->data, sizeof(uint8_t));
  bs.write(E_CLIENT_TO_SERVER_JOIN);

  enet_peer_send(peer, 0, packet);
  if (packet->referenceCount == 0) {
      enet_packet_destroy(packet);
  }
}

void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  uint8_t size = sizeof(uint8_t) + sizeof(Entity);
  ENetPacket *packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_RELIABLE);

  Bitstream bs(packet->data, size);
  bs.write(E_SERVER_TO_CLIENT_NEW_ENTITY);
  bs.write(ent);

  enet_peer_send(peer, 0, packet);
  if (packet->referenceCount == 0) {
      enet_packet_destroy(packet);
  }
}

void send_set_controlled_entity(ENetPeer *peer, uint16_t eid)
{
  uint8_t size = sizeof(uint8_t) + sizeof(uint16_t);
  ENetPacket *packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_RELIABLE);

  Bitstream bs(packet->data, size);
  bs.write(E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY);
  bs.write(eid);

  enet_peer_send(peer, 0, packet);
  if (packet->referenceCount == 0) {
      enet_packet_destroy(packet);
  }
}

void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y)
{
  uint8_t size = sizeof(uint8_t) + sizeof(uint16_t) + 2 * sizeof(float);
  ENetPacket *packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_UNSEQUENCED);
  
  Bitstream bs(packet->data, size);
  bs.write(E_CLIENT_TO_SERVER_STATE);
  bs.write(eid);
  bs.write(x);
  bs.write(y);

  enet_peer_send(peer, 1, packet);
  if (packet->referenceCount == 0) {
      enet_packet_destroy(packet);
  }
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y)
{
  uint8_t size = sizeof(uint8_t) + sizeof(uint16_t) + 2 * sizeof(float);
  ENetPacket *packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_UNSEQUENCED);

  Bitstream bs(packet->data, size);
  bs.write(E_SERVER_TO_CLIENT_SNAPSHOT);
  bs.write(eid);
  bs.write(x);
  bs.write(y);

  enet_peer_send(peer, 1, packet);
  if (packet->referenceCount == 0) {
      enet_packet_destroy(packet);
  }
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  uint8_t *ptr = packet->data; ptr += sizeof(uint8_t);
  ent = *(Entity*)(ptr); ptr += sizeof(Entity);
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  uint8_t *ptr = packet->data; ptr += sizeof(uint8_t);
  eid = *(uint16_t*)(ptr); ptr += sizeof(uint16_t);
}

void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y)
{
  uint8_t *ptr = packet->data; ptr += sizeof(uint8_t);
  eid = *(uint16_t*)(ptr); ptr += sizeof(uint16_t);
  x = *(float*)(ptr); ptr += sizeof(float);
  y = *(float*)(ptr); ptr += sizeof(float);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y)
{
  uint8_t *ptr = packet->data; ptr += sizeof(uint8_t);
  eid = *(uint16_t*)(ptr); ptr += sizeof(uint16_t);
  x = *(float*)(ptr); ptr += sizeof(float);
  y = *(float*)(ptr); ptr += sizeof(float);
}

