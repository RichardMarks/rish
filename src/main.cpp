#include <iostream>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <random>
#include <SFML/Graphics.hpp>

// constants

constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1440;

constexpr int SCREEN_WIDTH = 320;
constexpr int SCREEN_HEIGHT = 240;

constexpr int TILE_WIDTH = 16;
constexpr int TILE_HEIGHT = 16;

constexpr int FIELD_ITEM = 1;
constexpr int INVENTORY_ITEM = 2;
constexpr int USED_INVENTORY_ITEM = 3;

constexpr int ITEM_DATA_STRIDE = 3;
constexpr int TREASURE_CHEST_DATA_STRIDE = 5;

constexpr int CHEST_UNLOCKED_CLOSED = 0;
constexpr int CHEST_UNLOCKED_OPEN = 1;
constexpr int CHEST_LOCKED_CLOSED = 2;

// typedefs

typedef std::function<bool()> BoolFn;
typedef std::function<void()> VoidFn;

typedef int MapItemState;
typedef int MapItemId;
typedef std::unique_ptr<sf::Sprite> MapItemSprite;
typedef std::pair<int, int> MapItemCoordinate;
typedef std::tuple<MapItemState, MapItemSprite, MapItemId, MapItemCoordinate> MapItem;

typedef int TreasureChestKind;
typedef std::unique_ptr<sf::Sprite> TreasureChestSprite;
typedef std::pair<int, int> TreasureChestCoordinate;
typedef int TreasureChestContentItemId;
typedef int TreasureChestContentItemQuantity;
typedef std::pair<TreasureChestContentItemId, TreasureChestContentItemQuantity> TreasureChestContents;
typedef std::tuple<TreasureChestKind, TreasureChestCoordinate, TreasureChestContents, TreasureChestSprite> TreasureChest;

typedef std::shared_ptr<MapItem> InventoryItemRef;
typedef std::unique_ptr<sf::Sprite> InventoryItemUISprite;
typedef std::unique_ptr<sf::Text> InventoryItemUIText;
typedef std::tuple<InventoryItemRef, InventoryItemUISprite, InventoryItemUIText> InventoryItem;

typedef int TileId;

/*

Item

  map: {
    [TileId]: (
      Name,
      Kind,
      CanUse,
      Use,
    )
  }

*/

typedef std::string ItemDBEntryName;
typedef int ItemDBEntryKind;
typedef std::tuple<ItemDBEntryName, ItemDBEntryKind, BoolFn, VoidFn> ItemDBEntry;
typedef std::unordered_map<TileId, ItemDBEntry> ItemDB;

void itemDBAdd(ItemDB &db, TileId tileId, const ItemDBEntryName &name, ItemDBEntryKind kind, BoolFn canUse, VoidFn onUse);
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

ItemDBEntry &getItemDBEntry(ItemDB &db, TileId tileId);
ItemDBEntry &getItemDBEntry(ItemDB &db, TileId tileId)
{
  if (db.count(tileId))
  {
    return db.at(tileId);
  }
  throw std::runtime_error("No item database entry for item: " + std::to_string(tileId));
}

ItemDBEntryName getItemDBEntryName(ItemDBEntry &entry);
ItemDBEntryName getItemDBEntryName(ItemDBEntry &entry)
{
  return std::get<0>(entry);
}

ItemDBEntryKind getItemDBEntryKind(ItemDBEntry &entry);
ItemDBEntryKind getItemDBEntryKind(ItemDBEntry &entry)
{
  return std::get<1>(entry);
}

BoolFn &getItemDBEntryCanUse(ItemDBEntry &entry);
BoolFn &getItemDBEntryCanUse(ItemDBEntry &entry)
{
  return std::get<2>(entry);
}

VoidFn &getItemDBEntryUse(ItemDBEntry &entry);
VoidFn &getItemDBEntryUse(ItemDBEntry &entry)
{
  return std::get<3>(entry);
}

// convenience functions for working with map item objects

MapItemState getMapItemState(const MapItem &item);
MapItemState getMapItemState(const MapItem &item)
{
  MapItemState mapItemState = std::get<0>(item);
  return mapItemState;
}

