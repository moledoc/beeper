#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>

#include "raylib.h"
#include "beep.h"

#define VERSION_MAJOR (0)
#define VERSION_MINOR (1)
#define PACKET_SIZE (255)

#define W_HEIGHT 75
#define W_WIDTH (W_HEIGHT*4)
#define T_MAG (W_HEIGHT/3)
#define M_HEIGHT (GetMonitorHeight(0))
#define M_WIDTH (GetMonitorWidth(0))
#define FONT_SIZE 13

enum IDX {
	IDX_VERSION_MINOR,
	IDX_VERSION_MAJOR,
	IDX_REPEAT,
	IDX_TIMER_FRAC,
	IDX_TIMER_BASE,
	METADATA_SIZE,
};

uint8_t MSG_SIZE = PACKET_SIZE-METADATA_SIZE;

Color BG_COLOR = { .r = 255, .g = 255, .b = 204, .a = 255}; // very pale yellow
Color FG_COLOR = { .r = 0, .g = 0, .b = 0, .a = 128}; // black
int SECOND = 1;

Broot *broot;

pthread_mutex_t window_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t graceful_mutex = PTHREAD_MUTEX_INITIALIZER;
bool shutting_down = false;

typedef struct {
	uint32_t count;
	pthread_mutex_t mutex;
} Counter;

Counter *cinit() {
	Counter *counter = malloc(sizeof(Counter));
	counter->count = 1;
	pthread_mutex_init(&counter->mutex, NULL);
	return counter;
}

void cfree(Counter *counter) {
	free(counter);
}

uint32_t get_count(Counter *counter) {
	uint32_t count = 0;

	pthread_mutex_lock(&(counter->mutex));
	count = counter->count;
	++counter->count;
	pthread_mutex_unlock(&(counter->mutex));
	return count;
}


Counter *counter;

void my_memcpy(char *dest, char *src, size_t len) {
	for (int i=0; i<len; ++i) {
		dest[i] = src[i];
	}
}


void wrap_msg(Beep *bp, char *wrapped_msg) {
	int measured_text = MeasureText(bp->msg, FONT_SIZE);
	int newline_count = measured_text/W_WIDTH;
	double avg_char_size = (double)measured_text/(double)bp->msg_len;
	int line_char_count = W_WIDTH/avg_char_size;
	for (int i=0; i<newline_count; ++i) {
		wrapped_msg[(i+1)*line_char_count+i] = '\n';
		my_memcpy(wrapped_msg+(i*line_char_count)+i, bp->msg+(i*line_char_count), line_char_count);
	}
	my_memcpy(wrapped_msg+(newline_count*line_char_count)+newline_count, bp->msg+(newline_count*line_char_count), bp->msg_len-(newline_count*line_char_count));
	
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

void beep(Beep *bp) {
	pthread_mutex_lock(&window_mutex);
	if (bp->msg_len == 0) {
		bp->msg_len = strlen((const char *)bp->msg);
	}
	if (!bp->id) {
		bp->id = get_count(counter);
	}
	bp_print("Beep with", bp, __FILE__, __LINE__);

	InitWindow(W_WIDTH, W_HEIGHT, "beep");
	SetTargetFPS(60);
	ClearWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
	SetWindowState(FLAG_WINDOW_UNDECORATED);
	SetExitKey(KEY_Q);
	SetWindowMonitor(0);
	SetWindowPosition(M_WIDTH/10-W_WIDTH/2, M_HEIGHT/10-W_HEIGHT/2);
	SetTextLineSpacing(FONT_SIZE);

	ClearBackground(BG_COLOR);

	double start_time = GetTime();

	int is_copy_paste = 0;

	while (!WindowShouldClose()) {
		BeginDrawing();
		SetClipboardText(bp->msg);

		size_t wrapped_msg_len = bp->msg_len+MeasureText(bp->msg, FONT_SIZE)/W_WIDTH+1+1;
		char wrapped_msg[wrapped_msg_len+1];
		memset(wrapped_msg, '\0', wrapped_msg_len);
		wrap_msg(bp, wrapped_msg);
		DrawText(wrapped_msg, W_WIDTH/(T_MAG*2), W_HEIGHT/(T_MAG), FONT_SIZE, FG_COLOR);

		if (bp->timer && GetTime()-start_time >= bp->timer) {
			if (bp->repeat) {
				bpush(broot, bp);
			}
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
			SetClipboardText(bp->msg);
			is_copy_paste = 1;
			break;
		} else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && 
			0 <= mouse_pos.x && mouse_pos.x <= W_WIDTH &&
			0 <= mouse_pos.y && mouse_pos.y <= W_HEIGHT) {
			bpush(broot, bp);
			break;
		}
		EndDrawing();
	}
	SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
	if (is_copy_paste) {
		copy_paste_buffer_time(5*SECOND);
	}
	CloseWindow();
	pthread_mutex_unlock(&window_mutex);
	if (broot->bueue){
			printf("+++ + + %s\n", broot->bueue->bp->msg);
	}
}

