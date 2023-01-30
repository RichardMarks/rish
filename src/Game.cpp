#include "Game.h"

int mapWidth = 7;
int mapHeight = 7;

int map[] = {
    // map tile ids
    40, 40, 45, 40, 40, 40, 40,
    40, 48, 48, 48, 48, 41, 40,
    40, 48, 48, 48, 48, 48, 40,
    40, 48, 48, 48, 48, 48, 40,
    40, 48, 48, 41, 48, 48, 40,
    40, 48, 41, 48, 48, 48, 40,
    40, 40, 40, 40, 40, 45, 40,
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
    // BEGIN TREASURE CHESTS DATA SECTION
    // NUM_CHESTS
    3,
    // CHEST_KIND, COLUMN, ROW, OPTIONAL_CONTENTS, OPTIONAL_QTY
    CHEST_UNLOCKED_CLOSED, 1, 3, 0, 0,
    CHEST_UNLOCKED_CLOSED, 1, 5, 0, 0,
    CHEST_UNLOCKED_CLOSED, 3, 1, 116, 4,
    //
};

rish::Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "rish"),
                     view(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT))
{
  setup();
  run();
}

void rish::Game::setup()
{
  if (!gfxTexture.loadFromFile("assets/gfx.png"))
  {
    throw std::runtime_error("Unable to load assets/gfx.png");
  }

  if (!font.loadFromFile("assets/Px437_DOS-V_re_JPN16.ttf"))
  {
    throw std::runtime_error("Unable to load assets/Px437_DOS-V_re_JPN16.ttf");
  }

  // random number generation
  std::seed_seq seed{
      std::random_device{}(),
      static_cast<unsigned int>(::time(nullptr)),
      static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())};
  rng.seed(seed);

  // hotkey mapping vars
  inventorySlotKeys.emplace(sf::Keyboard::Num1, 0);
  inventorySlotKeys.emplace(sf::Keyboard::Num2, 1);
  inventorySlotKeys.emplace(sf::Keyboard::Num3, 2);
  inventorySlotKeys.emplace(sf::Keyboard::Num4, 3);
  inventorySlotKeys.emplace(sf::Keyboard::Num5, 4);
  inventorySlotKeys.emplace(sf::Keyboard::Num6, 5);
  inventorySlotKeys.emplace(sf::Keyboard::Num7, 6);
  inventorySlotKeys.emplace(sf::Keyboard::Num8, 7);
  inventorySlotKeys.emplace(sf::Keyboard::Num9, 8);

  walkableTiles.emplace(48);
  walkableTiles.emplace(41);
  walkableTiles.emplace(OPENED_DOOR_TILE_ID);

  hazardTiles.emplace(41);

  numTilesAcrossTexture = gfxTexture.getSize().x / TILE_WIDTH;

  mapItemDataIndex = mapWidth * mapHeight;
  numMapItems = map[mapItemDataIndex];
  treasureChestDataIndex = mapItemDataIndex + 1;
  treasureChestDataIndex += (ITEM_DATA_STRIDE * numMapItems);
  numTreasureChests = map[treasureChestDataIndex];
  treasureChestDataIndex += 1;

  itemDBAdd(
      itemsDatabase,
      114,
      "Health Potion",
      0,
      std::bind(&rish::Game::canUseHealthPotion, this),
      std::bind(&rish::Game::useHealthPotion, this));

  itemDBAdd(
      itemsDatabase,
      116,
      "Mana Potion",
      0,
      std::bind(&rish::Game::canUseManaPotion, this),
      std::bind(&rish::Game::useManaPotion, this));

  setupHero();
  setupEnemy();
  setupTilemap();
}

void rish::Game::run()
{
  while (window.isOpen())
  {
    processEvents();

    sf::Time deltaTime = clock.restart();

    handleSpiderMovement(deltaTime);
    handleHeroWasDamagedEvent();
    handleSpiderWasDamagedEvent();

    // rendering starts here
    prepareRender();
    renderTilemap();
    renderTreasureChests();
    renderMapItems();
    renderEnemy();
    renderHero();
    renderInventoryUI();
    finishRender();
  }
}

