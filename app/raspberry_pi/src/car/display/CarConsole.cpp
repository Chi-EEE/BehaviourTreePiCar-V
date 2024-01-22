#include "CarConsole.h"

namespace car::display {
	CarConsole::CarConsole(std::shared_ptr<CarSystem> car_system, std::shared_ptr<logging::vector_sink_mt> vector_sink) : car_system(std::move(car_system)), vector_sink(vector_sink) {
	};

	void CarConsole::initialize() {
		this->car_system->initialize();
	};

	void CarConsole::run() {
		ScreenInteractive screen = ScreenInteractive::Fullscreen();
		
		std::function<void()> exit = screen.ExitLoopClosure();
		
		MainScreen main_screen(this->car_system, exit);
		auto main_screen_container = main_screen.element();

		SettingsScreen settings_screen(this->car_system);
		auto settings_screen_container = settings_screen.element();

		//LoggingScreen logging_screen;
		//auto logging_container = logging_screen.element();

		int selected_tab = 0;
		std::vector<std::string> tab_titles = {
			"Main",
			"Settings",
			"Logs",
		};

		auto tab_selection = Toggle(&tab_titles, &selected_tab);

		ftxui::Elements line_elements;
		auto tab_content = Container::Tab(
			{
				main_screen_container,
				settings_screen_container,
				Renderer(
					[&] {
						// TODO: Improve this so it doesn't create a log of lines
						line_elements.clear();
						for (const std::string& message : this->vector_sink->get_log_messages())
						{
							line_elements.push_back(paragraph(message));
							line_elements.push_back(separator());
						}
						// TODO: Make the log scrollable
						return vbox(line_elements) | xflex | vscroll_indicator | frame;
					}
				)
			}
		, &selected_tab);

		auto main_container = Container::Vertical(
			{
			tab_selection,
			tab_content,
			}
		);

		auto main_renderer = Renderer(main_container, [&]
			{
				return vbox({
					text("Car Application") | bold | hcenter,
					tab_selection->Render() | flex | center,
					tab_content->Render() | xflex ,
					}
				);
			}
		);

		std::atomic<bool> refresh_ui_continue = true;
		std::thread refresh_ui([&]
			{
				while (refresh_ui_continue) {
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(0.05s);
					screen.Post([&]
						{
							settings_screen.update();
						}
					);
					this->car_system->update();
					screen.Post(Event::Custom);
				}
			}
		);

		std::thread screen_thread([&]
			{
				screen.Loop(main_renderer);
			}
		);
		refresh_ui.detach();
		screen_thread.join();

		spdlog::info("Exiting CarConsole::run()");
		this->car_system->stop();
	};
}
