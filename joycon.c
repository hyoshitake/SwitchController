#include "joycon.h"
#include <stdio.h>

SDL_Gamepad* joycon_wait_connection(void) {
    printf("[DEBUG] Joy-Con接続待機を開始します\n");
    printf("Please connect joy-con...\n");

    SDL_Gamepad* gamepad = NULL;
    SDL_Event event;

    // イベントループでGamepadの接続を待つ
    while (gamepad == NULL) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                printf("[DEBUG] Gamepad追加イベントを検出: Instance ID = %d\n", event.gdevice.which);
                gamepad = SDL_OpenGamepad(event.gdevice.which);
                if (gamepad) {
                    printf("Connected!!\n");
                    printf("[DEBUG] Gamepad名: %s\n", SDL_GetGamepadName(gamepad));
                } else {
                    printf("[DEBUG] Gamepadのオープンに失敗: %s\n", SDL_GetError());
                }
            }
        }

        // CPU負荷を下げるため少し待機
        SDL_Delay(100);
    }

    return gamepad;
}

bool joycon_check_gyro(SDL_Gamepad* gamepad) {
    if (!gamepad) {
        printf("[DEBUG] gamepadがNULLです\n");
        return false;
    }

    printf("[DEBUG] Gyroセンサーの有無をチェックします\n");
    bool has_gyro = SDL_GamepadHasSensor(gamepad, SDL_SENSOR_GYRO);

    if (has_gyro) {
        printf("Gyro OK (^_^)\n");
    } else {
        printf("Gyro NG (x_x)\n");
    }

    return has_gyro;
}

bool joycon_enable_gyro(SDL_Gamepad* gamepad) {
    if (!gamepad) {
        printf("[DEBUG] gamepadがNULLです\n");
        return false;
    }

    printf("[DEBUG] Gyroセンサーを有効化します\n");
    if (SDL_SetGamepadSensorEnabled(gamepad, SDL_SENSOR_GYRO, true) < 0) {
        printf("[DEBUG] Gyroセンサーの有効化に失敗: %s\n", SDL_GetError());
        return false;
    }

    printf("[DEBUG] Gyroセンサーの有効化に成功しました\n");
    return true;
}

bool joycon_read_and_display_gyro(SDL_Gamepad* gamepad) {
    if (!gamepad) {
        return false;
    }

    float gyro_data[3];

    // Gyroセンサーのデータを取得
    if (SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_GYRO, gyro_data, 3) < 0) {
        printf("[DEBUG] Gyroデータの取得に失敗: %s\n", SDL_GetError());
        return false;
    }

    // Gyroデータを表示（X, Y, Z軸の角速度）
    printf("Gyro Data - X: %8.3f, Y: %8.3f, Z: %8.3f (rad/s)\n",
           gyro_data[0], gyro_data[1], gyro_data[2]);

    return true;
}

bool joycon_is_connected(SDL_Gamepad* gamepad) {
    if (!gamepad) {
        return false;
    }

    return SDL_GamepadConnected(gamepad);
}
