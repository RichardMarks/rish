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

  sf::Font font;
  if (!font.loadFromFile("assets/Px437_DOS-V_re_JPN16.ttf"))
  {
    throw std::runtime_error("Unable to load assets/Px437_DOS-V_re_JPN16.ttf");
  }

  sf::Sprite sprite;
  sf::Sprite enemy;
  sprite.setTexture(gfxTexture);
  enemy.setTexture(gfxTexture);

  sf::Text text;
  text.setFont(font);
  text.setCharacterSize(16);
  text.setOutlineThickness(1);
  text.setString("Hello, World!");

  int mapWidth = 7;
  int mapHeight = 7;

  int map[] = {
      // map tile ids
      40, 40, 40, 40, 40, 40, 40,
      40, 48, 48, 48, 48, 48, 40,
      40, 48, 48, 48, 48, 48, 40,
      40, 48, 48, 48, 48, 48, 40,
      40, 48, 48, 48, 48, 48, 40,
      40, 48, 48, 48, 48, 48, 40,
      40, 40, 40, 40, 40, 40, 40,
      //
  };

  int tileWidth = 16;
  int tileHeight = 16;
  int numTilesAcrossTexture = gfxTexture.getSize().x / tileWidth;
  int numTilesDownTexture = gfxTexture.getSize().y / tileHeight;

  sf::VertexArray mapVerts;
  mapVerts.setPrimitiveType(sf::PrimitiveType::Quads);
  mapVerts.resize(4 * mapWidth * mapHeight);

  for (int i = 0; i < mapWidth * mapHeight; i++)
  {
    int x = i % mapWidth;
    int y = i / mapWidth;
    int tileId = map[i];
    int tileU = tileId % numTilesAcrossTexture;
    int tileV = tileId / numTilesDownTexture;
    sf::Vertex *quad = &mapVerts[4 * (x + y * mapWidth)];
    quad[0].position = sf::Vector2f(x * tileWidth, y * tileHeight);
    quad[1].position = sf::Vector2f((x + 1) * tileWidth, y * tileHeight);
    quad[2].position = sf::Vector2f((x + 1) * tileWidth, (y + 1) * tileHeight);
    quad[3].position = sf::Vector2f(x * tileWidth, (y + 1) * tileHeight);
    quad[0].texCoords = sf::Vector2f(tileU * tileWidth, tileV * tileHeight);
    quad[1].texCoords = sf::Vector2f((tileU + 1) * tileWidth, tileV * tileHeight);
    quad[2].texCoords = sf::Vector2f((tileU + 1) * tileWidth, (tileV + 1) * tileHeight);
    quad[3].texCoords = sf::Vector2f(tileU * tileWidth, (tileV + 1) * tileHeight);
  }

  sprite.setTextureRect(sf::IntRect(tileWidth, 8 * tileHeight, tileWidth, tileHeight));
  enemy.setTextureRect(sf::IntRect(2 * tileWidth, 10 * tileHeight, tileWidth, tileHeight));

  int heroColumn = 3;
  int heroRow = 3;
  bool isHeroAlive = true;

  int spiderColumn = 5;
  int spiderRow = 1;

  sprite.setPosition(sf::Vector2f(tileWidth * heroColumn, tileHeight * heroRow));
  enemy.setPosition(sf::Vector2f(tileWidth * spiderColumn, tileHeight * spiderRow));

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
          bool didHeroMove = false;
          int heroX = heroColumn;
          int heroY = heroRow;

          if (event.key.code == sf::Keyboard::Up)
          {
            if (heroY > 0)
            {
              heroY -= 1;
              didHeroMove = true;
            }
          }
          else if (event.key.code == sf::Keyboard::Down)
          {
            if (heroY < mapHeight - 1)
            {
              heroY += 1;
              didHeroMove = true;
            }
          }
          else if (event.key.code == sf::Keyboard::Left)
          {
            if (heroX > 0)
            {
              heroX -= 1;
              didHeroMove = true;
            }
          }
          else if (event.key.code == sf::Keyboard::Right)
          {
            if (heroX < mapWidth - 1)
            {
              heroX += 1;
              didHeroMove = true;
            }
          }

          if (didHeroMove)
          {
            int tileId = map[heroX + heroY * mapWidth];
            if (tileId == 48)
            {
              heroColumn = heroX;
              heroRow = heroY;
              sprite.setPosition(sf::Vector2f(tileWidth * heroColumn, tileHeight * heroRow));

              if (heroColumn == spiderColumn && heroRow == spiderRow)
              {
                isHeroAlive = false;
              }
            }
          }
        }
      }
    }
    window.clear();
    window.draw(mapVerts, &gfxTexture);
    window.draw(enemy);
    if (isHeroAlive)
    {
      window.draw(sprite);
    }
    window.draw(text);
    window.display();
  }

  return 0;
}