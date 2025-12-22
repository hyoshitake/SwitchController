#include "joycon.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

// 大きな動きの検出に関する静的変数
static float prev_gyro_data[3] = {0.0f, 0.0f, 0.0f};
static bool prev_gyro_initialized = false;
static time_t message_display_until = 0;

#ifdef _WIN32
/**
 * Windowsキーを押す（押して離す）
 */
static void press_windows_key(void) {
    INPUT inputs[2] = {0};

    // キーダウン
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LWIN;  // 左Windowsキー
    inputs[0].ki.dwFlags = 0;

    // キーアップ
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_LWIN;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    // キーイベントを送信
    SendInput(2, inputs, sizeof(INPUT));

    printf("[DEBUG] Windowsキーを押しました\n");
}
#endif

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

bool joycon_detect_big_motion(SDL_Gamepad* gamepad) {
    if (!gamepad) {
        return false;
    }

    time_t current_time = time(NULL);

    // メッセージ表示期間中は変化量の測定も停止
    if (current_time < message_display_until) {
        // 待機中は何もしない
        return true;
    }

    float gyro_data[3];

    // Gyroセンサーのデータを取得
    if (SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_GYRO, gyro_data, 3) < 0) {
        return false;
    }

    // 前回の値が初期化されていない場合は初期化する
    if (!prev_gyro_initialized) {
        prev_gyro_data[0] = gyro_data[0];
        prev_gyro_data[1] = gyro_data[1];
        prev_gyro_data[2] = gyro_data[2];
        prev_gyro_initialized = true;
        return true;
    }

    // 変化量を計算（各軸の変化量の二乗和の平方根）
    float delta_x = gyro_data[0] - prev_gyro_data[0];
    float delta_y = gyro_data[1] - prev_gyro_data[1];
    float delta_z = gyro_data[2] - prev_gyro_data[2];
    float magnitude = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

    // 前回の値を更新
    prev_gyro_data[0] = gyro_data[0];
    prev_gyro_data[1] = gyro_data[1];
    prev_gyro_data[2] = gyro_data[2];

    // 閾値を超えた場合に「手裏剣ファイヤー！！！！！！」を表示
    // 閾値は実験的に調整が必要（ここでは15.0 rad/s）
    const float THRESHOLD = 15.0f;

    if (magnitude > THRESHOLD) {
        printf("\n====================================\n");
        printf("   手裏剣ファイヤー！！！！！！\n");
        printf("====================================\n\n");

#ifdef _WIN32
        // Windowsキーを押す
        press_windows_key();
#endif

        // 2秒間表示を維持する時刻を記録
        message_display_until = current_time + 2;
        return true;
    }

    // Gyroデータを表示（X, Y, Z軸の角速度）
    printf("Gyro Data - X: %8.3f, Y: %8.3f, Z: %8.3f (rad/s) [変化量: %8.3f]\n",
           gyro_data[0], gyro_data[1], gyro_data[2], magnitude);

    return true;
}
