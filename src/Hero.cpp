#include "Game.h"
#include "Hero.h"

using namespace rish;

Hero::Hero()
{
  healthBarShape.setFillColor(sf::Color::Green);
  healthBarShape.setSize(sf::Vector2f(TILE_WIDTH, 2));
}

void Hero::setGame(Game *gamePtr)
{
  game = gamePtr;
}

sf::Sprite &Hero::getSprite()
{
  return sprite;
}

void Hero::setPosition(int newColumn, int newRow, bool setRespawn)
{
  if (setRespawn)
  {
    respawnColumn = newColumn;
    respawnRow = newRow;
  }
  column = newColumn;
  row = newRow;
  sprite.setPosition(sf::Vector2f(TILE_WIDTH * column, TILE_HEIGHT * row));
  healthBarShape.setPosition(sprite.getPosition() + sf::Vector2f(0, 1 + TILE_HEIGHT));
}

std::pair<int, int> Hero::getPosition()
{
  return std::make_pair(column, row);
}

void Hero::setDead(bool isDead)
{
  dead = isDead;
}

void Hero::setDamaged(bool isDamaged)
{
  damaged = isDamaged;
}

void Hero::setMaxHealth(int value)
{
  maxHealth = value;
  health = value;
}

void Hero::update([[maybe_unused]] sf::Time &deltaTime)
{
  if (!dead)
  {
    handleWasDamagedEvent();
  }
}

void Hero::render(sf::RenderWindow &window)
{
  if (!dead)
  {
    window.draw(sprite);
    window.draw(healthBarShape);
  }
}

void Hero::damage(int amount)
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
  }
}

void Hero::updateHealthBar()
{
  float p = static_cast<float>(health) / static_cast<float>(maxHealth);
  float barMaxWidth = static_cast<float>(TILE_WIDTH);
  float healthFill = barMaxWidth * p;
  healthBarShape.setSize(sf::Vector2f(healthFill, 2));
}

void Hero::handleWasDamagedEvent()
{
  if (damaged)
  {
    setDamaged(false);
    damage(maxHealth / 4);
    updateHealthBar();
  }
}

void Hero::applyDamage(int amount)
{
  damage(amount);
  updateHealthBar();
}

void Hero::respawn()
{
  setDead(false);
  setDamaged(false);
  health = maxHealth;
  updateHealthBar();
  setPosition(respawnColumn, respawnRow, false);
}