int rish::Game::rollInt(int low, int high)
{
  std::uniform_int_distribution<int> dist(low, high);
  return dist(rng);
}

bool rish::Game::chanceOf(float percentage)
{
  std::uniform_real_distribution<float> dist(1.0f, 100.0f);
  return dist(rng) < percentage;
}

void rish::Game::setupHero()
{
  sprite.setTexture(gfxTexture);
  sprite.setTextureRect(sf::IntRect(TILE_WIDTH, 8 * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

  sprite.setPosition(sf::Vector2f(TILE_WIDTH * heroColumn, TILE_HEIGHT * heroRow));

  heroHealthBarShape.setFillColor(sf::Color::Green);
  heroHealthBarShape.setSize(sf::Vector2f(16, 2));
  heroHealthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, 1 + TILE_HEIGHT));
}

void rish::Game::setupEnemy()
{
  spiderColumn = 5;
  spiderRow = 1;
  isSpiderAlive = true;
  spiderTime = 0.0f;
  spiderTimeToMove = 1.0f;

  int spiderPathPairs[] = {
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
  std::copy(
      &spiderPathPairs[0],
      &spiderPathPairs[(sizeof(spiderPathPairs) / (sizeof(int)))],
      std::back_inserter(spiderPath));

  spiderCurrentPathIndex = 0;
  spiderFinalPathIndex = (spiderPath.size() / 2) - 1;
  spiderHealth = 15;
  spiderMaxHealth = 15;
  spiderWasDamaged = false;
  spiderIsResting = false;
  spiderRestTime = 0.0f;
  spiderTimeToRest = 2.0f;

  enemy.setTexture(gfxTexture);
  enemy.setTextureRect(sf::IntRect(2 * TILE_WIDTH, 10 * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

  enemy.setPosition(sf::Vector2f(TILE_WIDTH * spiderColumn, TILE_HEIGHT * spiderRow));

  spiderHealthBarShape.setFillColor(sf::Color::Red);
  spiderHealthBarShape.setSize(sf::Vector2f(16, 2));
  spiderHealthBarShape.setPosition(enemy.getPosition() + sf::Vector2f(0, 1 + TILE_HEIGHT));
}

void rish::Game::addMapItem(TileId itemId, int itemColumn, int itemRow)
{
  MapItem item = std::make_tuple(
      FIELD_ITEM,
      std::make_unique<sf::Sprite>(),
      itemId,
      std::make_pair(itemColumn, itemRow));

  auto &spr = getMapItemSprite(item);
  auto id = getMapItemId(item);
  auto &coord = getMapItemCoordinates(item);

  spr->setTexture(gfxTexture);
  int srcX = id % numTilesAcrossTexture;
  int srcY = id / numTilesAcrossTexture;
  spr->setTextureRect(sf::IntRect(srcX * TILE_WIDTH, srcY * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));
  spr->setPosition(sf::Vector2f(coord.first * TILE_WIDTH, coord.second * TILE_HEIGHT));
  mapItems.push_back(std::move(item));
}

void rish::Game::setTreasureChestSprite(TreasureChest &chest)
{
  auto &spr = getTreasureChestSprite(chest);
  auto chestKind = getTreasureChestKind(chest);

  constexpr int CLOSED_CHEST_ID = 89;
  constexpr int OPEN_CHEST_ID = 91;

  int id = 0;
  if (chestKind == CHEST_UNLOCKED_CLOSED || chestKind == CHEST_LOCKED_CLOSED)
  {
    id = CLOSED_CHEST_ID;
  }
  else if (chestKind == CHEST_UNLOCKED_OPEN)
  {
    id = OPEN_CHEST_ID;
  }

  int srcX = id % numTilesAcrossTexture;
  int srcY = id / numTilesAcrossTexture;
  spr->setTextureRect(sf::IntRect(srcX * TILE_WIDTH, srcY * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));
}

void rish::Game::addMapTreasureChest(int chestKind, int chestColumn, int chestRow, int chestContent, int chestQty)
{
  TreasureChest chest = std::make_tuple(
      chestKind,
      std::make_pair(chestColumn, chestRow),
      std::make_pair(chestContent, chestQty),
      std::make_unique<sf::Sprite>());

  auto &spr = getTreasureChestSprite(chest);

  spr->setTexture(gfxTexture);
  setTreasureChestSprite(chest);
  spr->setPosition(sf::Vector2f(chestColumn * TILE_WIDTH, chestRow * TILE_HEIGHT));

  treasureChests.push_back(std::move(chest));
}

void rish::Game::setTilemapTileVertices(int mapIndex)
{
  int x = mapIndex % mapWidth;
  int y = mapIndex / mapWidth;
  int tileId = map[mapIndex];
  int tileU = tileId % numTilesAcrossTexture;
  int tileV = tileId / numTilesAcrossTexture;
  sf::Vertex *quad = &mapVerts[4 * mapIndex];
  quad[0].position = sf::Vector2f(x * TILE_WIDTH, y * TILE_HEIGHT);
  quad[1].position = sf::Vector2f((x + 1) * TILE_WIDTH, y * TILE_HEIGHT);
  quad[2].position = sf::Vector2f((x + 1) * TILE_WIDTH, (y + 1) * TILE_HEIGHT);
  quad[3].position = sf::Vector2f(x * TILE_WIDTH, (y + 1) * TILE_HEIGHT);
  quad[0].texCoords = sf::Vector2f(tileU * TILE_WIDTH, tileV * TILE_HEIGHT);
  quad[1].texCoords = sf::Vector2f((tileU + 1) * TILE_WIDTH, tileV * TILE_HEIGHT);
  quad[2].texCoords = sf::Vector2f((tileU + 1) * TILE_WIDTH, (tileV + 1) * TILE_HEIGHT);
  quad[3].texCoords = sf::Vector2f(tileU * TILE_WIDTH, (tileV + 1) * TILE_HEIGHT);
}

void rish::Game::changeTilemapTile(int mapIndex, TileId tileId)
{
  map[mapIndex] = tileId;
  setTilemapTileVertices(mapIndex);
}

void rish::Game::setupTilemap()
{
  mapVerts.setPrimitiveType(sf::PrimitiveType::Quads);
  mapVerts.resize(4 * mapWidth * mapHeight);

  for (int i = 0; i < mapWidth * mapHeight; i++)
  {
    setTilemapTileVertices(i);
  }

  for (int i = 0; i < numMapItems; i++)
  {
    int itemId = map[mapItemDataIndex + 1 + i * 3];
    int itemColumn = map[mapItemDataIndex + 1 + (i * 3) + 1];
    int itemRow = map[mapItemDataIndex + 1 + (i * 3) + 2];
    addMapItem(itemId, itemColumn, itemRow);
  }

  for (int i = 0; i < numTreasureChests; i++)
  {
    int base = treasureChestDataIndex + (i * TREASURE_CHEST_DATA_STRIDE);
    int chestKind = map[base + 0];
    int chestColumn = map[base + 1];
    int chestRow = map[base + 2];
    int chestContent = map[base + 3];
    int chestQty = map[base + 4];

    addMapTreasureChest(chestKind, chestColumn, chestRow, chestContent, chestQty);
  }
}

void rish::Game::setupInventoryItemUI(InventoryItem &inventoryItem, unsigned long slotNumber)
{
  auto &inventorySprite = getInventoryItemUISprite(inventoryItem);
  auto &inventoryText = getInventoryItemUIText(inventoryItem);
  auto &mapItem = getInventoryItemRef(inventoryItem);
  auto mapItemId = getMapItemId(*mapItem);
  auto &mapItemSprite = getMapItemSprite(*mapItem);

  // ui item uses the same graphic as the field map item
  inventorySprite->setTexture(*(mapItemSprite->getTexture()));
  inventorySprite->setTextureRect(mapItemSprite->getTextureRect());

  // the size of the ui item taking into account scale if used
  float spriteWidth = inventorySprite->getTextureRect().width * inventorySprite->getScale().x;
  float spriteHeight = inventorySprite->getTextureRect().height * inventorySprite->getScale().y;

  // the amount of distance between ui elements
  float uiMargin = 4.0f;

  // position the sprite at the right edge of the screen with margin
  float screenRight = view.getSize().x;
  float uiX = screenRight - (uiMargin + spriteWidth);

  // the slot number is used to calculate the Y axis offset
  float uiY = uiMargin + (static_cast<float>(slotNumber) * spriteHeight);

  // position the sprite
  inventorySprite->setPosition(sf::Vector2f(uiX, uiY));

  // setup the ui text
  inventoryText->setFont(font);
  inventoryText->setCharacterSize(24);
  inventoryText->setOutlineThickness(4);
  inventoryText->setScale(sf::Vector2f(0.5f, 0.33f));

  // the slot numbers are zero indexed, but displayed as one-indexed
  // because the number keys 1 to 9 are used to trigger the item
  auto slotTriggerText = "[" + std::to_string(slotNumber + 1) + "] ";

  // the item name is found by looking up the item id in the items database
  auto itemName = getItemDBEntryName(getItemDBEntry(itemsDatabase, mapItemId));

  // set the text of the slot
  inventoryText->setString(slotTriggerText + itemName);

  // use the rendered text bounds to position the text left of the sprite
  auto textBounds = inventoryText->getLocalBounds();
  float textWidth = textBounds.width * inventoryText->getScale().x;
  float textX = uiX - (uiMargin + textWidth);
  float textY = uiY;
  inventoryText->setPosition(sf::Vector2f(textX, textY));
}

void rish::Game::collectMapItem(MapItem *itemCollected)
{
  setMapItemState(*itemCollected, INVENTORY_ITEM);

  InventoryItem inventoryItem = std::make_tuple(
      std::shared_ptr<MapItem>(itemCollected, [](MapItem *) {}),
      std::make_unique<sf::Sprite>(),
      std::make_unique<sf::Text>());

  setupInventoryItemUI(inventoryItem, heroItems.size());

  heroItems.push_back(std::move(inventoryItem));
}

void rish::Game::addItemToInventory(TileId tileId)
{
  InventoryItem inventoryItem = std::make_tuple(
      std::make_shared<MapItem>(),
      // std::shared_ptr<MapItem>(itemCollected, [](MapItem *) {}),
      std::make_unique<sf::Sprite>(),
      std::make_unique<sf::Text>());
  InventoryItemRef &mapItem = std::get<0>(inventoryItem);
  std::get<0>(*mapItem) = INVENTORY_ITEM;
  std::get<1>(*mapItem) = std::make_unique<sf::Sprite>();
  std::get<2>(*mapItem) = tileId;
  std::get<3>(*mapItem) = std::make_pair(-1, -1);

  auto &spr = getMapItemSprite(*mapItem);

  spr->setTexture(gfxTexture);
  int srcX = tileId % numTilesAcrossTexture;
  int srcY = tileId / numTilesAcrossTexture;
  spr->setTextureRect(sf::IntRect(srcX * TILE_WIDTH, srcY * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));
  spr->setPosition(sf::Vector2f(-9999.0f, -9999.0f));

  setupInventoryItemUI(inventoryItem, heroItems.size());
  heroItems.push_back(std::move(inventoryItem));
}

bool rish::Game::isUsedInventoryItem(InventoryItem &item)
{
  auto &mapItem = getInventoryItemRef(item);
  auto state = getMapItemState(*mapItem);
  return state == USED_INVENTORY_ITEM;
}

void rish::Game::updateInventoryUI()
{
  // erase the items which have been used
  heroItems.erase(
      std::remove_if(
          heroItems.begin(),
          heroItems.end(),
          std::bind(&rish::Game::isUsedInventoryItem, this, std::placeholders::_1)),
      heroItems.end());
  // update the remaining items ui
  for (unsigned long i = 0; i < heroItems.size(); i++)
  {
    setupInventoryItemUI(heroItems.at(i), i);
  }
}

void rish::Game::handleHeroAttackAction()
{
  bool hitAbove = heroRow - 1 == spiderRow && heroColumn == spiderColumn;
  bool hitBelow = heroRow + 1 == spiderRow && heroColumn == spiderColumn;
  bool hitLeft = heroRow == spiderRow && heroColumn - 1 == spiderColumn;
  bool hitRight = heroRow == spiderRow && heroColumn + 1 == spiderColumn;
  bool hitSpider = hitAbove || hitBelow || hitLeft || hitRight;
  if (hitSpider)
  {
    spiderWasDamaged = true;
  }
}

bool rish::Game::handleOpenTreasureChestAction()
{
  for (auto &chest : treasureChests)
  {
    auto kind = getTreasureChestKind(chest);

    // skip the chests that are already open
    if (kind == CHEST_UNLOCKED_OPEN)
    {
      continue;
    }
    else if (kind == CHEST_LOCKED_CLOSED)
    {
      // TODO: locked chests require a key to open
      continue;
    }

    auto &coords = getTreasureChestCoordinates(chest);
    auto chestColumn = coords.first;
    auto chestRow = coords.second;

    bool hitAbove = heroRow - 1 == chestRow && heroColumn == chestColumn;
    bool hitBelow = heroRow + 1 == chestRow && heroColumn == chestColumn;
    bool hitLeft = heroRow == chestRow && heroColumn - 1 == chestColumn;
    bool hitRight = heroRow == chestRow && heroColumn + 1 == chestColumn;
    bool hitChest = hitAbove || hitBelow || hitLeft || hitRight;

    if (!hitChest)
    {
      continue;
    }

    setTreasureChestKind(chest, CHEST_UNLOCKED_OPEN);
    setTreasureChestSprite(chest);

    int contentId = getTreasureChestContentsItemId(chest);
    // if chest content is 0, use a random item
    if (contentId == 0)
    {
      int index = rollInt(0, itemsDatabase.size() - 1);
      contentId = std::next(itemsDatabase.begin(), index)->first;
    }
    auto quantity = getTreasureChestContentsItemQuantity(chest);
    // if chest quantity is zero, chance for a second item
    bool chanceForSecondItem = quantity == 0;

    // award the hero with the item qtx times
    if (quantity == 0)
    {
      quantity = 1;
    }

    for (int i = 0; i < quantity; i++)
    {
      addItemToInventory(contentId);
    }

    if (chanceForSecondItem)
    {
      if (chanceOf(10.0f))
      {
        // award the hero with another item
        addItemToInventory(contentId);
      }
    }

    return true;
  }

  return false;
}

bool rish::Game::handleOpenDoorAction()
{
  std::vector<std::pair<TileId, int>> neighboringTileIds;

  // if there is a tile above the hero, add to list of neighboring tile ids
  if (heroRow - 1 >= 0)
  {
    auto x = heroColumn;
    auto y = heroRow - 1;
    int index = x + (y * mapWidth);
    auto tileId = map[index];
    neighboringTileIds.push_back(std::make_pair(tileId, index));
  }

  // if there is a tile below the hero, add to the list of neighboring tile ids
  if (heroRow + 1 < mapHeight)
  {
    auto x = heroColumn;
    auto y = heroRow + 1;
    int index = x + (y * mapWidth);
    auto tileId = map[index];
    neighboringTileIds.push_back(std::make_pair(tileId, index));
  }

  // if there is a tile to the left of the hero, add to the list of neighboring tile ids
  if (heroColumn - 1 >= 0)
  {
    auto x = heroColumn - 1;
    auto y = heroRow;
    int index = x + (y * mapWidth);
    auto tileId = map[index];
    neighboringTileIds.push_back(std::make_pair(tileId, index));
  }

  // if there is a tile to the right of the hero, add to the list of neighboring tile ids
  if (heroColumn + 1 < mapWidth)
  {
    auto x = heroColumn + 1;
    auto y = heroRow;
    int index = x + (y * mapWidth);
    auto tileId = map[index];
    neighboringTileIds.push_back(std::make_pair(tileId, index));
  }

  for (auto tileInfo : neighboringTileIds)
  {
    auto [tileId, tileIndex] = tileInfo;
    bool isDoorTile = tileId == CLOSED_DOOR_TILE_ID || tileId == OPENED_DOOR_TILE_ID;
    if (!isDoorTile)
    {
      continue;
    }
    if (tileId == CLOSED_DOOR_TILE_ID)
    {
      changeTilemapTile(tileIndex, OPENED_DOOR_TILE_ID);
    }
    else
    {
      changeTilemapTile(tileIndex, CLOSED_DOOR_TILE_ID);
    }
    return true;
  }

  return false;
}

bool rish::Game::handleHeroInteractAction()
{
  // if we opened a treasure chest, the interaction was successful
  if (handleOpenTreasureChestAction())
  {
    return true;
  }
  // if we opened a door, the interaction was successful
  if (handleOpenDoorAction())
  {
    return true;
  }

  // nothing to interact with, interaction was not successful
  return false;
}

void rish::Game::checkForHeroVsMapItemCollisions()
{
  for (auto &item : mapItems)
  {
    auto &coord = getMapItemCoordinates(item);
    auto state = getMapItemState(item);
    if (state == FIELD_ITEM)
    {
      if (coord.first == heroColumn && coord.second == heroRow)
      {
        collectMapItem(&item);
      }
    }
  }
}

void rish::Game::setHeroPosition(int column, int row)
{
  // update the hero world position
  heroColumn = column;
  heroRow = row;

  // update the hero sprite position
  sprite.setPosition(sf::Vector2f(TILE_WIDTH * heroColumn, TILE_HEIGHT * heroRow));

  // update the hero health bar position
  heroHealthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, TILE_HEIGHT));
}

void rish::Game::checkForHeroVsEnemyCollisions(int lastColumn, int lastRow)
{
  // colliding with dead invisible spiders is bad mmkay
  if (!isSpiderAlive)
  {
    return;
  }
  if (heroColumn == spiderColumn && heroRow == spiderRow)
  {
    heroWasDamaged = true;
    setHeroPosition(lastColumn, lastRow);
  }
}

void rish::Game::checkForHeroVsTreasureChestCollisions(int lastColumn, int lastRow)
{
  for (auto &chest : treasureChests)
  {
    auto &coords = getTreasureChestCoordinates(chest);
    if (heroColumn == coords.first && heroRow == coords.second)
    {
      setHeroPosition(lastColumn, lastRow);
      return;
    }
  }
}

void rish::Game::activateInventoryItemSlot(unsigned long slotNumber)
{
  if (slotNumber >= heroItems.size())
  {
    return;
  }
  std::cout << "activate inventory item slot number: " << std::to_string(slotNumber) << std::endl;
  auto &item = heroItems.at(slotNumber);
  auto &mapItem = getInventoryItemRef(item);
  auto state = getMapItemState(*mapItem);
  if (state == INVENTORY_ITEM)
  {
    auto tileId = getMapItemId(*mapItem);
    auto entry = getItemDBEntry(itemsDatabase, tileId);
    auto canUse = getItemDBEntryCanUse(entry);
    if (canUse())
    {
      auto useItem = getItemDBEntryUse(entry);
      useItem();
      setMapItemState(*mapItem, USED_INVENTORY_ITEM);
      updateInventoryUI();
    }
  }
}

bool rish::Game::handleInventoryHotkeys(sf::Event &event)
{
  for (auto &hotkey : inventorySlotKeys)
  {
    if (event.key.code == hotkey.first)
    {
      activateInventoryItemSlot(hotkey.second);
      return true;
    }
  }
  return false;
}

void rish::Game::handleHeroKeyPressedEvent(sf::Event &event)
{
  if (handleInventoryHotkeys(event))
  {
    return;
  }
  if (event.key.code == sf::Keyboard::Space)
  {
    if (!handleHeroInteractAction())
    {
      handleHeroAttackAction();
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
    bool isWalkableTile = walkableTiles.count(tileId) != 0;
    bool isHazardTile = hazardTiles.count(tileId) != 0;
    if (isWalkableTile)
    {
      int lastColumn = heroColumn;
      int lastRow = heroRow;
      setHeroPosition(heroX, heroY);
      checkForHeroVsTreasureChestCollisions(lastColumn, lastRow);
      checkForHeroVsEnemyCollisions(lastColumn, lastRow);
      checkForHeroVsMapItemCollisions();
      if (isHazardTile)
      {
        heroWasDamaged = true;
      }
    }
  }
}

void rish::Game::processEvents()
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
        handleHeroKeyPressedEvent(event);
      }
    }
  }
}

