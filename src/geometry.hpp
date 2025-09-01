#pragma once

#include <cstddef>

enum class Direction { Right, Down, Left, Up };
inline Direction invertDirection(Direction& direction) {
    switch (direction) {
        case Direction::Up:
            return Direction::Down;
        case Direction::Down:
            return Direction::Up;
        case Direction::Left:
            return Direction::Right;
        case Direction::Right:
            return Direction::Left;
    }
    return direction;
}
struct Position {
    int x;
    int y;

    void applyOffset(const Direction& direction, int offset) {
        switch (direction) {
            case Direction::Right:
                x += offset;
                break;
            case Direction::Down:
                y += offset;
                break;
            case Direction::Left:
                x -= offset;
                break;
            case Direction::Up:
                y -= offset;
                break;
        }
    }
};

struct Dimension {
    size_t width;
    size_t height;
};
