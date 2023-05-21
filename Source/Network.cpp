#include "Network.h"
#include "Game.h"

static Network* network;

#ifdef EMSCRIPTEN

#else
#include <windows.h>
#include <tlhelp32.h>

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
        positionPacket.index = 255;
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
    auto programCount = []() {
        std::wstring programName = L"Cubic.exe";

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }

        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(snapshot, &processEntry)) {
            CloseHandle(snapshot);
            return 0;
        }

        int count = 0;

        do {
            std::wstring currentProcessName = processEntry.szExeFile;
            if (currentProcessName == programName) {
                count++;
            }
        } while (Process32Next(snapshot, &processEntry));

        CloseHandle(snapshot);
        return count;
    };

    connected = true;

    json message;

    if (programCount() > 1)
    {
        message["type"] = "join";
        message["id"] = "hi";
    }
    else
    {
        message["type"] = "create";
        message["size"] = 254;
    }

    std::string dumpedMessage = message.dump();
    send(dumpedMessage);
}

void Network::onClose()
{
    connected = false;
}

void Network::onMessage(const std::string& text)
{
    auto message = json::parse(text);

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
        players.erase(players.begin() + message["index"]);
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