bool rish::Game::determineIfSpiderShouldMove(sf::Time &deltaTime)
{
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
  return shouldSpiderMove;
}

void rish::Game::handleHeroWasDamagedEvent()
{
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
}

void rish::Game::spawnRandomItemOnMapAt(int column, int row)
{
  std::cout << "spawn random item at " << std::to_string(column) << "," << std::to_string(row) << std::endl;
  int index = rollInt(0, itemsDatabase.size() - 1);
  auto it = itemsDatabase.begin();
  auto tileId = std::next(it, index)->first;
  addMapItem(tileId, column, row);
}

void rish::Game::handleSpiderWasDamagedEvent()
{
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
      if (chanceOf(47.0f))
      {
        spawnRandomItemOnMapAt(spiderColumn, spiderRow);
      }
    }
  }
}

void rish::Game::handleSpiderMovement(sf::Time &deltaTime)
{
  // dead spiders should not move
  if (!isSpiderAlive)
  {
    return;
  }
  bool shouldSpiderMove = determineIfSpiderShouldMove(deltaTime);

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
    enemy.setPosition(sf::Vector2f(TILE_WIDTH * spiderColumn, TILE_HEIGHT * spiderRow));
    spiderHealthBarShape.setPosition(enemy.getPosition() + sf::Vector2f(0, 1 + TILE_HEIGHT));
    spiderCurrentPathIndex += 1;
    if (spiderCurrentPathIndex > spiderFinalPathIndex)
    {
      spiderCurrentPathIndex = 0;
    }
  }
}

