#pragma once
#include "Board.h"
#include <string>

void init_tables();
int Evaluate(Board& board);

void print_tables();
void LoadNetwork(const std::string& filepath);
void resetAccumulators(const Board& board, AccumulatorPair& accumulator);
void resetWhiteAccumulator(const Board& board, AccumulatorPair& accumulator, bool flipFile);
void resetBlackAccumulator(const Board& board, AccumulatorPair& accumulator, bool flipFile);
