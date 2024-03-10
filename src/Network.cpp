#include "Network.h"
#include "Game.h"
#include "LZ.h"
#include "JSON.h"

#include <cstdlib>

static Network* network;

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

static EMSCRIPTEN_WEBSOCKET_T socket;

EM_JS(void, setHash, (const char* hash), { location.hash = UTF8ToString(hash); });
EM_JS(bool, hasHash, (), { return !!location.hash; });
EM_JS(char*, getHash, (), { return stringToNewUTF8( location.hash.replace("#","") ); });

EM_BOOL emscripten_on_message(int event_type, const EmscriptenWebSocketMessageEvent* websocket_event, void* user_data)
{
    if (websocket_event->isText)
    {
        std::string text(websocket_event->data, websocket_event->data + websocket_event->numBytes);

        network->onMessage(text);
    }
    else
    {
        network->onBinaryMessage(websocket_event->data, websocket_event->numBytes);
    }

    return EM_TRUE;
}

EM_BOOL emscripten_on_error(int event_type, const EmscriptenWebSocketErrorEvent* websocket_event, void* user_data)
{
    network->onClose();

    return EM_TRUE;
}

EM_BOOL emscripten_on_close(int event_type, const EmscriptenWebSocketCloseEvent* websocket_event, void* user_data)
{
    network->onClose();

    return EM_TRUE;
}

EM_BOOL emscripten_on_open(int event_type, const EmscriptenWebSocketOpenEvent* websocket_event, void* user_data)
{
    network->onOpen();

    return EM_TRUE;
}
#else
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_

#if defined(_WIN32)
#pragma warning(push, 0)
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#if defined(_WIN32)
#pragma warning(pop)
#else
#pragma clang diagnostic pop
#endif

typedef websocketpp::client<websocketpp::config::asio_client> websocketpp_client;
typedef websocketpp::config::asio_client::message_type::ptr websocketpp_message_ptr;
typedef websocketpp::connection_hdl websocketpp_connection_handle;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

static websocketpp_connection_handle socket_connection_handle;
static websocketpp_client* socket_client;

void websocketpp_on_message(websocketpp_client* socket, websocketpp_connection_handle connection_handle, websocketpp_message_ptr message)
{
    if (message->get_opcode() == websocketpp::frame::opcode::text)
    {
        network->onMessage(message->get_payload());
    }
    else if (message->get_opcode() == websocketpp::frame::opcode::binary)
    {
        network->onBinaryMessage(
            reinterpret_cast<const unsigned char*>(message->get_payload().data()),
            message->get_payload().size()
        );
    }
}

void websocketpp_on_close(websocketpp_client* socket, websocketpp_connection_handle connection_handle)
{
    network->onClose();
}

void websocketpp_on_open(websocketpp_client* socket, websocketpp_connection_handle connection_handle)
{
    socket_connection_handle = connection_handle;

    network->onOpen();
}
#endif

void Network::init()
{
    url = "...";
    connected = false;
    network = this;
}

