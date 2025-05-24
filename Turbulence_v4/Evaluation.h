#pragma once
#include "Board.h"
#include <string>

void init_tables();
int Evaluate(Board& board);

void print_tables();
void LoadNetworkFromMemory(const uint8_t* data, size_t size);
void resetAccumulators(const Board& board, AccumulatorPair& accumulator);
void resetWhiteAccumulator(const Board& board, AccumulatorPair& accumulator, bool flipFile);
void resetBlackAccumulator(const Board& board, AccumulatorPair& accumulator, bool flipFile);