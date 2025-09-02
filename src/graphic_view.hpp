#pragma once

#include <SFML/Graphics.hpp>
#include <deque>
#include <filesystem>
#include <stdexcept>
#include <string>

#include "game_ui.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "move_representation.hpp"

class GraphicUI : public GameUI {
   public:
    GraphicUI(onPlayerMoveFn onPlayerMoveCallback)
        : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Batalha Naval") {
        this->onPlayerMoveCallback = onPlayerMoveCallback;
        window.setFramerateLimit(60);

        if (!ImGui::SFML::Init(window)) {
            throw std::runtime_error("Falha ao inicializar o ImGui-SFML.");
        }

        auto fontPath = resourcesPath / "arial-regular.ttf";
        if (!font.loadFromFile(fontPath.string())) {
            throw std::runtime_error("Nao foi possivel carregar a fonte: " +
                                     fontPath.string());
        }
    }

    ~GraphicUI() { ImGui::SFML::Shutdown(); }

    void onNewGame() override {}
    void onGameClosed() override {}
    bool isOpen() const override { return window.isOpen(); }
    void processInput(bool shouldReceivePlayerMove) override {}
    void render(const RenderData& renderData) override {}
    void onBotMove(const Position& pos) override {}
    void onPlayerMove(const Position& pos) override {}
    void onInvalidMoveMessage() override {}
    void onParseError(MoveParseError moveError) override {}
    void onGameOver(GameSide winner) override {
        isGameOver = true;
        this->winner = winner;
    }

   private:
    static constexpr unsigned int WINDOW_WIDTH = 900;
    static constexpr unsigned int WINDOW_HEIGHT = 600;
    static constexpr float CELL_SIZE = 32.0f;

    static inline const auto resourcesPath =
        std::filesystem::path(RESOURCES_PATH);

    sf::RenderWindow window;
    sf::Font font;
    sf::Clock deltaClock;

    std::deque<std::string> logMessages;
    bool isGameOver = false;
    GameSide winner = GameSide::None;
};
