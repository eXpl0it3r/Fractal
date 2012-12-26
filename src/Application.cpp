#include "Application.hpp"

#include "Utility.hpp"

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Sleep.hpp>

#include <cmath>
#include <sstream>

Application::Application() :
    m_window(sf::VideoMode(1000, 765), "Fractal - Mandelbrot"),
    m_fractal(sf::Vector2u(1000, 765), 4),
    m_down(-1, -1),
    m_font(std::make_shared<sf::Font>()),
    m_resized(true)
{
    m_window.setFramerateLimit(60);

    // Init select frame
    m_select.setFillColor(sf::Color(0, 0, 0, 0));
    m_select.setOutlineColor(sf::Color(255, 255, 255, 255));
    m_select.setOutlineThickness(-2.f);

    m_font->loadFromFile("DejaVuSans.ttf");

    m_wait.setFont(*m_font);
    m_wait.setCharacterSize(30);
    m_wait.setString("Please Wait!");
    m_wait.setPosition(static_cast<sf::Vector2f>(m_window.getSize())/2.f-sf::Vector2f(m_wait.getLocalBounds().width, m_wait.getLocalBounds().height)/2.f);

    m_dimRect.setSize(static_cast<sf::Vector2f>(m_window.getSize()));
    m_dimRect.setFillColor(sf::Color(10, 10, 10, 210));

    // Init GUI
    createGUI();

    // Init fractal
    regenerate(sf::Vector2i(0, 0), sf::Vector2i(m_window.getSize().x+m_sidebarWidth, m_window.getSize().y), false);
}

void Application::createGUI()
{
    m_sidebarWidth = 180.f;

    // Let SFGUI know about the loaded font
    m_desktop.GetEngine().GetResourceManager().AddFont("DejaVuSans", m_font);

    // Set SFGUI properties
    m_desktop.SetProperty("*", "FontName", "DejaVuSans");
    m_desktop.SetProperty("Window", "BackgroundColor", sf::Color(0x2D, 0x3D, 0x5B, 0xFF));
    m_desktop.SetProperty("Window", "BorderWidth", 0.f);
    m_desktop.SetProperty("Window", "Padding", 5.f);
    m_desktop.SetProperty("Label", "Color", sf::Color(0xDD, 0xDD, 0xDD, 0xFF));
    m_desktop.SetProperty("Entry", "BackgroundColor", sf::Color(0x48, 0x62, 0x91, 0xFF));

    // Create sidebar columns
    m_columnLeftBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
    m_columnRightBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);

    // Create labels & entries
    m_zoomLabel = sfg::Label::Create("Zoom Factor");
    m_zoomEntry = sfg::Entry::Create();

    m_xLabel = sfg::Label::Create("X Position");
    m_xEntry = sfg::Entry::Create();
    m_columnLeftBox->Pack(m_xLabel);
    m_columnLeftBox->Pack(m_xEntry);

    m_yLabel = sfg::Label::Create("Y Position");
    m_yEntry = sfg::Entry::Create();
    m_columnRightBox->Pack(m_yLabel);
    m_columnRightBox->Pack(m_yEntry);

    m_precisionLabel = sfg::Label::Create("Precision");
    m_precisionEntry = sfg::Entry::Create();
    m_columnLeftBox->Pack(m_precisionLabel);
    m_columnLeftBox->Pack(m_precisionEntry);

    m_dividorLabel = sfg::Label::Create("Dividor");
    m_dividorEntry = sfg::Entry::Create();
    m_columnRightBox->Pack(m_dividorLabel);
    m_columnRightBox->Pack(m_dividorEntry);

    // Pack columns
    m_columnsBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
    m_columnsBox->Pack(m_columnLeftBox);
    m_columnsBox->Pack(m_columnRightBox);

    m_verticalBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
    m_verticalBox->SetRequisition(sf::Vector2f(160.f, m_window.getSize().y));
    m_verticalBox->Pack(m_zoomLabel, false, false);
    m_verticalBox->Pack(m_zoomEntry, false, false);
    m_verticalBox->Pack(m_columnsBox);

    m_horizontalBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL);
    m_horizontalBox->Pack(m_verticalBox, false, false);

    // Create window
    m_sidebar = sfg::Window::Create(sfg::Window::BACKGROUND);
    m_sidebar->SetAllocation(sf::FloatRect(m_window.getSize().x-m_sidebarWidth, 0, 300, m_window.getSize().y));
	m_sidebar->Add(m_horizontalBox);

    // Add window to the desktop
	m_desktop.Add(m_sidebar);
}

void Application::run()
{
    while(m_window.isOpen())
    {
        update();
        draw();
    }
}

