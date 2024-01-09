#ifndef CARSYSTEM_H
#define CARSYSTEM_H

#pragma once

#include <memory>

#include <nlohmann/json.hpp>

#include "lidar/LidarDevice.cxx"
#include "messaging/MessagingSystem.cxx"
#include "movement/MovementSystem.cxx"

using json = nlohmann::json;

using namespace car::system::lidar;
using namespace car::system::messaging;
using namespace car::system::movement;

namespace car::system {
	class CarSystem
	{
	public:
		CarSystem(const std::string& websocket_url, std::unique_ptr<LidarDevice> lidar_device, std::unique_ptr<MessagingSystem> messaging_system, std::unique_ptr<MovementSystem> movement_system);
		~CarSystem();

		void initialize();
		void start();
		void stop();

		void update();

		void start_lidar_device();
		void stop_lidar_device();

		void set_rear_wheels_speed(const SpeedCommand& speed_command);
		void set_front_wheels_angle(const AngleCommand& angle_command);

		void set_rear_left_wheel_speed(const SpeedCommand& speed_command);
		void set_rear_right_wheel_speed(const SpeedCommand& speed_command);

		void set_front_left_wheel_angle(const AngleCommand& angle_command);
		void set_front_right_wheel_angle(const AngleCommand& angle_command);

		const std::vector<Measure>& get_scan_data() const { return this->scan_data; }

	private:
		std::unique_ptr<LidarDevice> lidar_device;
		std::unique_ptr<MessagingSystem> messaging_system;
		std::unique_ptr<MovementSystem> movement_system;

		std::vector<Measure> scan_data;

		bool running = false;
	};
}

#endif