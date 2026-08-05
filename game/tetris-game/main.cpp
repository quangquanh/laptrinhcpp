#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

class Terminal {
public:
    Terminal() {
        tcgetattr(STDIN_FILENO, &old_);
        termios raw = old_;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        std::cout << "\033[?25l\033[2J" << std::flush;
    }

    ~Terminal() {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_);
        std::cout << "\033[?25h\033[0m\n" << std::flush;
    }

    bool hasInput() const {
        timeval timeout{0, 0};
        fd_set set;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        return select(STDIN_FILENO + 1, &set, nullptr, nullptr, &timeout) > 0;
    }

    char readKey() const {
        char key = 0;
        ::read(STDIN_FILENO, &key, 1);
        if (key == '\033' && hasInput()) {
            char bracket = 0;
            ::read(STDIN_FILENO, &bracket, 1);
            if (bracket == '[' && hasInput()) {
                char arrow = 0;
                ::read(STDIN_FILENO, &arrow, 1);
                if (arrow == 'A') return 'w';
                if (arrow == 'B') return 's';
                if (arrow == 'C') return 'd';
                if (arrow == 'D') return 'a';
            }
        }
        return key;
    }

private:
    termios old_{};
};

class Tetris {
public:
    Tetris() : random_(std::random_device{}()) { reset(); }

    void run() {
        Terminal terminal;
        auto lastFall = std::chrono::steady_clock::now();
        render();

        while (!quit_) {
            bool changed = processInput(terminal);
            const auto now = std::chrono::steady_clock::now();
            if (!paused_ && !gameOver_ &&
                now - lastFall >= std::chrono::milliseconds(fallDelay())) {
                if (!tryMove(0, 1)) lockPiece();
                lastFall = now;
                changed = true;
            }
            if (changed) render();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

private:
    static constexpr int kWidth = 10;
    static constexpr int kHeight = 20;
    using Board = std::array<std::array<int, kWidth>, kHeight>;

    // Cac khoi theo thu tu: I, J, L, O, S, T, Z.
    const std::array<std::string, 7> shapes_{
        "....XXXX........", ".X...XXX........", "..X..XXX........",
        ".XX..XX.........", ".XX.XX..........", ".X..XXX.........",
        "XX...XX........."
    };
    const std::array<const char*, 8> colors_{
        "\033[48;5;234m", "\033[48;5;51m", "\033[48;5;27m",
        "\033[48;5;208m", "\033[48;5;226m", "\033[48;5;46m",
        "\033[48;5;129m", "\033[48;5;196m"
    };

    Board board_{};
    std::mt19937 random_;
    int piece_ = 0;
    int nextPiece_ = 0;
    int rotation_ = 0;
    int pieceX_ = 3;
    int pieceY_ = 0;
    int score_ = 0;
    int lines_ = 0;
    int highScore_ = 0;
    bool paused_ = false;
    bool gameOver_ = false;
    bool quit_ = false;

    static int rotatedIndex(int x, int y, int rotation) {
        switch (rotation % 4) {
            case 0: return y * 4 + x;
            case 1: return 12 + y - x * 4;
            case 2: return 15 - y * 4 - x;
            default: return 3 - y + x * 4;
        }
    }

    int randomPiece() {
        return std::uniform_int_distribution<int>(0, 6)(random_);
    }

    void reset() {
        for (auto& row : board_) row.fill(0);
        score_ = 0;
        lines_ = 0;
        paused_ = false;
        gameOver_ = false;
        nextPiece_ = randomPiece();
        spawnPiece();
    }

    void spawnPiece() {
        piece_ = nextPiece_;
        nextPiece_ = randomPiece();
        rotation_ = 0;
        pieceX_ = 3;
        pieceY_ = 0;
        if (!fits(pieceX_, pieceY_, rotation_)) {
            gameOver_ = true;
            highScore_ = std::max(highScore_, score_);
        }
    }

    bool isBlock(int piece, int rotation, int x, int y) const {
        return shapes_[piece][rotatedIndex(x, y, rotation)] == 'X';
    }

    bool fits(int newX, int newY, int newRotation) const {
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                if (!isBlock(piece_, newRotation, x, y)) continue;
                const int boardX = newX + x;
                const int boardY = newY + y;
                if (boardX < 0 || boardX >= kWidth || boardY >= kHeight) return false;
                if (boardY >= 0 && board_[boardY][boardX] != 0) return false;
            }
        }
        return true;
    }

    bool tryMove(int dx, int dy) {
        if (!fits(pieceX_ + dx, pieceY_ + dy, rotation_)) return false;
        pieceX_ += dx;
        pieceY_ += dy;
        return true;
    }

    void rotate() {
        const int wanted = (rotation_ + 1) % 4;
        // Wall kick don gian de xoay khoi sat tuong.
        for (int offset : {0, -1, 1, -2, 2}) {
            if (fits(pieceX_ + offset, pieceY_, wanted)) {
                pieceX_ += offset;
                rotation_ = wanted;
                return;
            }
        }
    }

    int clearLines() {
        int cleared = 0;
        for (int y = kHeight - 1; y >= 0; --y) {
            const bool full = std::all_of(board_[y].begin(), board_[y].end(),
                                          [](int cell) { return cell != 0; });
            if (!full) continue;
            for (int row = y; row > 0; --row) board_[row] = board_[row - 1];
            board_[0].fill(0);
            ++cleared;
            ++y;
        }
        return cleared;
    }

