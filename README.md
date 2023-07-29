# Perlin-Noiser

An object dedicated to generation of Perlin Noise.

## External Specification

### Constructors

PerlinNoiser(const PerlinNoiser& other);
PerlinNoiser(PerlinNoiser&& other);
PerlinNoiser(const sf::Vector2f& area_size, float chunk_side);
PerlinNoiser(sf::Vector2f&& area_size, float chunk_side);

Copy constructor and move constructor are available. Two others take the size of the area (either by copying from a reference of by moving a temporary rvalue) and the proposed side length of a chunk. The chunk should be more or less square-like, thus the user has to give only one side.

### Getters

sf::Vector2f __getAreaSize__();
sf::Vector2f __getChunkSize__();

__getChunkSize__() returns both dimensions of the chunk! (See _Internal Specification_ for details.)

### Setters

void __setValueRange__(float _lower_limit_, float _upper_limit_);

Sets the range of values obtained in __generateNoise__.
The initial results will be mapped to the interval \[_lower_limit_, _upper_limit_\]. It is certain that there will be at least one final result equal to _lower_limit_ and at least one equal to _upper_limit_.
The default values are _lower_limit_ = 0.0f, _upper_limit_ = 1.0f.

void __setSmoothstep__(std::function<float(float)> _new_smoothstep_);

Sets the smoothstep function.
It should have the following features:
1. it maps the interval \[_lower_limit_, _upper_limit_\] on itself (see: __setValueRange__);
2. _new_smoothstep_(_lower_limit_) = _lower_limit_, _new_smoothstep_(_upper_limit_) = _upper_limit_;
3. the function should be increasing;
4. derivatives: _new_smoothstep_'(_lower_limit_) = _new_smoothstep_'(_upper_limit_) = 0.
The user is responsible for providing appropriate function. Otherwise, no problems should happen but the results will be incorrect.
The default smoothstep function is _y_ = 3*x*^2 - 2*x*^3, _x_ in \[0.0f, 1.0f\].

void __setOctavesCount__(int _new_octaves_count_);

Sets the number of octaves. It must be a positive number.
The default value is 1.

void __setLacunarity__(float _new_lacunarity_);

Sets the lacunarity. (The factor the frequency is multiplied by in generation of successive octaves.)
The _new_lacunarity_ must be greater than 1.0f.
It is equal to 2.0f by default.

void __setPersistence__(float _new_persistence_);

Sets the persistence. (The factor the amplitude is multiplied by in generation of successive octaves.)
The _new_persistence_ must be from set (0.0f, 1.0f).
It is equal to 0.5f by default.

void __generateNoise__(const std::vector\<sf::Vector2f>& _points_, std::vector<float>& _values_);
void __generateNoise__(const Grid\<sf::Vector2f>& _points_, Grid<float>& _values_);

Methods thad generate the Perlin noise for given coordinates.
The first variant works on 1-dimensional vectors, the second one on vectors of vectors. (*Grid*<*T*> is an alias for std::vector\<std::vector\<*T*>>).
In both cases, _points_ store coordinates from the area defined on construction of the object (see: _Constructors_).
The results are stored in _values_. Result at _values_\[_i_] refers to coordinates at _points_\[_i_] in the first variant; analogously, _values_\[_i_]\[_j_] refers to _points_\[_i_]\[_j_] in the second variant.

Argument _values_ should be passed as empty. It will be cleared and recreated to provide identical structure with _points_.

## Internal Specification

The class was designed for a greater project using SFML library. Thus, SFML classes like sf::Vector2i and sf::Vector2f are used.
