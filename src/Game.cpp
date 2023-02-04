#include "Game.h"

#include "helpers.h"

using namespace rish;

int firstMap[] = {
    // map dimension
    7, 7,
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
    // BEGIN ENEMY DATA SECTION
    // NUM_ENEMIES
    1,
    // ENEMY_KIND, COLUMN, ROW
    SPIDER_ENEMY, 5, 1,
    //
};

Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "rish"),
               view(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT))
{
  setup();
  run();
}

void Game::setup()
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

  itemDBAdd(
      itemsDatabase,
      114,
      "Health Potion",
      0,
      std::bind(&Game::canUseHealthPotion, this),
      std::bind(&Game::useHealthPotion, this));

  itemDBAdd(
      itemsDatabase,
      116,
      "Mana Potion",
      0,
      std::bind(&Game::canUseManaPotion, this),
      std::bind(&Game::useManaPotion, this));

  setupHero();
  setupTilemap();
}

void Game::run()
{
  auto isDead = [&](std::unique_ptr<Enemy> &enemy)
  {
    const Enemy &enemyRef = *(enemy.get());
    return enemyRef.isDead();
  };

  while (window.isOpen())
  {
    processEvents();

    sf::Time deltaTime = clock.restart();

    hero.update(deltaTime);

    for (auto &enemy : enemies)
    {
      enemy->update(deltaTime);
    }

    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), isDead), enemies.end());

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

int Game::rollInt(int low, int high)
{
  std::uniform_int_distribution<int> dist(low, high);
  return dist(rng);
}

bool Game::chanceOf(float percentage)
{
  std::uniform_real_distribution<float> dist(1.0f, 100.0f);
  return dist(rng) < percentage;
}

void Game::setupHero()
{
  hero.setGame(this);
  setSpriteTile(hero.getSprite(), 97, gfxTexture);

  hero.setPosition(3, 3);
  hero.setDead(false);
  hero.setDamaged(false);
  hero.setMaxHealth(30);
}

void Game::addMapItem(TileId itemId, int itemColumn, int itemRow)
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

void Game::setTreasureChestSprite(TreasureChest &chest)
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

void Game::addMapTreasureChest(int chestKind, int chestColumn, int chestRow, int chestContent, int chestQty)
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

void Game::changeTilemapTile(int mapIndex, TileId tileId)
{
  level.setTile(mapIndex, tileId);
}

