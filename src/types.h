#ifndef RISH_TYPES_H
#define RISH_TYPES_H

#include <iostream>
#include <sstream>
#include <random>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <functional>
#include <chrono>
#include <SFML/Graphics.hpp>

typedef int *LevelSourceArray;

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

#endif
