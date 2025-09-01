#include <cctype>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "cell.hpp"
#include "game_defs.hpp"
#include "geometry.hpp"
#include "grid.hpp"
#include "move_representation.hpp"
#include "ship.hpp"
#include "terminal_view.hpp"
#include "utils.hpp"

constexpr int GRID_WIDTH = 10;
constexpr int GRID_HEIGHT = 10;
constexpr int SHIPS_AMOUNT = 6;

struct Game {
    Grid botGrid;
    Grid playerGrid;
    std::vector<Ship> botShips;
    std::vector<Ship> playerShips;
    SimpleGridView playerGridView;
    SimpleGridView botGridView;
    int targetTotalShipSize;
    int shipsAmount;

    Game(int gridWidth, int gridHeight, int shipsAmount)
        : botGrid(gridWidth, gridHeight),
          playerGrid(gridWidth, gridHeight),
          shipsAmount(shipsAmount),
          playerGridView(playerGrid.getCells(),
                         {{CellType::Ship, "█"},
                          {CellType::Water, "~"},
                          {CellType::AttackedShip, "X"},
                          {CellType::AttackedWater, "^"}}),
          botGridView(botGrid.getCells(), {{CellType::Ship, "~"},
                                           {CellType::Water, "~"},
                                           {CellType::AttackedShip, "X"},
                                           {CellType::AttackedWater, "^"}}) {}
};

class GameSetup {
   public:
    void setupGame(Game& game) const {
        ShipManager shipManager;
        game.botShips = selectRandomShips(shipManager, game.shipsAmount);
        game.playerShips = selectRandomShips(shipManager, game.shipsAmount);
        game.targetTotalShipSize = calculateTotalShipsSize(game.botShips);
        equalizeTotalSize(game.playerShips, game.targetTotalShipSize,
                          shipManager);
        for (auto& sh : game.botShips) placeRandomly(game.botGrid, sh);
        for (auto& sh : game.playerShips) placeRandomly(game.playerGrid, sh);
    }

   private:
    void placeRandomly(Grid& grid, const Ship& ship) const {
        auto [pos, direction] = getRandomPlacement(grid, ship.size);
        grid.placeShip(ship, pos, direction);
    }

    std::pair<Position, Direction> getRandomPlacement(const Grid& grid,
                                                      int size) const {
        std::vector<Direction> filteredDirections;
        Position position;
        do {
            position = grid.getRandomPosition();
            auto directions = grid.validDirections(position, size);
            filteredDirections = filterDirections(directions);
        } while (filteredDirections.empty());
        size_t randomDirectionIndex = randomIndex(filteredDirections);
        return {position, filteredDirections[randomDirectionIndex]};
    }

    std::vector<Direction> filterDirections(
        const std::vector<Direction>& dirs) const {
        std::vector<Direction> filtered;
        for (const auto& direction : dirs)
            if (direction == Direction::Right || direction == Direction::Down)
                filtered.push_back(direction);
        return filtered;
    }

    std::vector<Ship> selectRandomShips(const ShipManager& manager,
                                        int amount) const {
        std::vector<Ship> ships;
        ships.reserve(amount);
        for (int i = 0; i < amount; ++i)
            ships.push_back(manager.getRandomShip());
        return ships;
    }

    void equalizeTotalSize(std::vector<Ship>& playerShips,
                           const int botTotalSize,
                           const ShipManager& shipManager) const {
        int totalDiff = botTotalSize - calculateTotalShipsSize(playerShips);
        while (totalDiff != 0) {
            Ship candidate = shipManager.getRandomShip();
            Ship* currentPtr = &playerShips[randomIndex(playerShips)];
            const auto& [bestShip, diffChange] =
                chooseBestShip(totalDiff, *currentPtr, candidate);
            totalDiff -= diffChange;
            *currentPtr = bestShip;
        }
    }

    std::pair<const Ship&, int> chooseBestShip(int& totalDifference,
                                               const Ship& current,
                                               const Ship& candidate) const {
        int diffChange = candidate.size - current.size;
        if (std::abs(totalDifference - diffChange) < std::abs(totalDifference))
            return {candidate, diffChange};
        return {current, 0};
    }

    int calculateTotalShipsSize(const std::vector<Ship>& ships) const {
        int total = 0;
        for (const auto& ship : ships) total += ship.size;
        return total;
    }
};

class BotAI {
   public:
    Position computeBotMove(Grid& grid) {
        switch (state) {
            case BotState::Searching:
                return computeSearchingMove(grid);
            case BotState::Targeting:
                return computeTargetingMove(grid);
            case BotState::Finishing:
                return computeFinishingMove(grid);
        }
        return {};
    }