void Game::setupTilemap()
{
  level.setTexture(gfxTexture);
  level.loadFromDataArray(firstMap);
  int numObjs = level.getData().getNumObjects();
  for (int i = 0; i < numObjs; i++)
  {
    auto obj = level.getData().getObject(i);
    auto type = obj.getType();
    if (type == MAP_ITEM_OBJ)
    {
      int itemId = obj.getData().at(0);
      auto [itemColumn, itemRow] = obj.getCoordinates();
      addMapItem(itemId, itemColumn, itemRow);
    }
    else if (type == TREASURE_CHEST_OBJ)
    {
      int chestKind = obj.getData().at(0);
      int chestContent = obj.getData().at(1);
      int chestQty = obj.getData().at(2);
      auto [chestColumn, chestRow] = obj.getCoordinates();
      addMapTreasureChest(chestKind, chestColumn, chestRow, chestContent, chestQty);
    }
    else if (type == ENEMY_OBJ)
    {
      int enemyKind = obj.getData().at(0);
      auto [enemyColumn, enemyRow] = obj.getCoordinates();
      auto enemy = std::make_unique<Enemy>();
      enemy->setGame(this);
      enemy->setPosition(enemyColumn, enemyRow);
      if (enemyKind == SPIDER_ENEMY)
      {
        // TODO: the spider information needs to be stored in the map data
        setSpriteTile(enemy->getSprite(), 122, gfxTexture);
        enemy->setDead(false);
        enemy->setDamaged(false);
        enemy->setResting(false);
        enemy->setMaxHealth(15);
        enemy->setRateOfMovement(1.0f);
        enemy->setRateOfRest(2.0f);
        int path[] = {
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
        enemy->setMovementPath(8, path);
      }
      enemies.push_back(std::move(enemy));
    }
  }
}

void Game::setupInventoryItemUI(InventoryItem &inventoryItem, unsigned long slotNumber)
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

void Game::collectMapItem(MapItem *itemCollected)
{
  setMapItemState(*itemCollected, INVENTORY_ITEM);

  InventoryItem inventoryItem = std::make_tuple(
      std::shared_ptr<MapItem>(itemCollected, [](MapItem *) {}),
      std::make_unique<sf::Sprite>(),
      std::make_unique<sf::Text>());

  setupInventoryItemUI(inventoryItem, heroItems.size());

  heroItems.push_back(std::move(inventoryItem));
}

void Game::addItemToInventory(TileId tileId)
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

bool Game::isUsedInventoryItem(InventoryItem &item)
{
  auto &mapItem = getInventoryItemRef(item);
  auto state = getMapItemState(*mapItem);
  return state == USED_INVENTORY_ITEM;
}

void Game::updateInventoryUI()
{
  // erase the items which have been used
  heroItems.erase(
      std::remove_if(
          heroItems.begin(),
          heroItems.end(),
          std::bind(&Game::isUsedInventoryItem, this, std::placeholders::_1)),
      heroItems.end());
  // update the remaining items ui
  for (unsigned long i = 0; i < heroItems.size(); i++)
  {
    setupInventoryItemUI(heroItems.at(i), i);
  }
}

void Game::handleHeroAttackAction()
{
  for (auto &enemy : enemies)
  {
    auto [enemyColumn, enemyRow] = enemy->getPosition();
    auto [heroColumn, heroRow] = hero.getPosition();
    bool hitAbove = heroRow - 1 == enemyRow && heroColumn == enemyColumn;
    bool hitBelow = heroRow + 1 == enemyRow && heroColumn == enemyColumn;
    bool hitLeft = heroRow == enemyRow && heroColumn - 1 == enemyColumn;
    bool hitRight = heroRow == enemyRow && heroColumn + 1 == enemyColumn;
    bool hitEnemy = hitAbove || hitBelow || hitLeft || hitRight;
    if (hitEnemy)
    {
      enemy->setDamaged(true);
    }
  }
}

bool Game::handleOpenTreasureChestAction()
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

    auto [heroColumn, heroRow] = hero.getPosition();

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

bool Game::handleOpenDoorAction()
{
  std::vector<std::pair<TileId, int>> neighboringTileIds;
  auto [heroColumn, heroRow] = hero.getPosition();

  // if there is a tile above the hero, add to list of neighboring tile ids
  if (heroRow - 1 >= 0)
  {
    auto x = heroColumn;
    auto y = heroRow - 1;
    int index = x + (y * level.getData().getWidth());
    auto tileId = level.getData().getTile(index);
    neighboringTileIds.push_back(std::make_pair(tileId, index));
  }

  // if there is a tile below the hero, add to the list of neighboring tile ids
  if (heroRow + 1 < level.getData().getHeight())
  {
    auto x = heroColumn;
    auto y = heroRow + 1;
    int index = x + (y * level.getData().getWidth());
    auto tileId = level.getData().getTile(index);
    neighboringTileIds.push_back(std::make_pair(tileId, index));
  }

  // if there is a tile to the left of the hero, add to the list of neighboring tile ids
  if (heroColumn - 1 >= 0)
  {
    auto x = heroColumn - 1;
    auto y = heroRow;
    int index = x + (y * level.getData().getWidth());
    auto tileId = level.getData().getTile(index);
    neighboringTileIds.push_back(std::make_pair(tileId, index));
  }

  // if there is a tile to the right of the hero, add to the list of neighboring tile ids
  if (heroColumn + 1 < level.getData().getWidth())
  {
    auto x = heroColumn + 1;
    auto y = heroRow;
    int index = x + (y * level.getData().getWidth());
    auto tileId = level.getData().getTile(index);
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

bool Game::handleHeroInteractAction()
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

void Game::checkForHeroVsMapItemCollisions()
{
  auto [heroColumn, heroRow] = hero.getPosition();
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

void Game::setHeroPosition(int column, int row)
{
  hero.setPosition(column, row);
}

void Game::checkForHeroVsEnemyCollisionAt(int testColumn, int testRow)
{
  auto [heroColumn, heroRow] = hero.getPosition();
  if (heroColumn == testColumn && heroRow == testRow)
  {
    hero.setDamaged(true);
  }
}

void Game::checkForHeroVsEnemyCollisions(int lastColumn, int lastRow)
{
  for (auto &enemy : enemies)
  {
    if (enemy->isDead())
    {
      continue;
    }
    auto [enemyColumn, enemyRow] = enemy->getPosition();
    auto [heroColumn, heroRow] = hero.getPosition();
    if (heroColumn == enemyColumn && heroRow == enemyRow)
    {
      hero.setDamaged(true);
      setHeroPosition(lastColumn, lastRow);
    }
  }
}

void Game::checkForHeroVsTreasureChestCollisions(int lastColumn, int lastRow)
{
  auto [heroColumn, heroRow] = hero.getPosition();
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

void Game::activateInventoryItemSlot(unsigned long slotNumber)
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

bool Game::handleInventoryHotkeys(sf::Event &event)
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

void Game::handleHeroKeyPressedEvent(sf::Event &event)
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
  auto [heroColumn, heroRow] = hero.getPosition();
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
    if (heroY < level.getData().getHeight() - 1)
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
    if (heroX < level.getData().getWidth() - 1)
    {
      heroX += 1;
      didHeroMove = true;
    }
  }

  if (didHeroMove)
  {
    int tileId = level.getData().getTile(heroX, heroY);
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
        hero.setDamaged(true);
        std::cout << "hero was damaged by hazard tile" << std::endl;
      }
    }
  }
}

void Game::processEvents()
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

void Game::spawnRandomItemOnMapAt(int column, int row)
{
  std::cout << "spawn random item at " << std::to_string(column) << "," << std::to_string(row) << std::endl;
  int index = rollInt(0, itemsDatabase.size() - 1);
  auto it = itemsDatabase.begin();
  auto tileId = std::next(it, index)->first;
  addMapItem(tileId, column, row);
}

void Game::prepareRender()
{
  // before rendering, the render target should be cleared and the view transformation set

  window.clear();
  window.setView(view);
}

void Game::finishRender()
{
  // copies the rendered scene to the display

  window.display();
}

void Game::renderTilemap()
{
  level.renderTilemap(window);
}

void Game::renderTreasureChests()
{
  for (auto &chest : treasureChests)
  {
    auto &spr = getTreasureChestSprite(chest);
    window.draw(*(spr.get()));
  }
}

void Game::renderMapItems()
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

void Game::renderEnemy()
{
  for (auto &enemy : enemies)
  {
    enemy->render(window);
  }
}

void Game::renderHero()
{
  hero.render(window);
}

void Game::renderInventoryUI()
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

void Game::applyDamageToHero(float amount)
{
  hero.applyDamage(amount);
}

bool Game::canUseHealthPotion()
{
  bool result = !hero.isDead() && hero.getHealth() < hero.getMaxHealth();
  std::cout << (result ? "Hero is hurt - can use" : "Hero is fully healed - no use") << std::endl;
  return result;
}

void Game::useHealthPotion()
{
  std::cout << "Used Health Potion" << std::endl;
  float potionStrength = 0.34f;
  applyDamageToHero(-(static_cast<float>(hero.getMaxHealth()) * potionStrength));
}

bool Game::canUseManaPotion()
{
  return false;
}

void Game::useManaPotion()
{
}

void Game::setSpriteTile(sf::Sprite &sprite, TileId tileId, sf::Texture &tilesheetTexture)
{
  int srcX = tileId % numTilesAcrossTexture;
  int srcY = tileId / numTilesAcrossTexture;
  sprite.setTexture(tilesheetTexture);
  sprite.setTextureRect(sf::IntRect(srcX * TILE_WIDTH, srcY * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));
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