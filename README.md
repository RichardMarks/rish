# rish

A small rogue-like game developed with SFML and C++

## Roadmap

- slice 1

  - ✅ hero can move around a map using arrow keys
  - ✅ enemy kills hero on contact
  - ✅ hero can kill enemy by pressing attack when adjacent to the enemy

- slice 2

  - ✅ scale window to something reasonable
  - ✅ enemy should wander the map autonomously following a predetermined path moving once per second
  - ✅ hero should have a health bar and take 1/4 bar damage from enemy on contact
  - ✅ hero should be killed when health reaches zero

- slice 3

  - ✅ enemy should have a health bar and take 3 hits to kill
  - ✅ enemy should not follow their movement path for 2 seconds after taking damage
  - ✅ map should be populated with collectible items
  - ✅ hero should be able to collect items from the map
  - ✅ a visual inventory of the items the hero collects should be rendered on the screen

- slice 4

  - ✅ hero should be able to use the health potion from inventory to restore health
  - ✅ enemy should drop an item on the map when killed
  - ✅ map should be populated with static treasure chests
  - ✅ hero should not be able to move into the space a treasure chest occupies
  - ✅ hero should open treasure chests by pressing the attack button when adjacent to the chest
  - ✅ item within treasure chest should be added to hero inventory when chest is opened
  - ✅ open treasure chests should be displayed as opened

- slice 5

  - ✅ map should be populated with environmental hazards (traps)
  - ✅ hero should take damage on contact with a trap
  - ✅ map should have doors eventually leading to new maps
  - ✅ hero should be able to press the attack button adjacent to a door to use the door
  - ✅ refactor runtime into a Game class
  - ✅ refactor enemy code into an Enemy class
  - ✅ refactor map code into a Level class
  - ✅ refactor hero code into a Hero class
  - ✅ map should be populated with enemies
  - ✅ implement a second level map
  - ✅ hero should be able to navigate between two maps using a door

- slice 6

  - ✅ maps should be populated with movable objects such as crates and barrels
  - ✅ hero should not be able to move into the space a movable object occupies
  - ✅ hero should be able to push a movable object when moving against the object
  - ✅ movable objects should not move through solid walls or other movable objects

- slice 7

  - level data should be loaded from files instead of hard-coded data arrays
  - enemy movement data should be provided by level data
  - maps should be populated by multiple enemy types
  - enemy type data should be loaded from files

- slice 8

  - hero should have a logical facing direction in which attacks are directed
  - hero should have a logical facing direction in which interactions are directed
  - hero weapon should be visible momentarily when attacking

- slice 9

  - player should be able to switch between melee weapon and magic ranged weapon
  - hero should have a mana bar like the health bar, positioned above the hero when magic weapon is equipped
  - using magic weapon should consume mana
  - hero should not be able to attack using magic if mana is depleted
  - magic projectiles should travel until they hit an enemy or solid object

- slice 10

  - add sound effects for the hero movement, attacks, damage, interactions, and death
  - add sound effects for the enemy movement, attacks, damage, and death
  - add looping background music
  - add fade to black screen transition when changing levels
  - add fade from black screen transition when finished loading a level
  - prevent hero from taking actions during screen transitions

- slice 11

  - game should begin with a fade from black screen transition to a title screen
  - pressing space or enter on the title screen should transition to the play screen
  - pressing escape on the title screen should exit the game
  - pressing escape on the play screen should pause the game
  - pause screen should show an arrow key navigable menu to resume or quit the game

- slice 12

  - when the hero health is depleted, game should transition to a game over screen
  - game over screen should show an arrow key navigable menu to restart or quit the game
  - choosing restart from the game over screen will restart from the current level
  - choosing quit from the game over screen will return to the title screen
