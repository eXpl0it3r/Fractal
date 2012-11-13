#include "Fractal.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <functional>
#include <iostream>

Fractal::Fractal(const sf::Vector2u& size, unsigned int threads) :
    m_pixels(size.x*size.y*4, 0),
    m_pos(-0.7, 0., 0.003),
    m_pfact(3.f),
    m_precision(500.),
    m_X(-200, 2, 4, 255),
    m_sX(3, -1, -7)
{
    m_texture.create(size.x, size.y);
    m_fractal.setTexture(m_texture, true);

    std::vector<sf::Rect<unsigned int>> rects(threads*threads);

    unsigned int width = size.x/threads;
    unsigned int height = size.y/threads;

    for(unsigned int y=0; y < threads; ++y)
    {
        for(unsigned int x=0; x < threads; ++x)
        {
            rects[(y*threads)+x] = sf::Rect<unsigned int>(x*width, y*height, (x+1)*width, (y+1)*height);
        }
        rects[(y*threads)+(threads-1)] = sf::Rect<unsigned int>((threads-1)*width, y*height, size.x, (y+1)*height);
    }
    rects[((threads-1)*threads)+(threads-1)] = sf::Rect<unsigned int>((threads-1)*width, (threads-1)*height, size.x, size.y);

    for(auto &rect : rects)
    {
        m_threads.push_back(std::unique_ptr<sf::Thread>(new sf::Thread(std::bind(&Fractal::generate, this, rect))));
    }
}

void Fractal::resize(const sf::Vector2u& size)
{
    m_pixels.resize(size.x*size.y*4, 0);
    m_texture.create(size.x, size.y);
    m_fractal.setTextureRect(sf::IntRect(0, 0, m_texture.getSize().x, m_texture.getSize().y));
}

void Fractal::update(const sf::Vector2i& first, const sf::Vector2i& second)
{
    sf::Vector3<long double> posTemp;

    if(std::abs(first.x - second.x) < 5 && std::abs(first.y - second.y) < 5)
    {
        posTemp.x = m_pos.x + (first.x - m_texture.getSize().x/2.) * m_pos.z;
        posTemp.y = m_pos.y + (first.y - m_texture.getSize().y/2.) * m_pos.z;
        posTemp.z = m_pos.z;
    }
    else
    {
        if(second.x < first.x)
            posTemp.x = m_pos.x + (first.x - (first.x-second.x)/2. - m_texture.getSize().x/2.) * m_pos.z; //x-coord
        else
            posTemp.x = m_pos.x + (second.x - (second.x-first.x)/2. - m_texture.getSize().x/2.) * m_pos.z;

        if(second.y < first.y)
            posTemp.y = m_pos.y + (first.y - (first.y-second.y)/2. - m_texture.getSize().y/2.) * m_pos.z; //y-coord
        else
            posTemp.y = m_pos.y + (second.y - (second.y-first.y)/2. - m_texture.getSize().y/2.) * m_pos.z;

        posTemp.z = m_pos.z * (std::abs(first.x - second.x) / static_cast<long double>(m_texture.getSize().x) + std::abs(first.y - second.y) / static_cast<long double>(m_texture.getSize().y)) / 2.; //scale
    }

    m_pos = posTemp;

    for(auto &thread : m_threads)
        thread->launch();

    for(auto &thread : m_threads)
        thread->wait();

    m_texture.update(m_pixels.data());
}

void Fractal::precision(const long double& precision)
{
    if(precision > 0)
        m_precision = precision;
    else
        m_precision = 10;
}

const long double& Fractal::precision()
{
    return m_precision;
}

void Fractal::generate(sf::Rect<unsigned int> section)
{
    int mx = static_cast<int>(m_texture.getSize().x / 2.f);
    int my = static_cast<int>(m_texture.getSize().y / 2.f);

    int iteration = 0;

    long double ax = 0;
    long double ay = 0;
    long double a1 = 0;
    long double a2 = 0;
    long double b1 = 0;

    for(int x = section.left; x < section.width; ++x)
        for(int y = section.top; y < section.height; ++y)
        {
            // mathematical value
            ax = m_pos.x+(x-mx)*m_pos.z;
            ay = m_pos.y+(y-my)*m_pos.z;

            a1 = ax;
            b1 = ay;
            iteration = 0;

            do
            {
                ++iteration;
                a2 = a1*a1 - b1*b1 + ax; // square of a+bi, done component-wise
                b1 = 2*a1*b1 + ay;
                a1 = a2; // b1 = b2;
            } while(!((iteration > m_precision) || ((a1*a1) + (b1*b1) > 4)));
            //1. condition: we have convergence. 2. condition: we have divergence.

            if(iteration > m_precision)
                iteration = 0; //Punkt gehört zur Menge (innere schwarze Fläche)

            //(x, y, iteration, (a1*a1) + (b1*b1)); //draw berechnet die Farbe aufgrund der iteration (und ev. des letzten Reihenelements)

            sf::Color pixel(0, 0, 0, 255);

            if(iteration > 0)
            {
                if(m_sX.r < 0)
                    if(m_X.r < 0)
                        pixel.r = color(iteration, (a1*a1) + (b1*b1), m_X.r, m_sX.r, -1);
                    else
                        pixel.r = color(iteration, (a1*a1) + (b1*b1), m_X.r, m_sX.r, 1);
                else
                    pixel.r = iteration * m_sX.r/m_pfact;

                if(m_sX.g < 0)
                    if(m_X.g < 0)
                        pixel.g = color(iteration, (a1*a1) + (b1*b1), m_X.g, m_sX.g, -1);
                    else
                        pixel.g = color(iteration, (a1*a1) + (b1*b1), m_X.g, m_sX.g, 1);
                else
                    pixel.g = iteration * m_sX.g/m_pfact;

                if(m_sX.b < 0)
                    if(m_X.b < 0)
                        pixel.b = color(iteration, (a1*a1) + (b1*b1), m_X.b, m_sX.b, -1);
                    else
                        pixel.b = color(iteration, (a1*a1) + (b1*b1), m_X.b, m_sX.b, 1);
                else
                    pixel.b = iteration * m_sX.b/m_pfact;

                if(m_X.r > 0)
                    pixel.r = pixel.r % (255-m_X.r);
                else if(m_sX.r != 0)
                    pixel.r = 255 + m_X.r - (pixel.r % (255+m_X.r));

                if(m_X.g > 0)
                    pixel.g = pixel.g % (255-m_X.g);
                else if(m_sX.g != 0)
                    pixel.g = 255 + m_X.g - (pixel.g % (255+m_X.g));

                if(m_X.b > 0)
                    pixel.b = pixel.b % (255-m_X.b);
                else if(m_sX.b != 0)
                    pixel.b = 255 + m_X.b - (pixel.b % (255+m_X.b));
            }

            m_pixels[((y*1024)+x)*4] = pixel.r;
            m_pixels[((y*1024)+x)*4+1] = pixel.g;
            m_pixels[((y*1024)+x)*4+2] = pixel.b;
            m_pixels[((y*1024)+x)*4+3] = pixel.a;
        }
}

sf::Uint8 Fractal::color(const unsigned int c, const long double z, const sf::Uint8 X, const sf::Uint8 sX, const int sign) const
{
    return std::round(static_cast<long double>(c+1)/(std::log(z)/std::log(static_cast<double long>(10))+1) *
                      (255/(std::log(static_cast<double long>(255*+(sign*X)))/std::log(static_cast<double long>(10))))) * -sX/m_pfact;
}

void Fractal::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(m_fractal, states);
}
