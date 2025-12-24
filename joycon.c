#include "joycon.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

// 大きな動きの検出に関する静的変数
static float prev_accel_data[3] = {0.0f, 0.0f, 0.0f};
static bool prev_accel_initialized = false;
static time_t message_display_until = 0;

/**
 * mキーを押す（押して離す）
 */
static void press_m_key(void) {
    INPUT inputs[2] = {0};

    // キーダウン
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 'M';  // mキー
    inputs[0].ki.dwFlags = 0;

    // キーアップ
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'M';  // mキー
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    // キーイベントを送信
    SendInput(2, inputs, sizeof(INPUT));

    printf("[DEBUG] mキーを押しました\n");
}

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

bool joycon_check_accel(SDL_Gamepad* gamepad) {
    if (!gamepad) {
        printf("[DEBUG] gamepadがNULLです\n");
        return false;
    }

    printf("[DEBUG] Accelセンサーの有無をチェックします\n");
    bool has_accel = SDL_GamepadHasSensor(gamepad, SDL_SENSOR_ACCEL);

    if (has_accel) {
        printf("Accel OK (^_^)\n");
    } else {
        printf("Accel NG (x_x)\n");
    }

    return has_accel;
}

bool joycon_enable_accel(SDL_Gamepad* gamepad) {
    if (!gamepad) {
        printf("[DEBUG] gamepadがNULLです\n");
        return false;
    }

    printf("[DEBUG] Accelセンサーを有効化します\n");
    if (SDL_SetGamepadSensorEnabled(gamepad, SDL_SENSOR_ACCEL, true) < 0) {
        printf("[DEBUG] Accelセンサーの有効化に失敗: %s\n", SDL_GetError());
        return false;
    }

    printf("[DEBUG] Accelセンサーの有効化に成功しました\n");
    return true;
}

bool joycon_read_and_display_accel(SDL_Gamepad* gamepad) {
    if (!gamepad) {
        return false;
    }

    float accel_data[3];

    // Accelセンサーのデータを取得
    if (SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_ACCEL, accel_data, 3) < 0) {
        printf("[DEBUG] Accelデータの取得に失敗: %s\n", SDL_GetError());
        return false;
    }

    // Accelデータを表示（X, Y, Z軸の加速度）
    printf("Accel Data - X: %8.3f, Y: %8.3f, Z: %8.3f (m/s^2)\n",
           accel_data[0], accel_data[1], accel_data[2]);

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

    float accel_data[3];

    // Accelセンサーのデータを取得
    if (SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_ACCEL, accel_data, 3) < 0) {
        return false;
    }

    // メッセージ表示期間中は変化量の測定も停止
    if (current_time < message_display_until) {
        // 待機中もセンサーデータを読み取り、前回値を更新して
        // 待機終了後に誤った大きな変化量が計算されるのを防ぐ
        prev_accel_data[0] = accel_data[0];
        prev_accel_data[1] = accel_data[1];
        prev_accel_data[2] = accel_data[2];
        return true;
    }

    // 前回の値が初期化されていない場合は初期化する
    if (!prev_accel_initialized) {
        prev_accel_data[0] = accel_data[0];
        prev_accel_data[1] = accel_data[1];
        prev_accel_data[2] = accel_data[2];
        prev_accel_initialized = true;
        return true;
    }

    // 変化量を計算（各軸の変化量の二乗和の平方根）
    float delta_x = accel_data[0] - prev_accel_data[0];
    float delta_y = accel_data[1] - prev_accel_data[1];
    float delta_z = accel_data[2] - prev_accel_data[2];
    float magnitude = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

    // 前回の値を更新
    prev_accel_data[0] = accel_data[0];
    prev_accel_data[1] = accel_data[1];
    prev_accel_data[2] = accel_data[2];

    // 閾値を超えた場合に「手裏剣ファイヤー！！！！！！」を表示
    // 閾値は実験的に調整が必要（ここでは18.0 m/s^2）
    const float THRESHOLD = 18.0f;

    if (magnitude > THRESHOLD) {
        printf("\n====================================\n");
        printf("   手裏剣ファイヤー！！！！！！\n");
        printf("====================================\n\n");

        // Mキーを押す
        press_m_key();

        // 2秒間表示を維持する時刻を記録
        message_display_until = current_time + 2;
        return true;
    }

    // Accelデータを表示（X, Y, Z軸の加速度）
    printf("Accel Data - X: %8.3f, Y: %8.3f, Z: %8.3f (m/s^2) [変化量: %8.3f]\n",
           accel_data[0], accel_data[1], accel_data[2], magnitude);

    return true;
}
