#ifndef RISH_CONSTANTS_H
#define RISH_CONSTANTS_H

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

constexpr int MAP_ITEM_OBJ = 1000;
constexpr int TREASURE_CHEST_OBJ = 2000;

#endif
