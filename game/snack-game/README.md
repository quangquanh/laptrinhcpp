# Trò chơi Rắn săn mồi (C++)

Game chạy trong Terminal trên macOS hoặc Linux, không cần thư viện ngoài.

## Biên dịch và chạy

```bash
cd game/snack-game
make run
```

Hoặc biên dịch thủ công:

```bash
c++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o snake-game
./snake-game
```

## Điều khiển

- `W A S D` hoặc các phím mũi tên: di chuyển
- `P`: tạm dừng/tiếp tục
- `R`: chơi lại sau khi thua
- `Q`: thoát game

Rắn được hiển thị bằng các khối màu xanh và thức ăn là khối màu đỏ. Mỗi lần ăn được 10 điểm; tốc độ và cấp độ sẽ tăng dần theo điểm số. Game cũng hiển thị kỷ lục cao nhất trong phiên chơi hiện tại.
