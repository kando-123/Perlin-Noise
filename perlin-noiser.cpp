#include "perlin-noiser.h"
#include "error.h"
#include <cassert>
#include <numbers>
#include <random>

LinearTransformation::LinearTransformation(float a, float b)
{
	m_slope = a;
	m_intercept = b;
}

LinearTransformation::LinearTransformation(float x_min, float x_max, float y_min, float y_max)
{
	if (x_min != x_max)
	{
		m_slope = (y_max - y_min) / (x_max - x_min);
		m_intercept = y_min - m_slope * x_min;
	}
	else
	{
		m_slope = 0.0f;
		m_intercept = (y_max - y_min) / 2.0f;
	}
}

float LinearTransformation::operator()(float x)
{
	return m_slope * x + m_intercept;
}

// @param coords: coordinates of the point
// @return indexes of the chunk the point belongs to
sf::Vector2i PerlinNoiser::findChunk(const sf::Vector2f& point)
{
	return sf::Vector2i(
		static_cast<int>(std::floor(point.x / m_chunk_size.x)),
		static_cast<int>(std::floor(point.y / m_chunk_size.y)));
}

// Linear interpolation.
// @param v1: first node
// @param v2: second node
// @param w: weight
// @return v1 + w * (v2 - v1)
float PerlinNoiser::lerp(float v1, float v2, float w)
{
	return v1 + w * (v2 - v1);
}

// 
inline void PerlinNoiser::roll_back(sf::Vector2f& point, const sf::Vector2f& dimensions)
{
	point.x = std::fmod(point.x, dimensions.x);
	point.y = std::fmod(point.y, dimensions.y);
	if (point.x < 0.0f)
		point.x += dimensions.x;
	if (point.y < 0.0f)
		point.y += dimensions.y;
}

// Dot product.
// @param u: first vector
// @param v: second vector
// @return scalar product of u and v
float PerlinNoiser::dotProduct(const sf::Vector2f& u, const sf::Vector2f& v)
{
	return u.x * v.x + u.y * v.y;
}

// Calculates a single octave of noise for particular point.
float PerlinNoiser::calculateNoise(sf::Vector2f coords)
{
	roll_back(coords, m_area_size);
	sf::Vector2i chunk = findChunk(coords);
	float p1 = static_cast<float>(chunk.x) * m_chunk_size.x, p2 = p1 + m_chunk_size.x;
	float q1 = static_cast<float>(chunk.y) * m_chunk_size.y, q2 = q1 + m_chunk_size.y;
	float p = coords.x, q = coords.y;
	sf::Vector2f offset1((p - p1) / m_chunk_size.x, (q - q1) / m_chunk_size.y);
	sf::Vector2f offset2((p - p2) / m_chunk_size.x, (q - q1) / m_chunk_size.y);
	sf::Vector2f offset3((p - p1) / m_chunk_size.x, (q - q2) / m_chunk_size.y);
	sf::Vector2f offset4((p - p2) / m_chunk_size.x, (q - q2) / m_chunk_size.y);
	float product1 = dotProduct(offset1, m_gradients[chunk.x][chunk.y]);
	float product2 = dotProduct(offset2, m_gradients[chunk.x + 1][chunk.y]);
	float product3 = dotProduct(offset3, m_gradients[chunk.x][chunk.y + 1]);
	float product4 = dotProduct(offset4, m_gradients[chunk.x + 1][chunk.y + 1]);
	float horizontal1 = lerp(product1, product2, offset1.x);
	float horizontal2 = lerp(product3, product4, offset1.x);
	return lerp(horizontal1, horizontal2, offset1.y);
}

// Copy constructor.
PerlinNoiser::PerlinNoiser(const PerlinNoiser& other)
{
	m_area_size = other.m_area_size;
	m_chunk_size = other.m_chunk_size;
	m_gradients = other.m_gradients;
}

// Move constructor.
PerlinNoiser::PerlinNoiser(PerlinNoiser&& other) noexcept
{
	m_area_size = std::move(other.m_area_size);
	m_chunk_size = std::move(other.m_chunk_size);
	m_gradients = std::move(other.m_gradients);
}

// Constructor.
// The proposed side of a chunk is adjusted (in both dimensions)
// to provide whole numbers of chunks. Creates the grid of gradients.
// @param area_size: real size of the area
//    (width and height, not numbers of pixels)
// @param chunk_side: proposed side of the chunk
PerlinNoiser::PerlinNoiser(const sf::Vector2f& area_size, float chunk_side)
{
	assert(area_size.x > 0 && area_size.y > 0 && chunk_side > 0.0f);

	m_area_size = area_size;

	sf::Vector2f chunks_count;
	chunks_count.x = std::max(1.0f, std::round(area_size.x / chunk_side));
	chunks_count.y = std::max(1.0f, std::round(area_size.y / chunk_side));
	m_chunk_size.x = area_size.x / chunks_count.x;
	m_chunk_size.y = area_size.y / chunks_count.y;
	
	sf::Vector2i gradients_count;
	gradients_count.x = static_cast<int>(chunks_count.x) + 1;
	gradients_count.y = static_cast<int>(chunks_count.y) + 1;

	std::random_device device;
	std::mt19937 twister(device());
	std::uniform_real_distribution<float>
		angle(-std::numbers::pi_v<float>, +std::numbers::pi_v<float>);

	m_gradients.reserve(gradients_count.x);
	for (int i = 0; i < gradients_count.x; ++i)
	{
		m_gradients.emplace_back(gradients_count.y);
		for (int j = 0; j < gradients_count.y; ++j)
		{
			float theta = angle(twister);
			m_gradients[i][j] =
				sf::Vector2f(std::cos(theta), std::sin(theta));
		}
	}
}

