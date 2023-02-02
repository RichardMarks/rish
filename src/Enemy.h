#ifndef RISH_ENEMY_H
#define RISH_ENEMY_H

#include "types.h"

namespace rish
{
  class Game;
  class Enemy
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
    bool resting;

    float moveTime;
    float timeToMove;

    float restTime;
    float timeToRest;

    std::vector<int> path;
    int currentPathIndex;
    int finalPathIndex;

    bool determineIfShouldMove(sf::Time &deltaTime);
    void handleMovement(sf::Time &deltaTime);
    void handleWasDamagedEvent();
    void moveAlongPath();
    void damage(int amount);
    void updateHealthBar();

  public:
    Enemy();

    void setGame(Game *game);

    sf::Sprite &getSprite();

    void setPosition(int newColumn, int newRow);
    std::pair<int, int> getPosition();
    void setDead(bool isDead);
    bool isDead() const { return dead; }
    void setDamaged(bool isDamaged);
    void setResting(bool isResting);
    void setRateOfMovement(float rate);
    void setRateOfRest(float rate);
    void setMovementPath(int pathLength, int movementPath[]);
    void setMaxHealth(int value);
    void update(sf::Time &deltaTime);
    void render(sf::RenderWindow &window);
  };
}

#endif
