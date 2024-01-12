#ifndef MESSAGINGSYSTEM_CXX
#define MESSAGINGSYSTEM_CXX

#pragma once

#include <functional>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include <nod/nod.hpp>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace car::system::messaging {
	class MessagingSystem
	{
	public:
		MessagingSystem(const std::string& websocket_url) : websocket_url(websocket_url) {
		};

		void initialize()
		{
			ix::initNetSystem();
			this->websocket.setUrl(websocket_url);
			this->websocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
				{
					if (msg->type == ix::WebSocketMessageType::Message)
					{
						json message_json = json::parse(msg->str);
						if (message_json["type"] == "command") {
							if (message_json["command"] == "turn") {
								float angle = message_json["angle"].get<float>();
								this->angle_command_signal(angle);
								spdlog::info("Turning by {} angle", angle);
							}
							else if (message_json["command"] == "move") {
								int speed = message_json["speed"].get<int>();
								this->speed_command_signal(speed);
								spdlog::info("Moving with {} speed", speed);
							}
						}
					}
					//else if (msg->type == ix::WebSocketMessageType::Open)
					//{
					//	std::cout << "Connection established" << std::endl;
					//	std::cout << "> " << std::flush;
					//}
					//else if (msg->type == ix::WebSocketMessageType::Error)
					//{
					//	// Maybe SSL is not configured properly
					//	std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
					//	std::cout << "> " << std::flush;
					//}
					//else if (msg->type == ix::WebSocketMessageType::Close)
					//{
					//	std::cout << "Connection closed: " << msg->closeInfo.code << " " << msg->closeInfo.reason << std::endl;
					//	std::cout << "> " << std::flush;
					//}
				}
			);
		}

		void start()
		{
			this->websocket.start();
			bool open = false;
			for (int i = 0; i < 3; i++) {
				if (this->websocket.getReadyState() == ix::ReadyState::Open) {
					open = true;
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(i * 3000));
			}
			if (!open) {
				spdlog::error("Could not connect to websocket");
				return;
			}
			else
			{
				spdlog::info("Connected to websocket");
			}
		}

		void stop() {
			this->websocket.stop();
			ix::uninitNetSystem();
		}

		void sendMessage(const std::string& message) {
			this->websocket.send(message);
		}

		~MessagingSystem() {
		};

		nod::signal<void(const int)> speed_command_signal;
		nod::signal<void(const float)> angle_command_signal;
	private:
		ix::WebSocket websocket;
		std::string websocket_url;
	};
};

#endif