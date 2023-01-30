#include "Level.h"

#include "constants.h"
#include "helpers.h"

using namespace rish;

Level::Level(sf::Texture &mapTexture)
{
  texture = mapTexture;
  numTilesAcrossTexture = texture.getSize().x / TILE_WIDTH;
}

LevelData &Level::getData()
{
  return data;
}

void Level::loadFromDataArray(int dataArray[])
{
  std::cout << "Loading tilemap data..." << std::endl;
  int mapWidth = dataArray[0];
  int mapHeight = dataArray[1];
  int mapDataSize = mapWidth * mapHeight;
  std::cout << "map is " << mapWidth << "x" << mapHeight << " expecting " << mapDataSize << " tile ids..." << std::endl;
  int *mapData = dataArray;
  mapData++;
  mapData++;

  data.resize(mapWidth, mapHeight);

  vertexArray.setPrimitiveType(sf::PrimitiveType::Quads);
  vertexArray.resize(4 * mapDataSize);

  for (int mapIndex = 0; mapIndex < mapDataSize; mapIndex++)
  {
    int tileId = mapData[mapIndex];
    int x = mapIndex % mapWidth;
    int y = mapIndex / mapWidth;
    int tileU = tileId % numTilesAcrossTexture;
    int tileV = tileId / numTilesAcrossTexture;
    sf::Vertex *quad = &vertexArray[4 * mapIndex];
    quad[0].position = sf::Vector2f(x * TILE_WIDTH, y * TILE_HEIGHT);
    quad[1].position = sf::Vector2f((x + 1) * TILE_WIDTH, y * TILE_HEIGHT);
    quad[2].position = sf::Vector2f((x + 1) * TILE_WIDTH, (y + 1) * TILE_HEIGHT);
    quad[3].position = sf::Vector2f(x * TILE_WIDTH, (y + 1) * TILE_HEIGHT);
    quad[0].texCoords = sf::Vector2f(tileU * TILE_WIDTH, tileV * TILE_HEIGHT);
    quad[1].texCoords = sf::Vector2f((tileU + 1) * TILE_WIDTH, tileV * TILE_HEIGHT);
    quad[2].texCoords = sf::Vector2f((tileU + 1) * TILE_WIDTH, (tileV + 1) * TILE_HEIGHT);
    quad[3].texCoords = sf::Vector2f(tileU * TILE_WIDTH, (tileV + 1) * TILE_HEIGHT);
    std::cout << tileId << " ";
    if (x == mapWidth - 1)
    {
      std::cout << std::endl;
    }
  }

  std::cout << "Loading map item data..." << std::endl;

  int *mapItemData = mapData;
  mapItemData += mapDataSize;
  int numMapItems = mapItemData[0];
  mapItemData++;

  std::cout << "There are " << numMapItems << " items on the map" << std::endl;
  for (int i = 0; i < numMapItems; i++)
  {
    int itemId = mapItemData[(i * ITEM_DATA_STRIDE) + 0];
    int itemColumn = mapItemData[(i * ITEM_DATA_STRIDE) + 1];
    int itemRow = mapItemData[(i * ITEM_DATA_STRIDE) + 2];
    std::cout << "[" << i << "] " << itemId << " @ " << itemColumn << ", " << itemRow << std::endl;
    LevelObject obj;
    obj.setType(MAP_ITEM_OBJ);
    obj.setPosition(itemColumn, itemRow);
    obj.getData().push_back(itemId);
    obj.getData().push_back(FIELD_ITEM);
    data.addObject(obj);
  }

  std::cout << "Loading treasure chest data..." << std::endl;

  int *mapTreasureChestData = mapItemData;
  mapTreasureChestData += numMapItems * ITEM_DATA_STRIDE;
  int numTreasureChests = mapTreasureChestData[0];
  mapTreasureChestData++;
  std::cout << "There are " << numTreasureChests << " chests on the map" << std::endl;
  for (int i = 0; i < numTreasureChests; i++)
  {
    int base = (i * TREASURE_CHEST_DATA_STRIDE);
    int chestKind = mapTreasureChestData[base + 0];
    int chestColumn = mapTreasureChestData[base + 1];
    int chestRow = mapTreasureChestData[base + 2];
    int chestContent = mapTreasureChestData[base + 3];
    int chestQty = mapTreasureChestData[base + 4];
    std::cout << "[" << i << "] " << chestKind << " @ " << chestColumn << ", " << chestRow << " {" << chestContent << "x" << chestQty << "}" << std::endl;
    LevelObject obj;
    obj.setType(TREASURE_CHEST_OBJ);
    obj.setPosition(chestColumn, chestRow);
    obj.getData().push_back(chestKind);
    obj.getData().push_back(chestContent);
    obj.getData().push_back(chestQty);
    data.addObject(obj);
  }

  std::cout << "load complete" << std::endl;
}

