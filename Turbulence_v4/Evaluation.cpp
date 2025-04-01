

#include "Evaluation.h"
//#include "MoveGeneration.h"
#include "Board.h"
#include "BitManipulation.h"
#include "const.h"
#include "Accumulator.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
//#include "Movegen.h"
//inline int getFile(int square)
//{
//    return (square) % 8;
//}
//
//inline int getRank(int square)
//{
//    return (square) != 0 ? 7 - (square) / 8 : 7;
//}
constexpr uint64_t A_file = 0x0101010101010101ULL;
//constexpr uint64_t H_file = 0x8080808080808080ULL;
constexpr uint64_t files_bitboard[8] = {
    A_file, A_file << 1, A_file << 2, A_file << 3,
    A_file << 4, A_file << 5, A_file << 6, A_file << 7
};
// file masks [square]
uint64_t fileMasks[64];

// rank masks [square]
uint64_t rankMasks[64];
uint64_t whitePassedMasks[64];
uint64_t blackPassedMasks[64];
Network Eval_Network;
//static int history[12][64];
inline int getFile(int square)
{
	return (square) % 8;
}

inline int getRank(int square)
{
	return (square) != 0 ? 7 - (square) / 8 : 7;
}
//int pawn_mg_passed_bonus[8] = {}
inline int getSide(int piece)
{
    return (piece > 5) ? Black : White;
    //Piece
};
int mg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0,
};

int eg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
};

int mg_knight_table[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

int eg_knight_table[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

int mg_bishop_table[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

int eg_bishop_table[64] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
};

int mg_rook_table[64] = {
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
};

int eg_rook_table[64] = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};

int mg_queen_table[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

int eg_queen_table[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

int* mg_pesto_table[6] =
{
    mg_pawn_table,
    mg_knight_table,
    mg_bishop_table,
    mg_rook_table,
    mg_queen_table,
    mg_king_table
};

int* eg_pesto_table[6] =
{
    eg_pawn_table,
    eg_knight_table,
    eg_bishop_table,
    eg_rook_table,
    eg_queen_table,
    eg_king_table
};

uint64_t white_pawn_span[64];
uint64_t black_pawn_span[64];


//enum Piece {
//    P = 0, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE
//};
int mg_piece_value[12]
{
    80, 320, 330, 470, 1025, 0, -80, -320, -330, -470, -1025, 0
};
int eg_piece_value[12]
{
    100, 320, 330, 530, 940, 0, -100, -320, -330, -530, -940, 0
};

int side_multiply[2]
{
    1, -1
};





void LoadNetwork(const std::string& filepath)
{
	std::ifstream stream(filepath, std::ios::binary);
	if (!stream.is_open()) {
		std::cerr << "Failed to open file: " << filepath << std::endl;
	}
	uint64_t sum = 0;
	//std::cout << sum << std::endl;
	// Load weightsToHL
	for (size_t row = 0; row < INPUT_SIZE; ++row) {
		for (size_t col = 0; col < HL_SIZE; ++col) {
			Eval_Network.accumulator_weights[row][col] = readLittleEndian<int16_t>(stream);
			sum *= 7;
			sum += (uint16_t)Eval_Network.accumulator_weights[row][col];
		}
	}
	//std::cout << sum << std::endl;
	// Load hiddenLayerBias
	for (size_t i = 0; i < HL_SIZE; ++i) {
		Eval_Network.accumulator_biases[i] = readLittleEndian<int16_t>(stream);
		sum *= 7;
		sum += (uint16_t)Eval_Network.accumulator_biases[i];
	}
	//std::cout << sum << std::endl;
	// Load weightsToOut
	for (size_t i = 0; i < 2 * HL_SIZE; ++i) {
		Eval_Network.output_weights[i] = readLittleEndian<int16_t>(stream);
		//std::cout << Eval_Network.output_weights[i] <<std::endl;
		sum *= 7;
		sum += (uint16_t)Eval_Network.output_weights[i];
	}
	//std::cout << sum << std::endl;
	// Load outputBias
	sum *= 7;

	
	Eval_Network.output_bias = readLittleEndian<int16_t>(stream);
	sum += (uint16_t)Eval_Network.output_bias;
	
	//std::cout << sum << std::endl;
	//std::cout << (uint16_t)Eval_Network.output_bias;
}
//int gamephaseInc[12] = { 0,0,1,1,1,1,2,2,4,4,0,0 };
int gamephaseInc[12] = { 0,1,1,2,4,0,0,1,1,2,4,0 };
int mg_table[12][64];
int eg_table[12][64];
void print_table(const int table[64]) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            std::cout << std::setw(4) << table[square] << " ";
        }
        std::cout << std::endl;
    }
}
void print_tables() {
    const char* piece_names[12] = {
        "White Pawn", "White Knight", "White Bishop", "White Rook", "White Queen", "White King",
        "Black Pawn", "Black Knight", "Black Bishop", "Black Rook", "Black Queen", "Black King"
    };

    for (int piece = 0; piece < 12; piece++) {
        std::cout << piece_names[piece] << " Middle-game Table:" << std::endl;
        print_table(mg_table[piece]);
        std::cout << std::endl;

        std::cout << piece_names[piece] << " End-game Table:" << std::endl;
        print_table(eg_table[piece]);
        std::cout << std::endl;
    }
}




