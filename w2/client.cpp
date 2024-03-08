#include "raylib.h"
#include <enet/enet.h>
#include <iostream>
#include <sstream>
#include <map>

bool debug_string = false;
bool info_string = true;
uint32_t ping_periud = 1000;

enum class MsgHeader {RequestGame, GotRedirectToGame, PlayerJoinedGame, AllPlayers, PlayersPings, SendPositionUpdate, Ping, Pong, ReceveName, UpdatePositions};

void SendStringReliable(ENetPeer* peer, std::string &str)
{
  if (debug_string)
    std::cout << "sending string " << str << " with length " << strlen(str.c_str()) + 1 << std::endl;
	ENetPacket* packet = enet_packet_create(str.c_str(), strlen(str.c_str()) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void SendStringFragment(ENetPeer* peer, std::string &str)
{
  if (debug_string)
    std::cout << "sending fragment " << str << " with length " << strlen(str.c_str()) + 1 << std::endl;
	ENetPacket* packet = enet_packet_create(str.c_str(), strlen(str.c_str()) + 1, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
	enet_peer_send(peer, 1, packet);
}

int main(int argc, const char **argv)
{
  // ================================================ Window init ================================================ //

  int width = 800;
  int height = 600;
  InitWindow(width, height, "MyClient");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < width || scrHeight < height)
  {
    width = std::min(scrWidth, width);
    height = std::min(scrHeight - 150, height);
    SetWindowSize(width, height);
  }
  SetTargetFPS(60);

  // ================================================= ENet init ================================================= //

  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  atexit(enet_deinitialize);


  // ============================================= Creating client =============================================== //

  ENetHost *client = enet_host_create(nullptr, 2, 2, 0, 0); // client/host, connections, channels, in and out bandwidth
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }
  atexit(enet_deinitialize);

  // =========================================== Connecting to lobby ============================================= //

  ENetAddress address;
  
  enet_address_set_host(&address, "localhost");
  address.port = 10887;

  ENetPeer *gamePeer;
  ENetPeer *lobbyPeer = enet_host_connect(client, &address, 2, 0);
  if (!lobbyPeer)
  {
    printf("Cannot connect to lobby");
    return 1;
  }

  // ======================================= Lobby Connection handling =========================================== //
  uint32_t timeStart = enet_time_get();
  uint32_t lastStartGameSended = timeStart;
  bool connected = false;
  bool connected_to_game = false;
  float posx = 0.f;
  float posy = 0.f;


  std::map<std::string, int> m{};
  // =============================================== Lobby loop ================================================== //
  while (!WindowShouldClose() && !connected_to_game)
  {
    const float dt = GetFrameTime();
    ENetEvent event;
    while (enet_host_service(client, &event, 10) > 0 && !connected_to_game)
    {
      std::string data;
      std::string delimiter = "||";
      std::string token;
      int int_data;
      MsgHeader header;
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        if (debug_string)
          printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        data = (char*)event.packet->data;
        if (debug_string)
          printf("Packet received '%s'\n", event.packet->data);

        token = data.substr(0, data.find(delimiter));
        header = (MsgHeader)std::stoi(token);

        switch (header)
        {
        case MsgHeader::GotRedirectToGame:
          data.erase(0, data.find(delimiter) + delimiter.length());
          if (info_string)
            std::cout << "Redirected to game at " << data << std::endl;

          delimiter = ":";
          token = data.substr(0, data.find(delimiter));
          data.erase(0, data.find(delimiter) + delimiter.length());
          int_data = std::stoi(data);

          enet_address_set_host(&address, token.c_str());
          address.port = int_data;

          gamePeer = enet_host_connect(client, &address, 2, 0);
          if (!gamePeer)
          {
            printf("Cannot connect to game");
            return 1;
          }
          connected_to_game = true;
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

    // Sending data

    uint32_t curTime = 0;
    if (connected)
    {
      // timestamp in ms
      uint32_t curTime = enet_time_get();
    
      // sending 
      bool enter = IsKeyDown(KEY_ENTER);
      if (enter && curTime - lastStartGameSended > 1000 && !connected_to_game) {
          lastStartGameSended = curTime;
          std::string request = std::to_string((int)MsgHeader::RequestGame) + "||";
          
          SendStringReliable(lobbyPeer, request);
      }
    }



    if (!connected_to_game) {
      BeginDrawing();
        ClearBackground(BLACK);
        DrawText(TextFormat("Press enter to request a game start"), 20, 20, 20, WHITE);
      EndDrawing();
    }
  }

  std::string name = "";
  std::map<std::string, int> pings{};
  std::map<std::string, std::pair<int, int>> positions{};

  // =============================================== Game loop ================================================== //
    uint32_t lastPositionSended = enet_time_get();
    while (!WindowShouldClose())
    {
      const float dt = GetFrameTime();
      ENetEvent event;
      while (enet_host_service(client, &event, 10) > 0)
      {
        std::string data;
        std::string delimiter = "||";
        std::string token;
        int position;
        MsgHeader header;
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
          if (debug_string)
            printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
          connected = true;
          break;
        case ENET_EVENT_TYPE_RECEIVE:
          data = (char*)event.packet->data;
          if (debug_string)
            printf("Packet received '%s'\n", event.packet->data);

          token = data.substr(0, data.find(delimiter));
          header = (MsgHeader)std::stoi(token);

          switch (header)
          {
          case MsgHeader::Ping:
            data.erase(0, data.find(delimiter) + delimiter.length());
            token = std::to_string((int)MsgHeader::Pong) + "||" + std::to_string(std::stoi(data));
            SendStringFragment(event.peer, token);
            break;
          case MsgHeader::PlayersPings:
            data.erase(0, data.find(delimiter) + delimiter.length());
            while (data.length() > 0) {
              token = data.substr(0, data.find(":"));
              data.erase(0, data.find(":") + 1);
              pings[token] = std::stoi(data.substr(0, data.find(",")));
              data.erase(0, data.find(",") + 1);
            }
            break;
          case MsgHeader::UpdatePositions:
            if (info_string)
              std::cout << "Recived positions " << std::endl;
            data.erase(0, data.find(delimiter) + delimiter.length());
            if (info_string)
              std::cout << data << std::endl;
            while (data.length() > 0) {
              token = data.substr(0, data.find(":"));
              data.erase(0, data.find(":") + 1);
              position = std::stoi(data.substr(0, data.find(":")));
              data.erase(0, data.find(":") + 1);
              positions[token] = std::pair{position, std::stoi(data.substr(0, data.find(",")))};         
              data.erase(0, data.find(",") + 1);    
            }
            break;
          case MsgHeader::ReceveName:
            if (info_string)
              std::cout << "Recived name " << std::endl;
            data.erase(0, data.find(delimiter) + delimiter.length());
            name = data.substr(0, data.find(":"));
            break;
          case MsgHeader::AllPlayers:
            if (info_string)
              std::cout << "Recived all players: " << std::endl;
            data.erase(0, data.find(delimiter) + delimiter.length());
            if (info_string)
              std::cout << data << std::endl;
            break;
          case MsgHeader::PlayerJoinedGame:
            if (info_string)
              std::cout << "New player joined: " << std::endl;
            data.erase(0, data.find(delimiter) + delimiter.length());
            if (info_string)
              std::cout << data << std::endl;
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

      uint32_t curTime = 0;
      bool left = IsKeyDown(KEY_LEFT);
      bool right = IsKeyDown(KEY_RIGHT);
      bool up = IsKeyDown(KEY_UP);
      bool down = IsKeyDown(KEY_DOWN);
      constexpr float spd = 10.f;
      posx += ((left ? -1.f : 0.f) + (right ? 1.f : 0.f)) * dt * spd;
      posy += ((up ? -1.f : 0.f) + (down ? 1.f : 0.f)) * dt * spd;

      if (connected)
      {
        // timestamp in ms
        uint32_t curTime = enet_time_get();
      
        // Slowed down to make logs readable
        if (curTime - lastPositionSended > ping_periud) {
            lastPositionSended = curTime;
            std::string request = std::to_string((int)MsgHeader::SendPositionUpdate) + "||" + std::to_string(posx) + ":" + std::to_string(posy);
            SendStringReliable(gamePeer, request);
        }
      }
        
      BeginDrawing();
        ClearBackground(BLACK);
        DrawText(TextFormat("Current status: %s", "In Game"), 20, 20, 20, WHITE);
        DrawText(TextFormat("My position: (%d, %d)", (int)posx, (int)posy), 20, 40, 20, WHITE);
        DrawText(("My name: " + name).c_str(), 20, 60, 20, WHITE);
        int p = 80;
        for (auto i: pings) {
          if (name != i.first) {
            DrawText(TextFormat((i.first + ": ping %dms").c_str(), i.second), 20, p, 20, WHITE);
            p += 20;
          }
        }

        for (auto i: positions) {
          if (name != i.first) {
            if (pings[i.first] < 1000) {
              DrawCircle(width/2 + i.second.first, height/2 + i.second.second, 5, GREEN);
            }
            else {
              DrawCircle(width/2 + i.second.first, height/2 + i.second.second, 5, DARKGREEN);
            }
              
            p += 20;
          }
        }

        DrawCircle(width/2 + (int)(posx), height/2 + (int)(posy), 5, BLUE);
      EndDrawing();
    }

  atexit(enet_deinitialize);
  return 0;
}