void Network::connect()
{
    const char* title = "Connecting";
    const char* description = "Attempting to connect...";

#if defined(EMSCRIPTEN)
    if (hasHash())
    {
        game.ui.openStatusMenu(title, description);
    }
    else
    {
        game.ui.closeMenu();
    }

    EmscriptenWebSocketCreateAttributes ws_attrs = {
        URI,
        NULL,
        EM_TRUE
    };

    socket = emscripten_websocket_new(&ws_attrs);
    emscripten_websocket_set_onmessage_callback(socket, NULL, emscripten_on_message);
    emscripten_websocket_set_onerror_callback(socket, NULL, emscripten_on_error);
    emscripten_websocket_set_onclose_callback(socket, NULL, emscripten_on_close);
    emscripten_websocket_set_onopen_callback(socket, NULL, emscripten_on_open);
#else
    game.ui.openStatusMenu(title, description);

    try
    {
        socket_client = new websocketpp_client;
        socket_client->set_access_channels(websocketpp::log::alevel::none);
        socket_client->clear_access_channels(websocketpp::log::alevel::none);

        socket_client->init_asio();
        socket_client->set_message_handler(bind(&websocketpp_on_message, socket_client, ::_1, ::_2));
        socket_client->set_fail_handler(bind(&websocketpp_on_close, socket_client, ::_1));
        socket_client->set_close_handler(bind(&websocketpp_on_close, socket_client, ::_1));
        socket_client->set_open_handler(bind(&websocketpp_on_open, socket_client, ::_1));

        websocketpp::lib::error_code error_code;

        auto connection = socket_client->get_connection(URI, error_code);
        connection->append_header("Origin", BASE_URL);

        if (error_code)
        {
            printf("Failed to connect: %s\n", error_code.message().c_str());
            return;
        }

        socket_client->connect(connection);
    }
    catch (websocketpp::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
#endif
}

void Network::tick()
{
#if !defined(EMSCRIPTEN)
    if (socket_client)
    {
        socket_client->poll();
    }
#endif

    sendPosition(game.localPlayer.position, game.localPlayer.rotation);

    for (auto positionPacket = positionPackets.begin(); positionPacket != positionPackets.end();)
    {
        const auto index = positionPacket->index;

        if (index < players.size())
        {
            auto& player = players[index];

            if (player)
            {
                player->updates++;

                if (player->updates == 1 || player->flushUpdates)
                {
                    player->tick();
                    player->rotate(positionPacket->rotation.x, positionPacket->rotation.y);
                    player->move(positionPacket->position.x, positionPacket->position.y, positionPacket->position.z);

                    positionPacket = positionPackets.erase(positionPacket);
                    continue;
                }
            }
        }
        else
        {
            printf("network error: index out of bounds for position packet.\n");

            positionPacket = positionPackets.erase(positionPacket);
            continue;
        }

        positionPacket++;
    }

    for (size_t index = 0; index < players.size(); index++)
    {
        auto& player = players[index];

        if (player)
        {
            if (player->updates)
            {
                player->flushUpdates = player->updates > 3;
                player->updates = 0;
            }
            else
            {
                player->tick();
            }
        }
    }
}

void Network::render()
{
    if (players.empty())
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, Player::playerTexture);

    for (const auto& player : players)
    {
        if (player)
        {
            player->render();
        }
    }
}

void Network::send(const std::string& text)
{
#if defined(EMSCRIPTEN)
    EMSCRIPTEN_RESULT result = emscripten_websocket_send_utf8_text(socket, text.c_str());
    if (result)
    {
        printf("Failed to send: %d\n", result);
    }
#else
    if (!socket_client)
    {
        printf("Failed to send, socket_client is nullptr\n");
        return;
    }

    websocketpp::lib::error_code error_code;
    socket_client->send(socket_connection_handle, text, websocketpp::frame::opcode::text, error_code);

    if (error_code)
    {
        printf("Failed to send: %s\n", error_code.message().c_str());
        return;
    }
#endif
}

void Network::sendBinary(unsigned char* data, size_t size)
{
#if defined(EMSCRIPTEN)
    EMSCRIPTEN_RESULT result = emscripten_websocket_send_binary(socket, (void*)data, size);
    if (result)
    {
        printf("Failed to send binary: %d\n", result);
    }
#else
    if (!socket_client)
    {
        printf("Failed to send, socket_client is nullptr\n");
        return;
    }

    websocketpp::lib::error_code error_code;
    socket_client->send(socket_connection_handle, (void*)data, size, websocketpp::frame::opcode::binary, error_code);

    if (error_code)
    {
        printf("Failed to send binary: %s\n", error_code.message().c_str());
        return;
    }
#endif
}

void Network::sendPosition(const glm::vec3& position, const glm::vec2& rotation)
{
    if (isConnected() && players.size() > 1)
    {
        PositionPacket packet = PositionPacket();
        packet.index = UCHAR_MAX;
        packet.position = position;
        packet.rotation = rotation;

        sendBinary((unsigned char*)&packet, sizeof(packet));
    }
}

