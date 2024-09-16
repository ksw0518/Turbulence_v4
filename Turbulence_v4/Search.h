#pragma once

#include "Board.h"

extern int pv_length[64];
extern Move pv_table[64][64];

void IterativeDeepening(Board& board, int depth, int timeMS = -1);

void printMoveSort(Board board);