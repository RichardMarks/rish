#ifndef RISH_LEVEL_H
#define RISH_LEVEL_H

#include "types.h"

namespace rish
{
  class LevelObject
  {
  private:
    int type;
    int column;
    int row;
    std::vector<int> data;
    friend auto operator<<(std::ostream &os, LevelObject const &m) -> std::ostream &
    {
      std::stringstream ss;
      ss << "{ "
         << "size: " << m.data.size() << ", values: { ";
      for (auto n : m.data)
      {
        ss << n << ", ";
      }
      ss << "}";

      os << "{ "
         << "type: " << m.type << ", "
         << "column: " << m.column << ", "
         << "row: " << m.row << ", "
         << "data: " << ss.str() << " "
         << "}";
      return os;
    }

  public:
    LevelObject();
    LevelObject(const LevelObject &other);
    LevelObject(LevelObject &&other);
    LevelObject &operator=(const LevelObject &other);
    LevelObject &operator=(LevelObject &&other);
    bool operator==(const LevelObject &other);
    bool operator<(const LevelObject &other);
    bool operator>(const LevelObject &other);

    int getType();
    int getColumn();
    int getRow();
    std::pair<int, int> getCoordinates();
    std::vector<int> &getData();
    void setType(int newType);
    void setColumn(int newColumn);
    void setRow(int newRow);
    void setPosition(int newColumn, int newRow);
    void setData(int n, int *source);
  };

  class LevelData
  {
  private:
    int width;
    int height;

    std::vector<TileId> tileData;
    std::vector<LevelObject> objects;
    std::unordered_map<std::string, std::string> properties;

    LevelData(const LevelData &) = delete;
    LevelData(LevelData &&) = delete;
    LevelData &operator=(const LevelData &) = delete;
    LevelData &operator=(LevelData &&) = delete;

  public:
    LevelData();
    LevelData(int startWidth, int startHeight);

    int getWidth() const;
    int getHeight() const;
    TileId getTile(int index);
    TileId getTile(int column, int row);
    int getNumObjects() const;
    LevelObject &getObject(int num);
    std::string getProperty(const std::string propertyName);
    bool hasProperty(const std::string propertyName);

    void resize(int newWidth, int newHeight);
    void removeAllObjects();
    void removeAllProperties();
    void addObject(const LevelObject &obj);
    void addProperty(const std::string &propertyName, const std::string &propertyValue);
    void removeProperty(const std::string &propertyName);
  };

  class Level
  {
  private:
    LevelData data;
    std::unordered_map<int, std::unique_ptr<sf::Sprite>> sprites;

    sf::VertexArray vertexArray;
    sf::Texture texture;

    int numTilesAcrossTexture;

  public:
    Level(sf::Texture &texture);
    Level(const Level &) = delete;
    Level(Level &&) = delete;
    Level &operator=(const Level &) = delete;
    Level &operator=(Level &&) = delete;

    LevelData &getData();
    void loadFromDataArray(int dataArray[]);

    void renderTilemap(sf::RenderWindow &window);
  };
}

#endif
