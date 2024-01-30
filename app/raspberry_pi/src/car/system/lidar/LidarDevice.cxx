#ifndef LidarDevice_CXX
#define LidarDevice_CXX

#pragma once

#include <vector>

#include <RPLidar.h>

using namespace rplidar;

namespace car::system::lidar {
	class LidarDevice
	{
	public:
		virtual void initialize() const = 0;
		virtual void start() const = 0;
		virtual std::vector<tl::expected<Measure, std::string>> scan() const = 0;
		virtual void stop() const = 0;
		virtual void terminate() const = 0;
		virtual void disconnect() const = 0;
	};
} 

#endif