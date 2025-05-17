#pragma once
//#include "const.h"
#include <cstdint>
#include <cstddef>
#include <iostream>

#include<string>
const int INPUT_SIZE = 768;
const int HL_SIZE = 768;

// These parameters are part of the training configuration
// These are the commonly used values as of 2024
const int SCALE = 400;
const int QA = 255;
const int QB = 64;

struct Network {
	alignas(64) int16_t accumulator_weights[INPUT_SIZE][HL_SIZE];
	alignas(64) int16_t accumulator_biases[HL_SIZE];
	alignas(64) int16_t output_weights[2 * HL_SIZE];
	alignas(64) int16_t output_bias;
};



struct Accumulator {
	alignas(64) int16_t values[HL_SIZE];
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
inline int mirrorLeftRight(int square)
{
	return square ^ 7;
}

inline std::string CoordinatesToChessNotation(int square)
{
	int rawFile = square % 8;
	int rawRank = square == 0 ? 8 : 8 - square / 8;
	char File = (char)('a' + rawFile); // Convert column index to letter ('a' to 'h')
	int row = rawRank; // Row number (1 to 8)

	// Validate row
	//if (row < 0 || row > 8)
	//{
	//    throw new ArgumentException("Invalid chess square.");
	//}
	std::string str(1, File);
	return str + std::to_string(row);
}
inline int calculateIndex(int perspective, int square, int pieceType, int side, bool flipFile)
{

	square ^= 56;
	if (perspective == 1) {
		square = flipSquare(square);
	}
	if (flipFile)
	{
		square = mirrorLeftRight(square);

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
