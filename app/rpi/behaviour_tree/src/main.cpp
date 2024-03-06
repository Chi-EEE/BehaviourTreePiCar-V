#include <iostream>
#include <chrono>
#include <filesystem>
#include <memory>

#include <cxxopts.hpp>

#include "car/system/CarSystem.h"

#include "car/system/lidar/LidarScanner.h"
#include "car/system/lidar/LidarDummy.h"

#include "car/system/movement/controller/DummyMovementController.h"
#include "car/system/movement/controller/DeviceMovementController.h"

#include "car/plugin/PluginManager.h"

#include "behaviour_tree/BehaviourTreeParser.hpp"
#include "behaviour_tree/node/custom/CarCustomNodeParser.hpp"

#include "behaviour_tree/BehaviourTreeHandler.hpp"

using namespace car::system;
using namespace car::system::lidar;
using namespace car::system::movement::controller;

using namespace car::configuration;
using namespace car::plugin;

using namespace behaviour_tree;

#include <chrono>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

#ifndef _WIN32
// From: https://gist.github.com/vsajip/1864660
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}
#endif

std::unique_ptr<LidarDevice> getLidarDevice(bool dummy);

int main(int argc, const char* argv[])
{
#ifdef __linux
	if (getuid())
	{
		std::cout << "This program will not work properly unless you are root. Please run this program as root using `sudo`.\n";
		return EXIT_FAILURE;
	}
#endif
	BehaviourTreeParser::instance().setCustomNodeParser(std::make_shared<node::custom::CarCustomNodeParser>(CarCustomNodeParser()));

	std::string exe_dir = std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path().string();

	std::string ip_address = "";
	int port = 0;
	std::string rpi_name = "";
	std::string room_name = "";

	std::shared_ptr<Configuration> configuration = std::make_shared<Configuration>(Configuration{
		ip_address,
		port,
		rpi_name,
		room_name,
		});



	constexpr bool test = true;

	std::string behaviour_tree_string;
	if (!test) {
		cxxopts::Options options("Behaviour Tree CLI", "Program to parse Behaviour Tree");

		options.add_options()
			("behaviour_tree", "Behaviour Tree XML to run", cxxopts::value<std::string>());

		auto cli_result = options.parse(argc, argv);

		behaviour_tree_string = cli_result["behaviour_tree"].as<std::string>();
	}
	else {
		behaviour_tree_string = R"(<BehaviourTree>
    <Root id='main'>
        <Sequence>
            <Action:SetAngle servo_type='FrontWheels' angle='180'/>
            <Action:SetSpeed wheel_type='Both' speed='50'/>
            <Action:SetWheelDirection wheel_type='Both' direction_type='Forward'/>
            <Action:SetWheelDirection wheel_type='Both' direction_type='Backward'/>
            <Invert>
                <Condition:NearbyPoints min_angle='0' max_angle='0' distance='200'/>
            </Invert>

            <Action:PauseExecution ms='5000'/>

            <Action:Log text='Waited 5 seconds'/>

            <UseRoot id='abc'/>

            <Invert>
                <Fail />
                <Succeed />
            </Invert>
            <Repeat count='2' break_on_fail='false'>
                <Action:Log text='Hey'/>
            </Repeat>
        </Sequence>
    </Root>
    <Root id='abc'>

    </Root>
</BehaviourTree>)";
	}

	const bool dummy = true;

	std::unique_ptr<LidarDevice> scanner = getLidarDevice(dummy);

	std::unique_ptr<MessagingSystem> messaging_system = std::make_unique<MessagingSystem>(MessagingSystem());

#ifdef __linux
	std::unique_ptr<MovementSystem> movement_system;
	if (!dummy)
	{
		movement_system = std::make_unique<MovementSystem>(std::make_unique<DeviceMovementController>());
	}
	else
	{
		movement_system = std::make_unique<MovementSystem>(std::make_unique<DummyMovementController>());
	}
#else
	std::unique_ptr<MovementSystem> movement_system = std::make_unique<MovementSystem>(std::make_unique<DummyMovementController>());
#endif

	auto behaviour_tree_result = BehaviourTreeParser::instance().parseXML(behaviour_tree_string);

	if (!behaviour_tree_result.has_value())
	{
		spdlog::error("Unable to parse the Behaviour Tree: {}", behaviour_tree_result.error());
		return EXIT_FAILURE;
	}

	std::shared_ptr<BehaviourTree> behaviour_tree = behaviour_tree_result.value();

	std::shared_ptr<BehaviourTreeHandler> behaviour_tree_handler = std::make_shared<BehaviourTreeHandler>(BehaviourTreeHandler());
	behaviour_tree_handler->setBehaviourTree(behaviour_tree);

	std::unique_ptr<PluginManager> plugin_manager = std::make_unique<PluginManager>();
	plugin_manager->addPlugin(behaviour_tree_handler);

	std::shared_ptr<CarSystem> car_system = std::make_shared<CarSystem>(
		configuration,
		std::move(scanner),
		std::move(messaging_system),
		std::move(movement_system),
		std::move(plugin_manager));

	car_system->initialize();

	behaviour_tree_handler->start();

	std::cout << "Press any key to exit the loop." << std::endl;
	while (!kbhit()) {
		car_system->update();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	car_system->terminate();

	return 0;
}

std::unique_ptr<LidarDevice> getLidarDevice(bool dummy)
{
	if (dummy)
	{
		return std::make_unique<LidarDummy>();
	}
	else
	{
#ifdef __linux
		auto maybe_scanner = LidarScanner::create("/dev/ttyUSB0");
#else
		auto maybe_scanner = LidarScanner::create("COM3");
#endif
		if (!maybe_scanner.has_value())
		{
			spdlog::error("Unable to connect to the Lidar Scanner");
			throw std::runtime_error("Unable to connect to the Lidar Scanner");
		}
		return std::move(maybe_scanner.value());
	}
}