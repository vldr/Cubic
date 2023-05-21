#include "Network.h"
#include "Game.h"

static Network* network;

#ifdef EMSCRIPTEN
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
    network->connection_handle = connection_handle;
    network->onOpen();
}
#endif

void Network::init(Game* game)
{
    this->game = game;
    this->connected = false;

    network = this;

#ifdef EMSCRIPTEN
    EmscriptenWebSocketCreateAttributes ws_attrs = {
        URI,
        NULL,
        EM_TRUE
    };

    socket = emscripten_websocket_new(&ws_attrs);
    emscripten_websocket_set_onopen_callback(socket, NULL, emscripten_on_open);
    emscripten_websocket_set_onclose_callback(socket, NULL, emscripten_on_close);
    emscripten_websocket_set_onmessage_callback(socket, NULL, emscripten_on_message);
#else
    try
    {
        socket.set_access_channels(websocketpp::log::alevel::all);
        socket.clear_access_channels(websocketpp::log::alevel::all);

        socket.init_asio();
        socket.set_message_handler(bind(&websocketpp_on_message, &socket, ::_1, ::_2));
        socket.set_close_handler(bind(&websocketpp_on_close, &socket, ::_1));
        socket.set_open_handler(bind(&websocketpp_on_open, &socket, ::_1));

        websocketpp::lib::error_code error_code;
        websocketpp_client::connection_ptr con = socket.get_connection(URI, error_code);

        if (error_code)
        {
            printf("Failed to connect: %s\n", error_code.message().c_str());
            return;
        }

        socket.connect(con);
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
    socket.poll();
#endif

    if (connected)
    {
        PositionPacket positionPacket = PositionPacket();
        positionPacket.index = UCHAR_MAX;
        positionPacket.position = game->localPlayer.position;
        positionPacket.rotation = game->localPlayer.viewAngles;

        sendBinary((unsigned char*)&positionPacket, sizeof(positionPacket));
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

void Network::send(std::string& text)
{
#ifdef EMSCRIPTEN
    EMSCRIPTEN_RESULT result = emscripten_websocket_send_utf8_text(socket, text.c_str());
    if (result)
    {
        printf("Failed to send: %d\n", result);
    }
#else
    websocketpp::lib::error_code error_code;
    socket.send(connection_handle, text, websocketpp::frame::opcode::text, error_code);

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
    socket.send(connection_handle, (void*)data, size, websocketpp::frame::opcode::binary, error_code);

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

void Network::onOpen()
{
    connected = true;

    printf("onOpen\n");
}

void Network::join()
{
    json message;
    message["type"] = "join";
    message["id"] = "hi";

    std::string dumpedMessage = message.dump();
    send(dumpedMessage);
}

void Network::create()
{
    json message;
    message["type"] = "create";
    message["size"] = UCHAR_MAX - 1;

    std::string dumpedMessage = message.dump();
    send(dumpedMessage);
}

void Network::onClose()
{
    connected = false;

    printf("onClose\n");
}

void Network::onMessage(const std::string& text)
{
    auto message = json::parse(text);

    printf("onMessage: %s\n", text.c_str());

    if (message["type"] == "create")
    {
        players.push_back(nullptr);
    }
    else if (message["type"] == "join")
    {
        if (message["size"].is_null())
        {
            Player* player = new Player();
            player->init(game);

            players.push_back(player);

            std::unique_ptr<LevelPacket> levelPacket = std::make_unique<LevelPacket>();
            levelPacket->index = players.size() - 1;

            memcpy(levelPacket->level, game->level.blocks, sizeof(levelPacket->level));
            sendBinary((unsigned char*)levelPacket.get(), sizeof(*levelPacket));
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
        }
    }
    else if (message["type"] == "leave")
    {
        unsigned int index = message["index"];
        delete players[index];
        players.erase(players.begin() + index); 
    }
}

void Network::onBinaryMessage(const unsigned char* data)
{
    unsigned char index = data[0];
    unsigned char type = data[1];

    if (type == (unsigned char)PacketType::Level)
    {
        LevelPacket* levelPacket = (LevelPacket*)data;
        memcpy(game->level.blocks, levelPacket->level, sizeof(levelPacket->level));

        game->level.calculateLightDepths(0, 0, game->level.width, game->level.depth);
        game->levelRenderer.loadChunks(0, 0, 0, game->level.width, game->level.height, game->level.depth);
    }
    else if (type == (unsigned char)PacketType::Position)
    {
        PositionPacket* positionPacket = (PositionPacket*)data;

        if (players[index])
        {
            players[index]->rotate(positionPacket->rotation.x, positionPacket->rotation.y);
            players[index]->move(positionPacket->position.x, positionPacket->position.y, positionPacket->position.z);
        }
    }
    else if (type == (unsigned char)PacketType::SetBlock)
    {
    }
}