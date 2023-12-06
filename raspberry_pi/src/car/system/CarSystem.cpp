#include "CarSystem.h"

namespace car::system {
	CarSystem::CarSystem(const std::string& websocket_url, std::unique_ptr<LidarDevice> lidar_device, std::unique_ptr<MessagingSystem> messaging_system) : lidar_device(std::move(lidar_device)), messaging_system(std::move(messaging_system))
	{
		this->initalize();
	}

	CarSystem::~CarSystem()
	{
		this->terminate();
	}

	void CarSystem::initalize()
	{
		this->messaging_system->initalize();

		this->messaging_system->move_command_signal.connect([this](MoveCommand move_command) {
			this->move(move_command);
		});

		this->messaging_system->turn_command_signal.connect([this](TurnCommand turn_command) {
			this->turn(turn_command);
		});
	}

	void CarSystem::start()
	{
		this->messaging_system->start();
		this->lidar_device->start();
	}

	void CarSystem::update()
	{
		json output_json;
		output_json["data"] = json::array();
		std::vector<Measure> scan = this->lidar_device->scan();
		for (const Measure& measure : scan)
		{
			output_json["data"].push_back(
				{
				{"distance", measure.distance},
				{"angle", measure.angle},
				}
			);
		}
		this->messaging_system->sendMessage(output_json.dump());
	}

	void CarSystem::terminate()
	{
		this->lidar_device->terminate();
		this->messaging_system->terminate();
	}

	void CarSystem::move(MoveCommand& move_command)
	{
	}

	void CarSystem::turn(TurnCommand& turn_command)
	{
	}
}