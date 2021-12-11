#include "Application.hpp"

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <format>

Application::Application() :
	m_window{ { 1000, 765 }, "Fractal - Mandelbrot", sf::Style::Default, sf::ContextSettings{ 0U, 0U, 8U } },
	m_fractal{ { 1000, 765 } },
	m_down{ -1, -1 },
	m_resized{ true }
{
	m_window.setFramerateLimit(60);

	// Init select frame
	m_select.setFillColor({ 0, 0, 0, 0 });
	m_select.setOutlineColor({ 255, 255, 255, 255 });
	m_select.setOutlineThickness(-2.f);

	if (!m_font.loadFromFile("DejaVuSans.ttf"))
	{
		throw std::runtime_error{ "Unable to open font" };
	}
	m_precision.setFont(m_font);
	m_precision.setPosition(10.f, 10.f);
	m_precision.setCharacterSize(20.f);
	m_precision.setString(std::format("{:4.0f}", m_fractal.precision()));

	// Init fractal
	paused(true);
	m_fractal.update({ 0, 0 }, sf::Vector2i{ m_window.getSize() });
}

void Application::run()
{
	while (m_window.isOpen())
	{
		update();
		draw();
	}
}

void Application::update()
{
	for (auto event = sf::Event{}; m_window.pollEvent(event);)
	{
		// Ignore the events happened before/while resizing
		if (m_resized)
		{
			continue;
		}

		if (event.type == sf::Event::Closed)
		{
			m_window.close();
		}
		else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
		{
			// Set select frame
			m_down.x = event.mouseButton.x;
			m_down.y = event.mouseButton.y;
		}
		else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
		{
			// Update

			// Feedback for user
			paused();

			// Update fractal
			m_fractal.update(m_down, { event.mouseButton.x, event.mouseButton.y });

			// No select frame
			m_down = { -1, -1 };
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down)
			{
				if (event.key.code == sf::Keyboard::Up)
				{
					m_fractal.precision(m_fractal.precision() + 10);
				}
				else if (event.key.code == sf::Keyboard::Down)
				{
					m_fractal.precision(m_fractal.precision() - 10);
				}

				m_precision.setString(std::format("{:4.0f}", m_fractal.precision()));
			}
		}
		else if (event.type == sf::Event::KeyReleased)
		{
			if (event.key.code == sf::Keyboard::Return)
			{
				paused();
				m_fractal.update({ 0, 0 }, sf::Vector2i{ m_window.getSize() });
			}
			else if (event.key.code == sf::Keyboard::R)
			{
				paused();
				m_fractal.reset(m_window.getSize());
			}
		}
		else if (event.type == sf::Event::Resized)
		{
			onResize();
		}
	}
	m_resized = false;

	// Change select frame
	if (m_down.x != -1 && m_down.y != -1)
	{
		m_select.setPosition(sf::Vector2f{ m_down });
		m_select.setSize(sf::Vector2f{ sf::Mouse::getPosition(m_window) } - sf::Vector2f{ m_down });
	}
	else
	{
		m_select.setSize({ 0, 0 });
	}

	// Update precision text
	m_precision.setString(std::format("{:4.0f}", m_fractal.precision()));
}

void Application::draw()
{
	m_window.resetGLStates();

	m_window.clear();
	m_window.draw(m_fractal);
	m_window.draw(m_precision);
	m_window.draw(m_select);
	m_window.display();
}

void Application::paused(const bool fractal)
{
	sf::Text wait;
	wait.setFont(m_font);
	wait.setCharacterSize(30);
	wait.setString("Please Wait!");
	wait.setPosition(sf::Vector2f{ m_window.getSize() } / 2.f - sf::Vector2f{ wait.getLocalBounds().width, wait.getLocalBounds().height } / 2.f);

	sf::RectangleShape rect;
	rect.setSize(sf::Vector2f{ m_window.getSize() });
	rect.setFillColor({ 10, 10, 10, 220 });

	m_window.clear();
	if (fractal)
	{
		m_window.draw(m_fractal);
	}
	m_window.draw(rect);
	m_window.draw(wait);
	m_window.display();
}

void Application::onResize()
{
	m_resized = true;

	auto size = sf::Vector2f{ m_window.getSize() };

	// Minimum size
	if (size.x < 800)
	{
		size.x = 800;
	}
	if (size.y < 600)
	{
		size.y = 600;
	}

	// Apply possible size changes
	m_window.setSize(sf::Vector2u{ size });

	// Reset static (1:1) view
	m_staticView = sf::View{ { 0.f, 0.f, size.x, size.y } };
	m_window.setView(m_staticView);

	// Reset the fractal
	paused();
	m_fractal.reset(sf::Vector2u{ size });
}
