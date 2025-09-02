#pragma once

#include "game_defs.hpp"
#include "grid.hpp"

struct RenderData {
    const GridView* const playerView;
    const GridView* const botView;
    const bool changedGrids;
};

class GameUI {
   public:
    virtual ~GameUI() = default;
    using onPlayerMoveFn = std::function<void(std::string)>;

    virtual void onNewGame() = 0;
    virtual void onGameClosed() = 0;
    virtual bool isOpen() const = 0;
    virtual void processInput(bool shouldReceivePlayerMove) = 0;
    virtual void render(const RenderData& renderData) = 0;
    virtual void onBotMove(const Position& pos) = 0;
    virtual void onPlayerMove(const Position& pos) = 0;
    virtual void onInvalidMoveMessage() = 0;
    virtual void onParseError(MoveParseError moveError) = 0;
    virtual void onGameOver(GameSide winner) = 0;

   protected:
    onPlayerMoveFn onPlayerMoveCallback;
};
