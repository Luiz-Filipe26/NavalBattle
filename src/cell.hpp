#pragma once

#include "geometry.hpp"
#include "ship.hpp"
enum class CellType { Ship, Water, AttackedShip, AttackedWater };

inline CellType attackedVersion(CellType type) {
    switch (type) {
        case CellType::Ship:
            return CellType::AttackedShip;
        case CellType::Water:
            return CellType::AttackedWater;
        default:
            return type;
    }
}

struct ShipBody {
    const Ship* ship;
    Position initialPos;
    Direction direction;
};

struct Cell {
    Cell() { this->type = CellType::Water; }
    ShipBody shipBody;
    CellType type;
    void placeShip(const Ship* ship, Position pos, Direction direction) {
        shipBody.ship = ship;
        shipBody.direction = direction;
        shipBody.initialPos = pos;
        type = CellType::Ship;
    }
};
