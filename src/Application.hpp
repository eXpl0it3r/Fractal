#pragma once

#include "Fractal.hpp"

#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Box.hpp>
#include <SFGUI/Desktop.hpp>
#include <SFGUI/Entry.hpp>
#include <SFGUI/Label.hpp>
#include <SFGUI/Window.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>

class Application
{
public:
    Application();
    void run();

private:
    void update();
    void draw();

    void regenerate(const sf::Vector2i& first, const sf::Vector2i& second, bool fractal = true);

    void createGUI();
    void onResize();

private:
    sf::RenderWindow m_window;
    Fractal m_fractal;
    sf::Vector2i m_down;
    sf::RectangleShape m_select;
    std::shared_ptr<sf::Font> m_font;
    sf::Clock m_frametime;

    sf::Text m_wait;
    sf::RectangleShape m_dimRect;

    sf::View m_staticView;
    bool m_resized;

    sfg::SFGUI m_sfgui;
    sfg::Desktop m_desktop;
    sfg::Window::Ptr m_sidebar;
    float m_sidebarWidth;
    sfg::Box::Ptr m_verticalBox;
    sfg::Box::Ptr m_horizontalBox;
    sfg::Box::Ptr m_columnsBox;
    sfg::Box::Ptr m_columnLeftBox;
    sfg::Box::Ptr m_columnRightBox;

    sfg::Entry::Ptr m_xEntry;
    sfg::Entry::Ptr m_yEntry;
    sfg::Entry::Ptr m_zoomEntry;
    sfg::Entry::Ptr m_precisionEntry;
    sfg::Entry::Ptr m_dividorEntry;

    sfg::Label::Ptr m_xLabel;
    sfg::Label::Ptr m_yLabel;
    sfg::Label::Ptr m_zoomLabel;
    sfg::Label::Ptr m_precisionLabel;
    sfg::Label::Ptr m_dividorLabel;
};
