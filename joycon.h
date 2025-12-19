#ifndef JOYCON_H
#define JOYCON_H

#include <SDL3/SDL.h>
#include <stdbool.h>

// Joy-Con関連の関数

/**
 * Joy-Conの接続を待機する
 * @return 接続されたGamepadのポインタ（失敗時はNULL）
 */
SDL_Gamepad* joycon_wait_connection(void);

/**
 * Gyroセンサーが使用可能かチェックする
 * @param gamepad チェックするGamepad
 * @return 使用可能ならtrue、不可能ならfalse
 */
bool joycon_check_gyro(SDL_Gamepad* gamepad);

/**
 * Gyroセンサーを有効化する
 * @param gamepad 対象のGamepad
 * @return 成功時true、失敗時false
 */
bool joycon_enable_gyro(SDL_Gamepad* gamepad);

/**
 * Gyroセンサーのデータを取得して表示する
 * @param gamepad 対象のGamepad
 * @return 成功時true、失敗時false
 */
bool joycon_read_and_display_gyro(SDL_Gamepad* gamepad);

/**
 * Gyroセンサーで大きな動きを検出し、必要に応じてメッセージを表示する
 * @param gamepad 対象のGamepad
 * @return 成功時true、失敗時false
 */
bool joycon_detect_big_motion(SDL_Gamepad* gamepad);

/**
 * Gamepadが接続されているかチェックする
 * @param gamepad チェックするGamepad
 * @return 接続されていればtrue、切断されていればfalse
 */
bool joycon_is_connected(SDL_Gamepad* gamepad);

#endif // JOYCON_H