void rish::Game::prepareRender()
{
  // before rendering, the render target should be cleared and the view transformation set

  window.clear();
  window.setView(view);
}

void rish::Game::finishRender()
{
  // copies the rendered scene to the display

  window.display();
}

void rish::Game::renderTilemap()
{
  window.draw(mapVerts, &gfxTexture);
}

void rish::Game::renderTreasureChests()
{
  for (auto &chest : treasureChests)
  {
    auto &spr = getTreasureChestSprite(chest);
    window.draw(*(spr.get()));
  }
}

void rish::Game::renderMapItems()
{
  for (auto &item : mapItems)
  {
    auto &spr = getMapItemSprite(item);
    auto state = getMapItemState(item);
    if (state == FIELD_ITEM)
    {
      window.draw(*(spr.get()));
    }
  }
}

void rish::Game::renderEnemy()
{
  if (isSpiderAlive)
  {
    window.draw(enemy);
    window.draw(spiderHealthBarShape);
  }
  else
  {
    window.draw(enemy);
  }
}

void rish::Game::renderHero()
{
  if (isHeroAlive)
  {
    window.draw(sprite);
    window.draw(heroHealthBarShape);
  }
}

void rish::Game::renderInventoryUI()
{
  for (auto &item : heroItems)
  {
    auto &mapItem = getInventoryItemRef(item);
    auto &inventorySprite = getInventoryItemUISprite(item);
    auto &inventoryText = getInventoryItemUIText(item);
    auto state = getMapItemState(*mapItem);
    if (state == INVENTORY_ITEM)
    {
      window.draw(*(inventorySprite.get()));
      window.draw(*(inventoryText.get()));
    }
  }
}

