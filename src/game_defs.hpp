#pragma once

#include "geometry.hpp"

enum class GameSide { None, Player, Bot };
enum class MoveParseError { None, InvalidFormat, OutOfBounds };

struct MoveParseResult {
    Position pos;
    MoveParseError error;
};
