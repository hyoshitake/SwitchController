#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include "joycon.h"

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * メイン関数
 * Joy-Conの接続を管理し、Accelセンサーのデータを表示する
 */
int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Windowsコンソールの出力コードページをUTF-8に設定
    SetConsoleOutputCP(CP_UTF8);
#endif

    printf("[DEBUG] プログラムを起動します\n");

    // SDLの初期化
    if (!SDL_Init(SDL_INIT_GAMEPAD)) {
        printf("[ERROR] SDLの初期化に失敗: %s\n", SDL_GetError());
        return 1;
    }
    printf("[DEBUG] SDL初期化完了\n");

    bool running = true;
    SDL_Gamepad* gamepad = NULL;
    bool accel_enabled = false;

    while (running) {
        // 1. Joy-Conの接続を待つ
        gamepad = joycon_wait_connection();

        // 2. Accelセンサーのチェック
        bool has_accel = joycon_check_accel(gamepad);

        accel_enabled = false;
        if (has_accel) {
            // 3. Accelセンサーを有効化
            accel_enabled = joycon_enable_accel(gamepad);
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
                    if (gamepad && SDL_GetGamepadID(gamepad) == event.gdevice.which) {
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

            // 3. Accelセンサーが有効な場合、大きな動きを検出
            if (accel_enabled) {
                joycon_detect_big_motion(gamepad);
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
