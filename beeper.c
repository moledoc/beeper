#include <stdio.h>

#include "raylib.h"

#define W_HEIGHT 75
#define W_WIDTH W_HEIGHT*4
#define S_HEIGHT GetScreenHeight()
#define S_WIDTH GetScreenWidth()
#define FONT_SIZE 11

double timer = 0; // 1.5; // TODO: add flags/etc that control this variable

int main(int argc, char **argv) {
	InitWindow(W_WIDTH, W_HEIGHT, "beep");
	// SetWindowPosition(S_WIDTH/2, S_HEIGHT/2);
	SetWindowPosition(20, 20);
	ClearBackground(ORANGE);
	SetTargetFPS(60);
	SetExitKey(KEY_Q);
	SetWindowState(FLAG_WINDOW_UNDECORATED);

	double prev_time = GetTime();
	double new_time = GetTime();

	while (!WindowShouldClose()) {
		BeginDrawing();
		char *msg = "notification message";
		DrawText(msg, W_WIDTH/10, W_HEIGHT/4, FONT_SIZE, BLACK);


		// copy-paste the beep contexts
		// FIXME: only keeps it in clipboard while the beep still exists
		if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_C)) {
			SetClipboardText(msg);
		}

		// timer on beep
		new_time = GetTime();
		if (timer && new_time-prev_time >= timer) {
			break;
		}

		// manually acknowledge beep
		Vector2 mouse_pos = GetMousePosition();
		// printf("%f %f -- %f %f\n", win_pos.x, win_pos.y, mouse_pos.x, mouse_pos.y);
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && 
			0 <= mouse_pos.x && mouse_pos.x <= W_WIDTH &&
			0 <= mouse_pos.y && mouse_pos.y <= W_HEIGHT) {
			break;
		}
		EndDrawing();
	}
	CloseWindow();
}