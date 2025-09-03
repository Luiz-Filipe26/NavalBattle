#pragma once

#include <vector>

#include "cell.hpp"

using GridCells = std::vector<std::vector<Cell>>;
class Grid {
   public:
    Grid(int width, int height) : grid(height, std::vector<Cell>(width)) {}

    Dimension dimension() const { return {grid[0].size(), grid.size()}; }
    const GridCells& getCells() const { return grid; }
    const bool hasCell(const Position& pos) const {
        return !isPointOutOfGrid(pos);
    }
    Cell& getCell(const Position& pos) { return grid[pos.y][pos.x]; }
    const Cell& getConstCell(const Position& pos) const {
        return grid[pos.y][pos.x];
    }
    bool isType(const Position& pos, CellType type) const {
        return grid[pos.y][pos.x].type == type;
    }

    void placeShip(const Ship& ship, Position pos, Direction direction) {
        Position currentPos = pos;
        for (int offset = 0; offset < ship.size; ++offset) {
            Cell& cell = grid[currentPos.y][currentPos.x];
            cell.placeShip(&ship, pos, direction);
            currentPos.applyOffset(direction, 1);
        }
    }

    Cell& getRandomCell() {
        Position pos = getRandomPosition();
        return grid[pos.y][pos.x];
    }

    Position getRandomPosition() const {
        return {randomIndex(grid[0]), randomIndex(grid)};
    }

    std::vector<Direction> validDirections(Position pos, int shipSize) const {
        std::vector<Direction> available;
        for (Direction dir : {Direction::Right, Direction::Down,
                              Direction::Left, Direction::Up}) {
            if (isValidPlacement(pos, dir, shipSize)) available.push_back(dir);
        }
        return available;
    }

   private:
    bool isValidPlacement(const Position& position, const Direction& direction,
                          int size) const {
        if (isLineOutOfGrid(position, size, direction)) return false;
        Position currentPos = position;
        for (int offset = 0; offset < size; ++offset) {
            if (!isCellAndNeighborsFree(currentPos)) return false;
            currentPos.applyOffset(direction, 1);
        }
        return true;
    }

    bool isCellAndNeighborsFree(const Position& pos) const {
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
                Position neighborPos{pos.x + dx, pos.y + dy};
                if (!isPointOutOfGrid(neighborPos) &&
                    isType(neighborPos, CellType::Ship))
                    return false;
            }
        return true;
    }

    bool isLineOutOfGrid(const Position& pos, int size,
                         const Direction& direction) const {
        Position endPos = pos;
        endPos.applyOffset(direction, size - 1);
        return isPointOutOfGrid(pos) || isPointOutOfGrid(endPos);
    }

    bool isPointOutOfGrid(const Position& pos) const {
        return pos.x < 0 || pos.x >= grid[0].size() || pos.y < 0 ||
               pos.y >= grid.size();
    }

    GridCells grid;
};

class GridView {
   public:
    explicit GridView(const Grid& grid) : gameGrid(&grid) {}
    Dimension dimension() const { return gameGrid->dimension(); }
    CellType get(int x, int y) const {
        const Cell& cell = gameGrid->getConstCell({x, y});
        return cell.type;
    }

   private:
    const Grid* gameGrid;
};
