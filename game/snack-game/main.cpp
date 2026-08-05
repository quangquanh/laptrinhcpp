#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

struct Point {
    int x;
    int y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

enum class Direction { Up, Down, Left, Right };

class Terminal {
public:
    Terminal() {
        tcgetattr(STDIN_FILENO, &oldSettings_);
        termios settings = oldSettings_;
        settings.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        settings.c_cc[VMIN] = 0;
        settings.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &settings);
        std::cout << "\033[?25l\033[2J" << std::flush;
    }

    ~Terminal() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings_);
        std::cout << "\033[?25h\033[0m\n" << std::flush;
    }

    bool hasInput() const {
        timeval timeout{0, 0};
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(STDIN_FILENO, &readSet);
        return select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout) > 0;
    }

    char readKey() const {
        char key = 0;
        ::read(STDIN_FILENO, &key, 1);

        // Chuyen phim mui ten thanh W/A/S/D.
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
    termios oldSettings_{};
};

class SnakeGame {
public:
    SnakeGame() : randomEngine_(std::random_device{}()) {
        reset();
    }

    void run() {
        Terminal terminal;
        render();

        while (!quit_) {
            const auto frameStart = std::chrono::steady_clock::now();
            processInput(terminal);

            if (!paused_ && !gameOver_) {
                update();
            }

            render();
            const auto elapsed = std::chrono::steady_clock::now() - frameStart;
            const auto delay = std::chrono::milliseconds(speedMs_);
            if (elapsed < delay) {
                std::this_thread::sleep_for(delay - elapsed);
            }
        }
    }

private:
    static constexpr int kWidth = 30;
    static constexpr int kHeight = 18;

    std::deque<Point> snake_;
    Point food_{};
    Direction direction_ = Direction::Right;
    Direction nextDirection_ = Direction::Right;
    std::mt19937 randomEngine_;
    int score_ = 0;
    int highScore_ = 0;
    int speedMs_ = 140;
    bool paused_ = false;
    bool gameOver_ = false;
    bool quit_ = false;

    void reset() {
        snake_.clear();
        snake_.push_back({kWidth / 2, kHeight / 2});
        snake_.push_back({kWidth / 2 - 1, kHeight / 2});
        snake_.push_back({kWidth / 2 - 2, kHeight / 2});
        direction_ = Direction::Right;
        nextDirection_ = Direction::Right;
        score_ = 0;
        speedMs_ = 140;
        paused_ = false;
        gameOver_ = false;
        placeFood();
    }

    bool occupies(const Point& point) const {
        return std::find(snake_.begin(), snake_.end(), point) != snake_.end();
    }

    void placeFood() {
        std::uniform_int_distribution<int> xDistribution(1, kWidth - 2);
        std::uniform_int_distribution<int> yDistribution(1, kHeight - 2);
        do {
            food_ = {xDistribution(randomEngine_), yDistribution(randomEngine_)};
        } while (occupies(food_));
    }

    static bool isOpposite(Direction first, Direction second) {
        return (first == Direction::Up && second == Direction::Down) ||
               (first == Direction::Down && second == Direction::Up) ||
               (first == Direction::Left && second == Direction::Right) ||
               (first == Direction::Right && second == Direction::Left);
    }

    void processInput(const Terminal& terminal) {
        while (terminal.hasInput()) {
            char key = terminal.readKey();
            if (key >= 'A' && key <= 'Z') key += 'a' - 'A';

            if (key == 'q') {
                quit_ = true;
            } else if (key == 'p' && !gameOver_) {
                paused_ = !paused_;
            } else if (key == 'r' && gameOver_) {
                reset();
            } else {
                Direction requested = nextDirection_;
                if (key == 'w') requested = Direction::Up;
                if (key == 's') requested = Direction::Down;
                if (key == 'a') requested = Direction::Left;
                if (key == 'd') requested = Direction::Right;
                if (!isOpposite(direction_, requested)) nextDirection_ = requested;
            }
        }
    }