void init_tables()
{
    //int pc, sq;

    for (int piece = P; piece < K + 1; piece++)
    {
        for (int square = 0; square < 64; square++)
        {
            mg_table[piece][square] = mg_piece_value[piece] + mg_pesto_table[piece][square];
            eg_table[piece][square] = eg_piece_value[piece] + eg_pesto_table[piece][square];
        }


    }
    for (int piece = p; piece < k + 1; piece++)
    {
        for (int square = 0; square < 64; square++)
        {
            mg_table[piece][square] = mg_piece_value[piece] - mg_pesto_table[get_piece(piece, White)][getFile(square) + 8 * getRank(square)];
            eg_table[piece][square] = eg_piece_value[piece] - eg_pesto_table[get_piece(piece, White)][getFile(square) + 8 * getRank(square)];
        }


    }
    //precalculate_pawn_spans();
    //PrintBitboard(files_bitboard[5]);
}
uint64_t getFileBitboard(uint64_t pieces, int file) {
    return pieces & files_bitboard[file];
}
//inline int flipSquare(int square)//flip square so a1 = 0
//{
//
//	return square ^ 56;
//}

int32_t SCReLU(int32_t value, int32_t min, int32_t max)
{
	const int32_t clamped = std::clamp(value, min, max);
	return clamped * clamped;
}

//SCReLU activation function
int32_t activation(int16_t value)
{
	return SCReLU(value, 0, QA);
}
void accumulatorAdd(struct Network* const network, struct Accumulator* accumulator, size_t index)
{
	for (int i = 0; i < HL_SIZE; i++)
		accumulator->values[i] += network->accumulator_weights[index][i];
}

void accumulatorSub(struct Network* const network, struct Accumulator* accumulator, size_t index)
{
	for (int i = 0; i < HL_SIZE; i++)
		accumulator->values[i] -= network->accumulator_weights[index][i];
}

