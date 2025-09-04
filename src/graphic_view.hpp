#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

#include "SFML/Window/Event.hpp"
#include "game_ui.hpp"
#include "grid.hpp"
#include "move_representation.hpp"

static inline const std::filesystem::path FS_RESOURCES_PATH =
    std::filesystem::path(RESOURCES_PATH);

struct TextStyle {
    unsigned int size;
    sf::Color color;
};

struct TextDetails {
    TextStyle style;
    sf::Font* font;
    sf::Vector2f position;
};

class EventUtils {
   public:
    static bool isLeftMousePress(const sf::Event& event) {
        return event.type == sf::Event::MouseButtonPressed &&
               event.mouseButton.button == sf::Mouse::Left;
    }

    static bool isRightMousePress(const sf::Event& event) {
        return event.type == sf::Event::MouseButtonPressed &&
               event.mouseButton.button == sf::Mouse::Right;
    }
};

inline void drawText(sf::RenderWindow& window, const std::string& text,
                     const TextDetails& details) {
    sf::Text sfText(text, *details.font, details.style.size);
    sfText.setFillColor(details.style.color);
    sfText.setPosition(details.position);
    window.draw(sfText);
}

class GraphicUI : public GameUI {
   public:
    GraphicUI(onPlayerMoveFn onPlayerMoveCallback,
              const GridView& playerGridView, const GridView& botGridView)
        : window(sf::VideoMode(WINDOW_DIMENSION.width, WINDOW_DIMENSION.height),
                 "Batalha Naval"),
          playerView(playerGridView),
          botView(botGridView) {
        this->onPlayerMoveCallback = onPlayerMoveCallback;
        window.setFramerateLimit(GAME_FPS);

        auto fontPath = FS_RESOURCES_PATH / "arial-regular.ttf";
        if (!font.loadFromFile(fontPath.string()))
            throw std::runtime_error("Não foi possível carregar a fonte: " +
                                     fontPath.string());

        deltaClock.restart();
    }

    ~GraphicUI() override = default;

    void onNewGame() override {
        statusText = "Novo jogo iniciado";
        frozen = false;
        isGameOver = false;
        winner = GameSide::None;
    }

    void onGameClosed() override {
        frozen = true;
        statusText = "Jogo encerrado. Tela congelada.";
    }

    bool isOpen() const override { return window.isOpen(); }

    void processInput(bool shouldReceivePlayerMove) override {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (frozen) continue;
            auto botCellPosition = getBotCellPosition(event);
            if (shouldReceivePlayerMove && botCellPosition) {
                std::string moveStr =
                    MoveRepresentation::moveToStrCoordinate(*botCellPosition);
                onPlayerMoveCallback(moveStr);
            }
        }
    }

    void render(const RenderData&) override {
        deltaClock.restart();
        window.clear(BG_COLOR);
    
        drawTitles();
    
        TextDetails statusDetails{
            STATUS_STYLE,
            &font,
            {WINDOW_DIMENSION.width * 0.5f - STATUS_WIDTH * 0.5f, 8.0f}};
        drawText(window, statusText, statusDetails);
    
        drawGrid(playerView, GRID_LEFT_X, GRID_TOP_Y, true);
        drawGrid(botView, GRID_RIGHT_X, GRID_TOP_Y, false);
    
        if (isGameOver) {
            TextDetails gameOverDetails{
                GAME_OVER_STYLE,
                &font,
                {WINDOW_DIMENSION.width * 0.5f - STATUS_WIDTH * 0.5f, 40.0f}};
            drawText(window, "Fim de jogo!", gameOverDetails);
    
            gameOverDetails.position.y = 64.0f;
            drawText(window, "Vencedor: " + gameSideToString(winner),
                     gameOverDetails);
        }
    
        window.display();
    }
    

    void onBotMove(const Position& pos) override {
        statusText =
            "Bot atacou: " + MoveRepresentation::moveToStrCoordinate(pos);
    }

    void onPlayerMove(const Position& pos) override {
        statusText =
            "Você atacou: " + MoveRepresentation::moveToStrCoordinate(pos);
    }

    void onInvalidMoveMessage() override { statusText = u8"Jogada inválida."; }

    void onParseError(MoveParseError moveError) override {
        if (moveError == MoveParseError::InvalidFormat)
            statusText = "Formato inválido. Use letra+numero (ex: A5).";
        else if (moveError == MoveParseError::OutOfBounds)
            statusText = "Movimento fora dos limites do tabuleiro.";
        else
            statusText = "Erro ao interpretar movimento.";
    }

    void onGameOver(GameSide winnerSide) override {
        isGameOver = true;
        winner = winnerSide;
        statusText = "Fim de jogo! Vencedor: " + gameSideToString(winner);
    }

   private:
    std::optional<Position> getBotCellPosition(const sf::Event& event) const {
        if (!EventUtils::isLeftMousePress(event)) return std::nullopt;
        auto mouse = sf::Mouse::getPosition(window);
        return mapMouseToBotCell(mouse);
    }

    void drawTitles() {
        TextDetails leftTitle{
            TITLE_STYLE, &font, {GRID_LEFT_X, GRID_TOP_Y - 40}};
        drawText(window, "GRID DO JOGADOR", leftTitle);

        TextDetails rightTitle{
            TITLE_STYLE, &font, {GRID_RIGHT_X, GRID_TOP_Y - 40}};
        drawText(window, "GRID DO BOT", rightTitle);
    }

    void drawGrid(const GridView& gridView, float originX, float originY,
                  bool showShips) {
        Dimension dim = gridView.dimension();
        const int w = static_cast<int>(dim.width);
        const int h = static_cast<int>(dim.height);

        sf::RectangleShape cellShape(
            sf::Vector2f(CELL_SIZE - CELL_PADDING, CELL_SIZE - CELL_PADDING));

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                CellType cell = gridView.get(x, y);
                std::string cellSymbol = cellTypeToSymbol(cell);

                sf::Color fill = WATER_COLOR;
                if (cell == CellType::Ship) {
                    fill = showShips ? SHIP_COLOR : WATER_COLOR;
                } else if (cell == CellType::AttackedShip) {
                    fill = ATTACKED_SHIP_COLOR;
                } else if (cell == CellType::AttackedWater) {
                    fill = ATTACKED_WATER_COLOR;
                }

                cellShape.setFillColor(fill);
                cellShape.setOutlineThickness(1.0f);
                cellShape.setOutlineColor(GRID_LINE_COLOR);
                cellShape.setPosition(originX + x * CELL_SIZE,
                                      originY + y * CELL_SIZE);
                window.draw(cellShape);

                // Desenhar labels das colunas (A, B, C...)
                if (y == 0) {
                    TextDetails colLabel{
                        TITLE_STYLE,
                        &font,
                        {originX + x * CELL_SIZE + 6, originY - 18}};
                    drawText(window, std::string(1, static_cast<char>('A' + x)),
                             colLabel);
                }
                // Desenhar labels das linhas (1, 2, 3...)
                if (x == 0) {
                    TextDetails rowLabel{
                        TITLE_STYLE,
                        &font,
                        {originX - 22, originY + y * CELL_SIZE + 4}};
                    drawText(window, std::to_string(y + 1), rowLabel);
                }
            }
        }
    }

