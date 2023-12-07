#include <optional>
#include <string>

#include <fmt/format.h>

#include "global/Config.hpp"

#include "car/display/CarConsole.hpp"

#include "car/system/CarSystem.h"

#include "car/system/lidar/LidarScanner.hpp"
#include "car/system/lidar/LidarDummy.hpp"

std::string getWebSocketUrl()
{
	std::optional<int> maybe_port = GET_CONFIG_VALUE(port);
	std::string host_name;
	if (maybe_port.has_value())
	{
		host_name = fmt::format("{}:{}", GET_CONFIG_VALUE(host), maybe_port.value());
	}
	else
	{
		host_name = GET_CONFIG_VALUE(host);
	}
	return fmt::format("ws://{}/ws/room?request=join&type=car&room_name={}", host_name, GET_CONFIG_VALUE(room));
}

int main()
{
	using namespace car::display;
	using namespace car::system;
	using namespace car::system::lidar;
	using namespace car::system::messaging;
	using namespace rplidar;

	// spdlog::set_level(spdlog::level::off);
	//test();
	
	std::string websocket_url = getWebSocketUrl();
	spdlog::info("Got websocket url: {}", websocket_url);

	//std::unique_ptr<LidarDummy> scanner = std::make_unique<LidarDummy>();
	std::unique_ptr<LidarScanner> scanner = std::make_unique<LidarScanner>(GET_CONFIG_VALUE(lidar_port));

	std::unique_ptr<MessagingSystem> messaging_system = std::make_unique<MessagingSystem>(websocket_url);

	auto car_system = std::make_unique<CarSystem>(
		websocket_url,
		std::move(scanner),
		std::move(messaging_system)
	);

	// The CarConsole object will display the UI and handle user input:
	CarConsole car_console(std::move(car_system));
	car_console.run();
	return 0;
}
