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

void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y, float e_size)
{
  uint8_t size = sizeof(uint8_t) + sizeof(uint16_t) + 3 * sizeof(float);
  ENetPacket *packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_UNSEQUENCED);
  
  Bitstream bs(packet->data, size);
  bs.write(E_CLIENT_TO_SERVER_STATE);
  bs.write(eid);
  bs.write(x);
  bs.write(y);
  bs.write(e_size);

  enet_peer_send(peer, 1, packet);
  if (packet->referenceCount == 0) {
      enet_packet_destroy(packet);
  }
}

void send_entity_update(ENetPeer *peer, uint16_t eid, float x, float y, float e_size)
{
  uint8_t size = sizeof(uint8_t) + sizeof(uint16_t) + 3 * sizeof(float);
  ENetPacket *packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_RELIABLE);
  
  Bitstream bs(packet->data, size);
  bs.write(E_SERVER_TO_CLIENT_STATE);
  bs.write(eid);
  bs.write(x);
  bs.write(y);
  bs.write(e_size);

  enet_peer_send(peer, 0, packet);
  if (packet->referenceCount == 0) {
      enet_packet_destroy(packet);
  }
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float e_size)
{
  uint8_t size = sizeof(uint8_t) + sizeof(uint16_t) + 3 * sizeof(float);
  ENetPacket *packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_UNSEQUENCED);

  Bitstream bs(packet->data, size);
  bs.write(E_SERVER_TO_CLIENT_SNAPSHOT);
  bs.write(eid);
  bs.write(x);
  bs.write(y);
  bs.write(e_size);

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
  Bitstream bs(packet->data + sizeof(uint8_t), 0);
  bs.read(ent);
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  Bitstream bs(packet->data + sizeof(uint8_t), 0);
  bs.read(eid);
}

void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &size)
{
  Bitstream bs(packet->data + sizeof(uint8_t), 0);
  bs.read(eid);
  bs.read(x);
  bs.read(y);
  bs.read(size);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &size)
{
  Bitstream bs(packet->data + sizeof(uint8_t), 0);
  bs.read(eid);
  bs.read(x);
  bs.read(y);
  bs.read(size);
}

