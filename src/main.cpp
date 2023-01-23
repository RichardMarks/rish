#include <iostream>
#include <SFML/Graphics.hpp>

int main()
{
  sf::RenderWindow window(sf::VideoMode(320, 240), "rish");

  sf::Texture gfxTexture;
  if (!gfxTexture.loadFromFile("assets/gfx.png"))
  {
    throw std::runtime_error("Unable to load assets/gfx.png");
  }

  sf::Sprite sprite;
  sprite.setTexture(gfxTexture);

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        window.close();
      }
      else if (event.type == sf::Event::KeyPressed)
      {
        if (event.key.code == sf::Keyboard::Escape)
        {
          window.close();
        }
        else
        {
          float motionX = 0.0f;
          float motionY = 0.0f;

          if (event.key.code == sf::Keyboard::Up)
          {
            motionY = -1.0f;
          }
          else if (event.key.code == sf::Keyboard::Down)
          {
            motionY = 1.0f;
          }
          else if (event.key.code == sf::Keyboard::Left)
          {
            motionX = -1.0f;
          }
          else if (event.key.code == sf::Keyboard::Right)
          {
            motionX = 1.0f;
          }

          sprite.move(sf::Vector2f(motionX, motionY));
        }
      }
    }
    window.clear();
    window.draw(sprite);
    window.display();
  }

  return 0;
}