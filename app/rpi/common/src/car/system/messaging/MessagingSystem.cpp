#include "car/system/messaging/MessagingSystem.h"

#include <functional>
#include <memory>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include <nod/nod.hpp>

#include <spdlog/spdlog.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <fmt/format.h>

#include "car/configuration/Configuration.h"

namespace car::system::messaging
{
    MessagingSystem::MessagingSystem(std::shared_ptr<configuration::Configuration> configuration) : configuration(configuration)
    {
    }

    void MessagingSystem::initialize()
    {
        ix::initNetSystem();
    }

    void MessagingSystem::initializeWebSocket()
    {
        this->websocket = std::make_unique<ix::WebSocket>();
        std::string websocket_url = this->getWebSocketUrl();
        this->websocket->setUrl(websocket_url);
        this->websocket->setOnMessageCallback(
            std::bind(&MessagingSystem::onMessageCallback, this, std::placeholders::_1));
        this->handle_message_signal.connect([this](const std::string message)
                                            { this->handleMessage(message); });
    }

    void MessagingSystem::start()
    {
        initializeWebSocket();
        this->websocket->start();
        bool open = false;
        for (int i = 0; i < 3; i++)
        {
            ix::ReadyState ready_state = this->websocket->getReadyState();
            if (ready_state == ix::ReadyState::Open)
            {
                open = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(i * 3000));
        }
        if (!open)
        {
            spdlog::error("Could not connect to websocket");
            return;
        }
        else
        {
            spdlog::info("Connected to websocket");
        }
    }

    void MessagingSystem::stop()
    {
        if (this->websocket != nullptr)
        {
            this->websocket->stop();
            this->websocket = nullptr;
        }
    }

    void MessagingSystem::terminate()
    {
        this->stop();
        ix::uninitNetSystem();
    }

    void MessagingSystem::onMessageCallback(const ix::WebSocketMessagePtr &msg) const
    {
        switch (msg->type)
        {
        case ix::WebSocketMessageType::Open:
        {
            spdlog::info("WebSocket connected");
            this->on_websocket_connect_signal();
            break;
        }
        case ix::WebSocketMessageType::Close:
        {
            spdlog::info("WebSocket closed");
            this->on_websocket_disconnect_signal();
            break;
        }
        case ix::WebSocketMessageType::Message:
        {
            this->handle_message_signal(msg->str);
            break;
        }
        default:
            break;
        }
    }

    void MessagingSystem::handleMessage(const std::string &message) const
    {
        rapidjson::Document message_json;
        message_json.Parse(message.c_str());
        if (message_json.HasParseError() || !message_json.IsObject())
        {
            spdlog::error("An error has occurred while handling the message: {}", message);
            return;
        }
        if (!message_json.HasMember("type") || !message_json["type"].IsString())
        {
            spdlog::error("Type does not exist in json", message);
            return;
        }
        const std::string type = message_json["type"].GetString();
        if (type == "car")
        {
            return;
        }

        try
        {
            switch (hash(type))
            {
            case hash("command"):
                this->handleCommand(message_json);
                break;
            case hash("status"):
                spdlog::info("Received status message");
                break;
            default:
                break;
            }
        }
        catch (std::exception e)
        {
            spdlog::error("An error has occurred while handling the message: {} | {}", message, e.what());
        }
    }

    void MessagingSystem::handleCommand(const rapidjson::Value &message_json) const
    {
        if (!message_json.HasMember("command") || !message_json["command"].IsString())
        {
            spdlog::error("Command not found or not a string in the JSON.");
            return;
        }

        const std::string command = message_json["command"].GetString();

        switch (hash(command))
        {
        case hash("turn"):
        {
            if (message_json.HasMember("angle") && message_json["angle"].IsFloat())
            {
                float angle = message_json["angle"].GetFloat();
                this->angle_command_signal(angle);
                spdlog::info("Turning by {} angle", angle);
            }
            else
            {
                spdlog::error("Invalid or missing 'angle' in the JSON for 'turn' command.");
            }
            break;
        }
        case hash("move"):
        {
            if (message_json.HasMember("speed") && message_json["speed"].IsInt())
            {
                int speed = message_json["speed"].GetInt();
                speed_command_signal(speed);
                spdlog::info("Moving with {} speed", speed);
            }
            else
            {
                spdlog::error("Invalid or missing 'speed' in the JSON for 'move' command.");
            }
            break;
        }
        case hash("custom"):
        {
            if (message_json.HasMember("custom_type") && message_json.HasMember("custom") &&
                message_json["custom_type"].IsString() && message_json["custom"].IsString())
            {
                const std::string custom_type = message_json["custom_type"].GetString();
                const std::string custom = message_json["custom"].GetString();
                this->custom_command_signal(custom_type, custom);
            }
            else
            {
                spdlog::error("Invalid or missing 'custom_type' or 'custom' in the JSON for 'custom' command.");
            }
            break;
        }
        default:
            spdlog::error("Unknown command: {}", command);
        }
    }

    void MessagingSystem::sendMessage(const std::string &message)
    {
        if (this->websocket != nullptr)
            this->websocket->send(message);
    }
    
    std::string MessagingSystem::getWebSocketUrl()
    {
        std::optional<int> maybe_port = this->configuration->port;
        std::string full_ip_address;
        if (maybe_port.has_value())
        {
            full_ip_address = fmt::format("{}:{}", this->configuration->ip_address, maybe_port.value());
        }
        else
        {
            full_ip_address = this->configuration->ip_address;
        }
        return fmt::format("ws://{}/ws/room?request=join&type=car&room_name={}", full_ip_address, this->configuration->room);
    }
};