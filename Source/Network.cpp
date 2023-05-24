#include "Network.h"
#include "Game.h"

#include <cstdlib>
#include <json.hpp>

static Network* network;

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

static EMSCRIPTEN_WEBSOCKET_T socket;

EM_JS(void, set_hash, (const char* hash), { location.hash = UTF8ToString(hash); });
EM_JS(char*, get_hash, (), { 
    return stringToNewUTF8(
        location.hash.replace("#","")
    );
});

EM_BOOL emscripten_on_message(int event_type, const EmscriptenWebSocketMessageEvent* websocket_event, void* user_data)
{
    if (websocket_event->isText)
    {
        std::string text(websocket_event->data, websocket_event->data + websocket_event->numBytes);

        network->onMessage(text);
    }
    else
    {
        network->onBinaryMessage(websocket_event->data);
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

static websocketpp_connection_handle socket_connection_handle;
static websocketpp_client socket_client;

void websocketpp_on_message(websocketpp_client* socket, websocketpp_connection_handle connection_handle, websocketpp_message_ptr message)
{
    if (message->get_opcode() == websocketpp::frame::opcode::text)
    {
        network->onMessage(message->get_payload());
    }
    else if (message->get_opcode() == websocketpp::frame::opcode::binary)
    {
        network->onBinaryMessage(
            reinterpret_cast<const unsigned char*>(message->get_payload().data())
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

void Network::init(Game* game)
{
    this->game = game;
    this->connected = false;

    network = this;
}

void Network::connect()
{
    game->ui.openStatusMenu("Connecting", "Attempting to connect...");

#ifdef EMSCRIPTEN
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
    try
    {
        socket_client.set_access_channels(websocketpp::log::alevel::none);
        socket_client.clear_access_channels(websocketpp::log::alevel::none);

        socket_client.init_asio();
        socket_client.set_message_handler(bind(&websocketpp_on_message, &socket_client, ::_1, ::_2));
        socket_client.set_fail_handler(bind(&websocketpp_on_close, &socket_client, ::_1));
        socket_client.set_close_handler(bind(&websocketpp_on_close, &socket_client, ::_1));
        socket_client.set_open_handler(bind(&websocketpp_on_open, &socket_client, ::_1));

        websocketpp::lib::error_code error_code;
        websocketpp_client::connection_ptr con = socket_client.get_connection(URI, error_code);

        if (error_code)
        {
            printf("Failed to connect: %s\n", error_code.message().c_str());
            return;
        }

        socket_client.connect(con);
    }
    catch (websocketpp::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
#endif
}

void Network::tick()
{
#ifndef EMSCRIPTEN
    socket_client.poll();
#endif

    if (isConnected() && players.size() > 1)
    {
        PositionPacket positionPacket = PositionPacket();
        positionPacket.index = UCHAR_MAX;
        positionPacket.position = game->localPlayer.position;
        positionPacket.rotation = game->localPlayer.viewAngles;

        sendBinary((unsigned char*)&positionPacket, sizeof(positionPacket));
    }

    for (const auto& positionPacket : positionPackets)
    {
        const auto index = positionPacket.index;

        if (index < players.size())
        {
            Player* player = players[index];

            if (player)
            {
                player->rotate(positionPacket.rotation.x, positionPacket.rotation.y);
                player->move(positionPacket.position.x, positionPacket.position.y, positionPacket.position.z);
            }
        }
        else
        {
            printf("network error: index out of bounds for position packet.\n");
        }
    }

    positionPackets.clear();
}

void Network::render()
{
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
#ifdef EMSCRIPTEN
    EMSCRIPTEN_RESULT result = emscripten_websocket_send_utf8_text(socket, text.c_str());
    if (result)
    {
        printf("Failed to send: %d\n", result);
    }
#else
    websocketpp::lib::error_code error_code;
    socket_client.send(socket_connection_handle, text, websocketpp::frame::opcode::text, error_code);

    if (error_code)
    {
        printf("Failed to send: %s\n", error_code.message().c_str());
        return;
    }
#endif
}

void Network::sendBinary(unsigned char* data, size_t size)
{
#ifdef EMSCRIPTEN
    EMSCRIPTEN_RESULT result = emscripten_websocket_send_binary(socket, (void*)data, size);
    if (result)
    {
        printf("Failed to send binary: %d\n", result);
    }
#else
    websocketpp::lib::error_code error_code;
    socket_client.send(socket_connection_handle, (void*)data, size, websocketpp::frame::opcode::binary, error_code);

    if (error_code)
    {
        printf("Failed to send binary: %s\n", error_code.message().c_str());
        return;
    }
#endif
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

void Network::join(const std::string& id)
{
    if (isConnected())
    {
        game->ui.openStatusMenu("Joining Room", "Attempting to join room...");

        nlohmann::json message;
        message["type"] = "join";
        message["id"] = id;

        send(message.dump());
    }
}

void Network::create()
{
    if (isConnected())
    {
        game->ui.openStatusMenu("Creating Room", "Attempting to create room...");

        nlohmann::json message;
        message["type"] = "create";
        message["size"] = UCHAR_MAX - 1;

        send(message.dump());
    }
}

void Network::setBlock(int x, int y, int z, unsigned char blockType)
{
    if (isConnected() && players.size() > 1)
    {
        SetBlockPacket setBlockPacket = SetBlockPacket();

        if (isHost())
        {
            setBlockPacket.index = UCHAR_MAX;
        }
        else
        {
            setBlockPacket.index = 0;
        }

        setBlockPacket.blockType = blockType;
        setBlockPacket.position = glm::ivec3(x, y, z);

        sendBinary((unsigned char*)&setBlockPacket, sizeof(setBlockPacket));
    }
}

void Network::onOpen()
{
    connected = true;

#ifdef EMSCRIPTEN
    const char* hash = get_hash();

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
    char input[255];

    printf("Enter a room identifier to join (leave blank to create a room): ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = '\0';

    if (strlen(input) == 0) 
    {
        create();
    }
    else 
    {
        join(input);
    }
#endif
}

void Network::onClose()
{
    connected = false;

#ifdef EMSCRIPTEN
    set_hash("");
#endif

    game->ui.openStatusMenu("Disconnected", "The connection was closed.", true);
}

void Network::onMessage(const std::string& text)
{
    auto message = nlohmann::json::parse(text);

    if (message["type"] == "error")
    {
        std::string reason = message["message"];
        game->ui.openStatusMenu("Disconnected", reason.c_str(), true);
    }
    else if (message["type"] == "create")
    {
        std::string id = message["id"]; 

        players.push_back(nullptr); 

#ifdef EMSCRIPTEN
        set_hash(id.c_str());
#else
        printf("Room: %s\n", id.c_str());
#endif

        game->ui.log("Connected! Invite friends by sharing the link.");
        game->ui.closeMenu();
    }
    else if (message["type"] == "join")
    {
        if (message["size"].is_null())
        {
            Player* player = new Player();
            player->init(game);

            players.push_back(player);

            if (isHost())
            {
                std::unique_ptr<LevelPacket> levelPacket = std::make_unique<LevelPacket>();
                levelPacket->index = (unsigned int)players.size() - 1;
                levelPacket->respawn = true;

                memcpy(levelPacket->level, game->level.blocks, sizeof(levelPacket->level));
                sendBinary((unsigned char*)levelPacket.get(), sizeof(*levelPacket));
            } 

            game->ui.log("A player has connected to the room.");
        }
        else
        {
            unsigned int size = message["size"];

            for (unsigned int i = 0; i < size; i++)
            {
                Player* player = new Player();
                player->init(game);

                players.push_back(player);
            }

            players.push_back(nullptr);

            game->ui.log("Connected! Invite friends by sharing the link.");
        }
    }
    else if (message["type"] == "leave")
    {
        unsigned int index = message["index"];

        delete players[index];
        players.erase(players.begin() + index); 

        if (isHost())
        {
            std::unique_ptr<LevelPacket> levelPacket = std::make_unique<LevelPacket>();
            levelPacket->index = UCHAR_MAX;
            levelPacket->respawn = false;

            memcpy(levelPacket->level, game->level.blocks, sizeof(levelPacket->level));
            sendBinary((unsigned char*)levelPacket.get(), sizeof(*levelPacket));
        }
        else if (!index)
        {
            game->ui.openStatusMenu("Migrating Host", "Attempting to migrate the host...");
        }

        game->ui.log("A player has left the room.");
    }
}

void Network::onBinaryMessage(const unsigned char* data)
{
    unsigned char index = data[0];
    unsigned char type = data[1];

    if (type == (unsigned char)PacketType::Level)
    {
        if (index)
        {
            printf("network error: cannot process level packet from a non-host.\n");
            return;
        }

        LevelPacket* levelPacket = (LevelPacket*)data;
        memcpy(game->level.blocks, levelPacket->level, sizeof(levelPacket->level));

        game->level.calculateLightDepths(0, 0, game->level.width, game->level.depth);
        game->levelRenderer.loadChunks(0, 0, 0, game->level.width, game->level.height, game->level.depth);

        if (levelPacket->respawn)
        {
            game->level.calculateSpawnPosition();
        }

        game->ui.closeMenu();
    }
    else if (type == (unsigned char)PacketType::Position)
    {
        positionPackets.push_back(*(PositionPacket*)data);
    } 
    else if (type == (unsigned char)PacketType::SetBlock)
    {
        SetBlockPacket* setBlockPacket = (SetBlockPacket*)data;

        auto previousBlockType = game->level.getTile(
            setBlockPacket->position.x,
            setBlockPacket->position.y,
            setBlockPacket->position.z
        );

        if (isHost())
        {
            if (game->level.isWaterTile(setBlockPacket->blockType) || game->level.isLavaTile(setBlockPacket->blockType))
            {
                setBlock(
                    setBlockPacket->position.x,
                    setBlockPacket->position.y,
                    setBlockPacket->position.z,
                    previousBlockType
                );

                printf("network error: attempted to place a forbidden block.\n");
                return;
            }

            game->level.setTileWithNeighborChange(
                setBlockPacket->position.x, 
                setBlockPacket->position.y, 
                setBlockPacket->position.z, 
                setBlockPacket->blockType
            );
        }
        else
        {
            if (index)
            {
                printf("network error: cannot process set block packet from a non-host.\n");
                return;
            }

            game->level.setTileWithRender(
                setBlockPacket->position.x,
                setBlockPacket->position.y,
                setBlockPacket->position.z,
                setBlockPacket->blockType
            );
        }

        if (
            !game->level.isAirTile(previousBlockType) && 
            !game->level.isWaterTile(previousBlockType) &&
            !game->level.isLavaTile(previousBlockType) &&
            game->level.isAirTile(setBlockPacket->blockType)
        )
        {
            game->particleManager.spawn(
                (float)setBlockPacket->position.x,
                (float)setBlockPacket->position.y,
                (float)setBlockPacket->position.z,
                previousBlockType
            );
        }
    }
}