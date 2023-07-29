#pragma once
#include <functional>
#include <vector>
#include <SFML/System.hpp>

template <typename T>
using Grid = std::vector<std::vector<T>>;

class LinearTransformation
{
private:

	float m_slope, m_intercept;

public:

	LinearTransformation(float a = 1.0f, float b = 0.0f);
	LinearTransformation(float x_min, float x_max, float y_min, float y_max);

	float operator()(float x);
};

const std::function<float(float)> default_smoothstep
	= [](float t) { return (3.0f - 2.0f * t) * t * t; };

class PerlinNoiser
{
private:

	sf::Vector2f m_area_size;
	sf::Vector2f m_chunk_size;
	Grid<sf::Vector2f> m_gradients;
	float m_lower_limit = 0.0f, m_upper_limit = 1.0f;
	int m_octaves_count = 1;
	float m_persistence = 0.5f, m_lacunarity = 2.0f;
	std::function<float(float)> smoothstep = default_smoothstep;

	inline sf::Vector2i findChunk(const sf::Vector2f& point);
	inline float lerp(float v1, float v2, float w);
	inline void roll_back(sf::Vector2f& point, const sf::Vector2f& dimensions);
	inline float dotProduct(const sf::Vector2f& u, const sf::Vector2f& v);
	inline float calculateNoise(sf::Vector2f coords);

public:

	PerlinNoiser(const PerlinNoiser& other);
	PerlinNoiser(PerlinNoiser&& other) noexcept;
	PerlinNoiser(const sf::Vector2f& area_size, float chunk_side);
	PerlinNoiser(sf::Vector2f&& area_size, float chunk_side);

	inline sf::Vector2f getAreaSize();
	inline sf::Vector2f getChunkSize();

	void setValueRange(float lower_limit, float upper_limit);
	void setSmoothstep(std::function<float(float)> new_smoothstep);

	void setOctavesCount(int new_octaves_count);
	void setLacunarity(float new_lacunarity);
	void setPersistence(float new_persistence);

	void generateNoise(const Grid<sf::Vector2f>& points, Grid<float>& values);
};

