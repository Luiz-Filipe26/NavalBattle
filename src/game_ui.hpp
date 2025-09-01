#pragma once

#include "game_defs.hpp"
#include "grid.hpp"

class GameUI {
   public:
    virtual ~GameUI() = default;
    using onPlayerMoveFn = std::function<void(std::string)>;

    virtual void onNewGame() = 0;
    virtual void onGameClosed() = 0;
    virtual void onWaitingPlayerMove() = 0;
    virtual void showGrids(const GridView& player, const GridView& bot) = 0;
    virtual void showBotMove(const Position& pos) = 0;
    virtual void showPlayerMove(const Position& pos) = 0;
    virtual void showInvalidMoveMessage() = 0;
    virtual void showParseError(MoveParseError moveError) = 0;
    virtual void showGameOver(GameSide winner) = 0;

   protected:
    onPlayerMoveFn onPlayerMove;
};
