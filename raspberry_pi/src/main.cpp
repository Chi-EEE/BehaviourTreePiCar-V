#include <iostream>
#include <fstream>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include <fmt/format.h>

#include <nlohmann/json.hpp>

#include <chrono> // std::chrono::microseconds
#include <thread> // std::this_thread::sleep_for

#include <RPLidar.h>

using json = nlohmann::json;
using namespace rplidar;

int main()
{
	spdlog::set_level(spdlog::level::off);

	ix::initNetSystem();

	ix::WebSocket web_socket;

	const std::string url("ws://localhost:8848/chat");
	web_socket.setUrl(url);
	web_socket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg)
		{
			//if (msg->type == ix::WebSocketMessageType::Message)
			//{
			//	std::cout << "received message: " << msg->str << std::endl;
			//	std::cout << "> " << std::flush;
			//}
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
	web_socket.start();

	std::unique_ptr<RPLidar> lidar = RPLidar::create("COM3").value();
	lidar->reset();
	lidar->stop();
	lidar->stop_motor();

	auto info_result = lidar->get_info();
	if (!info_result.has_value()) {
		std::cout << "get_info failed: " << info_result.error() << std::endl;
		return EXIT_FAILURE;
	}
	auto info = info_result.value();
	std::cout << fmt::format("model: {}, firmware: ({}, {}), hardware: {}, serialnumber: {}\n", info.model, info.firmware.first, info.firmware.second, info.hardware, info.serialNumber);

	const auto health_result = lidar->get_health();
	if (!health_result.has_value()) {
		std::cout << "get_health failed: " << health_result.error() << std::endl;
		return EXIT_FAILURE;
	}
	const auto health = health_result.value();
	std::cout << fmt::format("({}, {})\n", health.status, health.errorCode);

	lidar->start_motor();
	const std::function<std::vector<Measure>()> scan_generator = lidar->iter_scans();
	while (true)
	{
		json output = json::array();
		std::vector<Measure> scan = scan_generator();
		for (const Measure& measure : scan)
		{
			output.push_back(
				{
				{"distance", measure.distance},
				{"angle", measure.angle},
				}
			);
		}
		web_socket.send(output.dump());
	}

	lidar->stop();
	lidar->stop_motor();
	lidar->disconnect();
	
    ix::uninitNetSystem();
	
	return 0;
}