    void onLastHitSunkShip() { state = BotState::Searching; }

   private:
    Position computeSearchingMove(Grid& grid) {
        Position pos = pickRandomAvailableCell(grid);
        if (grid.isType(pos, CellType::Ship)) {
            state = BotState::Targeting;
            initialHitPos = pos;
            remainingDirections = {Direction::Up, Direction::Down,
                                   Direction::Left, Direction::Right};
        }
        return pos;
    }

    Position pickRandomAvailableCell(Grid& grid) const {
        Position pos{};
        do {
            pos = grid.getRandomPosition();
        } while (!isAttackableCell(grid, pos));
        return pos;
    }

    Position computeTargetingMove(Grid& grid) {
        remainingDirections = filterAttackableDirections(grid, initialHitPos,
                                                         remainingDirections);
        if (remainingDirections.empty()) {
            state = BotState::Searching;
            return computeSearchingMove(grid);
        }
        shipDirection = targetDirectionFromRemaining();
        lastPos = incrementToDirection(initialHitPos, shipDirection);
        if (grid.isType(lastPos, CellType::Ship)) state = BotState::Finishing;
        return lastPos;
    }

    std::vector<Direction> filterAttackableDirections(
        Grid& grid, const Position& pos,
        const std::vector<Direction>& directions) const {
        std::vector<Direction> filtered;
        for (auto& direction : directions)
            if (isAttackableCell(grid, incrementToDirection(pos, direction)))
                filtered.push_back(direction);
        return filtered;
    }

    Direction targetDirectionFromRemaining() {
        std::shuffle(remainingDirections.begin(), remainingDirections.end(),
                     RandomEngine::instance().getGenerator());
        Direction direction = remainingDirections.back();
        remainingDirections.pop_back();
        return direction;
    }

    Position incrementToDirection(const Position& pos,
                                  const Direction& direction) const {
        Position newPos = pos;
        newPos.applyOffset(direction, 1);
        return newPos;
    }

    bool isAttackableCell(Grid& grid, const Position& pos) const {
        if (!grid.hasCell(pos)) return false;
        const Cell& cell = grid.getCell(pos);
        return attackedVersion(cell.type) != cell.type;
    }

    Position computeFinishingMove(Grid& grid) {
        lastPos = incrementToDirection(lastPos, shipDirection);
        if (isAfterEdge(grid)) invertDirectionAfterEdge(grid);
        return lastPos;
    }

    bool isAfterEdge(Grid& grid) const {
        return !grid.hasCell(lastPos) || !grid.isType(lastPos, CellType::Ship);
    }

    void invertDirectionAfterEdge(Grid& grid) {
        lastPos = initialHitPos;
        shipDirection = invertDirection(shipDirection);
        lastPos.applyOffset(shipDirection, 1);
    }

   private:
    enum class BotState { Searching, Targeting, Finishing };
    BotState state = BotState::Searching;
    Position initialHitPos;
    Position lastPos;
    Direction shipDirection;
    std::vector<Direction> remainingDirections;
};

struct CellAttackResult {
    CellType cellType;
    bool changedCell;
};

class GameLogic {
   public:
    GameLogic(std::unique_ptr<Game> game) : game(std::move(game)) {
        botAI = {};
    }

    void setup(const GameSetup& setup) {
        setup.setupGame(*game);
        turn = GameSide::Player;
    }

    const GridView& playerView() const { return game->playerGridView; }
    const GridView& botView() const { return game->botGridView; }

    bool isGameOver() {
        return totalBotShipHit == game->targetTotalShipSize ||
               totalPlayerShipHit == game->targetTotalShipSize;
    }

    GameSide winner() {
        if (totalPlayerShipHit == game->targetTotalShipSize)
            return GameSide::Player;
        if (totalBotShipHit == game->targetTotalShipSize) return GameSide::Bot;
        return GameSide::None;
    }

    GameSide currentTurn() const { return turn; }

    Position botMove() {
        Position pos = botAI.computeBotMove(game->playerGrid);
        processMove(game->playerGrid, pos);
        lastBotMoves.push_back(pos);
        return pos;
    }

    std::vector<Position> popAllBotMoves() {
        return std::exchange(lastBotMoves, {});
    }

    bool playerMove(const Position& move) {
        CellAttackResult result = processMove(game->botGrid, move);
        return result.changedCell;
    }

    bool hitBotShipSuccess(const Position& move) {
        return game->botGrid.isType(move, CellType::AttackedShip);
    }

    MoveParseResult parsePlayerMove(const std::string& input) const {
        return MoveRepresentation::parseMove(input,
                                             game->playerGrid.dimension());
    }

    std::string moveToStrCoordinate(const Position& move) const {
        return MoveRepresentation::moveToStrCoordinate(move);
    }

