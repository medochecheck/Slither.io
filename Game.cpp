#include "GameData.h"
#include "Engine.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <cstring>
#include <cstdio>
#include <Windows.h>

// ���������� ����������
Snake playerSnake;
std::vector<Snake> enemySnakes;
std::vector<Food> foods(FOOD_COUNT);
int score;
int finalScore = 0;
bool game_over = false; // ���� ��������� ����

// ������� ������������� ���� � �������������� �������������� ����� (8x10 �������� ��� ������ �����)
static const uint8_t font[10][10] = {
    // 0
    {0x1C, 0x36, 0x6B, 0x6B, 0x6B, 0x6B, 0x6B, 0x36, 0x1C, 0x00},
    // 1
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    // 2
    {0x3E, 0x63, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x7F, 0x00, 0x00},
    // 3
    {0x00, 0x7C, 0xC6, 0x06, 0x3C, 0x06, 0xC6, 0xC6, 0x7C, 0x00},
    // 4
    {0x0E, 0x1E, 0x3E, 0x7E, 0xFE, 0xFE, 0x0E, 0x0E, 0x0E, 0x00},
    // 5
    {0x7F, 0x60, 0x60, 0x7E, 0x01, 0x01, 0x01, 0x01, 0x7E, 0x00},
    // 6
    {0x1E, 0x33, 0x60, 0x7E, 0x61, 0x61, 0x61, 0x33, 0x1E, 0x00},
    // 7
    {0xFE, 0xC6, 0x06, 0x0E, 0x1C, 0x30, 0x30, 0x30, 0x30, 0x00},
    // 8
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00},
    // 9
    {0x1E, 0x33, 0x33, 0x1F, 0x01, 0x01, 0x33, 0x1E, 0x00, 0x00}
};

// ������� ������ ��� ���������� ��������� � �������� �����
const int FONT_SCALE_X = 8; // ������ ������ �������
const int FONT_SCALE_Y = 10; // ������ ������ �������

