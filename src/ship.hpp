#pragma once

#include <array>
#include <string>

#include "utils.hpp"

struct Ship {
    std::string name;
    int size;
};

class ShipManager {
   public:
    ShipManager() {
        ships = {{{"Aircraft Carrier", 5},
                  {"Battleship", 4},
                  {"Cruiser", 3},
                  {"Submarine", 3},
                  {"Destroyer", 2}}};
    }

    Ship getRandomShip() const { return ships[randomIndex(ships)]; }

   private:
    std::array<Ship, 5> ships;
};
