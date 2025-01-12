

#include "Evaluation.h"
#include "MoveGeneration.h"
#include "Board.h"
#include "BitManipulation.h"
#include "const.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>

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

Network nnueNetwork;
int mirrorSquare(int square) {

	int file = square % 8;       // Get the file (column) 0-7
	int rank = square / 8;       // Get the rank (row) 0-7
	int mirrored_file = 7 - file; // Mirror the file

	return rank * 8 + mirrored_file;
}
int calculateIndex(int perspective, int square, int pieceType, int side)
{
	square = mirrorSquare(square);
	//square = 64 - square;
	if (perspective == Black)
	{
		side = ~side;             // Flip side
		square = square ^ 0b111000; // Mirror square
	}
	
	return side * 64 * 6 + Get_Whitepiece[pieceType] * 64 + square;
}
void initializeAccumulator(struct Network* network, struct Accumulator* accumulator) {
	for (int i = 0; i < HL_SIZE; i++) {
		accumulator->values[i] = network->accumulator_biases[i];
	}
}
void accumulatorAdd(struct Network* const network, struct Accumulator* accumulator, size_t index)
{
	//if (index >= INPUT_SIZE) {
	//	std::cerr << "Error: index out of bounds: " << index << std::endl;
	//	// You might want to throw an exception or return an error code here
	//	return; // Or throw an exception
	//}
	//if (!network) {
	//	std::cerr << "Error: network is null" << std::endl;
	//	return;
	//}
	//if (!accumulator) {
	//	std::cerr << "Error: accumulator is null" << std::endl;
	//	return;
	//}
	for (int i = 0; i < HL_SIZE; i++)
		accumulator->values[i] += network->accumulator_weights[index][i];
}

void accumulatorSub(struct Network* const network, struct Accumulator* accumulator, size_t index)
{
	for (int i = 0; i < HL_SIZE; i++)
		accumulator->values[i] -= network->accumulator_weights[index][i];
}

int16_t SCReLU(int16_t value, int16_t min, int16_t max)
{
	return  std::clamp(static_cast<int>(value), 0, static_cast<int>(max)) * std::clamp(static_cast<int>(value), 0, static_cast<int>(max));
}


int32_t activation(int16_t value)
{
	return SCReLU(value, 0, QA);
}

// When forwarding the accumulator values, the network does not consider the color of the perspectives.
// Rather, we are more interested in whether the accumulator is from the perspective of the side-to-move.
int32_t forward(struct Network* const network,
	struct Accumulator* const stm_accumulator,
	struct Accumulator* const nstm_accumulator)
{
	int32_t eval = 0;

	// Dot product to the weights
	for (int i = 0; i < HL_SIZE; i++)
	{
		// BEWARE of integer overflows here.
		eval += activation(stm_accumulator->values[i]) * network->output_weights[i];
		eval += activation(nstm_accumulator->values[i]) * network->output_weights[i + HL_SIZE];
	}

	// Uncomment the following dequantization step when using SCReLU
	 eval /= QA;
	eval += network->output_bias;

	eval *= SCALE;
	eval /= QA * QB;

	return eval;
}






// Function to refresh accumulator based on current board state
void refreshAccumulator(const Board& board, struct Network* network,
	struct Accumulator* whiteAcc, struct Accumulator* blackAcc) {
	// Initialize accumulators with biases
	initializeAccumulator(network, whiteAcc);
	initializeAccumulator(network, blackAcc);

	// Process each piece type's bitboard
	for (int pieceIdx = P; pieceIdx < k; pieceIdx++) {
		uint64_t bitboard = board.bitboards[pieceIdx];
		bool side = pieceIdx >= 6;  // true for black pieces
		int pieceType = pieceIdx % 6;  // piece type without color

		// Process each piece of current type
		while (bitboard) {
			int square = get_ls1b(bitboard);  // Get least significant 1-bit

			// Add to white perspective accumulator
			int whiteIndex = calculateIndex(White, square, pieceType, side);
			accumulatorAdd(network, whiteAcc, whiteIndex);

			// Add to black perspective accumulator
			int blackIndex = calculateIndex(Black, square, pieceType, side);
			accumulatorAdd(network, blackAcc, blackIndex);

			Pop_bit(bitboard, square);  // Clear least significant 1-bit
		}
	}
}
bool loadNNUE(const std::string& filename, Network& network) {
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Error opening NNUE file: " << filename << std::endl;
		return false;
	}

	// Read the network data. Ensure correct sizes and types.
	file.read(reinterpret_cast<char*>(&network.accumulator_weights), sizeof(network.accumulator_weights));
	file.read(reinterpret_cast<char*>(&network.accumulator_biases), sizeof(network.accumulator_biases));
	file.read(reinterpret_cast<char*>(&network.output_weights), sizeof(network.output_weights));
	file.read(reinterpret_cast<char*>(&network.output_bias), sizeof(network.output_bias));

	if (file.fail()) {
		std::cerr << "Error reading NNUE data from file." << std::endl;
		file.close();
		return false;
	}

	file.close();
	return true;
}
void init_tables()
{
	std::string fileName = "quantised.bin";
	loadNNUE(fileName, nnueNetwork);
}
int Evaluate(Board& board)
{
	int eval = 0;
	Accumulator whiteAcc, blackAcc;
	refreshAccumulator(board, &nnueNetwork, &whiteAcc, &blackAcc);
	Accumulator* stmAcc, * nstmAcc;
	if (board.side == White) {
		stmAcc = &whiteAcc;
		nstmAcc = &blackAcc;
	}
	else {
		stmAcc = &blackAcc;
		nstmAcc = &whiteAcc;
	}
	eval = forward(&nnueNetwork, stmAcc, nstmAcc);
	//if (board.side == Black)
	//{
	//	eval *= -1;
	//}
	return eval;
}