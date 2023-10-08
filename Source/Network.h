#pragma once
#include "Player.h"

#include <vector>
#include <string>
#include <memory>

class Game;

class Network
{
public:
	void init(Game* game);
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

#ifdef EMSCRIPTEN
	const char* URI = "wss://relay.vldr.org/";
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
        PacketType type = PacketType::Level;

        bool respawn;

        int length;
        unsigned char data[(128 * 64 * 128) * 2];
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct PositionPacket : Packet
    {
        PacketType type = PacketType::Position;

        glm::vec3 position;
        glm::vec2 rotation;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct SetBlockPacket : Packet
    {
        PacketType type = PacketType::SetBlock;

        glm::ivec3 position;
        unsigned char blockType;
        bool mode;
    };
#pragma pack(pop)

	std::vector<std::unique_ptr<Player>> players;
    std::vector<PositionPacket> positionPackets;

    bool connected;

	Game* game;
};