// Constructor.
// The proposed side of a chunk is adjusted (in both dimensions)
// to provide whole numbers of chunks. Creates the grid of gradients.
// @param area_size: real size of the area
//    (width and height, not numbers of pixels)
// @param chunk_side: proposed side of the chunk
PerlinNoiser::PerlinNoiser(sf::Vector2f&& area_size, float chunk_side)
{
	assert(area_size.x > 0 && area_size.y > 0 && chunk_side > 0.0f);

	m_area_size = std::move(area_size);

	sf::Vector2f chunks_count;
	chunks_count.x = std::max(1.0f, std::round(area_size.x / chunk_side));
	chunks_count.y = std::max(1.0f, std::round(area_size.y / chunk_side));
	m_chunk_size.x = area_size.x / chunks_count.x;
	m_chunk_size.y = area_size.y / chunks_count.y;

	sf::Vector2i gradients_count;
	gradients_count.x = static_cast<int>(chunks_count.x) + 1;
	gradients_count.y = static_cast<int>(chunks_count.y) + 1;

	std::random_device device;
	std::mt19937 twister(device());
	std::uniform_real_distribution<float>
		angle(-std::numbers::pi_v<float>, +std::numbers::pi_v<float>);

	m_gradients.reserve(gradients_count.x);
	for (int i = 0; i < gradients_count.x; ++i)
	{
		m_gradients.emplace_back(gradients_count.y);
		for (int j = 0; j < gradients_count.y; ++j)
		{
			float theta = angle(twister);
			m_gradients[i][j] =
				sf::Vector2f(std::cos(theta), std::sin(theta));
		}
	}
}

sf::Vector2f PerlinNoiser::getAreaSize()
{
	return m_area_size;
}

sf::Vector2f PerlinNoiser::getChunkSize()
{
	return m_chunk_size;
}

void PerlinNoiser::setValueRange(float lower_limit, float upper_limit)
{
	if (lower_limit < upper_limit)
	{
		m_lower_limit = lower_limit;
		m_upper_limit = upper_limit;
	}
}

// Sets the smoothstep function.
// It should have the following features:
// (a) it maps the interval [lower_limit, upper_limit] on itself (see: setValueRange);
// (b) smoothstep(lower_limit) = lower_limit, smoothstep(upper_limit) = upper_limit;
// (c) the function should be increasing;
// (d) derivatives: smoothstep'(lower_limit) = smoothstep'(upper_limit) = 0.
// The user is responsible for providing appropriate function!
// The default smoothstep function is y = 3x^2 - 2x^3, x in [0, 1].
void PerlinNoiser::setSmoothstep(std::function<float(float)> new_smoothstep)
{
	smoothstep = new_smoothstep;
}

// Sets the number of octaves. Must be positive.
void PerlinNoiser::setOctavesCount(int new_octaves_count)
{
	if (new_octaves_count > 0)
		m_octaves_count = new_octaves_count;
}

// Sets the lacunarity. (The factor the frequency is multiplied by for successive octaves.)
// Must be greater than 1.0; equal to 2.0 by default.
void PerlinNoiser::setLacunarity(float new_lacunarity)
{
	if (new_lacunarity > 1.0f)
		m_lacunarity = new_lacunarity;
}

// Sets the persistence. The factor the amplitude is multiplied by for successive octaves.)
// Must be from set (0., 1.); equal to 0.5 by default.
void PerlinNoiser::setPersistence(float new_persistence)
{
	if (new_persistence > 0.0f && new_persistence < 1.0f)
		m_persistence = new_persistence;
}

// Generates Perlin noise for given coordinates.
// @param points: grid of coordinates to generate noise for
// @param values: grid for values; it should be empty (it will be cleared and recreated anyway);
// the value at values[i][j] refers to coordinates at points[i][j]
void PerlinNoiser::generateNoise(const Grid<sf::Vector2f>& points, Grid<float>& values)
{
	float minimum = std::numeric_limits<float>::infinity(), maximum = -minimum;
	values.clear();
	values.reserve(points.size());
	for (int i = 0; i < points.size(); ++i)
	{
		values.emplace_back(points[i].size());
		for (int j = 0; j < points[i].size(); ++j)
		{
			values[i][j] = calculateNoise(points[i][j]);
			float frequency = m_lacunarity;
			float amplitude = m_persistence;
			for (int k = 1; k < m_octaves_count; ++k)
			{
				values[i][j] += amplitude * calculateNoise(frequency * points[i][j]);
				frequency *= m_lacunarity;
				amplitude *= m_persistence;
			}
			if (values[i][j] > maximum)
				maximum = values[i][j];
			if (values[i][j] < minimum)
				minimum = values[i][j];
		}
	}
	LinearTransformation scaler(minimum, maximum, m_lower_limit, m_upper_limit);
	for (int i = 0; i < points.size(); ++i)
		for (int j = 0; j < points[i].size(); ++j)
			values[i][j] = smoothstep(scaler(values[i][j]));
}