void resetAccumulators(const Board& board, AccumulatorPair& accumulator) {
	uint64_t whitePieces = board.occupancies[White];
	uint64_t blackPieces = board.occupancies[Black];

	
	memcpy(accumulator.white.values, Eval_Network.accumulator_biases, sizeof(Eval_Network.accumulator_biases));
	memcpy(accumulator.black.values, Eval_Network.accumulator_biases, sizeof(Eval_Network.accumulator_biases));
	//for (int i = 0; i < 16; i++)
	//{
	//	std::cout << accumulator.white.values[i] << " ";
	//}
	
	while (whitePieces) {
		int sq = get_ls1b(whitePieces);

		uint16_t whiteInputFeature = calculateIndex(White, sq, get_piece(board.mailbox[sq], White), White);
		//std::cout << whiteInputFeature<<" ";
		uint16_t blackInputFeature = calculateIndex(Black, sq, get_piece(board.mailbox[sq], White), White);
	    //std::cout << blackInputFeature << " ";
		for (size_t i = 0; i < HL_SIZE; i++) {
			accumulator.white.values[i] += Eval_Network.accumulator_weights[whiteInputFeature][i];
			accumulator.black.values[i] += Eval_Network.accumulator_weights[blackInputFeature][i];
		}

		Pop_bit(whitePieces, sq);
	}
	while (blackPieces) {
		int sq = get_ls1b(blackPieces);

		uint16_t whiteInputFeature = calculateIndex(White, sq, get_piece(board.mailbox[sq], White), Black);
		//std::cout << whiteInputFeature << " ";
		uint16_t blackInputFeature = calculateIndex(Black, sq, get_piece(board.mailbox[sq], White), Black);
		//std::cout << blackInputFeature << " ";
		for (size_t i = 0; i < HL_SIZE; i++) {
			accumulator.white.values[i] += Eval_Network.accumulator_weights[whiteInputFeature][i];
			accumulator.black.values[i] += Eval_Network.accumulator_weights[blackInputFeature][i];
		}
		Pop_bit(blackPieces, sq);
	}
	//for (int i = 0; i < 16; i++)
	//{
	//	std::cout<<accumulator.white.values[i]<<" ";
	//}
}


std::int32_t autovectorised_screlu(Network const * network, Accumulator const * stm, Accumulator const *nstm) {
	std::int32_t accumulator{};
	for (int i = 0; i < HL_SIZE; i++) {
		accumulator += (int32_t)activation(stm->values[i]) * network->output_weights[i];
		accumulator += (int32_t)activation(nstm->values[i]) * network->output_weights[i + HL_SIZE];
	}
	return accumulator;
}

#if defined(__x86_64__) || defined(__amd64__) || (defined(_WIN64) && (defined(_M_X64) ||  defined(_M_AMD64)))
#define is_x86 1
#if defined(__AVX512F__) || defined(__AVX2__) || defined(__SSE__)
#define has_simd 1
#else
#define has_simd 0
#endif
#else
// TODO: arm
#define is_x86 0
#define has_simd 0
#endif

#if is_x86 && has_simd
#include <immintrin.h>
#endif

std::int32_t vectorised_screlu(Network const * network, Accumulator const * stm, Accumulator const *nstm) {
#if is_x86 && has_simd
#if defined(__AVX512F__)
	using native_vector = __m512i;
	#define set1_epi16 _mm512_set1_epi16
	#define load_epi16 _mm512_load_si512
	#define min_epi16 _mm512_min_epi16
	#define max_epi16 _mm512_max_epi16
	#define madd_epi16 _mm512_madd_epi16
	#define mullo_epi16 _mm512_mullo_epi16
	#define add_epi32 _mm512_add_epi32
	#define reduce_epi32 _mm512_reduce_add_epi32 
#elif defined(__AVX2__)
	using native_vector = __m256i;
	#define set1_epi16 _mm256_set1_epi16
	#define load_epi16 [](void *ptr) { return _mm256_load_si256((native_vector const *) ptr; }
	#define min_epi16 _mm256_min_epi16
	#define max_epi16 _mm256_max_epi16
	#define madd_epi16 _mm256_madd_epi16
	#define mullo_epi16 _mm256_mullo_epi16
	#define add_epi32 _mm256_add_epi32
	// based on output from zig 0.14.0 for @reduce(.Add, @as(@Vector(8, i32), x)
	#define reduce_epi32 [](native_vector vec) {         \
		__m128i xmm1 = _mm256_extracti128_si256(vec, 1); \
		__m128i xmm0 = _mm256_castsi256_si128(vec);      \
		xmm0 = _mm_add_epi32(xmm0, xmm1);                \
		xmm1 = _mm_shuffle_epi32(xmm0, 238);             \
		xmm0 = _mm_add_epi32(xmm0, xmm1);                \
		xmm1 = _mm_shuffle_epi32(xmm0, 85);              \
		xmm0 = _mm_add_epi32(xmm0, xmm1);                \
		return _mm_cvtsi128_si32(xmm0);                  \
	}
