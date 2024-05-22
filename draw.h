#pragma once

#ifndef DRAW_H_
#define DRAW_H_

#include "beep.h"
#include "raylib.h"

#define W_HEIGHT 75
#define W_WIDTH (W_HEIGHT * 4)
#define T_MAG (W_HEIGHT / 3)
#define M_HEIGHT (GetMonitorHeight(0))
#define M_WIDTH (GetMonitorWidth(0))
#define FONT_SIZE 13

Color BG_COLOR = {.r = 255, .g = 255, .b = 204, .a = 255}; // very pale yellow
Color FG_COLOR = {.r = 0, .g = 0, .b = 0, .a = 255};       // black
int SECOND = 1;

// void wrap_msg(Beep *beep, char *wrapped_msg);
void draw(Beeps beep);

#endif // DRAW_H_

#ifndef DRAW_IMPLEMENTATION
#define DRAW_IMPLEMENTATIN
#include "beep.h"
#include "raylib.h"

/*
void wrap_msg(Beep *bp, char *wrapped_msg) {
        int measured_text = MeasureText(bp->msg, FONT_SIZE);
        int newline_count = measured_text/W_WIDTH;
        double avg_char_size = (double)measured_text/(double)bp->msg_len;
        int line_char_count = W_WIDTH/avg_char_size;
        for (int i=0; i<newline_count; ++i) {
                wrapped_msg[(i+1)*line_char_count+i] = '\n';
                my_memcpy(wrapped_msg+(i*line_char_count)+i,
bp->msg+(i*line_char_count), line_char_count);
        }
        my_memcpy(wrapped_msg+(newline_count*line_char_count)+newline_count,
bp->msg+(newline_count*line_char_count),
bp->msg_len-(newline_count*line_char_count));
}
*/

void draw(Beeps beeps) {
  InitWindow(W_WIDTH, W_HEIGHT, "beeper");
  SetTargetFPS(30);
  SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIDDEN);
  SetExitKey(KEY_Q);
  SetWindowMonitor(0);
  SetWindowPosition(M_WIDTH / 10 - W_WIDTH / 2, M_HEIGHT / 10 - W_HEIGHT / 2);
  SetTextLineSpacing(FONT_SIZE);

  ClearBackground(BLANK);

  double start_time = GetTime();

  while (!WindowShouldClose()) {
    Beep *beep = head_of_array(beeps);
    if (beep == NULL) {
      if (!IsWindowState(FLAG_WINDOW_HIDDEN)) {
        SetWindowState(FLAG_WINDOW_HIDDEN);
      }
      break; // REMOVEME:
      continue;
    }

    if (IsWindowState(FLAG_WINDOW_HIDDEN)) {
      ClearWindowState(FLAG_WINDOW_HIDDEN);
    }

    BeginDrawing();
    DrawRectangle(0, 0, W_WIDTH, W_HEIGHT, BG_COLOR);

    int x = W_WIDTH - FONT_SIZE;
    int y = W_HEIGHT - FONT_SIZE;
    float r = (float)FONT_SIZE;
    DrawCircle(x, y, r, RED);
    char count[32];
    memset(count, '\0', sizeof(count));
    sprintf(count, "%d", beeps_count(beeps));
    DrawText(count, x - MeasureText(count, FONT_SIZE) / 2, y - FONT_SIZE / 2,
             FONT_SIZE, FG_COLOR);

    char msg[256];
    memset(msg, '\0', sizeof(msg));
    memcpy(msg, beep->msg, beep->msg_len);
    DrawText(msg, W_WIDTH / (T_MAG * 2), W_HEIGHT / (T_MAG), FONT_SIZE,
             FG_COLOR);

    Vector2 mouse_pos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && 0 <= mouse_pos.x &&
        mouse_pos.x <= W_WIDTH && 0 <= mouse_pos.y && mouse_pos.y <= W_HEIGHT) {
      rm_from_array(beeps, beep);
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && 0 <= mouse_pos.x &&
               mouse_pos.x <= W_WIDTH && 0 <= mouse_pos.y &&
               mouse_pos.y <= W_HEIGHT) {
      SetClipboardText((const char *)beep->msg);
      rm_from_array(beeps, beep);
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && 0 <= mouse_pos.x &&
               mouse_pos.x <= W_WIDTH && 0 <= mouse_pos.y &&
               mouse_pos.y <= W_HEIGHT) {
      mv_to_end(beeps, beep);
    }
    EndDrawing();
  }
  CloseWindow();
}

#endif // DRAW_IMPLEMENTATION