#ifndef CARSYSTEM_H
#define CARSYSTEM_H

#pragma once

#include <memory>

#include "car/configuration/Configuration.h"

#include "car/system/device/DeviceManager.h"
#include "car/system/messaging/MessagingSystem.h"
#include "car/system/movement/MovementSystem.h"

#include "car/plugin/PluginManager.h"

using namespace car::configuration;
using namespace car::plugin;
using namespace car::system::device;
using namespace car::system::messaging;
using namespace car::system::movement;

namespace car::system
{
	// Make sure this is stored as a shared_ptr
	class CarSystem : public std::enable_shared_from_this<CarSystem>
	{
	public:
		CarSystem(
			std::shared_ptr<Configuration> configuration,
			std::unique_ptr<DeviceManager> device_manager,
			std::unique_ptr<MessagingSystem> messaging_system,
			std::unique_ptr<MovementSystem> movement_system,
			std::unique_ptr<PluginManager> plugin_manager);

		void initialize();
		void reload();

		void start();
		void stop();

		tl::expected<nullptr_t, std::string> tryConnect();
		void disconnect();

		void terminate();

		void update();

		const std::shared_ptr<Configuration> getConfiguration() const { return this->configuration_; };
		void setConfiguration(std::shared_ptr<Configuration> configuration);

		DeviceManager *getDeviceManager() const
		{
			return this->device_manager_.get();
		}

		MessagingSystem *getMessagingSystem() const
		{
			return this->messaging_system_.get();
		}

		MovementSystem *getMovementSystem() const
		{
			return this->movement_system_.get();
		}

		template <typename T>
		const std::shared_ptr<T> getPlugin() const { return this->plugin_manager_->getPlugin<T>(); }

	private:
		void sendData();

		std::shared_ptr<Configuration> configuration_;

		const std::unique_ptr<DeviceManager> device_manager_;
		const std::unique_ptr<MessagingSystem> messaging_system_;
		const std::unique_ptr<MovementSystem> movement_system_;
		const std::unique_ptr<PluginManager> plugin_manager_;

		bool initialized = false;
		bool started = false;
	};
}

#endif