#pragma once
#include "Player.h"

#include <vector>
#include <string>

class Game;

class Network
{
public:
	void init(Game* game);
    void connect();
	void tick();
	void render();

    void join(const std::string& id);
    void create();
    void setBlock(int x, int y, int z, unsigned char blockType);

    void send(const std::string& text);
    void sendBinary(unsigned char* data, size_t size);

    bool isConnected();
    bool isHost();

    void onOpen();
    void onClose();
    void onMessage(const std::string& text);
    void onBinaryMessage(const unsigned char* data);

private:
#ifdef EMSCRIPTEN
    const char* URI = "wss://vldr.org:1235";
#else
    const char* URI = "ws://vldr.org:1234";
#endif

    enum class PacketType : unsigned char
    {
        Level,
        Position,
        SetBlock,
    };

#pragma pack(push, 1)
    struct Packet
    {
        unsigned char index;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct LevelPacket : Packet
    {
        const PacketType type = PacketType::Level;

        unsigned char level[128 * 64 * 128];
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct PositionPacket : Packet
    {
        const PacketType type = PacketType::Position;

        glm::vec3 position;
        glm::vec2 rotation;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct SetBlockPacket : Packet
    {
        const PacketType type = PacketType::SetBlock;

        unsigned char blockType;
        glm::ivec3 position;
    };
#pragma pack(pop)

	std::vector<Player*> players;
    std::vector<PositionPacket> positionPackets;

    bool connected;

	Game* game;
};
