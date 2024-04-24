#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "raylib.h"
#include "beep.h"

#define W_HEIGHT 75
#define W_WIDTH (W_HEIGHT*4)
#define T_MAG (W_HEIGHT/3)
#define M_HEIGHT (GetMonitorHeight(0))
#define M_WIDTH (GetMonitorWidth(0))
#define FONT_SIZE 13

Color BG_COLOR = { .r = 255, .g = 255, .b = 204, .a = 255}; // very pale yellow
Color FG_COLOR = { .r = 0, .g = 0, .b = 0, .a = 128}; // black
int SECOND = 1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Broot broot = {.bueue = NULL};

size_t my_strlen(const char *s) {
	size_t len = 0;
	for (;s[len] != '\0';++len) {
		;
	}
	return len;
}

void my_memset(char *dest, char v, size_t len) {
	for (int i=0; i<len; ++i) {
		dest[i]=v;
	}
}

void my_memcpy(char *dest, char *src, size_t len) {
	for (int i=0; i<len; ++i) {
		dest[i] = src[i];
	}
}


void wrap_msg(Beep bp, char *wrapped_msg) {
	int measured_text = MeasureText(bp.msg, FONT_SIZE);
	int newline_count = measured_text/W_WIDTH;
	double avg_char_size = (double)measured_text/(double)bp.msg_len;
	int line_char_count = W_WIDTH/avg_char_size;
	for (int i=0; i<newline_count; ++i) {
		wrapped_msg[(i+1)*line_char_count+i] = '\n';
		my_memcpy(wrapped_msg+(i*line_char_count)+i, bp.msg+(i*line_char_count), line_char_count);
	}
	my_memcpy(wrapped_msg+(newline_count*line_char_count)+newline_count, bp.msg+(newline_count*line_char_count), bp.msg_len-(newline_count*line_char_count));
	
}

// NOTE: maybe slightly hacky to get copy pasting after closing notification for x seconds
void copy_paste_buffer_time(int buf_time) {
	SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
	int buffer_time_start = GetTime();
	while (GetTime()-buffer_time_start < buf_time) {
		BeginDrawing();
		EndDrawing();
	}
}

void beep(Beep bp) {
	pthread_mutex_lock(&mutex);
	bp.msg_len = my_strlen((const char *)bp.msg);

	InitWindow(W_WIDTH, W_HEIGHT, "beep");
	SetTargetFPS(60);
	ClearWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
	SetWindowState(FLAG_WINDOW_UNDECORATED);
	SetExitKey(KEY_Q);
	SetWindowMonitor(0);
	SetWindowPosition(M_WIDTH/10-W_WIDTH/2, M_HEIGHT/10-W_HEIGHT/2);
	SetTextLineSpacing(FONT_SIZE);

	ClearBackground(BG_COLOR);

	double prev_time = GetTime();

	int is_copy_paste = 0;

	while (!WindowShouldClose()) {
		BeginDrawing();
		SetClipboardText(bp.msg);

		size_t wrapped_msg_len = bp.msg_len+MeasureText(bp.msg, FONT_SIZE)/W_WIDTH+1+1;
		char wrapped_msg[wrapped_msg_len+1];
		my_memset(wrapped_msg, '\0', wrapped_msg_len);
		wrap_msg(bp, wrapped_msg);
		DrawText(wrapped_msg, W_WIDTH/(T_MAG*2), W_HEIGHT/(T_MAG), FONT_SIZE, FG_COLOR);

		if (bp.timer && GetTime()-prev_time >= bp.timer) {
			broot.bueue = bpq_push(broot.bueue, bp);
			break;
		}

		Vector2 mouse_pos = GetMousePosition();
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && 
			0 <= mouse_pos.x && mouse_pos.x <= W_WIDTH &&
			0 <= mouse_pos.y && mouse_pos.y <= W_HEIGHT) {
			break;
		} else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && 
			0 <= mouse_pos.x && mouse_pos.x <= W_WIDTH &&
			0 <= mouse_pos.y && mouse_pos.y <= W_HEIGHT) {
			SetClipboardText(bp.msg);
			is_copy_paste = 1;
			break;
		} else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && 
			0 <= mouse_pos.x && mouse_pos.x <= W_WIDTH &&
			0 <= mouse_pos.y && mouse_pos.y <= W_HEIGHT) {
			broot.bueue = bpq_push(broot.bueue, bp);
			break;
		}
		EndDrawing();
	}
	SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
	if (is_copy_paste) {
		copy_paste_buffer_time(5*SECOND);
	}
	CloseWindow();
	pthread_mutex_unlock(&mutex);
}

void *pager(void *_) {
	Beep bp = {.timer = 0, .msg = "this is a really long long long long long long long long long long long messagethis is a really long long long long long long long long long long long message"};
	broot.bueue = bpq_push(broot.bueue, bp);
	for (; broot.bueue != NULL;) {
		broot.bueue = bpq_pop(broot.bueue, &bp);
		beep(bp);
		sleep(1);
	}
	return 0;
}

void *sock_listener(void *_) {
	// TODO: listen to socket
	// TODO: implement encoding and it's parsing
	sleep(2);
	return 0;
}

int main(int argc, char **argv) {

	size_t threads_count = 2;
	pthread_t threads[threads_count];

	pthread_create(&threads[0], NULL, pager, NULL);
	pthread_create(&threads[1], NULL, sock_listener, NULL);

	for (int i=0; i<threads_count; ++i) {
		pthread_join(threads[i], NULL);
	}

	bpq_free(broot.bueue);
}