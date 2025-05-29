#include <iostream>
#include <vector>
#include <random>
#include <memory>
#include <iomanip>

constexpr int SIDE = 10;
constexpr int MINES = 10;

enum class CellState {
    Hidden,
    Revealed,
    Flagged
};

struct Cell {
    bool isMine = false;
    int adjacentMines = 0;
    CellState state = CellState::Hidden;
};

using Board = std::vector<std::vector<Cell>>;

class Minesweeper {
public:
    Minesweeper() {
        realBoard = std::make_unique<Board>(SIDE, std::vector<Cell>(SIDE));
        playerBoard = std::make_unique<Board>(SIDE, std::vector<Cell>(SIDE));
        placeMines();
        calculateAdjacents();
    }

    void run() {
        bool gameOver = false;
        int movesLeft = SIDE * SIDE - MINES;

        int turn = 0;
        while (!gameOver) {
            printBoard();
            std::cout << "\nEnter row, col and action (o = open, f = flag): ";
            int row, col;
            char action;
            std::cin >> row >> col >> action;

            if (!isValid(row, col)) {
                std::cout << "Invalid coordinates.\n";
                continue;
            }

            action = std::tolower(action);
            if (action == 'f') {
                toggleFlag(row, col);
                continue;
            } else if (action != 'o') {
                std::cout << "Unknown action.\n";
                continue;
            }

            if (turn == 0 && (*realBoard)[row][col].isMine) {
                relocateMine(row, col);
                calculateAdjacents();
            }

            if ((*playerBoard)[row][col].state == CellState::Flagged) {
                std::cout << "Cell is flagged. Unflag to open.\n";
                continue;
            }

            if (!openCell(row, col, movesLeft)) {
                revealMines();
                printBoard();
                std::cout << "\nBOOM! You hit a mine. Game Over.\n";

                gameOver = true;
            } else if (movesLeft == 0) {
                revealMines();
                printBoard();
                std::cout << "\nCongratulations! You cleared the board!\n";
                gameOver = true;
            }

            ++turn;
        }
    }

private:
    std::unique_ptr<Board> realBoard;
    std::unique_ptr<Board> playerBoard;

    void printBoard() const {
        std::cout << "\n   ";
        for (int i = 0; i < SIDE; ++i)
            std::cout << std::setw(2) << i;
        std::cout << "\n\n";

        for (int i = 0; i < SIDE; ++i) {
            std::cout << std::setw(2) << i << " ";
            for (int j = 0; j < SIDE; ++j) {
                const Cell& cell = (*playerBoard)[i][j];
                switch (cell.state) {
                    case CellState::Hidden: std::cout << " ."; break;
                    case CellState::Flagged: std::cout << " !"; break;
                    case CellState::Revealed:
                        if ((*realBoard)[i][j].isMine) std::cout << " *";
                        else std::cout << " " << (*realBoard)[i][j].adjacentMines;
                        break;
                }
            }
            std::cout << '\n';
        }
    }

    bool isValid(int row, int col) const {
        return row >= 0 && row < SIDE && col >= 0 && col < SIDE;
    }

    void placeMines() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, SIDE - 1);

        int placed = 0;
        while (placed < MINES) {
            int r = dis(gen), c = dis(gen);
            if (!(*realBoard)[r][c].isMine) {
                (*realBoard)[r][c].isMine = true;
                ++placed;
            }
        }
    }

    void relocateMine(int avoidRow, int avoidCol) {
        for (int r = 0; r < SIDE; ++r) {
            for (int c = 0; c < SIDE; ++c) {
                if (!(r == avoidRow && c == avoidCol) && !(*realBoard)[r][c].isMine) {
                    (*realBoard)[r][c].isMine = true;
                    (*realBoard)[avoidRow][avoidCol].isMine = false;
                    return;
                }
            }
        }
    }

    void calculateAdjacents() {
        const int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        const int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

        for (int r = 0; r < SIDE; ++r) {
            for (int c = 0; c < SIDE; ++c) {
                if ((*realBoard)[r][c].isMine) continue;
                int count = 0;
                for (int d = 0; d < 8; ++d) {
                    int nr = r + dr[d], nc = c + dc[d];
                    if (isValid(nr, nc) && (*realBoard)[nr][nc].isMine)
                        ++count;
                }
                (*realBoard)[r][c].adjacentMines = count;
            }
        }
    }

    bool openCell(int row, int col, int& movesLeft) {
        if (!isValid(row, col)) return true;
        Cell& cell = (*playerBoard)[row][col];
        if (cell.state == CellState::Revealed) return true;

        cell.state = CellState::Revealed;
        if ((*realBoard)[row][col].isMine) return false;

        --movesLeft;

        if ((*realBoard)[row][col].adjacentMines == 0) {
            const int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
            const int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};
            for (int d = 0; d < 8; ++d) {
                int nr = row + dr[d], nc = col + dc[d];
                if (isValid(nr, nc) && (*playerBoard)[nr][nc].state == CellState::Hidden)
                    openCell(nr, nc, movesLeft);
            }
        }

        return true;
    }

    void toggleFlag(int row, int col) {
        Cell& cell = (*playerBoard)[row][col];
        if (cell.state == CellState::Revealed) {
            std::cout << "Can't flag a revealed cell.\n";
        } else {
            cell.state = (cell.state == CellState::Flagged) ? CellState::Hidden : CellState::Flagged;
        }
    }

    void revealMines() {
        for (int r = 0; r < SIDE; ++r) {
            for (int c = 0; c < SIDE; ++c) {
                if ((*realBoard)[r][c].isMine)
                    (*playerBoard)[r][c].state = CellState::Revealed;
            }
        }
    }
};

int main() {
    std::cout << "Welcome to Minesweeper! (10x10 with 10 mines)\n";
    Minesweeper game;
    game.run();
    return 0;
}
