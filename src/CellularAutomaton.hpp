#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "Application.hpp"

using Squares = std::vector<sf::RectangleShape>;


class CellularAutomaton : public Application
{

public:
	CellularAutomaton(int seed = 0)
		: Application(consts::width_, consts::height_, consts::title_)
		, rng_(seed == 0 ? std::random_device{}() : seed)
		, random_(0, 5)
	{
	}

private:
	void setup() override
	{
		auto image = sf::Image{};
		if (image.loadFromFile("logo.png"))
		{
			window_.setIcon(image.getSize().x, image.getSize().y, image.getPixelsPtr());
		}

		restart();

		random_colors();
	}

	void events(const sf::Event& event) override
	{
		if (event.type == sf::Event::Closed)
		{
			window_.close();
		}

		if (event.mouseButton.button == sf::Mouse::Left)
		{
			if (event.type == sf::Event::MouseButtonPressed && !ImGui::GetIO().WantCaptureMouse)
			{
				left_mouse_pressed_ = true;
			}

			if (event.type == sf::Event::MouseButtonReleased)
			{
				left_mouse_pressed_ = false;
			}
		}

		if (event.type == sf::Event::MouseMoved && left_mouse_pressed_)
		{
			const auto mouse_pos = sf::Vector2f{ (float)event.mouseMove.x, (float)event.mouseMove.y };

			for (auto& square : squares_)
			{
				if (square.getGlobalBounds().contains(mouse_pos))
				{
					auto color = sf::Color(selected_color[0] * 255, selected_color[1] * 255, selected_color[2] * 255);
					square.setFillColor(color);
				}
			}
		}

	}

	void update(const sf::Time& dt) override
	{
		ImGui::Begin("CONTROLS", 0);

		if (ImGui::CollapsingHeader("HELP"))
		{
			if (ImGui::TreeNode("Instructions"))
			{
                ImGui::Text("(1) Set an initial state. Use the \"RANDOM\" button for convenient setup.");
				ImGui::Text("(2) Press \"START\".");
				ImGui::Text("(3) Wait and see what happens.");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Rule"))
			{
				ImGui::Text("Each cell takes the color of its neighboring cells.");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("More Info"))
			{
				ImGui::Text("For more info, hover your mouse over the question marks.");
				ImGui::TreePop();
			}
		}

		ImGui::SeparatorText("PLAY");
		{
			if (ImGui::Button(started_ ? "PAUSE" : "START"))
			{
				std::cout << "Start clicked\n";
				started_ = !started_;
			}
			ImGui::SameLine(); help_marker(
				"The cells will automatically update after each iteration.\n");

			ImGui::SameLine();

			if (ImGui::Button("NEXT") && !started_)
			{
				std::cout << "Next clicked\n";

				iterate();
			}
			ImGui::SameLine(); help_marker(
				"Go to the next iteration.\n");
			
			ImGui::SameLine();

			if (ImGui::Button("RESTART") && !started_)
			{
				restart();
			}
		}

		ImGui::SeparatorText("DRAWING");
		{
			if (ImGui::Button("RANDOM") && !started_)
			{
				random_colors();
			}
			ImGui::SameLine(); help_marker(
				"Every empty cell will receive a random color.\n");

			ImGui::Text("Select a color to draw");

			const char* items[] = { "Wall", "Empty", "Color 1", "Color 2", "Color 3", "Color 4", "Color 5", "Color 6" };
			static int selected_item_idx = 0;
			ImGui::Combo("Cell types", &selected_item_idx, items, IM_ARRAYSIZE(items));

			set_selected_color(selected_item_idx);

			ImGui::SameLine(); help_marker(
				"Cell type to identify the color to draw on the cells. Use mouse left click\n");

			ImGui::Text("Click and drag to draw with the selected color.");
		}

		ImGui::SeparatorText("CELLS COLORS");
		{
			ImGui::ColorEdit3("Wall color", wall_color);
			ImGui::SameLine(); help_marker(
				"Fixed cells. Cells with this color won't change.\n");

			ImGui::ColorEdit3("Empty color", empty_color);
			ImGui::SameLine(); help_marker(
				"Cells with this color might eventually change.\n");

			ImGui::ColorEdit3("Color 1", color1);
			ImGui::ColorEdit3("Color 2", color2);
			ImGui::ColorEdit3("Color 3", color3);
			ImGui::ColorEdit3("Color 4", color4);
			ImGui::ColorEdit3("Color 5", color5);
			ImGui::ColorEdit3("Color 6", color6);
		}

		ImGui::End();

		ImGui::Begin("STATUS", 0);

		ImGui::Text("FPS: %.1f", 1.0f / dt.asSeconds());
		
		ImGui::Text("Steps: %d", num_steps_);

		ImGui::Text("Color1: %d", get_num_color(0));
		ImGui::Text("Color2: %d", get_num_color(1));
		ImGui::Text("Color3: %d", get_num_color(2));
		ImGui::Text("Color4: %d", get_num_color(3));
		ImGui::Text("Color5: %d", get_num_color(4));
		ImGui::Text("Color6: %d", get_num_color(5));
		
		ImGui::Text("Empty: %d", get_num_color(6));
		ImGui::Text("Walls: %d", get_num_color(7));

		ImGui::End();

		if (!started_)
		{
			return;
		}
		
		static auto last_update_time = std::chrono::steady_clock::now();
		auto current_time = std::chrono::steady_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_update_time).count();

		if (elapsed_time >= consts::update_ms)
		{
			iterate();
			last_update_time = current_time;
		}
	}
	
