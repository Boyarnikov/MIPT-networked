#include <enet/enet.h>
#include <iostream>
#include <string>

bool debug_string = false;
bool info_string = true;
uint32_t ping_periud = 1000;

enum class MsgHeader {RequestGame, GotRedirectToGame, PlayerJoinedGame, AllPlayers, PlayersPings, SendPositionUpdate, Ping, Pong, ReceveName, UpdatePositions};

void BroadcastString(ENetHost* server, std::string &str)
{
  if (debug_string)
    std::cout << "broadcasting string " << str << " with length " << strlen(str.c_str()) + 1 << std::endl;
  char *data = str.data();
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, 0, packet);
}

void SendString(ENetPeer* peer, std::string &str)
{
  if (debug_string)
    std::cout << "sending string " << str << " with length " << strlen(str.c_str()) + 1 << std::endl;
	char *data = str.data();
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

// Waiting for START packege, then redirect to game_lobby
int main(int argc, const char **argv)
{
  // ================================================= ENet init ================================================= //

  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  atexit(enet_deinitialize);

  // ============================================= Creating client =============================================== //

  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10887;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  // ================================================ Game Loop ================================================= //

  bool game_started = false;

  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0)
    {
      std::string data;
      std::string delimiter = "||";
      std::string token;
      MsgHeader header;
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        if (debug_string)
          printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);

        if (game_started) {
          if (info_string)
            printf("Sending game server data to new client\n");
          data = std::to_string((int)MsgHeader::GotRedirectToGame) + "||localhost:10888";
          SendString(event.peer, data);
        }
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        data = (char*)event.packet->data;
        if (debug_string)
          printf("Packet received '%s'\n", event.packet->data);

        token = data.substr(0, data.find(delimiter));
        header = (MsgHeader)std::stoi(token);

        switch (header)
        {
        case MsgHeader::RequestGame:
          if (!game_started) {
            game_started = true;
            if (info_string) {
              printf("Game start request: starting the game");
              printf("Sending game server data to all current piers");
            }
            

            data = std::to_string((int)MsgHeader::GotRedirectToGame) + "||localhost:10888";
            BroadcastString(server, data);
          }
          break;
        default:
          if (debug_string)
            printf("Command is not processed due to not having it in MsgHeader\n");
          break;
        }
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
  }

  enet_host_destroy(server);
  atexit(enet_deinitialize);

  return 0;
}

