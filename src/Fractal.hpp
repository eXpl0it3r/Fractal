#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Thread.hpp>

#include <vector>
#include <memory>

namespace sf
{
    class RenderTarget;
}

class Fractal : public sf::Drawable
{
public:
    Fractal(const sf::Vector2u& size, const unsigned int threads = 3);

    void update(const sf::Vector2i& first, const sf::Vector2i& second);

    void resize(const sf::Vector2u& size, const unsigned int threads = 3);

    // Get/set state information
    void precision(const long double precision);
    const long double precision();

    const long double zoom();
    const long double devisor();
    const sf::Vector2<double long> position();

private:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const;

    void setThreads(const unsigned int threads);
    void generate(sf::Rect<unsigned int> section);

private:
    std::vector<std::unique_ptr<sf::Thread>> m_threads;

    // Drawing
    std::vector<sf::Uint8> m_pixels;
    sf::Texture m_texture;
    sf::Sprite m_fractal;

    // Parameters
    sf::Vector3<long double> m_pos;
    sf::Vector2<long double> m_min;
    sf::Vector2<long double> m_max;
    sf::Vector2<long double> m_factor;
    float m_pfact;
    long double m_precision;
};