    void lockPiece() {
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                if (!isBlock(piece_, rotation_, x, y)) continue;
                const int boardY = pieceY_ + y;
                if (boardY >= 0) board_[boardY][pieceX_ + x] = piece_ + 1;
            }
        }
        const int cleared = clearLines();
        const std::array<int, 5> points{0, 100, 300, 500, 800};
        score_ += points[cleared] * level();
        lines_ += cleared;
        highScore_ = std::max(highScore_, score_);
        spawnPiece();
    }

    void hardDrop() {
        int dropped = 0;
        while (tryMove(0, 1)) ++dropped;
        score_ += dropped * 2;
        lockPiece();
    }

    int level() const { return 1 + lines_ / 10; }
    int fallDelay() const { return std::max(80, 650 - (level() - 1) * 55); }

    bool processInput(const Terminal& terminal) {
        bool changed = false;
        while (terminal.hasInput()) {
            char key = terminal.readKey();
            if (key >= 'A' && key <= 'Z') key += 'a' - 'A';
            if (key == 'q') {
                quit_ = true;
            } else if (key == 'r' && gameOver_) {
                reset();
                changed = true;
            } else if (key == 'p' && !gameOver_) {
                paused_ = !paused_;
                changed = true;
            } else if (!paused_ && !gameOver_) {
                if (key == 'a') changed = tryMove(-1, 0) || changed;
                if (key == 'd') changed = tryMove(1, 0) || changed;
                if (key == 's') {
                    if (tryMove(0, 1)) ++score_;
                    else lockPiece();
                    changed = true;
                }
                if (key == 'w') { rotate(); changed = true; }
                if (key == ' ') { hardDrop(); changed = true; }
            }
        }
        return changed;
    }

    bool activeAt(int boardX, int boardY) const {
        const int x = boardX - pieceX_;
        const int y = boardY - pieceY_;
        return x >= 0 && x < 4 && y >= 0 && y < 4 && isBlock(piece_, rotation_, x, y);
    }

    int ghostY() const {
        int y = pieceY_;
        while (fits(pieceX_, y + 1, rotation_)) ++y;
        return y;
    }

    void render() const {
        constexpr const char* reset = "\033[0m";
        constexpr const char* border = "\033[38;5;45m";
        constexpr const char* title = "\033[38;5;51m";
        constexpr const char* text = "\033[38;5;252m";
        constexpr const char* accent = "\033[38;5;226m";
        constexpr const char* danger = "\033[38;5;203m";
        constexpr const char* ghost = "\033[48;5;240m";

        const int landingY = ghostY();
        std::string out = "\033[H";
        out += title;
        out += "       ╔══════════════════════╗\n";
        out += "       ║      T E T R I S     ║\n";
        out += "       ╚══════════════════════╝\n";
        out += reset;
        out += border;
        out += "       ╔════════════════════╗";
        out += reset;
        out += text;
        out += "   ĐIỂM: " + std::to_string(score_) + "\n";

        for (int y = 0; y < kHeight; ++y) {
            out += border;
            out += "       ║";
            int style = -1;
            for (int x = 0; x < kWidth; ++x) {
                int cell = board_[y][x];
                int wanted = cell;
                if (activeAt(x, y)) wanted = piece_ + 1;
                else {
                    const int gx = x - pieceX_;
                    const int gy = y - landingY;
                    if (cell == 0 && gx >= 0 && gx < 4 && gy >= 0 && gy < 4 &&
                        isBlock(piece_, rotation_, gx, gy)) wanted = 8;
                }
                if (wanted != style) {
                    out += wanted == 8 ? ghost : colors_[wanted];
                    style = wanted;
                }
                out += "  ";
            }
            out += reset;
            out += border;
            out += "║";
            out += reset;
            out += text;
            if (y == 1) out += "   KỶ LỤC: " + std::to_string(highScore_);
            if (y == 3) out += "   CẤP ĐỘ: " + std::to_string(level());
            if (y == 5) out += "   SỐ HÀNG: " + std::to_string(lines_);
            if (y == 8) out += "   KHỐI TIẾP THEO:";
            if (y >= 10 && y < 14) {
                out += "      ";
                for (int x = 0; x < 4; ++x) {
                    if (isBlock(nextPiece_, 0, x, y - 10)) out += colors_[nextPiece_ + 1];
                    else out += colors_[0];
                    out += "  ";
                }
                out += reset;
            }
            out += "\n";
        }
        out += border;
        out += "       ╚════════════════════╝\n";
        out += reset;
        out += text;
        out += "  [← →] Di chuyển  [↑] Xoay  [↓] Hạ  [SPACE] Thả\n";
        out += "             [P] Tạm dừng    [Q] Thoát\n";
        if (paused_) {
            out += accent;
            out += "                  ── ĐANG TẠM DỪNG ──\n";
        } else if (gameOver_) {
            out += danger;
            out += "             GAME OVER! [R] Chơi lại\n";
        } else {
            out += "                                      \n";
        }
        out += reset;
        std::cout << out << std::flush;
    }
};

int main() {
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        std::cerr << "Tro choi can duoc chay trong Terminal.\n";
        return 1;
    }
    Tetris game;
    game.run();
    return 0;
}
