#include "perlin-noiser.h"
#include "error.h"
#include <cassert>
#include <SFML/Graphics.hpp>

#include <iostream> // debugging

inline void paintValues(Grid<sf::CircleShape>& grid, const Grid<float>& pattern)
{
	assert(grid.size() == pattern.size());
	for (int i = 0; i < grid.size(); ++i)
		assert(grid[i].size() == pattern[i].size());

	for (int i = 0; i < grid.size(); ++i)
	{
		for (int j = 0; j < grid[i].size(); ++j)
		{
			int value = static_cast<int>((1.0f - std::abs(pattern[i][j])) * 255.0f);
			if (pattern[i][j] > 0.0f)
				grid[i][j].setFillColor(sf::Color(0xFF, value, value));
			else
				grid[i][j].setFillColor(sf::Color(value, value, 0xFF));
		}
	}
}

inline void paintTerrain(Grid<sf::CircleShape>& grid, const Grid<float>& pattern, float limit)
{
	assert(grid.size() == pattern.size());
	for (int i = 0; i < grid.size(); ++i)
		assert(grid[i].size() == pattern[i].size());

	for (int i = 0; i < grid.size(); ++i)
	{
		for (int j = 0; j < grid[i].size(); ++j)
		{
			if (pattern[i][j] < limit)
				grid[i][j].setFillColor(sf::Color(0x00, 0x20, 0xB0));
			else
				grid[i][j].setFillColor(sf::Color(0x00, 0xA0, 0x00));
		}
	}
}

inline float hex_height(float side)
{
	return std::sqrt(3.f) * side;
}

inline float hex_width(float side)
{
	return 2.f * side;
}

inline int parity(int x)
{
	return x & 1; // 0 -> even, 1 -> odd
}


int main()
{
	// exemplary hexagon
	const float radius = 10.f;
	sf::CircleShape hexagon(radius, 6);
	hexagon.setOrigin(radius, radius);
	hexagon.setRotation(30.f);
	hexagon.setFillColor(sf::Color::White);
	hexagon.setOutlineColor(sf::Color::Black);
	hexagon.setOutlineThickness(-1.f);

	// creating the grid
	Grid<sf::CircleShape> grid;
	sf::Vector2i hexes_count(72, 44);
	grid.reserve(hexes_count.x);

	const float height = hex_height(radius);
	const float inner_radius = 0.5f * height;
	const float horizontal_distance = 1.5f * radius;
	const sf::Vector2f origin(radius, radius);
	
	sf::Vector2f displacement = origin;

	sf::Vector2f total_size;
	total_size.x = static_cast<float>(hexes_count.x / 2) * 3.f * radius
		+ (parity(hexes_count.x) == 0 ? .5f : 2.f) * radius;
	total_size.y = hexes_count.y * hex_height(radius)
		+ (hexes_count.x > 1 ? .5f * hex_height(radius) : 0.f);

	float chunk_side = 8.0f * radius; // proposition
	
	Grid<sf::Vector2f> positions;
	positions.reserve(hexes_count.x);
	sf::Vector2f center(radius, inner_radius);

	for (int i = 0; i < hexes_count.x; ++i)
	{
		if (parity(i) == 1)
		{
			displacement.y = origin.y + inner_radius;
			center.y = height;
		}
		else
		{
			displacement.y = origin.y;
			center.y = inner_radius;
		}
		grid.emplace_back(hexes_count.y, hexagon);
		positions.emplace_back(hexes_count.y);
		for (int j = 0; j < hexes_count.y; ++j)
		{
			grid[i][j].setPosition(displacement);
			displacement.y += height;
			
			positions[i][j] = center;
			center.y += height;
		}
		displacement.x += horizontal_distance;
		center.x += horizontal_distance;
	}

	PerlinNoiser noiser(total_size, chunk_side);
	noiser.setValueRange(-1.0f, +1.0f);
	noiser.setSmoothstep(
		[](float t)
		{
			t = 0.5f * t + 0.5f;
			return 2.0f * t * t * t * (t * (t * 6.f - 15.f) + 10.f) - 1.0f;
		});
	
	noiser.setOctavesCount(3);

	Grid<float> results;

	noiser.generateNoise(positions, results);
	positions.clear();
	paintValues(grid, results);

	sf::RenderWindow window(sf::VideoMode(1300, 800), "", sf::Style::Close);
	window.setFramerateLimit(60);

	float limit = 0.0f;
	sf::Event e;
	while (window.isOpen())
	{
		while (window.pollEvent(e))
		{
			switch (e.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				switch (e.key.code)
				{
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::I:
					paintValues(grid, results);
					break;
				case sf::Keyboard::J:
					paintTerrain(grid, results, limit);
					break;
				case sf::Keyboard::O:
					limit = 0.0f;
					paintTerrain(grid, results, limit);
					break;
				case sf::Keyboard::P:
					limit = std::min(+1.1f, limit + 0.05f);
					paintTerrain(grid, results, limit);
					break;
				case sf::Keyboard::L:
					limit = std::max(-1.0f, limit - 0.05f);
					paintTerrain(grid, results, limit);
					break;
				}
			default:
				break;
			}
		}
		window.clear(sf::Color(0x40, 0x40, 0x40));
		for (int i = 0; i < grid.size(); ++i)
			for (int j = 0; j < grid[i].size(); ++j)
				window.draw(grid[i][j]);

		window.display();
	}
	return 0;
}