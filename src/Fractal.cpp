#include "Fractal.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <functional>
#include <future>

Fractal::Fractal(const sf::Vector2u& size, const unsigned int parallelization) :
	m_parallelization{ parallelization },
	m_position{ -0.7, 0., 0.003 },
	m_pFactor{ 3.f },
	m_precision{ 5000. },
	m_x{ static_cast<sf::Uint8>(-200), 2, 4, 255 },
	m_sX{ 3, static_cast<sf::Uint8>(-1), static_cast<sf::Uint8>(-7) }
{
	resize(size);
	generateParallel();
}

void Fractal::resize(const sf::Vector2u& size)
{
	// Reset values
	m_position = { -0.7, 0., 0.003 };
	m_pFactor = 3.f;
	m_precision = 500.;
	m_x = sf::Color(-200, 2, 4, 255);
	m_sX = sf::Color(3, -1, -7);

	m_pixels.resize(size.x * size.y * 4, 0);
	m_texture.create(size.x, size.y);
	m_fractal.setTexture(m_texture, true);
}

void Fractal::reset(const sf::Vector2u& size, const unsigned int parallelization)
{
	m_parallelization = parallelization;
	m_position = { -0.7, 0., 0.003 };
	m_pFactor = { 3.f };
	m_precision = { 5000. };
	m_x = sf::Color{ static_cast<sf::Uint8>(-200), 2, 4, 255 };
	m_sX = sf::Color{ 3, static_cast<sf::Uint8>(-1), static_cast<sf::Uint8>(-7) };
	resize(size);
	generateParallel();
	update({ 0, 0 }, sf::Vector2i{ size });
}

void Fractal::update(const sf::Vector2i& first, const sf::Vector2i& second)
{
	auto temporaryPosition = sf::Vector3<long double>{};

	if (std::abs(first.x - second.x) < 5 && std::abs(first.y - second.y) < 5)
	{
		temporaryPosition.x = m_position.x + (first.x - m_texture.getSize().x / 2.) * m_position.z;
		temporaryPosition.y = m_position.y + (first.y - m_texture.getSize().y / 2.) * m_position.z;
		temporaryPosition.z = m_position.z;
	}
	else
	{
		if (second.x < first.x)
		{
			temporaryPosition.x = m_position.x + (first.x - (first.x - second.x) / 2. - m_texture.getSize().x / 2.) * m_position.z; // x coordinate
		}
		else
		{
			temporaryPosition.x = m_position.x + (second.x - (second.x - first.x) / 2. - m_texture.getSize().x / 2.) * m_position.z;
		}

		if (second.y < first.y)
		{
			temporaryPosition.y = m_position.y + (first.y - (first.y - second.y) / 2. - m_texture.getSize().y / 2.) * m_position.z; // y coordinate
		}
		else
		{
			temporaryPosition.y = m_position.y + (second.y - (second.y - first.y) / 2. - m_texture.getSize().y / 2.) * m_position.z;
		}

		temporaryPosition.z = m_position.z * (std::abs(first.x - second.x) / static_cast<long double>(m_texture.getSize().x) + std::abs(first.y - second.y) / static_cast<long double>(m_texture.getSize().y)) / 2.; // scale
	}

	m_position = temporaryPosition;

	generateParallel();

	m_texture.update(m_pixels.data());
}

void Fractal::precision(const long double& precision)
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

const long double& Fractal::precision() const
{
	return m_precision;
}

sf::Uint8 Fractal::color(const unsigned int c, const long double z, const sf::Uint8 x, const sf::Uint8 sX, const int sign) const
{
	return std::round(static_cast<long double>(c + 1) / (std::log(z) / std::log(static_cast<double long>(10)) + 1) *
		(255 / (std::log(static_cast<double long>(255 * +(sign * x))) / std::log(static_cast<double long>(10))))) * -sX / m_pFactor;
}