void Level::renderTilemap(sf::RenderWindow &window)
{
  window.draw(vertexArray, &texture);
}

// level object
LevelObject::LevelObject() : type(0), column(0), row(0), data()
{
}

LevelObject::LevelObject(const LevelObject &other)
{
  type = other.type;
  column = other.column;
  row = other.row;
  data.clear();
  for (auto n : other.data)
  {
    data.push_back(n);
  }
}

LevelObject::LevelObject(LevelObject &&other)
{
  type = std::move(other.type);
  column = std::move(other.column);
  row = std::move(other.row);
  data = std::move(other.data);
}

LevelObject &LevelObject::operator=(const LevelObject &other)
{
  type = other.type;
  column = other.column;
  row = other.row;
  data.clear();
  for (auto n : other.data)
  {
    data.push_back(n);
  }
  return *this;
}

LevelObject &LevelObject::operator=(LevelObject &&other)
{
  type = std::move(other.type);
  column = std::move(other.column);
  row = std::move(other.row);
  data = std::move(other.data);
  return *this;
}

bool LevelObject::operator==(const LevelObject &other)
{
  return type == other.type && column == other.column && row == other.row && data == other.data;
}

bool LevelObject::operator<(const LevelObject &other)
{
  return column < other.column || row < other.row;
}

bool LevelObject::operator>(const LevelObject &other)
{
  return column > other.column || row > other.row;
}

int LevelObject::getType()
{
  return type;
}

int LevelObject::getColumn()
{
  return column;
}

int LevelObject::getRow()
{
  return row;
}

std::pair<int, int> LevelObject::getCoordinates()
{
  return std::make_pair(column, row);
}

std::vector<int> &LevelObject::getData()
{
  return data;
}

void LevelObject::setType(int newType)
{
  type = newType;
}

void LevelObject::setColumn(int newColumn)
{
  column = newColumn;
}

void LevelObject::setRow(int newRow)
{
  row = newRow;
}

void LevelObject::setPosition(int newColumn, int newRow)
{
  column = newColumn;
  row = newRow;
}

void LevelObject::setData(int n, int *source)
{
  data.clear();
  for (int i = 0; i < n; i++)
  {
    data.push_back(source[i]);
  }
}

// level data
LevelData::LevelData() : width(0), height(0), tileData(), objects(), properties()
{
}

LevelData::LevelData(int startWidth, int startHeight) : width(startWidth), height(startHeight), tileData(), objects(), properties()
{
  tileData.reserve(startWidth * startHeight);
}

int LevelData::getWidth() const
{
  return width;
}

int LevelData::getHeight() const
{
  return height;
}

TileId LevelData::getTile(int index)
{
  return tileData.at(index);
}

TileId LevelData::getTile(int column, int row)
{
  return tileData.at(column + row * width);
}

int LevelData::getNumObjects() const
{
  return static_cast<int>(objects.size());
}

LevelObject &LevelData::getObject(int num)
{
  return objects.at(num);
}

std::string LevelData::getProperty(const std::string propertyName)
{
  if (properties.count(propertyName))
  {
    return properties.at(propertyName);
  }
  return std::string();
}

bool LevelData::hasProperty(const std::string propertyName)
{
  return properties.count(propertyName);
}

void LevelData::resize(int newWidth, int newHeight)
{
  width = newWidth;
  height = newHeight;
  tileData.resize(newWidth * newHeight);
  removeAllObjects();
}

void LevelData::removeAllObjects()
{
  objects.clear();
}

void LevelData::removeAllProperties()
{
  properties.clear();
}

void LevelData::addObject(const LevelObject &obj)
{
  objects.push_back(obj);
}

void LevelData::addProperty(const std::string &propertyName, const std::string &propertyValue)
{
  properties[propertyName] = propertyValue;
}

void LevelData::removeProperty(const std::string &propertyName)
{
  properties.erase(propertyName);
}