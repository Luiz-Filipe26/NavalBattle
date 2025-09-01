#pragma once

#include <regex>

#include "game_defs.hpp"
#include "geometry.hpp"

class MoveRepresentation {
   public:
    static MoveParseResult parseMove(const std::string& input,
                                     const Dimension& dimension) {
        if (!std::regex_match(input, MOVE_PATTERN))
            return {{}, MoveParseError::InvalidFormat};

        int column = input[0] - 'A';
        int line = std::stoi(input.substr(1)) - 1;

        if (column >= dimension.width || line < 0 || line >= dimension.height)
            return {{}, MoveParseError::OutOfBounds};

        return {{column, line}, MoveParseError::None};
    }

    static std::string moveToStrCoordinate(const Position& move) {
        return std::string(1, 'A' + move.x) + std::to_string(move.y + 1);
    }

   private:
    inline static const std::regex MOVE_PATTERN{"^[A-Z]\\d{1,2}$"};
};
