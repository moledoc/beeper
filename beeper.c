#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#define W_HEIGHT 75
#define W_WIDTH W_HEIGHT*4
#define T_MAG W_HEIGHT/3
#define M_HEIGHT GetMonitorHeight(0)
#define M_WIDTH GetMonitorWidth(0)
#define FONT_SIZE 13

Color BG_COLOR = { .r = 255, .g = 255, .b = 204, .a = 255};
Color FG_COLOR = { .r = 0, .g = 0, .b = 0, .a = 128};
int SECOND = 1;

// enum { false, true} bool;

typedef struct {
	// bool cpy_paste;
	double timer;
	char *msg;
	size_t msg_len;
	size_t msg_pxls;
} Beep;

size_t my_strlen(const char *s) {
	size_t len = 0;
	for (;s[len] != '\0';++len) {
		;
	}
	return len;
}

void carve(char *dest, char *src, size_t carve) {
	for (int i=0; i<carve; ++i) {
		dest[i] = src[i];
	}
	dest[carve] = '\0';
}

void wrap_msg(char *wrapped_msg, Beep bp) {
	int j=0;
	for (int i=0, offset=0;i<bp.msg_len; ++i, ++j) {
		// char tmp[bp.msg_len+1];
		// carve(tmp, bp.msg+offset, i+1);
		char tmp[2];
		tmp[1] = '\0';
		tmp[0] = bp.msg[i];
		offset += MeasureText(tmp, FONT_SIZE);
		// printf("%d %d\n", MeasureText(tmp, FONT_SIZE), W_WIDTH);
		if (offset>W_WIDTH) {
			wrapped_msg[j] = '\n';
			offset=0;
			++j;
		}
		wrapped_msg[j] = bp.msg[i];
	}
	wrapped_msg[j] = '\0';
}

// MAYBE: slightly hacky to get copy pasting after closing notification for x seconds
void copy_paste_buffer_time(int buf_time) {
	SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
	int buffer_time_start = GetTime();
	while (GetTime()-buffer_time_start < buf_time) {
		BeginDrawing();
		EndDrawing();
	}
}

void beep(Beep bp) {
	bp.msg_len = my_strlen((const char *)bp.msg);
	bp.msg_pxls = bp.msg_len*sizeof(char);

	InitWindow(W_WIDTH, W_HEIGHT, "beep");

	SetTargetFPS(60);
	SetWindowState(FLAG_WINDOW_UNDECORATED);
	SetExitKey(KEY_Q);
	SetWindowMonitor(0);
	SetWindowPosition(M_WIDTH/10-W_WIDTH/2, M_HEIGHT/10-W_HEIGHT/2);
	SetTextLineSpacing(FONT_SIZE);

	ClearBackground(BG_COLOR);

	double prev_time = GetTime();

	while (!WindowShouldClose()) {
		BeginDrawing();
		SetClipboardText(bp.msg);

		// wrap text
		char wrapped_msg[bp.msg_len+MeasureText(bp.msg, FONT_SIZE)/W_WIDTH+1];
		wrap_msg(wrapped_msg, bp);
		DrawText(wrapped_msg, W_WIDTH/(T_MAG*2), W_HEIGHT/(T_MAG), FONT_SIZE, FG_COLOR);

		// copy-paste the beep contexts
		// if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_C)) {
			
		// }

		// timer on beep
		if (bp.timer && GetTime()-prev_time >= bp.timer) {
			break;
		}

		// manually acknowledge beep
		Vector2 mouse_pos = GetMousePosition();
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && 
			0 <= mouse_pos.x && mouse_pos.x <= W_WIDTH &&
			0 <= mouse_pos.y && mouse_pos.y <= W_HEIGHT) {
			SetClipboardText(bp.msg);
			break;
		}
		EndDrawing();
	}
	SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
	copy_paste_buffer_time(5*SECOND);

	CloseWindow();
}

int main(int argc, char **argv) {
	Beep bp = {.timer = 0, .msg = "https://www.youtube.com/watch?v=rTb6NFKUmQU&list=WL&index=5"};
	beep(bp);
}