void setMapItemState(MapItem &item, int mapItemState);
void setMapItemState(MapItem &item, int mapItemState)
{
  std::get<0>(item) = mapItemState;
};

const MapItemSprite &getMapItemSprite(const MapItem &item);
const MapItemSprite &getMapItemSprite(const MapItem &item)
{
  const MapItemSprite &mapItemSprite = std::get<1>(item);
  return mapItemSprite;
};

MapItemId getMapItemId(const MapItem &item);
MapItemId getMapItemId(const MapItem &item)
{
  MapItemId mapItemId = std::get<2>(item);
  return mapItemId;
};

const MapItemCoordinate &getMapItemCoordinates(const MapItem &item);
const MapItemCoordinate &getMapItemCoordinates(const MapItem &item)
{
  const MapItemCoordinate &mapItemCoordinates = std::get<3>(item);
  return mapItemCoordinates;
};

// convenience functions for working with treasure chest objects

void setTreasureChestKind(TreasureChest &chest, TreasureChestKind chestKind);
void setTreasureChestKind(TreasureChest &chest, TreasureChestKind chestKind)
{
  std::get<0>(chest) = chestKind;
}

TreasureChestKind getTreasureChestKind(const TreasureChest &chest);
TreasureChestKind getTreasureChestKind(const TreasureChest &chest)
{
  int chestKind = std::get<0>(chest);
  return chestKind;
}

const TreasureChestCoordinate &getTreasureChestCoordinates(const TreasureChest &chest);
const TreasureChestCoordinate &getTreasureChestCoordinates(const TreasureChest &chest)
{
  const TreasureChestCoordinate &chestCoordinates = std::get<1>(chest);
  return chestCoordinates;
}

const TreasureChestContents &getTreasureChestContents(const TreasureChest &chest);
const TreasureChestContents &getTreasureChestContents(const TreasureChest &chest)
{
  const TreasureChestContents &chestContents = std::get<2>(chest);
  return chestContents;
}

TreasureChestContentItemId getTreasureChestContentsItemId(const TreasureChest &chest);
TreasureChestContentItemId getTreasureChestContentsItemId(const TreasureChest &chest)
{
  const TreasureChestContents &chestContents = std::get<2>(chest);
  return chestContents.first;
}

TreasureChestContentItemQuantity getTreasureChestContentsItemQuantity(const TreasureChest &chest);
TreasureChestContentItemQuantity getTreasureChestContentsItemQuantity(const TreasureChest &chest)
{
  const TreasureChestContents &chestContents = std::get<2>(chest);
  return chestContents.second;
}

const TreasureChestSprite &getTreasureChestSprite(const TreasureChest &chest);
const TreasureChestSprite &getTreasureChestSprite(const TreasureChest &chest)
{
  const TreasureChestSprite &chestSprite = std::get<3>(chest);
  return chestSprite;
}

// convenience functions for working with map item objects

const InventoryItemRef &getInventoryItemRef(const InventoryItem &item);
const InventoryItemRef &getInventoryItemRef(const InventoryItem &item)
{
  const InventoryItemRef &itemRef = std::get<0>(item);
  return itemRef;
}

const InventoryItemUISprite &getInventoryItemUISprite(const InventoryItem &item);
const InventoryItemUISprite &getInventoryItemUISprite(const InventoryItem &item)
{
  const InventoryItemUISprite &uiSprite = std::get<1>(item);
  return uiSprite;
}

const InventoryItemUIText &getInventoryItemUIText(const InventoryItem &item);
const InventoryItemUIText &getInventoryItemUIText(const InventoryItem &item)
{
  const InventoryItemUIText &uiText = std::get<2>(item);
  return uiText;
}

