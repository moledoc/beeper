#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include "protocol.h"
#include "context.h"
#include "raylib.h"

#define W_HEIGHT 75
#define W_WIDTH (W_HEIGHT*4)
#define T_MAG (W_HEIGHT/3)
#define M_HEIGHT (GetMonitorHeight(0))
#define M_WIDTH (GetMonitorWidth(0))
#define FONT_SIZE 13

Color BG_COLOR = { .r = 255, .g = 255, .b = 204, .a = 255}; // very pale yellow
Color FG_COLOR = { .r = 0, .g = 0, .b = 0, .a = 128}; // black
int SECOND = 1;

Context *context = NULL;

void my_memcpy(char *dest, char *src, size_t len) {
	for (int i=0; i<len; ++i) {
		dest[i] = src[i];
	}
}

void wrap_msg(Packet *packet, char *wrapped_msg) {
	int measured_text = MeasureText((const char *)(packet->msg), FONT_SIZE);
	int newline_count = measured_text/W_WIDTH;
	double avg_char_size = (double)measured_text/(double)packet->msg_len;
	int line_char_count = W_WIDTH/avg_char_size;
	for (int i=0; i<newline_count; ++i) {
		wrapped_msg[(i+1)*line_char_count+i] = '\n';
		my_memcpy(wrapped_msg+(i*line_char_count)+i, (char *)(packet->msg+(i*line_char_count)), line_char_count);
	}
	my_memcpy(wrapped_msg+(newline_count*line_char_count)+newline_count, (char *)(packet->msg+(newline_count*line_char_count)), packet->msg_len-(newline_count*line_char_count));
	
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

void my_memset(char *dest, char c, size_t len) {
	for (int i=0; i<len; ++i) {
		dest[i] = c;
	}
}

void beep(Packet *packet) {
	TraceLog(LOG_INFO, "start beep");

	char win_name[32];
	my_memset(win_name, '\0', sizeof(win_name));
	snprintf(win_name, 32, "beep%d", packet->id);
	InitWindow(W_WIDTH, W_HEIGHT, win_name);
	SetTargetFPS(30);
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

		size_t wrapped_msg_len = packet->msg_len+MeasureText((char *)packet->msg, FONT_SIZE)/W_WIDTH+1+1;
		char wrapped_msg[wrapped_msg_len+1];
		my_memset(wrapped_msg, '\0', wrapped_msg_len);
		wrap_msg(packet, wrapped_msg);
		DrawText(wrapped_msg, W_WIDTH/(T_MAG*2), W_HEIGHT/(T_MAG), FONT_SIZE, FG_COLOR);

		if (packet->timer && GetTime()-start_time >= packet->timer) {
			if (packet->repeat) {
				store_packet(context, *packet);
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
			SetClipboardText((const char *)packet->msg);
			is_copy_paste = 1;
			break;
		} else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && 
			0 <= mouse_pos.x && mouse_pos.x <= W_WIDTH &&
			0 <= mouse_pos.y && mouse_pos.y <= W_HEIGHT) {
			store_packet(context, *packet);
			stored_packets_log(context);
			break;
		}
		EndDrawing();
	}
	SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
	if (is_copy_paste) {
		copy_paste_buffer_time(5*SECOND);
	}
	CloseWindow();

	TraceLog(LOG_INFO, "end beep");
}

void *sock_listener(void *_) {
	char *path = "/tmp/beeper.sock";
	TraceLog(LOG_INFO, "start listening to socket '%s'", path);
	int unlnk = unlink(path);
	if (unlnk == -1) {
		TraceLog(LOG_WARNING, "failed to unlink '%s': %s", path, strerror(errno));
		// should_shutdown(context, true);
		// return 0;
	}
		
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		TraceLog(LOG_ERROR, "failed to create unix socket at '%s': %s", path, strerror(errno));
		goto exit;
	}
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
	socklen_t addr_len = sizeof(addr);

	TraceLog(LOG_INFO, "binding to socket '%s'", path);
	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		TraceLog(LOG_ERROR, "failed to bind socket '%s': %s\n", path, strerror(errno));
		goto exit;
	}
	TraceLog(LOG_INFO, "bound to socket '%s'", path);
	if (listen(sockfd, 0) == -1) {
		TraceLog(LOG_ERROR, "failed to listen socket '%s': %s\n", path, strerror(errno));
		goto exit;
	}
	TraceLog(LOG_INFO, "listening to socket '%s'", path);

	for (;;) {
		int conn = accept(sockfd, (struct sockaddr *) &addr, &addr_len);
		if (conn == -1) {
			TraceLog(LOG_ERROR, "accepting connection failed: %s", strerror(errno));
			close(conn);
			goto exit;
		}
		TraceLog(LOG_INFO, "connection accepted");

		uint8_t buf[PACKET_SIZE];
		memset(buf, 0, sizeof(buf));
		int n = read(conn, buf, sizeof(buf));
		TraceLog(LOG_INFO, "read %d bytes", n);

		TraceLog(LOG_INFO, "closing connection");
		if(close(conn) == -1) {
			TraceLog(LOG_ERROR, "closing connection failed: %s", strerror(errno));
		}

		// TODO: closing listener
		if (n > 0 && buf[0] == 'q') {
			TraceLog(LOG_INFO, "quitting application");
			break;
		}

		Packet *packet = unmarshal(buf);
		beep(packet);
		// packet_log("unmarshalled payload: %s", packet);
		// uint8_t *buff = marshal(*packet);
		// buf_log("marshalled payload: %s", buff);

		// packet_log("head of queue: %s", stored_packets_head(context));

		// store_packet(context, *packet);
		// stored_packets_log(context);

		// pop_packet(context);
		// stored_packets_log(context);

		packet_free(packet);
		// free(buff);
	}

exit:
	should_shutdown(context, true);
	if(close(sockfd) == -1) {
		TraceLog(LOG_ERROR, "failed to close socket: %s\n", strerror(errno));
	}
	return 0;
}

void *pager(void *_) {
	for (;;) {
		Packet *packet = pop_packet(context);
		packet_log("popped packet: %s", packet);
		if (packet != NULL && *(packet->msg) != '\0') { // HACK: FIXME:
			beep(packet);
		}
		if (packet != NULL) {
			packet_free(packet);
		}
		if (should_shutdown(context, false)) {
			break;
		}
		sleep(2);
	}
	should_shutdown(context, true);
	return 0;
}

int main() {
	context = init_context();
	aid = init_atomic_id();

	uint8_t threads_count = 2;
	pthread_t threads[threads_count];

	pthread_create(&threads[0], NULL, sock_listener, NULL);
	pthread_create(&threads[1], NULL, pager, NULL);
	
	for (int i=0; i<threads_count; ++i) {
		pthread_join(threads[i], NULL);
	}
	free_context(context);
	free_atomic_id(aid);
}
