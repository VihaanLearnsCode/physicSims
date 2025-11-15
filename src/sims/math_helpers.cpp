#include <SFML/Graphics.hpp>
#include <cmath>

inline sf::Vector2f operator*(const sf::Vector2f& v, float s) {
    return { v.x * s, v.y * s };
}

inline sf::Vector2f operator*(float s, const sf::Vector2f& v) {
    return { v.x * s, v.y * s };
}

inline sf::Vector2f operator/(const sf::Vector2f& v, float s) {
    return { v.x / s, v.y / s };
}

inline float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    return a.x * b.x + a.y * b.y;
}

inline float length(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