void rish::Game::applyDamageToHero(float amount)
{
  float health = static_cast<float>(heroHealth);
  heroHealth = static_cast<int>(health - amount);
  if (heroHealth > heroMaxHealth)
  {
    heroHealth = heroMaxHealth;
  }
  else if (heroHealth <= 0)
  {
    heroHealth = 0;
    isHeroAlive = false;
  }
  float healthFill = 16 * (static_cast<float>(heroHealth) / static_cast<float>(heroMaxHealth));
  heroHealthBarShape.setSize(sf::Vector2f(healthFill, 2));
}

bool rish::Game::canUseHealthPotion()
{
  bool result = isHeroAlive && heroHealth < heroMaxHealth;
  std::cout << (result ? "Hero is hurt - can use" : "Hero is fully healed - no use") << std::endl;
  return result;
}

void rish::Game::useHealthPotion()
{
  std::cout << "Used Health Potion" << std::endl;
  float potionStrength = 0.34f;
  applyDamageToHero(-(static_cast<float>(heroMaxHealth) * potionStrength));
}

bool rish::Game::canUseManaPotion()
{
  return false;
}

void rish::Game::useManaPotion()
{
}

//

void itemDBAdd(ItemDB &db, TileId tileId, const ItemDBEntryName &name, ItemDBEntryKind kind, BoolFn canUse, VoidFn onUse)
{
  auto entry = std::make_tuple(
      name,
      kind,
      canUse,
      onUse);
  db.emplace(tileId, std::move(entry));
}

