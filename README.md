# Perlin-Noiser

An object dedicated to generation of Perlin Noise.

## External Specification

### Constructors

```
PerlinNoiser(const PerlinNoiser& other);
PerlinNoiser(PerlinNoiser&& other);
PerlinNoiser(const sf::Vector2f& area_size, float chunk_side);
PerlinNoiser(sf::Vector2f&& area_size, float chunk_side);
```

Copy constructor and move constructor are available. Two others take the size of the area (either by copying from a reference of by moving a temporary rvalue) and the proposed side length of a chunk. The chunk should be more or less square-like, thus the user has to give only one side.

### Getters

```
sf::Vector2f getAreaSize();
sf::Vector2f getChunkSize();
```

`getChunkSize()` returns both dimensions of the chunk! (See _Internal Specification_ for details.)

### Setters

```
void setValueRange(float lower_limit, float upper_limit);
```

Sets the range of values obtained in `generateNoise`.
The initial results will be mapped to the interval \[`lower_limit`, `upper_limit`\]. It is certain that there will be at least one final result equal to `lower_limit` and at least one equal to `upper_limit`.
The default values are `lower_limit` = 0.0f, `upper_limit` = 1.0f.

```
void setSmoothstep(std::function<float(float)> new_smoothstep);
```

Sets the smoothstep function.
It should have the following features:
1. it maps the interval \[`lower_limit`, `upper_limit`\] on itself;
2. `new_smoothstep`(`lower_limit`) = `lower_limit`, `new_smoothstep`(`upper_limit`) = `upper_limit`;
3. the function should be increasing;
4. derivatives: `new_smoothstep`'(`lower_limit`) = `new_smoothstep`'(`upper_limit`) = 0.
The user is responsible for providing appropriate function. Otherwise, no problems should happen but the results will be incorrect.
The default smoothstep function is _y_ = 3*x*² - 2*x*³, _x_ in \[0.0f, 1.0f\].

```
void setOctavesCount(int new_octaves_count);
```

Sets the number of octaves. It must be a positive number.
The default value is 1.

```
void setLacunarity(float new_lacunarity);
```

Sets the lacunarity. (The factor the frequency is multiplied by in generation of successive octaves.)
The new lacunarity must be greater than 1.0f.
It is equal to 2.0f by default.

```
void setPersistence(float new_persistence);
```

Sets the persistence. (The factor the amplitude is multiplied by in generation of successive octaves.)
The new persistence must be from set (0.0f, 1.0f).
It is equal to 0.5f by default.

```
void generateNoise(const std::vector\<sf::Vector2f>& points, std::vector<float>& values);
void generateNoise(const Grid\<sf::Vector2f>& points, Grid<float>& values);
```

Methods that generate the Perlin noise for given coordinates.
The first variant works on 1-dimensional vectors, the second one on vectors of vectors. (`Grid<T>` is an alias for `std::vector<std::vector\<*T*>>`).
In both cases, `points` store coordinates from the area defined on construction of the object (see: _Constructors_).
The results are stored in `values`. The result at `values[_i_]` refers to coordinates at `points[i]` in the first variant; analogously, `values[i][j]` refers to `points[i][j]` in the second variant.

Argument `values` should be passed as empty. It will be cleared and recreated to provide identical structure with `points`.

## Internal Specification

The class was designed for a greater project using SFML library. Thus, SFML classes like `sf::Vector2i` and `sf::Vector2f` are used. It should, however, be really easy to transform the code not to use SFML elements.

### Properties

The class has following properties:

- `sf::Vector2f m_area_size`
- `sf::Vector2f m_chunk_size`
- `Grid\<sf::Vector2f> m_gradients`
- `float m_lower_limit, m_upper_limit`
- `int m_octaves_count`
- `float m_persistence, m_lacunarity`
- `std::function<float(float)> smoothstep`

### Construction

To be created, a new object needs the real dimensions of the area to generate the noise for and the proposed side of the chunk. (Should there be any confusion, "real dimensions" are the lengths of the sides of the rectangular area, not e.g. counts of pixels. But counts of pixels times sizes of a single pixel are okay.) The size of a chunk will be adjusted to provide whole number of chunks in both dimensions, thus the actual size of a chunk can be slightly different from `chunk_size` times `chunk_side`. There is always at least one chunk.

On construction, the grid of gradient vectors is generated.

### Auxiliary private methods used in generation

```
sf::Vector2i findChunk(const sf::Vector2f& point);
```

Determines the indexes of the chunk within which `point` coordinates fall. The indexes are the coordinates devided by chunk sizes, rounded towards minus infinity (`std::floor`).

Since it is ***not*** checked if `point` belongs to the area at all, the indexes might indicate a non-existing chunk. The method must be provided with legal input to produce a legal output.

```
float lerp(float v1, float v2, float w);
```

Linear interpolation between values `v1` and `v2`, with weight `w`. The returned value is `v1` + `w` * (`v2` - `v1`).

```
void roll_back(sf::Vector2f& point, const sf::Vector2f& dimensions);
```

Modifies the coordinates in `point` by replacing them with remainders from division by the dimensions of the area. Function `std::fmod` is used.