#elif defined(__SSE__)
	using native_vector = __m128i;
	#define set1_epi16 _mm_set1_epi16
	#define load_epi16 [](void *ptr) { return _mm_load_si128((native_vector const *) ptr; }
	#define min_epi16 _mm_min_epi16
	#define max_epi16 _mm_max_epi16
	#define madd_epi16 _mm_madd_epi16
	#define mullo_epi16 _mm_mullo_epi16
	#define add_epi32 _mm_add_epi32
	// based on output from zig 0.14.0 for @reduce(.Add, @as(@Vector(4, i32), x)
	#define reduce_epi32 [](native_vector vec) {    \
		__m128i xmm1 = _mm_shuffle_epi32(vec, 238); \
		vec = _mm_add_epi32(vec, xmm1);             \
		xmm1 = _mm_shuffle_epi32(vec, 85);          \
		vec = _mm_add_epi32(vec, xmm1);             \
		return _mm_cvtsi128_si32(vec);              \
	}
#endif
	constexpr auto VECTOR_SIZE = sizeof(native_vector) / sizeof(std::int16_t);
	static_assert(HL_SIZE % VECTOR_SIZE == 0, "HL_SIZE must be divisible by the native register size for this vectorization implementation to work");
	const native_vector VEC_QA = set1_epi16(QA);
	const native_vector VEC_ZERO = set1_epi16(0);
	
	constexpr auto UNROLL = 2;

	std::array<native_vector, UNROLL> accumulator{};
	int i = 0;
	
	const auto one_iteration = [&](auto idx, native_vector& acc) {
		// load accumulator values
		const native_vector stm_accum_values = load_epi16(&stm->values[idx]);
		const native_vector nstm_accum_values = load_epi16(&nstm->values[idx]);

		// load network weights
		const native_vector stm_weights = load_epi16(&network->output_weights[idx]);
		const native_vector nstm_weights = load_epi16(&network->output_weights[idx + HL_SIZE]);
		
		// clamp the values to [0, QA] 
		const native_vector stm_clamped = min_epi16(VEC_QA, max_epi16(stm_accum_values, VEC_ZERO));
		const native_vector nstm_clamped = min_epi16(VEC_QA, max_epi16(nstm_accum_values, VEC_ZERO));
		
		// apply lizard screlu
		const native_vector stm_screlud = madd_epi16(stm_clamped, mullo_epi16(stm_clamped, stm_weights));
		const native_vector nstm_screlud = madd_epi16(nstm_clamped, mullo_epi16(nstm_clamped, nstm_weights));
		
		acc = add_epi32(acc, stm_screlud);
		acc = add_epi32(acc, nstm_screlud);
	};

	if constexpr (HL_SIZE >= UNROLL * VECTOR_SIZE) {
		for (;i < HL_SIZE; i += UNROLL * VECTOR_SIZE) {
			for (int j = 0; j < UNROLL; ++j) {
				one_iteration(i + j * VECTOR_SIZE, accumulator[j]);
			}
		}
	}
	for (;i < HL_SIZE; i += VECTOR_SIZE) {
		one_iteration(i, accumulator[0]);
	}
	if constexpr (HL_SIZE >= UNROLL * VECTOR_SIZE) {
		for (int j = 1; j < UNROLL; ++j) {
			accumulator[0] = add_epi32(accumulator[0], accumulator[j]);
		}
	}
	return reduce_epi32(accumulator[0]);