void Network::sendLevel(unsigned char index, bool respawn)
{
    if (isConnected() && players.size() > 1)
    {
        auto packet = std::make_unique<LevelPacket>();
        packet->index = index;
        packet->respawn = respawn;
        packet->length = fastlz_compress(game.level.blocks.get(), sizeof(packet->data) / 2, packet->data);

        sendBinary(
            (unsigned char*)packet.get(),
            sizeof(*packet) - sizeof(packet->data) + packet->length
        );
    }
}

void Network::sendSetBlock(int x, int y, int z, unsigned char blockType, bool mode)
{
    if (isConnected() && players.size() > 1)
    {
        SetBlockPacket packet = SetBlockPacket();

        if (isHost())
        {
            packet.index = UCHAR_MAX;
        }
        else
        {
            packet.index = 0;
        }

        packet.position = glm::ivec3(x, y, z);
        packet.blockType = blockType;
        packet.mode = mode;

        sendBinary(
            (unsigned char*)&packet,
            sizeof(packet)
        );
    }
}

bool Network::isConnected()
{
    return connected;
}

bool Network::isHost()
{
    if (!players.empty())
    {
        return players[0] == nullptr;
    }

    return false;
}

size_t Network::count()
{
    return players.size();
}

void Network::join(const std::string& id)
{
    if (isConnected())
    {
        game.ui.openStatusMenu("Joining Room", "Attempting to join room...");

        char message[512];
        size_t length = sizeof(message) / sizeof(*message);

        auto it = json_objOpen(message, nullptr, &length);
        it = json_str(it, "type", "join", &length);
        it = json_str(it, "id", id.c_str(), &length);
        it = json_objClose(it, &length);
        it = json_end(it, &length);

        send(message);

        url = BASE_URL + id;
    }
}

void Network::create()
{
    if (isConnected())
    {
        const char* title = "Creating Room";
        const char* description = "Attempting to create room...";

#if defined(EMSCRIPTEN)
        if (hasHash())
        {
            game.ui.openStatusMenu(title, description);
        }
#else
        game.ui.openStatusMenu(title, description);
#endif

        char message[512];
        size_t length = sizeof(message) / sizeof(*message);

        auto it = json_objOpen(message, nullptr, &length);
        it = json_str(it, "type", "create", &length);
        it = json_int(it, "size", UCHAR_MAX - 1, &length);
        it = json_objClose(it, &length);
        it = json_end(it, &length);

        send(message);
    }
}

void Network::onOpen()
{
    connected = true;

#if defined(EMSCRIPTEN)
    const char* hash = getHash();

    if (*hash)
    {
        join(hash);
    }
    else
    {
        create();
    }

    std::free((void*)hash);
#else
    game.ui.openStatusMenu("Connected", "Select an option to continue.", true);
#endif
}

void Network::onClose()
{
#if defined(EMSCRIPTEN)
    setHash("");
#endif

    url = "...";
    connected = false;

    players.clear();
    game.ui.openStatusMenu("Disconnected", "The connection was closed.", true);
}

void Network::onMessage(const std::string& text)
{
    const auto MAX_FIELDS = 4;
    json_t pool[MAX_FIELDS];

    auto json = text;
    auto message = json_create(json.data(), pool, MAX_FIELDS);
    
    std::string type = json_getValue(json_getProperty(message, "type"));
    if (type == "error")
    {
        std::string reason = json_getValue(json_getProperty(message, "message"));
        game.ui.openStatusMenu("Error", reason.c_str(), true);
    }
    else if (type == "create")
    {
        std::string id = json_getValue(json_getProperty(message, "id"));

        players.push_back(nullptr);

#if defined(EMSCRIPTEN)
        setHash(id.c_str());
#endif

        url = BASE_URL + id;

        game.ui.closeMenu();
    }
    else if (type == "join")
    {
        if (!json_getProperty(message, "size"))
        {
            auto player = std::make_unique<Player>();
            player->init();

            players.push_back(std::move(player));

            if (isHost())
            {
                sendLevel(
                    (unsigned char)players.size() - 1,
                    true
                );
            }

            game.ui.log("A player has connected to the room.");
        }
        else
        {
            unsigned int size = (unsigned int)json_getInteger(json_getProperty(message, "size"));

            for (unsigned int i = 0; i < size; i++)
            {
                auto player = std::make_unique<Player>();
                player->init();

                players.push_back(std::move(player));
            }

            players.push_back(nullptr);
        }
    }
    else if (type == "leave")
    {
        unsigned int index = (unsigned int)json_getInteger(json_getProperty(message, "index"));

        for (auto positionPacket = positionPackets.begin(); positionPacket != positionPackets.end();)
        {
            if (index == positionPacket->index)
            {
                positionPacket = positionPackets.erase(positionPacket);
            }
            else
            {
                positionPacket++;
            }
        }

        players.erase(players.begin() + index);

        if (isHost())
        {
            sendLevel(
                UCHAR_MAX,
                false
            );

            if (game.ui.state == UI::State::StatusMenu)
            {
                game.ui.closeMenu();
            }
        }
        else if (!index)
        {
            game.ui.openStatusMenu("Migrating Host", "Attempting to migrate the host...");
        }

        game.ui.log("A player has left the room.");
    }
}