void draw_text(int x, int y, const std::string& text, uint32_t color, bool large)
{
    int scaleX = large ? FONT_SCALE_X : FONT_SCALE_X / 2;
    int scaleY = large ? FONT_SCALE_Y : FONT_SCALE_Y / 2;

    int offsetX = x; // ��������� ������� �� X
    int offsetY = y; // ��������� ������� �� Y

    for (size_t i = 0; i < text.length(); ++i)
    {
        char c = text[i];
        if (c >= '0' && c <= '9')
        {
            int index = c - '0';
            const uint8_t* glyph = font[index];

            for (int dy = 0; dy < FONT_SCALE_Y; ++dy)
            {
                for (int dx = 0; dx < FONT_SCALE_X; ++dx)
                {
                    if (glyph[dy] & (1 << (7 - dx)))
                    {
                        int px = offsetX + dx * scaleX;
                        int py = offsetY + dy * scaleY;
                        if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                        {
                            for (int sy = 0; sy < scaleY; ++sy)
                            {
                                for (int sx = 0; sx < scaleX; ++sx)
                                {
                                    int pxScaled = px + sx;
                                    int pyScaled = py + sy;
                                    if (pxScaled >= 0 && pxScaled < SCREEN_WIDTH && pyScaled >= 0 && pyScaled < SCREEN_HEIGHT)
                                    {
                                        buffer[pyScaled][pxScaled] = color;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // �������� ��� ���������� �������
            offsetX += scaleX * FONT_SCALE_X; // ��������� ������ �������
        }
    }
}

void initialize()
{
    srand(static_cast<unsigned>(time(0)));
    // ������������� ������
    playerSnake.body = { {10, 10} };
    playerSnake.direction = 1; // ��������� �������������� ������
    playerSnake.growCount = 0;

    // ������������� ����������
    enemySnakes.clear();
    for (int i = 0; i < 1; ++i) // �������� � ������ ���������
    {
        Snake enemy;
        enemy.initialLength = rand() % 10 + 3; // ��������� ����� �� 3 �� 12 ���������
        enemy.body.clear();
        for (int j = 0; j < enemy.initialLength; ++j)
        {
            Point segment;
            segment.x = rand() % (SCREEN_WIDTH / GRID_SIZE);
            segment.y = rand() % (SCREEN_HEIGHT / GRID_SIZE);
            enemy.body.push_back(segment);
        }
        enemy.direction = rand() % 4;
        enemy.growCount = 0;
        enemySnakes.push_back(enemy);
    }

    // ������������� ���
    for (auto& food : foods)
    {
        food.position = { rand() % (SCREEN_WIDTH / GRID_SIZE), rand() % (SCREEN_HEIGHT / GRID_SIZE) };
    }

    // ��������� ����
    score = 0;
    game_over = false; // ���� �� ��������
}

void update_enemies()
{
    for (auto& enemy : enemySnakes)
    {
        move_snake(enemy);
    }

    // ��������� ������������ ����� �������
    for (auto it1 = enemySnakes.begin(); it1 != enemySnakes.end(); ++it1)
    {
        for (auto it2 = std::next(it1); it2 != enemySnakes.end(); ++it2)
        {
            if (check_collision(*it1, *it2))
            {
                // ��������� �������� ���� ��������� � ���
                for (const auto& segment : it2->body)
                {
                    foods.push_back({ segment });
                }

                // ������� ����������
                it2 = enemySnakes.erase(it2);
                if (it2 == enemySnakes.end())
                {
                    break;
                }
            }
        }
    }

    // ��������� ����� ������
    while (enemySnakes.size() < MAX_ENEMY_COUNT)
    {
        Snake newEnemy;
        newEnemy.initialLength = rand() % 10 + 3; // ��������� ����� �� 3 �� 12 ���������
        newEnemy.body.clear();
        for (int j = 0; j < newEnemy.initialLength; ++j)
        {
            Point segment;
            segment.x = rand() % (SCREEN_WIDTH / GRID_SIZE);
            segment.y = rand() % (SCREEN_HEIGHT / GRID_SIZE);
            newEnemy.body.push_back(segment);
        }
        newEnemy.direction = rand() % 4;
        newEnemy.growCount = 0;
        enemySnakes.push_back(newEnemy);
    }
}

void place_food()
{
    for (auto& food : foods)
    {
        food.position = { rand() % (SCREEN_WIDTH / GRID_SIZE), rand() % (SCREEN_HEIGHT / GRID_SIZE) };
    }
}

void move_snake(Snake& snake)
{
    Point head = snake.body.front();
    switch (snake.direction)
    {
    case 0: head.y--; break; // �����
    case 1: head.x++; break; // ������
    case 2: head.y++; break; // ����
    case 3: head.x--; break; // �����
    }

    // �������� �� ������������ � ���������
    if (head.x < 0) head.x = (SCREEN_WIDTH / GRID_SIZE) - 1;
    if (head.x >= (SCREEN_WIDTH / GRID_SIZE)) head.x = 0;
    if (head.y < 0) head.y = (SCREEN_HEIGHT / GRID_SIZE) - 1;
    if (head.y >= (SCREEN_HEIGHT / GRID_SIZE)) head.y = 0;

    // �������� �� ������������ � ����������� �����
    if (std::find(snake.body.begin(), snake.body.end(), head) != snake.body.end())
    {
        game_over = true;
        return;
    }

    snake.body.insert(snake.body.begin(), head);

    if (snake.growCount > 0)
    {
        snake.growCount--;
    }
    else
    {
        snake.body.pop_back();
    }
}

bool check_collision(const Snake& snake1, const Snake& snake2)
{
    return std::find(snake2.body.begin(), snake2.body.end(), snake1.body.front()) != snake2.body.end();
}

void handle_collision()
{
    if (game_over)
    {
        return; // ���������� ����������, ���� ���� ��������
    }

    // ���� ������ ������ ������������ � ����� ���������
    for (const auto& enemy : enemySnakes)
    {
        if (check_collision(playerSnake, enemy))
        {
            // ���������� �������� ���� ������ � ���
            for (const auto& segment : playerSnake.body)
            {
                foods.push_back({ segment });
            }
            playerSnake.body.clear(); // ���� ������ ��������
            game_over = true; // ���� ��������
            return; // ���������� �������� ������������
        }
    }
    // �������� ������������ ������ ���� ���������� �� ����� ������
    for (auto it = enemySnakes.begin(); it != enemySnakes.end();)
    {
        bool collided = false;
        for (const auto& segment : it->body)
        {
            if (std::find(playerSnake.body.begin(), playerSnake.body.end(), segment) != playerSnake.body.end())
            {
                collided = true;
                break;
            }
        }

        if (collided)
        {
            // ���������� �������� ���� ��������� � ���
            for (const auto& segment : it->body)
            {
                foods.push_back({ segment });
            }
            // ������� ���������
            it = enemySnakes.erase(it);
        }
        else
        {
            ++it;
        }
    }
    // �������� ������������ ����� �����������
    for (auto it1 = enemySnakes.begin(); it1 != enemySnakes.end(); ++it1)
    {
        for (auto it2 = std::next(it1); it2 != enemySnakes.end();)
        {
            if (check_collision(*it1, *it2))
            {
                // ���������� �������� ���� ��������� � ���
                for (const auto& segment : it2->body)
                {
                    foods.push_back({ segment });
                }
                // ������� ���������
                it2 = enemySnakes.erase(it2);
            }
            else
            {
                ++it2;
            }
        }
    }
}

void act(float dt)
{
    static float timeAccumulator = 0.0f;
    timeAccumulator += dt;

    if (game_over)
    {
        return; // ���������� ����������, ���� ���� ��������
    }

    // ���������� ������������ ���� ������
    if (is_key_pressed(VK_UP) && playerSnake.direction != 2) playerSnake.direction = 0;
    if (is_key_pressed(VK_RIGHT) && playerSnake.direction != 3) playerSnake.direction = 1;
    if (is_key_pressed(VK_DOWN) && playerSnake.direction != 0) playerSnake.direction = 2;
    if (is_key_pressed(VK_LEFT) && playerSnake.direction != 1) playerSnake.direction = 3;

    if (timeAccumulator >= MOVE_INTERVAL)
    {
        move_snake(playerSnake);
        timeAccumulator = 0.0f;

        // �������� �� ������������ � ����
        for (auto it = foods.begin(); it != foods.end();)
        {
            if (playerSnake.body.front().x == it->position.x &&
                playerSnake.body.front().y == it->position.y)
            {
                playerSnake.growCount += 3; // ����������� ������ ���� ������
                score += 1; // ���� �� ��������� ���
                it = foods.erase(it); // ������� ��������� ���
                if (foods.size() < FOOD_COUNT)
                {
                    foods.push_back({ {rand() % (SCREEN_WIDTH / GRID_SIZE), rand() % (SCREEN_HEIGHT / GRID_SIZE)} });
                }
            }
            else
            {
                ++it;
            }
        }

        update_enemies();
        handle_collision();
    }
}

void draw()
{
    // ������� ������
    memset(buffer, 0, sizeof(buffer));

    // ��������� ��� � ���� ���������
    for (const auto& food : foods)
    {
        int centerX = food.position.x * GRID_SIZE + GRID_SIZE / 2;
        int centerY = food.position.y * GRID_SIZE + GRID_SIZE / 2;
        int radius = GRID_SIZE / 2; // ������ ������

        for (int dy = -radius; dy <= radius; ++dy)
        {
            for (int dx = -radius; dx <= radius; ++dx)
            {
                if (dx * dx + dy * dy <= radius * radius) // ��������, ��������� �� ����� ������ ������
                {
                    int px = centerX + dx;
                    int py = centerY + dy;
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                    {
                        buffer[py][px] = 0xFFFF00; // ������ ���� ��� ���
                    }
                }
            }
        }
    }

    // ��������� ������
    for (const auto& segment : playerSnake.body)
    {
        for (int dy = 0; dy < GRID_SIZE; ++dy)
        {
            for (int dx = 0; dx < GRID_SIZE; ++dx)
            {
                int px = segment.x * GRID_SIZE + dx;
                int py = segment.y * GRID_SIZE + dy;
                if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                {
                    buffer[py][px] = 0x00FF00; // ������� ���� ��� ������
                }
            }
        }
    }

    // ��������� ����������
    for (const auto& enemy : enemySnakes)
    {
        for (const auto& segment : enemy.body)
        {
            for (int dy = 0; dy < GRID_SIZE; ++dy)
            {
                for (int dx = 0; dx < GRID_SIZE; ++dx)
                {
                    int px = segment.x * GRID_SIZE + dx;
                    int py = segment.y * GRID_SIZE + dy;
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                    {
                        buffer[py][px] = 0xFF0000; // ������� ���� ��� ���� ���������
                    }
                }
            }
        }
    }

    // ��������� �������� �����
    draw_text(10, 10, std::to_string(score).insert(0, 4 - std::to_string(score).length(), '0'), 0xFFFFFF, true); // ����� ���� ��� �����

    if (game_over)
    {
        draw_text(SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT / 2, "Game Over", 0xFF0000, true); // ������� ���� ��� ��������� � ����� ����
        draw_text(SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT / 2 + 20, "Score: " + std::to_string(score), 0xFFFFFF, true); // ����� ���� ��� ���������� �����
    }
}

void finalize()
{
    
}