   private:
    void switchTurn() {
        turn = (turn == GameSide::Player) ? GameSide::Bot : GameSide::Player;
    }

    CellAttackResult processMove(Grid& grid, const Position& move) {
        Cell& cell = grid.getCell(move);
        CellType attackedCellVersion = attackedVersion(cell.type);
        bool changedCell = attackedCellVersion != cell.type;
        if (changedCell) {
            cell.type = attackedCellVersion;
            processHit(grid, move);
        }
        return {cell.type, changedCell};
    }

    void processHit(Grid& grid, const Position& move) {
        if (grid.isType(move, CellType::AttackedShip)) {
            if (turn == GameSide::Player)
                ++totalPlayerShipHit;
            else
                ++totalBotShipHit;
        }
        if (turn == GameSide::Bot && isShipSunk(grid, move))
            botAI.onLastHitSunkShip();
        bool hitShipNotSunking = grid.isType(move, CellType::AttackedShip) &&
                                 !isShipSunk(grid, move);
        if (!hitShipNotSunking) switchTurn();
    }

    static bool isShipSunk(Grid& grid, const Position& pos) {
        if (!grid.hasCell(pos) || !grid.isType(pos, CellType::AttackedShip))
            return false;
        ShipBody& shipBody = grid.getCell(pos).shipBody;
        Position currentPos = shipBody.initialPos;
        for (int offset = 0; offset < shipBody.ship->size; ++offset) {
            if (grid.isType(currentPos, CellType::Ship)) return false;
            currentPos.applyOffset(shipBody.direction, 1);
        }
        return true;
    }

   private:
    std::unique_ptr<Game> game;
    std::vector<Position> lastBotMoves;
    BotAI botAI;
    int totalBotShipHit{};
    int totalPlayerShipHit{};
    GameSide turn;
};

class GameLoop {
   public:
    GameLoop(GameLogic& logic) : gameLogic(logic) {}

    void setup(GameUI& gameUI) { this->gameUI = &gameUI; }

    void onPlayerMove(std::string moveInput) {
        this->moveInput = moveInput;
        managePlayerMoveWaiting(false);
    }

    void run() {
        gameUI->onNewGame();
        readyForNewPlayerTurn = true;
        while (!gameLogic.isGameOver()) processTurn();
        gameUI->showGameOver(gameLogic.winner());
    }

   private:
    void processTurn() {
        if (gameLogic.currentTurn() == GameSide::Player) {
            handlePlayerTurn();
        } else {
            handleBotTurn();
        }
    }

    void handlePlayerTurn() {
        if (readyForNewPlayerTurn) handleNewPlayerTurn();
        if (waitingMove) return;
        auto move = processPlayerMove();
        if (processPlayerMoveResult(move)) readyForNewPlayerTurn = true;
    }

    void handleNewPlayerTurn() {
        gameUI->showGrids(gameLogic.playerView(), gameLogic.botView());
        for (auto botMove : gameLogic.popAllBotMoves())
            gameUI->showBotMove(botMove);
        managePlayerMoveWaiting(true);
        readyForNewPlayerTurn = false;
    }

    bool processPlayerMoveResult(const std::optional<Position>& move) {
        if (!move) {
            managePlayerMoveWaiting(true);
            return false;
        }
        managePlayerMoveWaiting(false);
        gameUI->showPlayerMove(*move);
        return true;
    }

    void managePlayerMoveWaiting(bool waiting) {
        this->waitingMove = waiting;
        if (waiting) gameUI->onWaitingPlayerMove();
    }

    std::optional<Position> processPlayerMove() const {
        auto [pos, error] = gameLogic.parsePlayerMove(moveInput);
        if (error == MoveParseError::None && gameLogic.playerMove(pos)) {
            return pos;
        }
        if (error != MoveParseError::None)
            gameUI->showParseError(error);
        else
            gameUI->showInvalidMoveMessage();
        return std::nullopt;
    }

    void handleBotTurn() {
        Position move = gameLogic.botMove();
        gameUI->showBotMove(move);
    }

   private:
    GameLogic& gameLogic;
    GameUI* gameUI;
    bool waitingMove;
    bool readyForNewPlayerTurn{};
    std::string moveInput{};
};

int main() {
    auto game = std::make_unique<Game>(GRID_WIDTH, GRID_HEIGHT, SHIPS_AMOUNT);
    GameLogic logic(std::move(game));
    GameSetup setup;
    logic.setup(setup);
    GameLoop gameLoop(logic);
    ConsoleUI gameUI([&](auto move) { gameLoop.onPlayerMove(move); });
    gameLoop.setup(gameUI);
    gameLoop.run();
}