std::string cellTypeToSymbol(CellType type) {
    static const std::map<CellType, std::string> typeToSymbol{
        {CellType::Water, " "},          
        {CellType::Ship, "█"},
        {CellType::AttackedShip, "X"},
        {CellType::AttackedWater, "^"}
    };

    return typeToSymbol.count(type) ? typeToSymbol.at(type) : "?";
}


    std::optional<Position> mapMouseToBotCell(const sf::Vector2i& mouse) const {
        const int gridW = 10;
        const int gridH = 10;

        int relX = mouse.x - static_cast<int>(GRID_RIGHT_X);
        int relY = mouse.y - static_cast<int>(GRID_TOP_Y);

        if (relX < 0 || relY < 0) return std::nullopt;
        int col = relX / static_cast<int>(CELL_SIZE);
        int row = relY / static_cast<int>(CELL_SIZE);
        if (col < 0 || col >= gridW || row < 0 || row >= gridH)
            return std::nullopt;
        return Position{col, row};
    }

    std::string gameSideToString(GameSide side) const {
        switch (side) {
            case GameSide::Player:
                return "Jogador";
            case GameSide::Bot:
                return "Bot";
            default:
                return "Nenhum";
        }
    }

   private:
    const GridView& playerView;
    const GridView& botView;

    sf::RenderWindow window;
    sf::Font font;
    sf::Clock deltaClock;

    std::string statusText{"Bem-vindo à Batalha Naval"};
    bool frozen{false};
    bool isGameOver{false};
    GameSide winner{GameSide::None};

    static constexpr unsigned int GAME_FPS = 60;
    static constexpr Dimension WINDOW_DIMENSION = {900, 600};
    static constexpr float CELL_SIZE = 32.0f;
    static constexpr float CELL_PADDING = 2.0f;
    static constexpr float GRID_TOP_Y = 80.0f;
    static constexpr float GRID_LEFT_X = 40.0f;
    static constexpr float GRID_RIGHT_X = 480.0f;
    static constexpr float STATUS_WIDTH = 420.0f;
    static inline const sf::Color BG_COLOR = sf::Color(30, 30, 30);
    static inline const sf::Color GRID_LINE_COLOR = sf::Color(80, 80, 80);
    static inline const sf::Color WATER_COLOR = sf::Color(65, 105, 225);
    static inline const sf::Color SHIP_COLOR = sf::Color(60, 179, 113);
    static inline const sf::Color ATTACKED_SHIP_COLOR = sf::Color(220, 20, 60);
    static inline const sf::Color ATTACKED_WATER_COLOR =
        sf::Color(200, 200, 200);

    static inline const TextStyle TITLE_STYLE = {16, sf::Color::White};
    static inline const TextStyle STATUS_STYLE = {16, sf::Color::White};
    static inline const TextStyle GAME_OVER_STYLE = {18, sf::Color::Yellow};
};
