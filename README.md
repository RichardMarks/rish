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
  - hero should be able to navigate between two maps using a door

- slice 6

  - maps should be populated with movable objects such as crates and barrels
  - hero should not be able to move into the space a movable object occupies
  - hero should be able to push a movable object when pressing attack
  - movable objects should not move through solid walls or other movable objects
