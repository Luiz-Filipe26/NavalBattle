#pragma once

#include <functional>
#include <string>

#include "SFML/System/Time.hpp"
#include "game_defs.hpp"

struct RenderData {
    const bool changedGrids;
    const sf::Time deltaTime;
};

class GameUI {
   public:
    virtual ~GameUI() = default;
    using onPlayerMoveFn = std::function<void(std::string)>;

    virtual sf::Time getPreferredRenderInterval() = 0;
    virtual void onNewGame() = 0;
    virtual void onGameClosed() = 0;
    virtual bool isOpen() const = 0;
    virtual void processInput(bool shouldReceivePlayerMove) = 0;
    virtual void onBotMove(const Position& pos) = 0;
    virtual void onPlayerMove(const Position& pos) = 0;
    virtual void onInvalidMoveMessage() = 0;
    virtual void onParseError(MoveParseError moveError) = 0;
    virtual void onGameOver(GameSide winner) = 0;
    virtual void render(const RenderData& renderData) = 0;

   protected:
    onPlayerMoveFn onPlayerMoveCallback;
};