int main()
{
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "rish");
  sf::View view(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

  // random number generation

  std::seed_seq seed{
      std::random_device{}(),
      static_cast<unsigned int>(::time(nullptr)),
      static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())};
  std::mt19937 rng(seed);

  auto rollInt = [&rng](int low, int high)
  {
    std::uniform_int_distribution<int> dist(low, high);
    return dist(rng);
  };

  auto chanceOf = [&rng](float percentage)
  {
    std::uniform_real_distribution<float> dist(1.0f, 100.0f);
    return dist(rng) < percentage;
  };

  // hotkey mapping vars

  std::unordered_map<sf::Keyboard::Key, unsigned long> inventorySlotKeys;

  inventorySlotKeys.emplace(sf::Keyboard::Num1, 0);
  inventorySlotKeys.emplace(sf::Keyboard::Num2, 1);
  inventorySlotKeys.emplace(sf::Keyboard::Num3, 2);
  inventorySlotKeys.emplace(sf::Keyboard::Num4, 3);
  inventorySlotKeys.emplace(sf::Keyboard::Num5, 4);
  inventorySlotKeys.emplace(sf::Keyboard::Num6, 5);
  inventorySlotKeys.emplace(sf::Keyboard::Num7, 6);
  inventorySlotKeys.emplace(sf::Keyboard::Num8, 7);
  inventorySlotKeys.emplace(sf::Keyboard::Num9, 8);

  // asset storage variables section

  sf::Texture gfxTexture;
  sf::Font font;

  auto loadAssets = [&]()
  {
    if (!gfxTexture.loadFromFile("assets/gfx.png"))
    {
      throw std::runtime_error("Unable to load assets/gfx.png");
    }

    if (!font.loadFromFile("assets/Px437_DOS-V_re_JPN16.ttf"))
    {
      throw std::runtime_error("Unable to load assets/Px437_DOS-V_re_JPN16.ttf");
    }
  };

  loadAssets();

  // variables which may depend on assets should go here

  int numTilesAcrossTexture = gfxTexture.getSize().x / TILE_WIDTH;

  sf::Clock clock;
  sf::Time elapsed;

  ItemDB itemsDatabase;

  // HERO VARS
  sf::Sprite sprite;
  std::vector<InventoryItem> heroItems;

  int heroColumn = 3;
  int heroRow = 3;
  bool isHeroAlive = true;

  int heroHealth = 30;
  int heroMaxHealth = 30;

  bool heroWasDamaged = false;

  // MAP VARS
  sf::VertexArray mapVerts;

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
      // BEGIN TREASURE CHESTS DATA SECTION
      // NUM_CHESTS
      3,
      // CHEST_KIND, COLUMN, ROW, OPTIONAL_CONTENTS, OPTIONAL_QTY
      CHEST_UNLOCKED_CLOSED, 1, 3, 0, 0,
      CHEST_UNLOCKED_CLOSED, 1, 5, 0, 0,
      CHEST_UNLOCKED_CLOSED, 3, 1, 116, 4,
      //
  };

  int mapItemDataIndex = mapWidth * mapHeight;
  std::vector<MapItem> mapItems;
  int numMapItems = map[mapItemDataIndex];

  int treasureChestDataIndex = mapItemDataIndex + 1;
  treasureChestDataIndex += (ITEM_DATA_STRIDE * numMapItems);
  int numTreasureChests = map[treasureChestDataIndex];
  treasureChestDataIndex += 1;

  std::vector<TreasureChest> treasureChests;

  // ENEMY VARS
  sf::Sprite enemy;

  int spiderColumn = 5;
  int spiderRow = 1;
  bool isSpiderAlive = true;

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

  int spiderHealth = 15;
  int spiderMaxHealth = 15;

  bool spiderWasDamaged = false;
  bool spiderIsResting = false;

  float spiderRestTime = 0.0f;
  float spiderTimeToRest = 2.0f;

  // UI VARS

  sf::RectangleShape heroHealthBarShape;
  sf::RectangleShape spiderHealthBarShape;

  // local lambda functions

  auto setupHero = [&]()
  {
    sprite.setTexture(gfxTexture);
    sprite.setTextureRect(sf::IntRect(TILE_WIDTH, 8 * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

    sprite.setPosition(sf::Vector2f(TILE_WIDTH * heroColumn, TILE_HEIGHT * heroRow));

    heroHealthBarShape.setFillColor(sf::Color::Green);
    heroHealthBarShape.setSize(sf::Vector2f(16, 2));
    heroHealthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, 1 + TILE_HEIGHT));
  };

  auto setupEnemy = [&]()
  {
    enemy.setTexture(gfxTexture);
    enemy.setTextureRect(sf::IntRect(2 * TILE_WIDTH, 10 * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

    enemy.setPosition(sf::Vector2f(TILE_WIDTH * spiderColumn, TILE_HEIGHT * spiderRow));

    spiderHealthBarShape.setFillColor(sf::Color::Red);
    spiderHealthBarShape.setSize(sf::Vector2f(16, 2));
    spiderHealthBarShape.setPosition(enemy.getPosition() + sf::Vector2f(0, 1 + TILE_HEIGHT));
  };

  auto addMapItem = [&](TileId itemId, int itemColumn, int itemRow)
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
  };

  auto setTreasureChestSprite = [&](TreasureChest &chest)
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
  };

  auto addMapTreasureChest = [&](
                                 int chestKind,
                                 int chestColumn,
                                 int chestRow,
                                 int chestContent,
                                 int chestQty)
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
  };

  auto setupTilemap = [&]()
  {
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
      quad[0].position = sf::Vector2f(x * TILE_WIDTH, y * TILE_HEIGHT);
      quad[1].position = sf::Vector2f((x + 1) * TILE_WIDTH, y * TILE_HEIGHT);
      quad[2].position = sf::Vector2f((x + 1) * TILE_WIDTH, (y + 1) * TILE_HEIGHT);
      quad[3].position = sf::Vector2f(x * TILE_WIDTH, (y + 1) * TILE_HEIGHT);
      quad[0].texCoords = sf::Vector2f(tileU * TILE_WIDTH, tileV * TILE_HEIGHT);
      quad[1].texCoords = sf::Vector2f((tileU + 1) * TILE_WIDTH, tileV * TILE_HEIGHT);
      quad[2].texCoords = sf::Vector2f((tileU + 1) * TILE_WIDTH, (tileV + 1) * TILE_HEIGHT);
      quad[3].texCoords = sf::Vector2f(tileU * TILE_WIDTH, (tileV + 1) * TILE_HEIGHT);
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
  };

  auto setupInventoryItemUI = [&](InventoryItem &inventoryItem, unsigned long slotNumber)
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
  };

  auto collectMapItem = [&](MapItem *itemCollected)
  {
    setMapItemState(*itemCollected, INVENTORY_ITEM);

    InventoryItem inventoryItem = std::make_tuple(
        std::shared_ptr<MapItem>(itemCollected, [](MapItem *) {}),
        std::make_unique<sf::Sprite>(),
        std::make_unique<sf::Text>());

    setupInventoryItemUI(inventoryItem, heroItems.size());

    heroItems.push_back(std::move(inventoryItem));
  };

  auto isUsedInventoryItem = [](InventoryItem &item)
  {
    auto &mapItem = getInventoryItemRef(item);
    auto state = getMapItemState(*mapItem);
    return state == USED_INVENTORY_ITEM;
  };

  auto updateInventoryUI = [&]()
  {
    // erase the items which have been used
    heroItems.erase(
        std::remove_if(
            heroItems.begin(),
            heroItems.end(),
            isUsedInventoryItem),
        heroItems.end());
    // update the remaining items ui
    for (unsigned long i = 0; i < heroItems.size(); i++)
    {
      setupInventoryItemUI(heroItems.at(i), i);
    }
  };

  auto handleHeroAttackAction = [&]()
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
  };

  auto handleHeroInteractAction = [&]()
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
      // TODO: handle what happens when the chest opens
      return true;
    }

    return false;
  };

  auto checkForHeroVsMapItemCollisions = [&]()
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
  };

  auto setHeroPosition = [&](int column, int row)
  {
    // update the hero world position
    heroColumn = column;
    heroRow = row;

    // update the hero sprite position
    sprite.setPosition(sf::Vector2f(TILE_WIDTH * heroColumn, TILE_HEIGHT * heroRow));

    // update the hero health bar position
    heroHealthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, TILE_HEIGHT));
  };

  auto checkForHeroVsEnemyCollisions = [&](int lastColumn, int lastRow)
  {
    if (heroColumn == spiderColumn && heroRow == spiderRow)
    {
      heroWasDamaged = true;
      setHeroPosition(lastColumn, lastRow);
    }
  };

  auto checkForHeroVsTreasureChestCollisions = [&](int lastColumn, int lastRow)
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
  };

  auto activateInventoryItemSlot = [&](unsigned long slotNumber)
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
  };

  auto handleInventoryHotkeys = [&](sf::Event &event)
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
  };

  auto handleHeroKeyPressedEvent = [&](sf::Event &event)
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
      if (tileId == 48)
      {
        int lastColumn = heroColumn;
        int lastRow = heroRow;
        setHeroPosition(heroX, heroY);
        checkForHeroVsTreasureChestCollisions(lastColumn, lastRow);
        checkForHeroVsEnemyCollisions(lastColumn, lastRow);
        checkForHeroVsMapItemCollisions();
      }
    }
  };

  auto processEvents = [&]()
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
  };

  auto determineIfSpiderShouldMove = [&](sf::Time &deltaTime)
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
  };

  auto handleHeroWasDamagedEvent = [&]()
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
  };

  auto spawnRandomItemOnMapAt = [&](int column, int row)
  {
    std::cout << "spawn random item at " << std::to_string(column) << "," << std::to_string(row) << std::endl;
    int index = rollInt(0, itemsDatabase.size() - 1);
    auto it = itemsDatabase.begin();
    auto tileId = std::next(it, index)->first;
    addMapItem(tileId, column, row);
  };

  auto handleSpiderWasDamagedEvent = [&]()
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
  };

  auto handleSpiderMovement = [&](sf::Time &deltaTime)
  {
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
  };

  // before rendering, the render target should be cleared and the view transformation set

  auto prepareRender = [&]()
  {
    window.clear();
    window.setView(view);
  };

  // copies the rendered scene to the display

  auto finishRender = [&]()
  {
    window.display();
  };

  // render the tilemap

  auto renderTilemap = [&]()
  {
    window.draw(mapVerts, &gfxTexture);
  };

  // render the treasure chests

  auto renderTreasureChests = [&]()
  {
    for (auto &chest : treasureChests)
    {
      auto &spr = getTreasureChestSprite(chest);
      window.draw(*(spr.get()));
    }
  };

  // render the map items

  auto renderMapItems = [&]()
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
  };

  // render the enemy

  auto renderEnemy = [&]()
  {
    if (isSpiderAlive)
    {
      window.draw(enemy);
      window.draw(spiderHealthBarShape);
    }
  };

  // render the hero

  auto renderHero = [&]()
  {
    if (isHeroAlive)
    {
      window.draw(sprite);
      window.draw(heroHealthBarShape);
    }
  };

  // render the inventory ui

  auto renderInventoryUI = [&]()
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
  };

  auto applyDamageToHero = [&](float amount)
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
  };

  auto canUseHealthPotion = [&]()
  {
    bool result = isHeroAlive && heroHealth < heroMaxHealth;
    std::cout << (result ? "Hero is hurt - can use" : "Hero is fully healed - no use") << std::endl;
    return result;
  };

  auto useHealthPotion = [&]()
  {
    std::cout << "Used Health Potion" << std::endl;
    float potionStrength = 0.34f;
    applyDamageToHero(-(static_cast<float>(heroMaxHealth) * potionStrength));
  };

  auto canUseManaPotion = [&]()
  {
    return false;
  };

  auto useManaPotion = [&]() {

  };

  itemDBAdd(itemsDatabase, 114, "Health Potion", 0, canUseHealthPotion, useHealthPotion);
  itemDBAdd(itemsDatabase, 116, "Mana Potion", 0, canUseManaPotion, useManaPotion);

  setupHero();
  setupEnemy();
  setupTilemap();

  // main loop

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

  return 0;
}