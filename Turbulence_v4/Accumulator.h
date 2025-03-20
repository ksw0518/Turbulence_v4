#pragma once
//#include "const.h"
#include <cstdint>
#include <cstddef>

const int INPUT_SIZE = 768;
const int HL_SIZE = 64;

// These parameters are part of the training configuration
// These are the commonly used values as of 2024
const int SCALE = 400;
const int QA = 255;
const int QB = 64;

struct Network {
	int16_t accumulator_weights[INPUT_SIZE][HL_SIZE];
	int16_t accumulator_biases[HL_SIZE];
	int16_t output_weights[2 * HL_SIZE];
	int16_t output_bias;
};

struct Accumulator {
	int16_t values[HL_SIZE];
};
struct AccumulatorPair {
	Accumulator white;
	Accumulator black;
}; 
extern Network Eval_Network;
inline int flipSquare(int square)//flip square so a1 = 0
{
	return square ^ 56;
}
inline int calculateIndex(int perspective, int square, int pieceType, int side)
{

	square ^= 56;
	if (perspective == 1) {
		square = flipSquare(square);
	}
	return 6 * 64 * (side != perspective) + 64 * pieceType + square;

	//return flippedSide * 64 * 6 + get_piece(pieceType, White) * 64 + flippedSquare;
}
inline void accumulatorAdd(const struct Network* const network, struct Accumulator* accumulator, size_t index)
{
	for (int i = 0; i < HL_SIZE; i++)
		accumulator->values[i] += network->accumulator_weights[index][i];
}

inline void accumulatorSub(const struct Network* const network, struct Accumulator* accumulator, size_t index)
{
	for (int i = 0; i < HL_SIZE; i++)
		accumulator->values[i] -= network->accumulator_weights[index][i];
}