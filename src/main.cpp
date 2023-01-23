#include <iostream>
#include <SFML/Graphics.hpp>

int main()
{
  sf::RenderWindow window(sf::VideoMode(1920, 1440), "rish");
  sf::View view(sf::FloatRect(0, 0, 320, 240));

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
  bool isSpiderAlive = true;

  sprite.setPosition(sf::Vector2f(tileWidth * heroColumn, tileHeight * heroRow));
  enemy.setPosition(sf::Vector2f(tileWidth * spiderColumn, tileHeight * spiderRow));

  sf::Clock clock;
  sf::Time elapsed;

  float spiderTime = 0.0f;
  float spiderTimeToMove = 1.0f;

  int spiderPath[] = {
      // pairs of tile coordinates to move the spider to each movement pass
      5, 2,
      5, 3,
      5, 4,
      5, 5,
      5, 4,
      5, 3,
      5, 2,
      5, 1,
      //
  };

  int spiderCurrentPathIndex = 0;
  int spiderFinalPathIndex = (sizeof(spiderPath) / (2 * sizeof(int))) - 1;

  sf::RectangleShape heroHealthBarShape;
  heroHealthBarShape.setFillColor(sf::Color::Green);
  heroHealthBarShape.setSize(sf::Vector2f(16, 2));
  heroHealthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, 1 + tileHeight));

  int heroHealth = 30;
  int heroMaxHealth = 30;

  bool heroWasDamaged = false;

  sf::RectangleShape spiderHealthBarShape;
  spiderHealthBarShape.setFillColor(sf::Color::Red);
  spiderHealthBarShape.setSize(sf::Vector2f(16, 2));
  spiderHealthBarShape.setPosition(enemy.getPosition() + sf::Vector2f(0, 1 + tileHeight));

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
          if (event.key.code == sf::Keyboard::Space)
          {
            bool hitAbove = heroRow - 1 == spiderRow && heroColumn == spiderColumn;
            bool hitBelow = heroRow + 1 == spiderRow && heroColumn == spiderColumn;
            bool hitLeft = heroRow == spiderRow && heroColumn - 1 == spiderColumn;
            bool hitRight = heroRow == spiderRow && heroColumn + 1 == spiderColumn;
            bool hitSpider = hitAbove || hitBelow || hitLeft || hitRight;
            if (hitSpider)
            {
              isSpiderAlive = false;
            }
          }
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
              int lastColumn = heroColumn;
              int lastRow = heroRow;
              heroColumn = heroX;
              heroRow = heroY;
              sprite.setPosition(sf::Vector2f(tileWidth * heroColumn, tileHeight * heroRow));

              heroHealthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, tileHeight));

              if (heroColumn == spiderColumn && heroRow == spiderRow)
              {
                heroWasDamaged = true;
                heroColumn = lastColumn;
                heroRow = lastRow;
                sprite.setPosition(sf::Vector2f(tileWidth * heroColumn, tileHeight * heroRow));
                heroHealthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, tileHeight));
              }
            }
          }
        }
      }
    }

    sf::Time deltaTime = clock.restart();

    float dt = deltaTime.asSeconds();
    spiderTime += dt;
    bool shouldSpiderMove = false;
    if (spiderTime >= spiderTimeToMove)
    {
      spiderTime -= spiderTimeToMove;
      shouldSpiderMove = true;
    }

    if (shouldSpiderMove)
    {
      int nextSpiderColumn = spiderPath[0 + spiderCurrentPathIndex * 2];
      int nextSpiderRow = spiderPath[1 + spiderCurrentPathIndex * 2];

      if (!heroWasDamaged && (heroColumn == nextSpiderColumn && heroRow == nextSpiderRow))
      {
        heroWasDamaged = true;
      }

      spiderColumn = nextSpiderColumn;
      spiderRow = nextSpiderRow;
      enemy.setPosition(sf::Vector2f(tileWidth * spiderColumn, tileHeight * spiderRow));
      spiderHealthBarShape.setPosition(enemy.getPosition() + sf::Vector2f(0, 1 + tileHeight));
      spiderCurrentPathIndex += 1;
      if (spiderCurrentPathIndex > spiderFinalPathIndex)
      {
        spiderCurrentPathIndex = 0;
      }
    }

    if (heroWasDamaged)
    {
      heroWasDamaged = false;
      float currentHealth = static_cast<float>(heroHealth);
      float maxHealth = static_cast<float>(heroMaxHealth);
      float reducedHealth = currentHealth - (maxHealth * 0.25f);
      heroHealth = static_cast<int>(reducedHealth);
      float healthFill = 16 * (static_cast<float>(heroHealth) / static_cast<float>(heroMaxHealth));
      heroHealthBarShape.setSize(sf::Vector2f(healthFill, 2));
      if (heroHealth <= 0)
      {
        heroHealth = 0;
        isHeroAlive = false;
      }
    }

    window.clear();
    window.setView(view);
    window.draw(mapVerts, &gfxTexture);
    if (isSpiderAlive)
    {
      window.draw(enemy);
      window.draw(spiderHealthBarShape);
    }
    if (isHeroAlive)
    {
      window.draw(sprite);
      window.draw(heroHealthBarShape);
    }
    window.draw(text);
    window.display();
  }

  return 0;
}