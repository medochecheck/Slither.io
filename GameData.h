#pragma once

#include <vector>
#include <string>
#include <cstdint>

const int GRID_SIZE = 10; // ������ ������ �����
const int FOOD_COUNT = 10; // ���������� ��� �� ����
const float MOVE_INTERVAL = 0.1f; // �������� ���������� ������� ����
const int MAX_ENEMY_COUNT = 10; // ������������ ���������� ����������
const int MIN_ENEMY_LENGTH = 5; // ����������� ����� ���� ���������
const int MAX_ENEMY_LENGTH = 15; // ������������ ����� ���� ���������

struct Point {
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Snake {
    std::vector<Point> body;
    int direction; // 0 - �����, 1 - ������, 2 - ����, 3 - �����
    int growCount; // ������� ����� ����
    int initialLength; // ��������� ����� ���� ���������

    // ����������� � �������������� �������� �� ���������
    Snake()
        : direction(0), growCount(0), initialLength(MIN_ENEMY_LENGTH) {}
};

struct Food {
    Point position;
};

// ���������� ����������
extern Snake playerSnake;
extern std::vector<Snake> enemySnakes;
extern std::vector<Food> foods;
extern int score;
extern bool gameOver;

// �������
void draw_text(int x, int y, const std::string& text, uint32_t color, bool large);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void initialize();
void place_food();
void move_snake(Snake& snake);
bool check_collision(const Snake& snake1, const Snake& snake2);
void handle_collision();
void handle_food_collision(Snake& snake);
void render();
void game_loop();
float get_delta_time();
void render_buffer();
void display_game_over(); // ���������� ������� ����������� ��������� ����
void finalize();