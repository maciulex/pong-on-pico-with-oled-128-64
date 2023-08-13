#ifndef PONG_F
#define PONG_F

#include <stdio.h>
#include <string>

#include "pico/stdlib.h"
#include "SH1106.cpp"
#include "font.cpp"


namespace PONG {
    const uint8_t ALLWAYS_HIGHT = 15;
    const uint8_t P1_DOWN = 16;
    const uint8_t P1_UP   = 17;
    const uint8_t P2_DOWN = 18;
    const uint8_t P2_UP   = 19;

    const uint8_t START_BUTTON= 20;
    uint8_t score[2] {0,0};

    uint8_t ballCords[2] {63, 31+6};
    int8_t ballDirection[2] {1,0};
    uint8_t ballVelocity    = 1;
    uint8_t player1 {32+6};
    uint8_t player2 {32+6};



    void cleanFrame() {
        for (int x = 1; x < 127; x++) {
            for (int y = 14; y < 63; y++) {
                SH1106::frame[x][y] = 0;
            }
        }
    }

    void drawBall() {
        for (int x = -1; x < 2; x++) {
            for (int y = -1; y < 2; y++) {
                SH1106::frame[x+ballCords[0]][y+ballCords[1]] = 1;
            }
        }
    }

    void drawPlayers() {
        for (int x = 0; x <= 3; x++) {
            for (int y = -5; y <= 5; y++) {
                SH1106::frame[5+x     ][y+player1] = 1;
                SH1106::frame[128-5-x ][y+player2] = 1;
            }
        }
    }

    void drawBorder() {
        for (int i = 0; i < 128; i++) {
            SH1106::frame[i][13]  = 1;
            SH1106::frame[i][63]  = 1;
        }
        for (int i = 13; i < 64; i++) {
            SH1106::frame[0]  [i]  = 1;
            SH1106::frame[127][i]  = 1;
        }
    }

    void drawScore() {
        std::string scoreSTR = std::to_string(score[0])+"  "+ std::to_string(score[1]);
        FONT::putString(scoreSTR, 63-((scoreSTR.length()/2)*8), 0, (bool *)(SH1106::frame), 2);
    }

    int8_t ballPhisisc() {
        if (ballCords[1] <= 14 && ballDirection[1] < 0) {
            ballDirection[1] *= -1;
        } else if (ballCords[1] >= 63 && ballDirection[1] > 0) {
            ballDirection[1] *= -1;
        }

        ballCords[0] += ballDirection[0] * ballVelocity;
        ballCords[1] += ballDirection[1] * ballVelocity;

        if (ballCords[0] <= 6) return 2;
        if (ballCords[0] >= 122) return 1;


        if ((ballCords[1] <= player1+5+1 && ballCords[1] >= player1-5-1) && ballCords[0] == 9) {
            if ((ballCords[1] <= player1+5+1 && ballCords[1] >= player1+2)) {
                ballDirection[0] = 1;
                ballDirection[1] = 1;
            }else if ((ballCords[1] <= player1-2 && ballCords[1] >= player1-5-1)) {
                ballDirection[0] = 1;
                ballDirection[1] = -1;
            } else {
                ballDirection[0] = 1;
                ballDirection[1] = 0;
            }
        }

        if ((ballCords[1] <= player2+5+1 && ballCords[1] >= player2-5-1) && ballCords[0] == 119) {
            if ((ballCords[1] <= player2+5+1 && ballCords[1] >= player2+2)) {
                ballDirection[0] = -1;
                ballDirection[1] = 1;
            }else if ((ballCords[1] <= player2-2 && ballCords[1] >= player2-5-1)) {
                ballDirection[0] = -1;
                ballDirection[1] = -1;
            } else {
                ballDirection[0] = -1;
                ballDirection[1] = 0;
            }
        }
        return 0;
    }

    void gameEnd() {
        player1 = 32+6;
        player2 = 32+6;
        ballCords[0] = 63;
        ballCords[1] = 31+6;
        ballDirection[0] = 1;
        ballDirection[1] = 0;

        cleanFrame();
        drawPlayers();
        drawScore();
        drawBall();

        SH1106::displayFrame();
    }

    uint8_t gameInProgress() {
        while (1) {
            int8_t p1Move, p2Move;
            for (int i = 0; i < 10; i++) {
                p1Move = (gpio_get(P1_DOWN)* -1) + gpio_get(P1_UP);
                p2Move = (gpio_get(P2_DOWN)* -1) + gpio_get(P2_UP);
                sleep_ms(1);
            }
            if (!(player1+p1Move > 63-5) && !(player1+p1Move < 13+5)) 
                player1 += p1Move;
            if (!(player2+p2Move > 63-5) && !(player2+p2Move < 13+5)) 
                player2 += p2Move;

            cleanFrame();
            int8_t ballResult = ballPhisisc();
            if (ballResult == 1) {
                score[0] += 1;
                return 1;
            } else if(ballResult == 2) {
                score[1] += 1;
                return 2;
            }
            drawPlayers();
            drawBall();

            SH1106::displayFrame();
        }
    }

    void mainGameLoop() {
        SH1106::displayFrame();
        while(1){
            gameInProgress();
            gameEnd();
            while (!gpio_get(START_BUTTON));
        }
    }


    void initControls() {
        gpio_init   (ALLWAYS_HIGHT);
        gpio_set_dir(ALLWAYS_HIGHT, GPIO_OUT);
        gpio_put    (ALLWAYS_HIGHT, 1);

        gpio_init     (P1_DOWN);
        gpio_set_dir  (P1_DOWN, GPIO_IN);
        gpio_pull_down(P1_DOWN);

        gpio_init     (P1_UP);
        gpio_set_dir  (P1_UP, GPIO_IN);
        gpio_pull_down(P1_UP);

        gpio_init     (P2_DOWN);
        gpio_set_dir  (P2_DOWN, GPIO_IN);
        gpio_pull_down(P2_DOWN);

        gpio_init     (P2_UP);
        gpio_set_dir  (P2_UP, GPIO_IN);
        gpio_pull_down(P2_UP);

        gpio_init     (START_BUTTON);
        gpio_set_dir  (START_BUTTON, GPIO_IN);
        gpio_pull_down(START_BUTTON);
    }

    void gameStart() {
        initControls();

        drawBorder();
        drawScore();
        drawBall();
        drawPlayers();
        mainGameLoop();
    }

} // namespace PONG

#endif