bool graceful_shutdown(bool shutdown) {
	pthread_mutex_lock(&graceful_mutex);
	if (shutdown) {
		shutting_down = shutdown;
	}
	pthread_mutex_unlock(&graceful_mutex);
	return shutting_down;
}

void *pager(void *_) {
	for (;;) {
		// FIXME: for some reason printing/popping broot messes up bp.msg
		bprint(broot);
		Beep *bp = NULL; // bpop(broot);
		if (broot->bueue) {
			printf("=== = = %s\n", broot->bueue->bp->msg);
		}

		if (bp != NULL) {
			bp_print("bpopped beep", bp, __FILE__, __LINE__);
			beep(bp);
		}
		if (graceful_shutdown(false)) {
			break;
		}
		sleep(3);
	}
	graceful_shutdown(true);
	return 0;
}

double int_to_double(uint8_t ifrac) {
	double frac = (double)ifrac;
	for (; frac>0; frac /= 10) {
		;
	}
	return frac;
}

bool decode_comms(uint8_t *buf, uint8_t n, Beep *bp) {
	if (buf[IDX_VERSION_MINOR]-'0' != VERSION_MINOR || buf[IDX_VERSION_MAJOR]-'0' != VERSION_MAJOR) {
		fprintf(stderr, "unsupported version, expected %d.%d, got %d.%d\n", VERSION_MAJOR, VERSION_MINOR, buf[IDX_VERSION_MAJOR]-'0', buf[IDX_VERSION_MINOR]-'0');
		return 0;
	}

	(*bp).repeat = (bool)(buf[IDX_REPEAT]-'0');
	(*bp).timer = (double)buf[IDX_TIMER_BASE]-'0'+int_to_double(buf[IDX_TIMER_FRAC]-'0');
	memcpy((*bp).msg, buf+METADATA_SIZE, MSG_SIZE);
	return 1;
}

void *sock_listener(void *_) {
	char *path = "/tmp/beeper.sock";
	unlink(path);
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if (sockfd == -1) {
		fprintf(stderr, "failed to get socket file descriptor: %s\n", strerror(errno));
		goto exit;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
	socklen_t addr_len = sizeof(addr);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		fprintf(stderr, "failed to bind socket '%s': %s\n", path, strerror(errno));
		goto exit;
	}
	if (listen(sockfd, 0) == -1) {
		fprintf(stderr, "failed to listen socket '%s': %s\n", path, strerror(errno));
		goto exit;
	}

	for (;;) {
		int conn = accept(sockfd, (struct sockaddr *) &addr, &addr_len);
		// printf("conn: %d\n", conn);
		if (conn == -1) {
			fprintf(stderr, "failed to accept socket '%s': %s\n", path, strerror(errno));
			close(conn);
			goto exit;
		}
	
		uint8_t buf[PACKET_SIZE+1];
		memset(buf, 0, sizeof(buf));
		int n = read(conn, buf, sizeof(buf)-1);
		if(close(conn) == -1) {
			fprintf(stderr, "failed to close conn: %s\n", strerror(errno));
		}

		// TODO: closing listener
		if (n > 0 && buf[0] == 'q') {
			break;
		}

		if (n < METADATA_SIZE) { // TODO: make 'why' clear
			fprintf(stderr, "unexpected payload, ignoring it: '%s'\n", buf);
			continue;
		}

		char msg[MSG_SIZE+1];
		memset(msg, 0, sizeof(msg));
		Beep *bp = calloc(1, sizeof(Beep));
		bp->msg = msg;
		if (!decode_comms(buf, n, bp)) {
			continue;
		}
		bp_print("received beep", bp, __FILE__, __LINE__);
		beep(bp);
	}

exit:
	graceful_shutdown(true);
	if(close(sockfd) == -1) {
		fprintf(stderr, "failed to close socket: %s\n", strerror(errno));
	}
	return 0;
}


int main(int argc, char **argv) {
	counter = cinit();
	broot = binit();

	size_t threads_count = 2;
	pthread_t threads[threads_count];

	pthread_create(&threads[0], NULL, sock_listener, NULL);
	pthread_create(&threads[1], NULL, pager, NULL);

	for (int i=0; i<threads_count; ++i) {
		pthread_join(threads[i], NULL);
	}

	bfree(broot);
	cfree(counter);
}