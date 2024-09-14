#pragma once

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <string_view>

#include "Consts.hpp"

class Application
{
public:

	Application(
		uint32_t width,
		uint32_t height,
		std::string_view title)
		: window_({ width, height }, title.data(), sf::Style::Close)
	{
	}

	virtual ~Application() = default;

	void run()
	{
		window_.setFramerateLimit(consts::framerate_);

		if (!ImGui::SFML::Init(window_))
		{
			return;
		}

		setup();

		sf::Clock clock;
		while (window_.isOpen())
		{
			for (auto event = sf::Event{}; window_.pollEvent(event);)
			{
				ImGui::SFML::ProcessEvent(window_, event);
				events(event);
			}

			auto dt = clock.restart();
			ImGui::SFML::Update(window_, dt);

			update(dt);

			window_.clear();

			render();

			ImGui::SFML::Render(window_);

			window_.display();
		}

		ImGui::SFML::Shutdown();
	}

protected:
	virtual void setup() {}
	virtual void events(const sf::Event& event) {}
	virtual void render() {}
	virtual void update(const sf::Time& dt) {}

	sf::RenderWindow window_;
};
