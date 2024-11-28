#pragma once
#include "Player.h"
#include "Level.h"

#include <vector>
#include <string>
#include <memory>

class Network
{
public:
  void init();
  void connect();
  void tick();
  void render();

  void sendPosition(const glm::vec3& position, const glm::vec2& rotation);
  void sendLevel(unsigned char index, bool respawn);
  void sendSetBlock(int x, int y, int z, unsigned char blockType, bool mode = false);

  void join(const std::string& id);
  void create();

  bool isConnected();
  bool isHost();
  size_t count();

  void onOpen();
  void onClose();
  void onMessage(const std::string& text);
  void onBinaryMessage(const unsigned char* data, size_t size);

  std::string url;
private:
  void send(const std::string& text);
  void sendBinary(unsigned char* data, size_t size);

  const char* BASE_URL = "https://cubic.vldr.org/#";

#if defined(EMSCRIPTEN)
  const char* URI = "wss://relay.vldr.org/";
#else
  const char* URI = "ws://relay.vldr.org/";
#endif

#pragma pack(push, 1)
  enum class PacketType : uint8_t
  {
    Level,
    Position,
    SetBlock,
  };

  struct Packet
  {
    uint8_t index;
  };

  struct LevelPacket : Packet
  {
    PacketType type = PacketType::Level;

    uint8_t respawn;

    uint32_t length;
    uint8_t data[2 * Level::WIDTH * Level::HEIGHT * Level::DEPTH];
  };

  struct PositionPacket : Packet
  {
    PacketType type = PacketType::Position;

    glm::f32vec3 position;
    glm::f32vec2 rotation;
  };

  struct SetBlockPacket : Packet
  {
    PacketType type = PacketType::SetBlock;

    glm::i32vec3 position;

    uint8_t blockType;
    uint8_t mode;
  };
#pragma pack(pop)

  bool connected;

  std::vector<std::unique_ptr<Player>> players;
  std::vector<PositionPacket> positionPackets;
};
