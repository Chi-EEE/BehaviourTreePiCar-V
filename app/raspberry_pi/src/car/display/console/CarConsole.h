#ifndef CARCONSOLE_H
#define CARCONSOLE_H

#pragma once

#include <memory>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/loop.hpp>

#include <nod/nod.hpp>

#include "../../system/CarSystem.h"
#include "../../system/logging/VectorSink.cxx"

#include "screen/MainScreen.cxx"
#include "screen/SettingsScreen.cxx"
#include "screen/LoggingScreen.cxx"

using namespace car::system;
using namespace car::display::console::screen;

using namespace ftxui;

namespace car::display::console
{
	class CarConsole
	{
	public:
		CarConsole(std::shared_ptr<CarSystem> car_system, std::shared_ptr<logging::vector_sink_mt> vector_sink);

		void initialize();

		void run();

		void terminate();

	private:
		std::shared_ptr<CarSystem> car_system;
		std::shared_ptr<logging::vector_sink_mt> vector_sink;
	};
}

#endif