	void render() override
	{
		for (const auto& square : squares_)
		{
			window_.draw(square);
		}
	}

	void iterate()
	{
		++num_steps_;

		auto squares = squares_;

		const int dx[8] = { 1, 1, 1, 0, 0, -1, -1, -1 };
		const int dy[8] = { -1, 0, 1, -1,1, -1, 0, 1 };

		for (auto i{ 0 }; i < squares.size(); ++i)
		{
			const auto& square = squares[i];

			if (same_color(square.getFillColor(), wall_color))
			{
				continue;
			}

			const auto row = i / consts::num_cols;
			const auto col = i % consts::num_cols;

			std::vector<sf::Color> new_colors;

			for (auto j{ 0 }; j < 8; ++j)
			{
				const auto new_row = dx[j] + row;
				const auto new_col = dy[j] + col;

				if (new_row < 0 || new_row >= consts::num_rows)
				{
					continue;
				}

				if (new_col < 0 || new_col >= consts::num_cols)
				{
					continue;
				}

				const auto neighbor_idx = new_row * consts::num_cols + new_col;
				const auto neighbor_color = squares[neighbor_idx].getFillColor();
				
				if (same_color(neighbor_color, wall_color) || same_color(neighbor_color, empty_color))
				{
					continue;
				}
				
				new_colors.push_back(neighbor_color);
			}

			if (new_colors.empty())
			{
				continue;
			}

			random_ = std::uniform_int_distribution<std::mt19937::result_type>(0, new_colors.size() - 1);
			const auto idx = random_(rng_);
			const auto new_color = new_colors[idx];
			squares_[i].setFillColor(new_color);

		}
	}

	void restart()
	{
		num_steps_ = 0;
		squares_ = create_squares();
	}

	Squares create_squares()
	{
		const auto size = window_.getSize();

		const auto margin = consts::margin_;

		const auto width = size.x - 2 * margin;
		const auto height = 0.6 * size.y - 2 * margin;
		
		Squares squares;

		for (auto i = 0; i < consts::num_rows; i++)
		{
			for (auto j = 0; j < consts::num_cols; j++)
			{
				auto square = sf::RectangleShape({ consts::cell_size_, consts::cell_size_ });
				square.setPosition({ margin +  j * consts::cell_size_, margin + i * consts::cell_size_ });
				square.setOutlineThickness(consts::cell_size_outline_thickness_);
				square.setOutlineColor(sf::Color::Black);
				square.setFillColor(sf::Color::White);
				squares.push_back(square);
			}
		}

		return squares;
	}

