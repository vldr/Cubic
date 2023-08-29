#include "Network.h"
#include "Game.h"
#include "FastLZ.h"

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

void Network::init(Game* game)
{
    this->game = game;
    this->connected = false;

    network = this;

#ifndef EMSCRIPTEN
	try
	{
		socket_client.set_access_channels(websocketpp::log::alevel::none);
		socket_client.clear_access_channels(websocketpp::log::alevel::none);

		socket_client.init_asio();
		socket_client.set_message_handler(bind(&websocketpp_on_message, &socket_client, ::_1, ::_2));
		socket_client.set_fail_handler(bind(&websocketpp_on_close, &socket_client, ::_1));
		socket_client.set_close_handler(bind(&websocketpp_on_close, &socket_client, ::_1));
		socket_client.set_open_handler(bind(&websocketpp_on_open, &socket_client, ::_1));
	}
	catch (websocketpp::exception const& e)
	{
		std::cout << e.what() << std::endl;
	}
#endif
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
        websocketpp::lib::error_code error_code;
        websocketpp_client::connection_ptr con = socket_client.get_connection(URI, error_code);
        con->append_header("Origin", BASE_URL);

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

    sendPosition(game->localPlayer.position, game->localPlayer.viewAngles);

    for (const auto& positionPacket : positionPackets)
    {
        const auto index = positionPacket.index;

        if (index < players.size())
        {
            Player* player = players[index];

            if (player)
            {
                player->updated = true;

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

    for (int index = 0; index < players.size(); index++)
    {
        Player* player = players[index];

        if (player)
        {
            if (player->updated)
            {
                player->updated = false;
            }
            else
            {
                player->rotate(player->rotation.x, player->rotation.y);
                player->move(player->position.x, player->position.y + 1.62f, player->position.z);
            }
        }
    }
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
        LevelPacket* packet = new LevelPacket();
        packet->index = index;
        packet->respawn = respawn;
        packet->length = fastlz_compress(game->level.blocks, sizeof(packet->data) / 2, packet->data);

        sendBinary(
            (unsigned char*)packet,
            sizeof(*packet) - sizeof(packet->data) + packet->length
        );

        delete packet;
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

void Network::join(const std::string& id)
{
    if (isConnected())
    {
        game->ui.openStatusMenu("Joining Room", "Attempting to join room...");

        nlohmann::json message;
        message["type"] = "join";
        message["id"] = id;

        send(message.dump());

		url = BASE_URL + id;
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

    url = "Disconnected...";

	for (auto iterator = players.begin(); iterator != players.end(); iterator = players.erase(iterator))
	{
		if (*iterator)
		{
            delete *iterator;
		}
	}

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

        url = BASE_URL + id;

        game->ui.logMotd();
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
                sendLevel(
                    (unsigned char)players.size() - 1, 
                    true
                );
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

            game->ui.logMotd();
        }
    }
    else if (message["type"] == "leave")
    {
        unsigned int index = message["index"];

        delete players[index];
        players.erase(players.begin() + index); 

        if (isHost())
        {
            sendLevel(
                UCHAR_MAX, 
                false
            );

            if (game->ui.state == UI::State::StatusMenu)
            {
				game->ui.closeMenu();
            }
        }
        else if (!index)
        {
            game->ui.openStatusMenu("Migrating Host", "Attempting to migrate the host...");
        }

        game->ui.log("A player has left the room.");
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

        if (!fastlz_decompress(packet->data, packet->length, game->level.blocks, sizeof(packet->data) / 2))
        {
            printf("network error: failed to decompress level data.\n");
            return;
        }

        game->level.calculateLightDepths(0, 0, game->level.width, game->level.depth);
        game->levelRenderer.loadChunks(0, 0, 0, game->level.width, game->level.height, game->level.depth);

        if (packet->respawn)
        {
            game->level.calculateSpawnPosition();
        }

        game->ui.closeMenu();
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

        auto previousBlockType = game->level.getTile(packet->position.x, packet->position.y, packet->position.z);

        if (isHost())
        {
            if (game->level.isWaterTile(packet->blockType) || game->level.isLavaTile(packet->blockType))
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
                !game->level.isAirTile(previousBlockType) &&
                !game->level.isWaterTile(previousBlockType) &&
                !game->level.isLavaTile(previousBlockType) &&
                game->level.isAirTile(packet->blockType)
            )
            {
                game->particleManager.spawn(
                    (float)packet->position.x,
                    (float)packet->position.y,
                    (float)packet->position.z,
                    previousBlockType
                );

                mode = true;
            }

            game->level.setTileWithNeighborChange(
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
                game->level.setTileWithNoNeighborChange(
                    packet->position.x,
                    packet->position.y,
                    packet->position.z,
                    packet->blockType
                ) 
                &&
                packet->mode
            )
            {
                game->particleManager.spawn(
                    (float)packet->position.x,
                    (float)packet->position.y,
                    (float)packet->position.z,
                    previousBlockType
                );
            }
        }
    }
}