void Application::update()
{
    sf::Event event;
    while(m_window.pollEvent(event))
    {
        // Ignore the events happened before/while resizing
        if(m_resized)
            continue;

        // Pass events to SFGUI
        m_desktop.HandleEvent(event);

        if(event.type == sf::Event::Closed)
            m_window.close();
        // Set select frame
        else if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            // Only start a selection if the mouse is not over the GUI
            if(event.mouseButton.x < m_window.getSize().x-m_sidebarWidth)
            {
                m_down.x = event.mouseButton.x;
                m_down.y = event.mouseButton.y;
            }
        }
        // Update
        else if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
        {
            // Only end a selection if there was a start
            if(m_down.x != -1 && m_down.y != -1)
            {
                // Update fractal
                regenerate(m_down, sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

                // Reset selection start
                m_down.x = -1;
                m_down.y = -1;
            }
        }
        else if(event.type == sf::Event::KeyPressed)
        {
            if(event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down)
            {
                if(event.key.code == sf::Keyboard::Up)
                    m_fractal.precision(m_fractal.precision()+10);
                else if(event.key.code == sf::Keyboard::Down)
                    m_fractal.precision(m_fractal.precision()-10);

                m_precisionEntry->SetText(utility::toString(static_cast<int>(m_fractal.precision())));
            }
        }
        else if(event.type == sf::Event::KeyReleased)
        {
            if(event.key.code == sf::Keyboard::Return)
            {
                regenerate(sf::Vector2i(0, 0), static_cast<sf::Vector2i>(m_window.getSize()));
            }
        }
        else if(event.type == sf::Event::Resized)
            onResize();
    }
    m_resized = false;

    // Update SFGUI with elapsed seconds since last call.
    m_desktop.Update(m_frametime.restart().asSeconds());

    // Change select frame
    if(m_down.x != -1 && m_down.x != -1)
    {
        m_select.setPosition(static_cast<sf::Vector2f>(m_down));
        m_select.setSize(static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_window))-static_cast<sf::Vector2f>(m_down));
    }
    else
        m_select.setSize(sf::Vector2f(0, 0));
}

void Application::draw()
{
    m_window.resetGLStates(); // Needed for SFGUI

    m_window.clear();
    m_window.draw(m_fractal);
    m_window.draw(m_select);
    m_sfgui.Display(m_window);
    m_window.display();
}

void Application::regenerate(const sf::Vector2i& first, const sf::Vector2i& second, bool fractal)
{
    // Set entry values
    m_zoomEntry->SetText(utility::toString(static_cast<double>(m_fractal.zoom())));
    m_xEntry->SetText(utility::toString(static_cast<double>(m_fractal.position().x)));
    m_yEntry->SetText(utility::toString(static_cast<double>(m_fractal.position().y)));
    m_precisionEntry->SetText(utility::toString(static_cast<int>(m_fractal.precision())));
    m_dividorEntry->SetText(utility::toString(static_cast<int>(m_fractal.devisor())));

    m_window.resetGLStates();

    m_window.clear();
    if(fractal)
        m_window.draw(m_fractal);
    m_sfgui.Display(m_window);
    m_window.draw(m_dimRect);
    m_window.draw(m_wait);
    m_window.display();

    m_fractal.update(first, second);

    // Reset entry values
    m_zoomEntry->SetText(utility::toString(static_cast<double>(m_fractal.zoom())));
    m_xEntry->SetText(utility::toString(static_cast<double>(m_fractal.position().x)));
    m_yEntry->SetText(utility::toString(static_cast<double>(m_fractal.position().y)));
    m_precisionEntry->SetText(utility::toString(static_cast<int>(m_fractal.precision())));
    m_dividorEntry->SetText(utility::toString(static_cast<int>(m_fractal.devisor())));
}

void Application::onResize()
{
    m_resized = true;

	sf::Vector2f size = static_cast<sf::Vector2f>(m_window.getSize());

	// Minimum size
	if(size.x < 800)
		size.x = 800;
	if(size.y < 600)
		size.y = 600;

	// Apply possible size changes
	m_window.setSize(static_cast<sf::Vector2u>(size));

	// Reset static (1:1) view
	m_staticView = sf::View(sf::FloatRect(0.f, 0.f, size.x, size.y));
	m_window.setView(m_staticView);

	// Resize widgets
    m_sidebar->SetAllocation(sf::FloatRect(size.x-m_sidebarWidth, 0, 300, size.y));

    // Resize the wait signs
    m_wait.setPosition(size/2.f-sf::Vector2f(m_wait.getLocalBounds().width, m_wait.getLocalBounds().height)/2.f);
    m_dimRect.setSize(size);

    // Resize & update the fractal
    m_fractal.resize(static_cast<sf::Vector2u>(size), 4);
    regenerate(sf::Vector2i(0, 0), sf::Vector2i(size.x+m_sidebarWidth, size.y), false);
}