    void update() {
        direction_ = nextDirection_;
        Point newHead = snake_.front();
        if (direction_ == Direction::Up) --newHead.y;
        if (direction_ == Direction::Down) ++newHead.y;
        if (direction_ == Direction::Left) --newHead.x;
        if (direction_ == Direction::Right) ++newHead.x;

        const bool hitsWall = newHead.x <= 0 || newHead.x >= kWidth - 1 ||
                              newHead.y <= 0 || newHead.y >= kHeight - 1;
        // Duoi se di chuyen trong cung luot, nen khong tinh no khi khong an moi.
        const bool eatsFood = newHead == food_;
        auto bodyEnd = snake_.end();
        if (!eatsFood && bodyEnd != snake_.begin()) --bodyEnd;
        const bool hitsBody = std::find(snake_.begin(), bodyEnd, newHead) != bodyEnd;

        if (hitsWall || hitsBody) {
            gameOver_ = true;
            highScore_ = std::max(highScore_, score_);
            return;
        }

        snake_.push_front(newHead);
        if (eatsFood) {
            score_ += 10;
            speedMs_ = std::max(55, 140 - (score_ / 50) * 10);
            placeFood();
        } else {
            snake_.pop_back();
        }
    }

    void render() const {
        constexpr const char* reset = "\033[0m";
        constexpr const char* green = "\033[38;5;82m";
        constexpr const char* softGreen = "\033[38;5;114m";
        constexpr const char* yellow = "\033[38;5;220m";
        constexpr const char* red = "\033[38;5;203m";
        constexpr const char* gray = "\033[38;5;245m";
        constexpr const char* border = "\033[38;5;42m";
        constexpr const char* field = "\033[48;5;234m";
        constexpr const char* snakeHead = "\033[48;5;46m";
        constexpr const char* snakeBody = "\033[48;5;34m";
        constexpr const char* foodColor = "\033[48;5;196m";

        const int level = 1 + score_ / 50;
        std::string screen = "\033[H";
        screen += green;
        screen += "                 ╔══════════════════════════╗\n";
        screen += "                 ║       S N A K E          ║\n";
        screen += "                 ╚══════════════════════════╝\n";
        screen += reset;
        screen += "   ";
        screen += yellow;
        screen += "ĐIỂM: " + std::to_string(score_);
        screen += "     ";
        screen += softGreen;
        screen += "KỶ LỤC: " + std::to_string(std::max(highScore_, score_));
        screen += "     ";
        screen += yellow;
        screen += "CẤP ĐỘ: " + std::to_string(level) + "\n";

        screen += border;
        screen += "╔";
        for (int x = 0; x < kWidth - 2; ++x) screen += "══";
        screen += "╗\n";

        for (int y = 1; y < kHeight - 1; ++y) {
            screen += border;
            screen += "║";
            int activeStyle = -1;
            for (int x = 1; x < kWidth - 1; ++x) {
                const Point current{x, y};
                int wantedStyle = 0;
                if (current == snake_.front()) {
                    wantedStyle = 1;
                } else if (current == food_) {
                    wantedStyle = 3;
                } else if (occupies(current)) {
                    wantedStyle = 2;
                }

                if (wantedStyle != activeStyle) {
                    if (wantedStyle == 0) screen += field;
                    if (wantedStyle == 1) screen += snakeHead;
                    if (wantedStyle == 2) screen += snakeBody;
                    if (wantedStyle == 3) screen += foodColor;
                    activeStyle = wantedStyle;
                }
                screen += "  ";
            }
            screen += reset;
            screen += border;
            screen += "║\n";
        }

        screen += border;
        screen += "╚";
        for (int x = 0; x < kWidth - 2; ++x) screen += "══";
        screen += "╝\n";
        screen += reset;
        screen += gray;
        screen += "  [WASD / MŨI TÊN] Di chuyển    [P] Tạm dừng    [Q] Thoát\n";
        screen += reset;

        if (paused_) {
            screen += yellow;
            screen += "                  ───  ĐANG TẠM DỪNG  ───                  \n";
        } else if (gameOver_) {
            screen += red;
            screen += "             GAME OVER!  [R] Chơi lại  [Q] Thoát             \n";
        } else {
            screen += softGreen;
            screen += "                   Ăn ô màu đỏ để ghi điểm!                  \n";
        }
        screen += reset;

        std::cout << screen << std::flush;
    }
};

int main() {
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        std::cerr << "Tro choi can duoc chay trong Terminal.\n";
        return 1;
    }

    SnakeGame game;
    game.run();
    return 0;
}