void Network::onBinaryMessage(const unsigned char* data, size_t size)
{
    if (size < 2)
    {
        printf("network error: packet is too small.\n");
        return;
    }

    unsigned char index = data[0];
    unsigned char type = data[1];

    if (type == (unsigned char)PacketType::Level)
    {
        if (size > sizeof(LevelPacket))
        {
            printf("network error: invalid level packet size.\n");
            return;
        }

        if (index)
        {
            printf("network error: cannot process level packet from a non-host.\n");
            return;
        }

        LevelPacket* packet = (LevelPacket*)data;

        if (!fastlz_decompress(packet->data, packet->length, game.level.blocks.get(), sizeof(packet->data) / 2))
        {
            printf("network error: failed to decompress level data.\n");
            return;
        }

        game.level.calculateLightDepths(0, 0, Level::WIDTH, Level::DEPTH);
        game.levelRenderer.loadChunks(0, 0, 0, Level::WIDTH, Level::HEIGHT, Level::DEPTH);

        if (packet->respawn)
        {
            game.level.calculateSpawnPosition();
        }

        game.level.reset();
        game.ui.closeMenu();
    }
    else if (type == (unsigned char)PacketType::Position)
    {
        if (size != sizeof(PositionPacket))
        {
            printf("network error: invalid position packet size.\n");
            return;
        }

        positionPackets.push_back(*(PositionPacket*)data);
    }
    else if (type == (unsigned char)PacketType::SetBlock)
    {
        if (size != sizeof(SetBlockPacket))
        {
            printf("network error: invalid set block packet size.\n");
            return;
        }

        SetBlockPacket* packet = (SetBlockPacket*)data;

        auto previousBlockType = game.level.getTile(packet->position.x, packet->position.y, packet->position.z);

        if (isHost())
        {
            if (game.level.isWaterTile(packet->blockType) || game.level.isLavaTile(packet->blockType))
            {
                sendSetBlock(
                    packet->position.x,
                    packet->position.y,
                    packet->position.z,
                    previousBlockType
                );

                printf("network error: attempted to place a forbidden block.\n");
                return;
            }

            bool mode = false;

            if (
                !game.level.isAirTile(previousBlockType) &&
                !game.level.isWaterTile(previousBlockType) &&
                !game.level.isLavaTile(previousBlockType) &&
                game.level.isAirTile(packet->blockType)
            )
            {
                game.particleManager.spawn(
                    (float)packet->position.x,
                    (float)packet->position.y,
                    (float)packet->position.z,
                    previousBlockType
                );

                mode = true;
            }

            game.level.setTileWithNeighborChange(
                packet->position.x,
                packet->position.y,
                packet->position.z,
                packet->blockType,
                mode
            );
        }
        else
        {
            if (index)
            {
                printf("network error: cannot process set block packet from a non-host.\n");
                return;
            }

            if (
                game.level.setTileWithNoNeighborChange(
                    packet->position.x,
                    packet->position.y,
                    packet->position.z,
                    packet->blockType
                ) &&
                packet->mode
            )
            {
                game.particleManager.spawn(
                    (float)packet->position.x,
                    (float)packet->position.y,
                    (float)packet->position.z,
                    previousBlockType
                );
            }
        }
    }
}