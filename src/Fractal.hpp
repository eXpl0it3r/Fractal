#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>

#include <vector>
#include <memory>

namespace sf
{
    class RenderTarget;
}

class Fractal final : public sf::Drawable
{
public:
    explicit Fractal(const sf::Vector2u& size, unsigned int parallelization = 3);

    void update(const sf::Vector2i& first, const sf::Vector2i& second);

    void resize(const sf::Vector2u& size);
    void precision(const long double& precision);
    const long double& precision() const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Uint8 color(unsigned int c, long double z, sf::Uint8 x, sf::Uint8 sX, int sign) const;
    void generate(sf::Rect<unsigned int> section);
    void generateParallel();

    // Drawing
    std::vector<sf::Uint8> m_pixels;
    sf::Texture m_texture;
    sf::Sprite m_fractal;

    // Parameters
    unsigned int m_parallelization;
    sf::Vector3<long double> m_pos;
    float m_pfact;
    long double m_precision;
    sf::Color m_x;
    sf::Color m_sX;
};