#else
	return autovectorised_screlu(network, stm, nstm);
#endif
}

// When forwarding the accumulator values, the network does not consider the color of the perspectives.
// Rather, we are more interested in whether the accumulator is from the perspective of the side-to-move.
int32_t forward(struct Network* const network,
	struct Accumulator* const stm_accumulator,
	struct Accumulator* const nstm_accumulator)
{
	int32_t eval = vectorised_screlu(network, stm_accumulator, nstm_accumulator);

	// Uncomment the following dequantization step when using SCReLU
	eval /= QA;
	eval += network->output_bias;

	eval *= SCALE;
	eval /= QA * QB;

	return eval;
}

int Evaluate(Board& board)
{

	//	for (int i = 0; i < 16; i++)
	//{
	//	std::cout<< eval_accumulator.white.values[i]<<" ";
	//}
		//return forward(&Eval_Network, &eval_accumulator.white, &eval_accumulator.black);
	if (board.side == White)
		return forward(&Eval_Network, &board.accumulator.white, &board.accumulator.black);
	else
		return forward(&Eval_Network, &board.accumulator.black, &board.accumulator.white);


//
//	if (board.side == White)
//		std::cout<< forward(&Eval_Network, &board.accumulator.white, &board.accumulator.black);
//	else
//		std::cout<< forward(&Eval_Network, &board.accumulator.black, &board.accumulator.white);
//
//	//if (board.side == White)
//	//	return forward(&Eval_Network, &board.accumulator.white, &board.accumulator.black);
//	//else
//	//	return forward(&Eval_Network, &board.accumulator.black, &board.accumulator.white);
//
//	AccumulatorPair eval_accumulator;
//	resetAccumulators(board, eval_accumulator);
////	for (int i = 0; i < 16; i++)
////{
////	std::cout<< eval_accumulator.white.values[i]<<" ";
////}
//	//return forward(&Eval_Network, &eval_accumulator.white, &eval_accumulator.black);
//	if (board.side == White)
//		return forward(&Eval_Network, &eval_accumulator.white, &eval_accumulator.black);
//	else
//		return forward(&Eval_Network, &eval_accumulator.black, &eval_accumulator.white);




    //int mg[2];
    //int eg[2];

    //mg[White] = 0;
    //mg[Black] = 0;
    //eg[White] = 0;
    //eg[Black] = 0;

    ////int eval = 0;
    //int gamePhase = 0;

    //int evalSide = board.side;

    //for (int sq = 0; sq < 64; ++sq) {
    //    int pc = board.mailbox[sq];
    //    if (pc != NO_PIECE) {
    //        int col = getSide(pc);
    //        mg[col] += mg_table[pc][sq];
    //        eg[col] += eg_table[pc][sq];
    //        gamePhase += gamephaseInc[pc];
    //    }
    //}
    //int mgScore = mg[evalSide] + mg[1 - evalSide];
    //int egScore = eg[evalSide] + eg[1 - evalSide];
    //int mgPhase = gamePhase;
    //if (mgPhase > 24) mgPhase = 24; /* in case of early promotion */
    //int egPhase = 24 - mgPhase;

    //int Whiteeval = (mgScore * mgPhase + egScore * egPhase) / 24;

    //int WhiteBishops = count_bits(board.bitboards[B]);
    //int BlackBishops = count_bits(board.bitboards[b]);

    //int white_bishoppair = 0;
    //int black_bishoppair = 0;

    //if (WhiteBishops >= 2)
    //{
    //    white_bishoppair = 50;
    //}
    //if (BlackBishops >= 2)
    //{
    //    black_bishoppair = 50;
    //}
    //Whiteeval += white_bishoppair;
    //Whiteeval -= black_bishoppair;
    //return Whiteeval * side_multiply[evalSide];

}
