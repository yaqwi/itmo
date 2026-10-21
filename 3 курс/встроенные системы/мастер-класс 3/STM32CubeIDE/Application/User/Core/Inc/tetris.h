#ifndef TETRIS_H_
#define TETRIS_H_

void Tetris_Init(void);
void Tetris_InputTask(void const *argument);
void Tetris_GameTask(void const *argument);
void Tetris_DisplayTask(void const *argument);

#endif