void Fractal::generate(const sf::Rect<unsigned int> section)
{
	const auto mx = static_cast<int>(m_texture.getSize().x / 2.f);
	const auto my = static_cast<int>(m_texture.getSize().y / 2.f);

	auto iteration = 0;

	long double ax = 0;
	long double ay = 0;
	long double a1 = 0;
	long double a2 = 0;
	long double b1 = 0;

	for (int x = section.left; x < section.width; ++x)
	{
		for (int y = section.top; y < section.height; ++y)
		{
			// Mathematical values
			ax = m_position.x + (x - mx) * m_position.z;
			ay = m_position.y + (y - my) * m_position.z;

			a1 = ax;
			b1 = ay;
			iteration = 0;

			do
			{
				++iteration;
				a2 = a1 * a1 - b1 * b1 + ax; // square of a+bi, done component-wise
				b1 = 2 * a1 * b1 + ay;
				a1 = a2; // b1 = b2;
			}
			while (!((iteration > m_precision) || ((a1 * a1) + (b1 * b1) > 4)));
			// 1. condition: we have convergence. 2. condition: we have divergence.

			if (iteration > m_precision)
			{
				iteration = 0; // Point belongs to the set (inner black area)
			}

			//(x, y, iteration, (a1*a1) + (b1*b1)); // calculates the color regarding the iteration (and maybe the last element of the series)
			auto pixel = sf::Color{ 0, 0, 0, 255 };

			if (iteration > 0)
			{
				if (m_sX.r < 0)
				{
					if (m_x.r < 0)
					{
						pixel.r = color(iteration, (a1 * a1) + (b1 * b1), m_x.r, m_sX.r, -1);
					}
					else
					{
						pixel.r = color(iteration, (a1 * a1) + (b1 * b1), m_x.r, m_sX.r, 1);
					}
				}
				pixel.r = iteration * m_sX.r / m_pFactor;

				if (m_sX.g < 0)
				{
					if (m_x.g < 0)
					{
						pixel.g = color(iteration, (a1 * a1) + (b1 * b1), m_x.g, m_sX.g, -1);
					}
					else
					{
						pixel.g = color(iteration, (a1 * a1) + (b1 * b1), m_x.g, m_sX.g, 1);
					}
				}
				pixel.g = iteration * m_sX.g / m_pFactor;

				if (m_sX.b < 0)
				{
					if (m_x.b < 0)
					{
						pixel.b = color(iteration, (a1 * a1) + (b1 * b1), m_x.b, m_sX.b, -1);
					}
					else
					{
						pixel.b = color(iteration, (a1 * a1) + (b1 * b1), m_x.b, m_sX.b, 1);
					}
				}
				pixel.b = iteration * m_sX.b / m_pFactor;

				if (m_x.r > 0)
				{
					pixel.r = pixel.r % (255 - m_x.r);
				}
				else if (m_sX.r != 0)
				{
					pixel.r = 255 + m_x.r - (pixel.r % (255 + m_x.r));
				}

				if (m_x.g > 0)
				{
					pixel.g = pixel.g % (255 - m_x.g);
				}
				else if (m_sX.g != 0)
				{
					pixel.g = 255 + m_x.g - (pixel.g % (255 + m_x.g));
				}

				if (m_x.b > 0)
				{
					pixel.b = pixel.b % (255 - m_x.b);
				}
				else if (m_sX.b != 0)
				{
					pixel.b = 255 + m_x.b - (pixel.b % (255 + m_x.b));
				}
			}

			m_pixels[((y * m_texture.getSize().x) + x) * 4] = pixel.r;
			m_pixels[((y * m_texture.getSize().x) + x) * 4 + 1] = pixel.g;
			m_pixels[((y * m_texture.getSize().x) + x) * 4 + 2] = pixel.b;
			m_pixels[((y * m_texture.getSize().x) + x) * 4 + 3] = pixel.a;
		}
	}
}

void Fractal::generateParallel()
{
	const auto split = m_parallelization % 2 == 0 ? m_parallelization : std::max(1U, m_parallelization - 1U);

	auto rects = std::vector<sf::Rect<unsigned int>>{ static_cast<std::size_t>(split * split) };

	const auto width = m_texture.getSize().x / split;
	const auto height = m_texture.getSize().y / split;

	for (auto y = 0U; y < split; ++y)
	{
		for (auto x = 0U; x < split; ++x)
		{
			auto currentWidth = (x + 1) * width;
			auto currentHeight = (y + 1) * height;

			if (x + 1 == split && width * split != m_texture.getSize().x)
			{
				currentWidth = m_texture.getSize().x;
			}

			if (y + 1 == split && width * split != m_texture.getSize().x)
			{
				currentHeight = m_texture.getSize().y;
			}

			rects[(y * split) + x] = sf::Rect{ x * width, y * height, currentWidth, currentHeight };
		}
	}

	std::vector<std::future<void>> workers;
	workers.reserve(split);

	for (auto& rect : rects)
	{
		workers.emplace_back(std::async(std::launch::async, &Fractal::generate, this, rect));
	}

	for (const auto& worker : workers)
	{
		worker.wait();
	}
}

void Fractal::draw(sf::RenderTarget& target, const sf::RenderStates states) const
{
	target.draw(m_fractal, states);
}
