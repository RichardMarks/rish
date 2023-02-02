#ifndef RISH_GAME_H
#define RISH_GAME_H

#include "constants.h"
#include "types.h"
#include "Level.h"
#include "Enemy.h"

// typedefs

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
    sf::Sprite heroSprite;
    std::vector<InventoryItem> heroItems;

    int heroColumn = 3;
    int heroRow = 3;
    bool isHeroAlive = true;

    int heroHealth = 30;
    int heroMaxHealth = 30;

    bool heroWasDamaged = false;

    // MAP VARS

    Level level;
    std::vector<MapItem> mapItems;
    std::vector<TreasureChest> treasureChests;

    std::set<int> walkableTiles;
    std::set<int> hazardTiles;

    // ENEMY VARS
    Enemy spider;

    // UI VARS

    sf::RectangleShape heroHealthBarShape;

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
    void checkForHeroVsEnemyCollisionAt(int testColumn, int testRow);
    void checkForHeroVsEnemyCollisions(int lastColumn, int lastRow);
    void checkForHeroVsTreasureChestCollisions(int lastColumn, int lastRow);
    void activateInventoryItemSlot(unsigned long slotNumber);
    bool handleInventoryHotkeys(sf::Event &event);
    void handleHeroKeyPressedEvent(sf::Event &event);
    void processEvents();

    void handleHeroWasDamagedEvent();
    void spawnRandomItemOnMapAt(int column, int row);

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

    void setSpriteTile(sf::Sprite &sprite, TileId tileId, sf::Texture &tilesheetTexture);
  };
}

#endif
