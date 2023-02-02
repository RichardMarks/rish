#include "Game.h"
#include "Enemy.h"
#include "constants.h"

using namespace rish;

Enemy::Enemy()
{
  healthBarShape.setFillColor(sf::Color::Red);
  healthBarShape.setSize(sf::Vector2f(TILE_WIDTH, 2));
}

void Enemy::setGame(Game *gamePtr)
{
  game = gamePtr;
}

sf::Sprite &Enemy::getSprite()
{
  return sprite;
}

void Enemy::setPosition(int newColumn, int newRow)
{
  column = newColumn;
  row = newRow;
  sprite.setPosition(sf::Vector2f(TILE_WIDTH * column, TILE_HEIGHT * row));
  healthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, 1 + TILE_HEIGHT));
}

std::pair<int, int> Enemy::getPosition()
{
  return std::make_pair(column, row);
}

void Enemy::setDead(bool isDead)
{
  dead = isDead;
}

void Enemy::setDamaged(bool isDamaged)
{
  damaged = isDamaged;
}

void Enemy::setResting(bool isResting)
{
  resting = isResting;
}

void Enemy::setRateOfMovement(float rate)
{
  moveTime = 0.0f;
  timeToMove = rate;
}

void Enemy::setRateOfRest(float rate)
{
  restTime = 0.0f;
  timeToRest = rate;
}

void Enemy::setMovementPath(int pathLength, int movementPath[])
{
  path.clear();
  for (int i = 0; i < pathLength; i++)
  {
    path.push_back(movementPath[0 + (i * 2)]);
    path.push_back(movementPath[1 + (i * 2)]);
  }
  currentPathIndex = 0;
  finalPathIndex = (path.size() / 2) - 1;
}

void Enemy::setMaxHealth(int value)
{
  maxHealth = value;
  health = value;
}

void Enemy::update(sf::Time &deltaTime)
{
  if (!dead)
  {
    handleMovement(deltaTime);
    handleWasDamagedEvent();
  }
}

void Enemy::render(sf::RenderWindow &window)
{
  if (!dead)
  {
    window.draw(sprite);
    window.draw(healthBarShape);
  }
}

bool Enemy::determineIfShouldMove(sf::Time &deltaTime)
{
  bool shouldMove = false;
  float dt = deltaTime.asSeconds();
  if (resting)
  {
    restTime += dt;
    if (restTime >= timeToRest)
    {
      restTime -= timeToRest;
      setResting(false);
    }
  }
  else
  {
    moveTime += dt;
    if (moveTime >= timeToMove)
    {
      moveTime -= timeToMove;
      shouldMove = true;
    }
  }
  return shouldMove;
}

void Enemy::handleMovement(sf::Time &deltaTime)
{
  bool shouldMove = determineIfShouldMove(deltaTime);
  if (shouldMove)
  {
    moveAlongPath();
    game->checkForHeroVsEnemyCollisionAt(column, row);
  }
}

void Enemy::handleWasDamagedEvent()
{
  if (damaged)
  {
    setResting(true);
    setDamaged(false);

    damage(maxHealth / 3);
    updateHealthBar();
  }
}

void Enemy::moveAlongPath()
{
  int nextColumn = path[0 + (currentPathIndex * 2)];
  int nextRow = path[1 + (currentPathIndex * 2)];

  setPosition(nextColumn, nextRow);

  currentPathIndex += 1;
  if (currentPathIndex > finalPathIndex)
  {
    currentPathIndex = 0;
  }
}

void Enemy::damage(int amount)
{
  health -= amount;
  if (health > maxHealth)
  {
    health = maxHealth;
  }
  else if (health <= 0)
  {
    health = 0;
    setDead(true);

    if (game->chanceOf(47.0f))
    {
      game->spawnRandomItemOnMapAt(column, row);
    }
  }
}

void Enemy::updateHealthBar()
{
  float p = static_cast<float>(health) / static_cast<float>(maxHealth);
  float barMaxWidth = static_cast<float>(TILE_WIDTH);
  float healthFill = barMaxWidth * p;
  healthBarShape.setSize(sf::Vector2f(healthFill, 2));
}