#pragma once

#include <iostream>
#include <map>

#include "game_ui.hpp"
#include "grid.hpp"
#include "move_representation.hpp"

class SimpleGridView : public GridView {
   public:
    SimpleGridView(const GridCells& cells,
                   const std::map<CellType, std::string>& cellMap)
        : gridCells(&cells), cellTypeMap(cellMap) {}

    Dimension dimension() const override {
        return {(*gridCells)[0].size(), (*gridCells).size()};
    }
    std::string get(int x, int y) const override {
        const CellType& type = (*gridCells)[y][x].type;
        return cellTypeMap.count(type) ? cellTypeMap.at(type) : "?";
    }

   private:
    const GridCells* gridCells;
    std::map<CellType, std::string> cellTypeMap;
};

class GridPrinter {
   public:
    static void printGrid(const GridView& gridView) {
        std::cout << render(gridView);
    }

   private:
    static std::string render(const GridView& gridView) {
        std::string result;
        const Dimension dimension = gridView.dimension();
        result.reserve(dimension.width * dimension.height * 5);

        result += makeTopBorder(dimension.width);
        for (size_t row = 0; row < dimension.height; ++row) {
            result += makeMiddleCells(gridView, dimension.width, row);
            const bool lastRow = row < dimension.height - 1;
            if (lastRow) result += makeMiddleBorder(dimension.width);
        }
        result += makeBottomBorder(dimension.width);
        return result;
    }

    static std::string makeTopBorder(int numOfColumns) {
        std::string result;
        result += "   ";
        for (int i = 0; i < numOfColumns; i++)
            result += " " + std::string(1, 'A' + i);
        result += "\n   ┌" + strutils::repeat("─┬", numOfColumns - 1) + "─┐\n";
        return result;
    }

    static std::string makeMiddleCells(const GridView& gridView, int width,
                                       int row) {
        std::string result;
        result.reserve(width * 2 + 3);
        result += strutils::padLeft(std::to_string(row + 1), 2, ' ') + " ";
        for (size_t column = 0; column < gridView.dimension().width; ++column)
            result += "│" + gridView.get(column, row);
        return result + "│\n";
    }

    static std::string makeMiddleBorder(int numOfColumns) {
        return "   ├" + strutils::repeat("─┼", numOfColumns - 1) + "─┤\n";
    }

    static std::string makeBottomBorder(int numOfColumns) {
        return "   └" + strutils::repeat("─┴", numOfColumns - 1) + "─┘\n";
    }
};

class ConsoleUI : public GameUI {
   public:
    ConsoleUI(onPlayerMoveFn onPlayerMoveCallback) {
        this->onPlayerMoveCallback = onPlayerMoveCallback;
    }

    void onNewGame() override {
        std::cout << "==========Jogo de Batalha Naval==========\n";
    }

    void onGameClosed() override { std::cout << "Fim de jogo!"; }

    void processInput(bool shouldReceivePlayerMove) override {
        if (!shouldReceivePlayerMove) return;
        std::string move = getPlayerMoveInput();
        onPlayerMoveCallback(move);
    }

    void render(const RenderData& renderData) override {
        if (!renderData.changedGrids) return;
        std::cout << "==========GRID DO JOGADOR==========\n";
        GridPrinter::printGrid(*renderData.playerView);
        std::cout << "==========GRID DO BOT==========\n";
        GridPrinter::printGrid(*renderData.botView);
        for (auto& botMove : botMoves)
            std::cout << "O bot jogou em " << botMove << "\n";
        botMoves.clear();
    }

    void onBotMove(const Position& pos) override {
        std::string moveStr = MoveRepresentation::moveToStrCoordinate(pos);
        botMoves.push_back(moveStr);
    }

    void onPlayerMove(const Position& pos) override {
        std::string moveStr = MoveRepresentation::moveToStrCoordinate(pos);
        std::cout << "Você jogou em " << moveStr << "\n";
    }

    void onInvalidMoveMessage() override {
        std::cout << "Jogada inválida. Tente novamente.\n";
    }

    void onParseError(MoveParseError moveError) override {
        if (moveError == MoveParseError::InvalidFormat)
            std::cout << "Formato inválido. Use letra + número (ex: A5).\n";
        else if (moveError == MoveParseError::OutOfBounds)
            std::cout << "Movimento fora dos limites do tabuleiro.\n";
    }

    void onGameOver(GameSide winner) override {
        std::string winnerName;
        if (winner == GameSide::Player)
            winnerName = "Jogador";
        else if (winner == GameSide::Bot)
            winnerName = "Bot";
        else
            winnerName = "Nenhum";
        std::cout << "Fim de jogo! Vencedor: " << winnerName << "\n";
    }

   private:
    std::string getPlayerMoveInput() {
        std::string input;
        std::cout << "Digite um movimento: ";
        std::getline(std::cin, input);
        return strutils::toUpper(strutils::trim(input));
    }

   private:
    std::vector<std::string> botMoves;
};
