#pragma once

#include "Fractal.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>

class Application
{
public:
	Application();
	void run();

private:
	void update();
	void draw();

	void paused(bool fractal = true);

	void onResize();

	sf::RenderWindow m_window;
	Fractal m_fractal;
	sf::Vector2i m_down;
	sf::RectangleShape m_select;
	sf::Font m_font;
	sf::Text m_precision;
	sf::Clock m_frameTime;

	sf::View m_staticView;
	bool m_resized;
};
