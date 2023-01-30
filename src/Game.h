#ifndef RISH_GAME_H
#define RISH_GAME_H
#include <iostream>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <random>
#include <set>
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

constexpr int CLOSED_DOOR_TILE_ID = 45;
constexpr int OPENED_DOOR_TILE_ID = 21;

extern int mapWidth;
extern int mapHeight;
extern int map[];

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

extern void itemDBAdd(ItemDB &db, TileId tileId, const ItemDBEntryName &name, ItemDBEntryKind kind, BoolFn canUse, VoidFn onUse);
extern ItemDBEntry &getItemDBEntry(ItemDB &db, TileId tileId);
extern ItemDBEntryName getItemDBEntryName(ItemDBEntry &entry);
extern ItemDBEntryKind getItemDBEntryKind(ItemDBEntry &entry);
extern BoolFn &getItemDBEntryCanUse(ItemDBEntry &entry);
extern VoidFn &getItemDBEntryUse(ItemDBEntry &entry);
extern MapItemState getMapItemState(const MapItem &item);
extern void setMapItemState(MapItem &item, int mapItemState);
extern const MapItemSprite &getMapItemSprite(const MapItem &item);
extern MapItemId getMapItemId(const MapItem &item);
extern const MapItemCoordinate &getMapItemCoordinates(const MapItem &item);
extern void setTreasureChestKind(TreasureChest &chest, TreasureChestKind chestKind);
extern TreasureChestKind getTreasureChestKind(const TreasureChest &chest);
extern const TreasureChestCoordinate &getTreasureChestCoordinates(const TreasureChest &chest);
extern const TreasureChestContents &getTreasureChestContents(const TreasureChest &chest);
extern TreasureChestContentItemId getTreasureChestContentsItemId(const TreasureChest &chest);
extern TreasureChestContentItemQuantity getTreasureChestContentsItemQuantity(const TreasureChest &chest);
extern const TreasureChestSprite &getTreasureChestSprite(const TreasureChest &chest);
extern const InventoryItemRef &getInventoryItemRef(const InventoryItem &item);
extern const InventoryItemUISprite &getInventoryItemUISprite(const InventoryItem &item);
extern const InventoryItemUIText &getInventoryItemUIText(const InventoryItem &item);

namespace rish
{
  class Game
  {
  private:
    std::mt19937 rng;
    sf::RenderWindow window;
    sf::View view;
    sf::Clock clock;
    sf::Time elapsed;
    sf::Texture gfxTexture;
    sf::Font font;

    std::unordered_map<sf::Keyboard::Key, unsigned long> inventorySlotKeys;

    int numTilesAcrossTexture;

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
    std::set<int> walkableTiles;
    std::set<int> hazardTiles;
    std::vector<MapItem> mapItems;
    std::vector<TreasureChest> treasureChests;
    int mapItemDataIndex;
    int numMapItems;
    int treasureChestDataIndex;
    int numTreasureChests;

    // ENEMY VARS
    sf::Sprite enemy;
    int spiderColumn;
    int spiderRow;
    bool isSpiderAlive;
    float spiderTime;
    float spiderTimeToMove;
    std::vector<int> spiderPath;
    int spiderCurrentPathIndex;
    int spiderFinalPathIndex;
    int spiderHealth;
    int spiderMaxHealth;
    bool spiderWasDamaged;
    bool spiderIsResting;
    float spiderRestTime;
    float spiderTimeToRest;

    // UI VARS

    sf::RectangleShape heroHealthBarShape;
    sf::RectangleShape spiderHealthBarShape;

  public:
    Game();
    void setup();
    void run();

    int rollInt(int low, int high);
    bool chanceOf(float percentage);

    void setupHero();
    void setupEnemy();

    void addMapItem(TileId itemId, int itemColumn, int itemRow);
    void setTreasureChestSprite(TreasureChest &chest);
    void addMapTreasureChest(int chestKind, int chestColumn, int chestRow, int chestContent, int chestQty);
    void setTilemapTileVertices(int mapIndex);
    void changeTilemapTile(int mapIndex, TileId tileId);
    void setupTilemap();
    void setupInventoryItemUI(InventoryItem &inventoryItem, unsigned long slotNumber);
    void collectMapItem(MapItem *itemCollected);
    void addItemToInventory(TileId tileId);
    bool isUsedInventoryItem(InventoryItem &item);
    void updateInventoryUI();
    void handleHeroAttackAction();
    bool handleOpenTreasureChestAction();
    bool handleOpenDoorAction();
    bool handleHeroInteractAction();
    void checkForHeroVsMapItemCollisions();
    void setHeroPosition(int column, int row);
    void checkForHeroVsEnemyCollisions(int lastColumn, int lastRow);
    void checkForHeroVsTreasureChestCollisions(int lastColumn, int lastRow);
    void activateInventoryItemSlot(unsigned long slotNumber);
    bool handleInventoryHotkeys(sf::Event &event);
    void handleHeroKeyPressedEvent(sf::Event &event);
    void processEvents();
    bool determineIfSpiderShouldMove(sf::Time &deltaTime);
    void handleHeroWasDamagedEvent();
    void spawnRandomItemOnMapAt(int column, int row);
    void handleSpiderWasDamagedEvent();
    void handleSpiderMovement(sf::Time &deltaTime);
    void prepareRender();
    void finishRender();
    void renderTilemap();
    void renderTreasureChests();
    void renderMapItems();
    void renderEnemy();
    void renderHero();
    void renderInventoryUI();
    void applyDamageToHero(float amount);

    bool canUseHealthPotion();
    void useHealthPotion();
    bool canUseManaPotion();
    void useManaPotion();
  };
}

#endif
