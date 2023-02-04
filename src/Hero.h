#ifndef RISH_HERO_H
#define RISH_HERO_H

#include "types.h"

namespace rish
{
  class Game;
  class Hero
  {
  private:
    Game *game;
    sf::Sprite sprite;
    sf::RectangleShape healthBarShape;

    int column;
    int row;

    int health;
    int maxHealth;

    bool dead;
    bool damaged;

    std::vector<InventoryItem> inventory;

    void damage(int amount);
    void updateHealthBar();
    void handleWasDamagedEvent();

  public:
    Hero();
    void setGame(Game *game);

    sf::Sprite &getSprite();

    void setPosition(int newColumn, int newRow);
    std::pair<int, int> getPosition();
    void setDead(bool isDead);
    bool isDead() const { return dead; }
    void setDamaged(bool isDamaged);

    void setMaxHealth(int value);
    void update(sf::Time &deltaTime);
    void render(sf::RenderWindow &window);

    void applyDamage(int amount);

    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
  };
}

#endif
