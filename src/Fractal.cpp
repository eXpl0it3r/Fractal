#include "Fractal.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <functional>
#include <iostream>

Fractal::Fractal(const sf::Vector2u& size, const unsigned int threads) :
    m_pos(-0.7, 0., 0.003),
    m_min(-2.0, 1.0),
    m_max(-1.2, (1.8 * size.y) / static_cast<long double>(size.x)),
    m_factor(3.0 / (size.x - 1), (m_max.y - m_min.y) / (size.y - 1)),
    m_pfact(3.f),
    m_precision(500.)
{
    resize(size, threads);
}

void Fractal::setThreads(const unsigned int threads)
{
    // Stop and clear threads
    for (auto& thread : m_threads)
    {
        thread->wait();
    }
    m_threads.clear();

    std::vector<sf::Rect<unsigned int>> rects(threads * threads);

    const auto width = m_texture.getSize().x / threads;
    const auto height = m_texture.getSize().y / threads;

    for (unsigned int y = 0; y < threads; ++y)
    {
        for (unsigned int x = 0; x < threads; ++x)
        {
            rects[(y * threads) + x] = sf::Rect<unsigned int>{x * width, y * height, (x + 1) * width, (y + 1) * height};
        }

        rects[(y * threads) + (threads - 1)] = sf::Rect<unsigned int>{
            (threads - 1) * width, y * height, m_texture.getSize().x, (y + 1) * height
        };
    }

    rects[((threads - 1) * threads) + (threads - 1)] = sf::Rect<unsigned int>{
        (threads - 1) * width, (threads - 1) * height, m_texture.getSize().x, m_texture.getSize().y
    };

    for (auto& rect : rects)
    {
        m_threads.push_back(std::make_unique<sf::Thread>([this, rect] { generate(rect); }));
    }
}

void Fractal::resize(const sf::Vector2u& size, const unsigned int threads)
{
    // Reset values
    m_pos = sf::Vector3<long double>(-0.7, 0., 0.003);
    m_pfact = 3.f;
    m_precision = 500.;

    //m_max.y = m_min.y+(m_max.x-m_min.x)*size.y/static_cast<long double>(size.x);
    //m_factor = sf::Vector2<long double>((m_max.x-m_min.x)/static_cast<long double>(size.x-1), (m_max.y-m_min.y)/static_cast<long double>(size.y-1));

    m_pixels.resize(size.x * size.y * 4, 0);
    m_texture.create(size.x, size.y);
    m_fractal.setTexture(m_texture, true);
    setThreads(threads);
}

void Fractal::update(const sf::Vector2i& first, const sf::Vector2i& second)
{
    sf::Vector2i firstTemp;
    sf::Vector2i secondTemp;

    // Not an actual selection but just a click
    if (std::abs(first.x - second.x) <= 5 && std::abs(first.y - second.y) <= 5)
    {
    }
        // New area selected
    else
    {
        // Transform to a top-left & bottom-right rectangle
        // 1st quadrant
        if (first.x < second.x && first.y > second.y)
        {
            firstTemp = sf::Vector2i(first.x, second.y);
            secondTemp = sf::Vector2i(second.x, first.y);
        }
            // 2nd quadrant
        else if (first.x > second.x && first.y > second.y)
        {
            firstTemp = second;
            secondTemp = first;
        }
            // 3rd quadrant
        else if (first.x > second.x && first.y < second.y)
        {
            firstTemp = sf::Vector2i(second.x, first.y);
            secondTemp = sf::Vector2i(first.x, second.y);
        }
            // 4th quadrant
        else
        {
            firstTemp = first;
            secondTemp = second;
        }

        // Reset values
        m_min = sf::Vector2<long double>(firstTemp.x*m_factor.x, firstTemp.y*m_factor.y);
        m_max = sf::Vector2<long double>(secondTemp.x*m_factor.x, secondTemp.y*m_factor.y);
        m_factor = sf::Vector2<long double>((m_max.x-m_min.x)/static_cast<long double>(m_texture.getSize().x-1), (m_max.y-m_min.y)/static_cast<long double>(m_texture.getSize().y-1));
    }

    // Recalculate image
    for (auto& thread : m_threads)
    {
        thread->launch();
    }

    for (auto& thread : m_threads)
    {
        thread->wait();
    }

    m_texture.update(m_pixels.data());
}

void Fractal::precision(const long double precision)
{
    if (precision > 0)
    {
        m_precision = precision;
    }
    else
    {
        m_precision = 10;
    }
}

long double Fractal::precision() const
{
    return m_precision;
}

long double Fractal::zoom() const
{
    return m_pos.z;
}

long double Fractal::divisor() const
{
    return m_pfact;
}

sf::Vector2<long double> Fractal::position() const
{
    return {m_pos.x, m_pos.y};
}

void Fractal::generate(sf::Rect<unsigned int> section)
{
    m_min.x = -2.0;
    m_min.y = 1.0;
    m_max.x = -1.2;
    m_max.y = m_min.y + (m_max.x - m_min.x) * 768.0 / 1000.0;
    m_factor.x = (m_max.x - m_min.x) / (1000.0 - 1);
    m_factor.y = (m_max.y - m_min.y) / (768.0 - 1);

    auto c = sf::Vector2<long double>{0., 0.};

    for (auto y = section.top; y < section.height; ++y)
    {
        c.y = m_max.y - y * m_factor.y;
        for (auto x = section.left; x < section.width; ++x)
        {
            c.x = m_min.x + x * m_factor.x;

            auto Z = sf::Vector2<long double>{c};
            auto isInside = true;
            auto N = 0u;
            for (auto n = 0u; n < m_precision; ++n)
            {
                N = n;
                auto Z2 = sf::Vector2<long double>{Z.x * Z.x, Z.y * Z.y};
                if (Z2.x + Z2.y > 4.0)
                {
                    isInside = false;
                    break;
                }
                Z.y = 2 * Z.x * Z.y + c.y;
                Z.x = Z2.x - Z2.y + c.x;
            }

            auto pixel = sf::Color{};

            if (isInside)
            {
                pixel = sf::Color::Red;
            }
            else
            {
                const double color = 3 * std::log(N) / std::log(m_precision - 1.0);

                if (color < 1)
                {
                    pixel = sf::Color{static_cast<sf::Uint8>(255 * color), 0, 0};
                }
                else if (color < 2)
                {
                    pixel = sf::Color{255, static_cast<sf::Uint8>(255 * (color - 1)), 0};
                }
                else
                {
                    pixel = sf::Color{255, 255, static_cast<sf::Uint8>(255 * (color - 2))};
                }
            }

            m_pixels[((y * m_texture.getSize().x) + x) * 4] = pixel.r;
            m_pixels[((y * m_texture.getSize().x) + x) * 4 + 1] = pixel.g;
            m_pixels[((y * m_texture.getSize().x) + x) * 4 + 2] = pixel.b;
            m_pixels[((y * m_texture.getSize().x) + x) * 4 + 3] = pixel.a;
        }
    }
}

void Fractal::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_fractal, states);
}
