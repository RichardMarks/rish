#ifndef RISH_HELPERS_H
#define RISH_HELPERS_H

#include "types.h"

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
extern void setMapItemCoordinates(MapItem &item, int column, int row);
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

#endif
