#pragma once
#include "Player.h"

#include <vector>
#include <json.hpp>

using nlohmann::json;

#ifdef EMSCRIPTEN


#else

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_

#pragma warning(push, 0)
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#pragma warning(pop)

typedef websocketpp::client<websocketpp::config::asio_client> websocketpp_client;
typedef websocketpp::config::asio_client::message_type::ptr websocketpp_message_ptr;
typedef websocketpp::connection_hdl websocketpp_connection_handle;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

#endif

class Game;

class Network
{
public:
	void init(Game* game);
	void tick();
	void render();

    void send(std::string& text);
    void sendBinary(unsigned char* data, size_t size);

    bool isConnected();
    bool isHost();

    void onOpen();
    void onClose();
    void onMessage(const std::string& text);
    void onBinaryMessage(const unsigned char* data);

#ifdef EMSCRIPTEN

#else
    websocketpp_connection_handle connection_handle;
    websocketpp_client socket;
#endif

private:
    const char* URI = "ws://127.0.0.1:1234";

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
    bool connected;

	Game* game;
};
