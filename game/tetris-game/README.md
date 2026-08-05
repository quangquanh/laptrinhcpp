# Tetris bằng C++

Game Tetris chạy trực tiếp trong Terminal macOS/Linux và không cần thư viện ngoài.

## Chạy game

```bash
cd game/tetris-game
make run
```

## Điều khiển

- `←` / `A`: sang trái
- `→` / `D`: sang phải
- `↑` / `W`: xoay khối
- `↓` / `S`: hạ nhanh
- `Space`: thả khối xuống ngay
- `P`: tạm dừng
- `R`: chơi lại sau khi thua
- `Q`: thoát

Game có đủ 7 khối Tetris, bóng vị trí rơi, xem trước khối tiếp theo, tính điểm, cấp độ, kỷ lục và tốc độ tăng dần.
