#ifndef MAINBUTTON_CXX
#define MAINBUTTON_CXX

#pragma once

#include <ftxui/component/component.hpp>

#include "../../../system/CarSystem.h"

using namespace ftxui;
using namespace car::system;

namespace car::display::component::main {
	class MainButton {
	public:
		MainButton(std::shared_ptr<CarSystem> car_system) : car_system(car_system) {}
		Component element() {
			auto main_button_lambda = [&] {
				if (this->main_debounce) return;
				this->main_debounce = true;
				this->button_pressed = !this->button_pressed;
				if (button_pressed) {
					this->main_button_text = "Connecting...";
					this->car_system->start();
					this->main_button_text = "Stop Car Application";
				}
				else {
					this->main_button_text = "Disconnecting...";
					this->car_system->stop();
					this->main_button_text = "Start Car Application";
				}
				this->main_debounce = false;
				};

			this->main_button = Button(&this->main_button_text, std::move(main_button_lambda), ButtonOption::Animated());
			return this->main_button;
		}

	private:
		std::shared_ptr<CarSystem> car_system;

		bool main_debounce = false;
		bool button_pressed = false;
		std::string main_button_text = "Start Car Application";

		Component main_button;
	};
}

#endif