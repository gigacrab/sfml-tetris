#include <SFML/Graphics.hpp>
#include "TileMap.hpp"
#include <iostream>
#include <optional>
#include <thread>
#include <chrono>
#include <time.h>

unsigned int screenWidth = 448;
unsigned int screenHeight = 800;
int fieldWidth = 12;
int fieldHeight = 22;

std::vector<std::vector<int>> tetromino;
std::vector<int> field;

int rotate(int px, int py, int r)
{
    int pi = 0;
    switch (r % 4)
    {
    case 0: // 0 degrees            // 0  1  2  3
        pi = py * 4 + px;           // 4  5  6  7
        break;                      // 8  9 10 11
                                    //12 13 14 15

    case 1: // 90 degrees           //12  8  4  0
        pi = 12 + py - (px * 4);    //13  9  5  1
        break;                      //14 10  6  2
                                    //15 11  7  3

    case 2: // 180 degrees          //15 14 13 12
        pi = 15 - (py * 4) - px;    //11 10  9  8
        break;                      // 7  6  5  4
                                    // 3  2  1  0

    case 3: // 270 degrees          // 3  7 11 15
        pi = 3 - py + (px * 4);     // 2  6 10 14
        break;                      // 1  5  9 13
    }                               // 0  4  8 12
    return pi;
}

bool doesPieceFit(int n, int rotation, int posX, int posY){
    for (int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++){
            int pi = rotate(px, py, rotation);
            int fi = (posY + py) * fieldWidth + (posX + px);
            if ((posX + px >= 0 && posX + px < fieldWidth) &&
                (posY + py >= 0 && posY + py < fieldHeight) &&
                (tetromino[n][pi] != 0 && field[fi] != 0)){
                return false;
            }
        }
    return true;
}

int main(){
    // square, left zigzag, right zigzag, t-shaped, l-shaped, line, reverse l-shaped
    tetromino  = {{0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0}, 
        {0, 2, 0, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0}, 
        {0, 0, 3, 0, 0, 3, 3, 0, 0, 3, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 0, 0, 0, 0, 0}, 
        {0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0}, 
        {0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0}, 
        {0, 7, 0, 0, 0, 7, 0, 0, 0, 7, 7, 0, 0, 0, 0, 0}};
        
    sf::RenderWindow window(sf::VideoMode({screenWidth, screenHeight}), "My Window");
    
    sf::Font font("/home/littlecrabby/sfml-tetris/assets/fonts/8-bit-font.ttf");
    sf::Text text(font);
    text.setString("Score: ");
    text.setCharacterSize(32);
    text.setFillColor(sf::Color::Black);

    TileMap map;
    
    srand(time(NULL));
    int currentPiece = rand() % 7;
    int currentX = fieldWidth / 2 - 2;
    int currentY = 0;
    int speedCount = 0;
    int speed = 15;
    int score = 0;

    int currentRotation = 0;
    bool rotateHold = false;
    bool gameOver = false;
    bool forceDown = false;

    std::vector<int> completedLines = {};

    for (int y = 0; y < fieldHeight; y++) {
        for (int x = 0; x < fieldWidth; x++) {
            if (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) {
                field.push_back(8);
            } else {
                field.push_back(0);
            }
        }
    }

    while(window.isOpen()){
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        speedCount++;
        forceDown = (speedCount == speed);

        while(const std::optional event = window.pollEvent()){
            if(event->is<sf::Event::Closed>()){
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                if (keyPressed->code == sf::Keyboard::Key::Left){
                    currentX -= (doesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) ? 1 : 0;
                } else if (keyPressed->code == sf::Keyboard::Key::Right){
                    currentX += (doesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) ? 1 : 0;
                } else if (keyPressed->code == sf::Keyboard::Key::Down){
                    currentY += (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) ? 1 : 0;
                }
                if ((keyPressed->code == sf::Keyboard::Key::Up) && !rotateHold){
                    currentRotation += (doesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) ? 1 : 0;
                    rotateHold = true;
                }
            }
            else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()){
                if (keyReleased->code == sf::Keyboard::Key::Up) {
                    rotateHold = false;
                }
            }
        }

        if (forceDown){
            speedCount = 0;
            if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)){
                currentY++;
            } else {
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++){
                        int block = tetromino[currentPiece][rotate(px, py, currentRotation)];
                        if (block != 0) field[(currentY + py) * fieldWidth + (currentX + px)] = block;
                    }
                for (int py = 0; py < 4; py++){
                    bool complete = true;
                    for (int fx = 1; fx < fieldWidth - 1; fx++){
                        int fi = (currentY + py) * fieldWidth + fx;
                        if (field[fi] == 0 || field[fi] == 8)
                            complete = false;
                    }
                    if (complete){
                        completedLines.push_back(currentY + py);
                        score += 100;
                        for (int fx = 1; fx < fieldWidth - 1; fx++){
                            field[(currentY + py) * fieldWidth + fx] = 0;
                        }
                    }
                }
                currentX = fieldWidth / 2 - 2;
                currentY = 0;
                currentRotation = 0;
                currentPiece = rand() % 7;

                gameOver = !doesPieceFit(currentPiece, currentRotation, currentX, currentY);
                if (gameOver) break;
            }
        }
        text.setString("Score: " + std::to_string(score));
        text.setPosition({64.f, 16.f});

        window.clear(sf::Color::White);

        std::vector<int> temp = field;
        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++){
                int block = tetromino[currentPiece][rotate(px, py, currentRotation)];
                if (block != 0) temp[(currentY + py) * fieldWidth + (currentX + px)] = block;
            }
        if(!map.load("/home/littlecrabby/sfml-tetris/assets/images/tileset.png", {32, 32}, temp.data(), 12, 22))
            return -1;
        map.setPosition({32.f, 64.f});

        window.draw(map);
        window.draw(text);

        window.display();

        if (!completedLines.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			for (auto &v : completedLines)
				for (int fx = 1; fx < fieldWidth - 1; fx++)
				{
					for (int fy = v; fy > 0; fy--)
						field[fy * fieldWidth + fx] = field[(fy - 1) * fieldWidth + fx];
					field[fx] = 0;
				}

			completedLines.clear();
		}
   }
}
