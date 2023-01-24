#include <iostream>
#include <tuple>
#include <vector>
#include <unordered_map>
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
      // BEGIN ITEMS DATA SECTION
      // NUM_ITEMS
      3,
      // ITEM_ID, COLUMN, ROW,
      // 114 - health potion
      // 116 - mana potion
      114, 2, 2,
      114, 4, 3,
      116, 4, 5,
      //
  };

  int tileWidth = 16;
  int tileHeight = 16;
  int numTilesAcrossTexture = gfxTexture.getSize().x / tileWidth;

  sf::VertexArray mapVerts;
  mapVerts.setPrimitiveType(sf::PrimitiveType::Quads);
  mapVerts.resize(4 * mapWidth * mapHeight);

  for (int i = 0; i < mapWidth * mapHeight; i++)
  {
    int x = i % mapWidth;
    int y = i / mapWidth;
    int tileId = map[i];
    int tileU = tileId % numTilesAcrossTexture;
    int tileV = tileId / numTilesAcrossTexture;
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

  int mapItemDataIndex = mapWidth * mapHeight;

  typedef std::tuple<bool, std::unique_ptr<sf::Sprite>, int, std::pair<int, int>> MapItem;
  std::vector<MapItem> mapItems;

  int numMapItems = map[mapItemDataIndex];
  for (int i = 0; i < numMapItems; i++)
  {
    int itemId = map[mapItemDataIndex + 1 + i * 3];
    int itemColumn = map[mapItemDataIndex + 1 + (i * 3) + 1];
    int itemRow = map[mapItemDataIndex + 1 + (i * 3) + 2];

    MapItem item = std::make_tuple(
        true,
        std::make_unique<sf::Sprite>(),
        itemId,
        std::make_pair(itemColumn, itemRow));
    auto &[visible, spr, id, coord] = item;
    spr->setTexture(gfxTexture);
    int srcX = id % numTilesAcrossTexture;
    int srcY = id / numTilesAcrossTexture;
    spr->setTextureRect(sf::IntRect(srcX * tileWidth, srcY * tileHeight, tileWidth, tileHeight));
    spr->setPosition(sf::Vector2f(coord.first * tileWidth, coord.second * tileHeight));
    mapItems.push_back(std::move(item));
  }

  std::unordered_map<int, std::string> itemNameDB;
  itemNameDB.emplace(114, "Health Potion");
  itemNameDB.emplace(116, "Mana Potion");

  typedef std::tuple<std::shared_ptr<MapItem>, std::unique_ptr<sf::Sprite>, std::unique_ptr<sf::Text>> InventoryItem;
  std::vector<InventoryItem> heroItems;

  auto collectMapItem = [&](MapItem *itemCollected)
  {
    auto &[visible, spr, id, coord] = *itemCollected;
    InventoryItem inventoryItem = std::make_tuple(
        std::shared_ptr<MapItem>(itemCollected, [](MapItem *) {}),
        std::make_unique<sf::Sprite>(),
        std::make_unique<sf::Text>());
    auto &inventorySprite = std::get<1>(inventoryItem);
    auto &inventoryText = std::get<2>(inventoryItem);

    inventorySprite->setTexture(*(spr->getTexture()));
    inventorySprite->setTextureRect(spr->getTextureRect());
    float uiMargin = 4.0f;
    float uiX = view.getSize().x - (uiMargin + (inventorySprite->getTextureRect().width * inventorySprite->getScale().x));
    float uiY = uiMargin + (static_cast<float>(heroItems.size()) * (inventorySprite->getTextureRect().height * inventorySprite->getScale().y));
    inventorySprite->setPosition(sf::Vector2f(uiX, uiY));

    inventoryText->setFont(font);
    inventoryText->setCharacterSize(24);
    inventoryText->setOutlineThickness(4);
    inventoryText->setString("[" + std::to_string(heroItems.size() + 1) + "] " + itemNameDB.at(id));
    inventoryText->setScale(sf::Vector2f(0.5f, 0.33f));
    auto textBounds = inventoryText->getLocalBounds();
    inventoryText->setPosition(sf::Vector2f(uiX - ((textBounds.width * inventoryText->getScale().x) + uiMargin), uiY));

    heroItems.push_back(std::move(inventoryItem));
  };

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

  int spiderHealth = 15;
  int spiderMaxHealth = 15;

  bool spiderWasDamaged = false;
  bool spiderIsResting = false;

  float spiderRestTime = 0.0f;
  float spiderTimeToRest = 2.0f;

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
              spiderWasDamaged = true;
              // isSpiderAlive = false;
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
              for (auto &item : mapItems)
              {
                auto &[visible, spr, id, coord] = item;
                if (visible)
                {
                  if (coord.first == heroColumn && coord.second == heroRow)
                  {
                    std::get<0>(item) = false;
                    collectMapItem(&item);
                  }
                }
              }
            }
          }
        }
      }
    }

    sf::Time deltaTime = clock.restart();

    bool shouldSpiderMove = false;
    float dt = deltaTime.asSeconds();
    if (spiderIsResting)
    {
      spiderRestTime += dt;
      if (spiderRestTime >= spiderTimeToRest)
      {
        spiderRestTime -= spiderTimeToRest;
        spiderIsResting = false;
      }
    }
    else
    {
      spiderTime += dt;
      if (spiderTime >= spiderTimeToMove)
      {
        spiderTime -= spiderTimeToMove;
        shouldSpiderMove = true;
      }
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

    if (spiderWasDamaged)
    {
      spiderIsResting = true;
      spiderWasDamaged = false;
      float currentHealth = static_cast<float>(spiderHealth);
      float maxHealth = static_cast<float>(spiderMaxHealth);
      float reducedHealth = currentHealth - (maxHealth / 3);
      spiderHealth = static_cast<int>(reducedHealth);
      float healthFill = 16 * (static_cast<float>(spiderHealth) / static_cast<float>(spiderMaxHealth));
      spiderHealthBarShape.setSize(sf::Vector2f(healthFill, 2));
      if (spiderHealth <= 0)
      {
        spiderHealth = 0;
        isSpiderAlive = false;
      }
    }

    window.clear();
    window.setView(view);
    window.draw(mapVerts, &gfxTexture);
    for (auto &item : mapItems)
    {
      auto &[visible, spr, id, coord] = item;
      if (visible)
      {
        window.draw(*(spr.get()));
      }
    }
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
    for (auto &heroItem : heroItems)
    {
      auto &[mapItem, inventorySprite, inventoryText] = heroItem;
      window.draw(*(inventorySprite.get()));
      window.draw(*(inventoryText.get()));
    }
    // window.draw(text);
    window.display();
  }

  return 0;
}