	void help_marker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void set_selected_color(int idx)
	{
		switch (idx)
		{
		case 0:
			selected_color[0] = wall_color[0];
			selected_color[1] = wall_color[1];
			selected_color[2] = wall_color[2];
			break;
		case 1:
			selected_color[0] = empty_color[0];
			selected_color[1] = empty_color[1];
			selected_color[2] = empty_color[2];
			break;
		case 2:
			selected_color[0] = color1[0];
			selected_color[1] = color1[1];
			selected_color[2] = color1[2];
			break;
		case 3:
			selected_color[0] = color2[0];
			selected_color[1] = color2[1];
			selected_color[2] = color2[2];
			break;
		case 4:
			selected_color[0] = color3[0];
			selected_color[1] = color3[1];
			selected_color[2] = color3[2];
			break;
		case 5:
			selected_color[0] = color4[0];
			selected_color[1] = color4[1];
			selected_color[2] = color4[2];
			break;
		case 6:
			selected_color[0] = color5[0];
			selected_color[1] = color5[1];
			selected_color[2] = color5[2];
			break;
		case 7:
			selected_color[0] = color6[0];
			selected_color[1] = color6[1];
			selected_color[2] = color6[2];
			break;
		}
	}

	void random_colors()
	{
		for (auto& s : squares_)
		{
			if (const auto color = s.getFillColor();
				!same_color(color, empty_color))
			{
				continue;
			}

			random_ = std::uniform_int_distribution<std::mt19937::result_type>(0, 5);
			const auto new_color_idx = random_(rng_);
			const auto new_color = get_color(new_color_idx);
			s.setFillColor(new_color);
		}
	}

	sf::Color get_color(int color_idx)
	{
		if (color_idx == 0)
		{
			return sf::Color(color1[0] * 255, color1[1] * 255, color1[2] * 255);
		}
		else if (color_idx == 1)
		{
			return sf::Color(color2[0] * 255, color2[1] * 255, color2[2] * 255);
		}
		else if (color_idx == 2)
		{
			return sf::Color(color3[0] * 255, color3[1] * 255, color3[2] * 255);
		}
		else if (color_idx == 3)
		{
			return sf::Color(color4[0] * 255, color4[1] * 255, color4[2] * 255);
		}
		else if (color_idx == 4)
		{
			return sf::Color(color5[0] * 255, color5[1] * 255, color5[2] * 255);
		}
		else if (color_idx == 5)
		{
			return sf::Color(color6[0] * 255, color6[1] * 255, color6[2] * 255);
		}
		else if (color_idx == 7)
		{
			return sf::Color(wall_color[0] * 255, wall_color[1] * 255, wall_color[2] * 255);
		}
		
		return sf::Color(empty_color[0] * 255, empty_color[1] * 255, empty_color[2] * 255);
	}

	int get_num_color(int color_idx)
	{
		const auto color = get_color(color_idx);
		int count = 0;
		for (const auto& s : squares_)
		{
			count += color == s.getFillColor();
		}
		return count;
	}

	bool same_color(const sf::Color& color, float cell_color[3])
	{
		const auto new_color = sf::Color(cell_color[0] * 255, cell_color[1] * 255, cell_color[2] * 255);
		return new_color.toInteger() == color.toInteger();
	}

private:
	Squares squares_;
	bool left_mouse_pressed_{ false };
	bool started_{ false };

	float wall_color[3] = { 0.24f, 0.22f, 0.22f };
	float empty_color[3] = { 1.0f, 1.0f, 1.0f };
    float color1[3] = { 1.0f, 1.0f, 0.0f }; // Yellow
    float color2[3] = { 0.0f, 1.0f, 0.0f }; // Green
    float color3[3] = { 0.0f, 0.0f, 1.0f }; // Blue
    float color4[3] = { 1.0f, 0.0f, 0.0f }; // Red
    float color5[3] = { 0.0f, 1.0f, 1.0f }; // Cyan
    float color6[3] = { 1.0f, 0.0f, 1.0f }; // Magenta

	float selected_color[3] = {wall_color[0], wall_color[1], wall_color[2]};

	std::mt19937 rng_;
	std::uniform_int_distribution<std::mt19937::result_type> random_;

	uint32_t num_steps_ = 0;
};