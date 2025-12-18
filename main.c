#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include "joycon.h"

/**
 * メイン関数
 * Joy-Conの接続を管理し、Gyroセンサーのデータを表示する
 */
int main(int argc, char* argv[]) {
    printf("[DEBUG] プログラムを起動します\n");

    // SDLの初期化
    if (!SDL_Init(SDL_INIT_GAMEPAD)) {
        printf("[ERROR] SDLの初期化に失敗: %s\n", SDL_GetError());
        return 1;
    }
    printf("[DEBUG] SDL初期化完了\n");

    bool running = true;
    SDL_Gamepad* gamepad = NULL;
    bool gyro_enabled = false;

    while (running) {
        // 1. Joy-Conの接続を待つ
        gamepad = joycon_wait_connection();

        // 2. Gyroセンサーのチェック
        bool has_gyro = joycon_check_gyro(gamepad);

        gyro_enabled = false;
        if (has_gyro) {
            // 3. Gyroセンサーを有効化
            gyro_enabled = joycon_enable_gyro(gamepad);
        }

        // メインループ
        SDL_Event event;
        bool connected = true;

        while (connected) {
            // イベント処理
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    printf("[DEBUG] 終了イベントを受信\n");
                    running = false;
                    connected = false;
                } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                    printf("[DEBUG] Gamepad切断イベントを検出: Instance ID = %d\n", event.gdevice.which);
                    if (gamepad && SDL_GetGamepadInstanceID(gamepad) == event.gdevice.which) {
                        // 4. 切断時の処理
                        printf("Disconnected!!\n");
                        SDL_CloseGamepad(gamepad);
                        gamepad = NULL;
                        connected = false;
                    }
                }
            }

            if (!connected) {
                break;
            }

            // 接続チェック
            if (!joycon_is_connected(gamepad)) {
                printf("Disconnected!!\n");
                SDL_CloseGamepad(gamepad);
                gamepad = NULL;
                connected = false;
                break;
            }

            // 3. Gyroセンサーが有効な場合、データを表示
            if (gyro_enabled) {
                joycon_read_and_display_gyro(gamepad);
            }

            // CPU負荷を下げるため少し待機（約60Hz）
            SDL_Delay(16);
        }

        if (!running) {
            break;
        }
    }

    // クリーンアップ
    if (gamepad) {
        SDL_CloseGamepad(gamepad);
    }
    SDL_Quit();

    printf("[DEBUG] プログラムを終了します\n");
    return 0;
}