// convenience functions for working with item database objects

ItemDBEntry &getItemDBEntry(ItemDB &db, TileId tileId)
{
  if (db.count(tileId))
  {
    return db.at(tileId);
  }
  throw std::runtime_error("No item database entry for item: " + std::to_string(tileId));
}

ItemDBEntryName getItemDBEntryName(ItemDBEntry &entry)
{
  return std::get<0>(entry);
}

ItemDBEntryKind getItemDBEntryKind(ItemDBEntry &entry)
{
  return std::get<1>(entry);
}

BoolFn &getItemDBEntryCanUse(ItemDBEntry &entry)
{
  return std::get<2>(entry);
}

VoidFn &getItemDBEntryUse(ItemDBEntry &entry)
{
  return std::get<3>(entry);
}

// convenience functions for working with map item objects

MapItemState getMapItemState(const MapItem &item)
{
  MapItemState mapItemState = std::get<0>(item);
  return mapItemState;
}

void setMapItemState(MapItem &item, int mapItemState)
{
  std::get<0>(item) = mapItemState;
};

const MapItemSprite &getMapItemSprite(const MapItem &item)
{
  const MapItemSprite &mapItemSprite = std::get<1>(item);
  return mapItemSprite;
};

MapItemId getMapItemId(const MapItem &item)
{
  MapItemId mapItemId = std::get<2>(item);
  return mapItemId;
};

const MapItemCoordinate &getMapItemCoordinates(const MapItem &item)
{
  const MapItemCoordinate &mapItemCoordinates = std::get<3>(item);
  return mapItemCoordinates;
};

// convenience functions for working with treasure chest objects

void setTreasureChestKind(TreasureChest &chest, TreasureChestKind chestKind)
{
  std::get<0>(chest) = chestKind;
}

TreasureChestKind getTreasureChestKind(const TreasureChest &chest)
{
  int chestKind = std::get<0>(chest);
  return chestKind;
}

const TreasureChestCoordinate &getTreasureChestCoordinates(const TreasureChest &chest)
{
  const TreasureChestCoordinate &chestCoordinates = std::get<1>(chest);
  return chestCoordinates;
}

const TreasureChestContents &getTreasureChestContents(const TreasureChest &chest)
{
  const TreasureChestContents &chestContents = std::get<2>(chest);
  return chestContents;
}

TreasureChestContentItemId getTreasureChestContentsItemId(const TreasureChest &chest)
{
  const TreasureChestContents &chestContents = std::get<2>(chest);
  return chestContents.first;
}

TreasureChestContentItemQuantity getTreasureChestContentsItemQuantity(const TreasureChest &chest)
{
  const TreasureChestContents &chestContents = std::get<2>(chest);
  return chestContents.second;
}

const TreasureChestSprite &getTreasureChestSprite(const TreasureChest &chest)
{
  const TreasureChestSprite &chestSprite = std::get<3>(chest);
  return chestSprite;
}

// convenience functions for working with map item objects

const InventoryItemRef &getInventoryItemRef(const InventoryItem &item)
{
  const InventoryItemRef &itemRef = std::get<0>(item);
  return itemRef;
}

const InventoryItemUISprite &getInventoryItemUISprite(const InventoryItem &item)
{
  const InventoryItemUISprite &uiSprite = std::get<1>(item);
  return uiSprite;
}

const InventoryItemUIText &getInventoryItemUIText(const InventoryItem &item)
{
  const InventoryItemUIText &uiText = std::get<2>(item);
  return uiText;
}

//