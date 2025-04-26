#pragma once
#include "Board.h"
#include <cstdint> 
#include <iostream>
#include <cstring>

enum Square {
    A8 = 0, B8, C8, D8, E8, F8, G8, H8,
    A7 = 8, B7, C7, D7, E7, F7, G7, H7,
    A6 = 16, B6, C6, D6, E6, F6, G6, H6,
    A5 = 24, B5, C5, D5, E5, F5, G5, H5,
    A4 = 32, B4, C4, D4, E4, F4, G4, H4,
    A3 = 40, B3, C3, D3, E3, F3, G3, H3,
    A2 = 48, B2, C2, D2, E2, F2, G2, H2,
    A1 = 56, B1, C1, D1, E1, F1, G1, H1,

    NO_SQ = 64
};

// Optionally, define const expressions for the enumeration values
constexpr int a8 = A8;
constexpr int b8 = B8;
constexpr int c8 = C8;
constexpr int d8 = D8;
constexpr int e8 = E8;
constexpr int f8 = F8;
constexpr int g8 = G8;
constexpr int h8 = H8;

constexpr int a7 = A7;
constexpr int b7 = B7;
constexpr int c7 = C7;
constexpr int d7 = D7;
constexpr int e7 = E7;
constexpr int f7 = F7;
constexpr int g7 = G7;
constexpr int h7 = H7;

constexpr int a6 = A6;
constexpr int b6 = B6;
constexpr int c6 = C6;
constexpr int d6 = D6;
constexpr int e6 = E6;
constexpr int f6 = F6;
constexpr int g6 = G6;
constexpr int h6 = H6;

constexpr int a5 = A5;
constexpr int b5 = B5;
constexpr int c5 = C5;
constexpr int d5 = D5;
constexpr int e5 = E5;
constexpr int f5 = F5;
constexpr int g5 = G5;
constexpr int h5 = H5;

constexpr int a4 = A4;
constexpr int b4 = B4;
constexpr int c4 = C4;
constexpr int d4 = D4;
constexpr int e4 = E4;
constexpr int f4 = F4;
constexpr int g4 = G4;
constexpr int h4 = H4;

constexpr int a3 = A3;
constexpr int b3 = B3;
constexpr int c3 = C3;
constexpr int d3 = D3;
constexpr int e3 = E3;
constexpr int f3 = F3;
constexpr int g3 = G3;
constexpr int h3 = H3;

constexpr int a2 = A2;
constexpr int b2 = B2;
constexpr int c2 = C2;
constexpr int d2 = D2;
constexpr int e2 = E2;
constexpr int f2 = F2;
constexpr int g2 = G2;
constexpr int h2 = H2;

constexpr int a1 = A1;
constexpr int b1 = B1;
constexpr int c1 = C1;
constexpr int d1 = D1;
constexpr int e1 = E1;
constexpr int f1 = F1;
constexpr int g1 = G1;
constexpr int h1 = H1;

constexpr int no_sq = NO_SQ;

constexpr int P = 0;
constexpr int N = 1;
constexpr int B = 2;
constexpr int R = 3;
constexpr int Q = 4;
constexpr int K = 5;
constexpr int p = 6;
constexpr int n = 7;
constexpr int b = 8;
constexpr int r = 9;
constexpr int q = 10;
constexpr int k = 11;
constexpr int NO_PIECE = 12;

constexpr uint64_t WhiteKingCastle = 0x0001;
constexpr uint64_t WhiteQueenCastle = 0x0010;
constexpr uint64_t BlackKingCastle = 0x0100;
constexpr uint64_t BlackQueenCastle = 0x1000;

constexpr int White = 0;
constexpr int Black = 1;
constexpr int Both = 2;


constexpr uint8_t promotionFlag = 0b1000;
constexpr uint8_t captureFlag = 0b0100;
constexpr uint8_t special1Flag = 0b0010;
constexpr uint8_t special0Flag = 0b0001;

constexpr uint8_t quiet_move = 0;
constexpr uint8_t double_pawn_push = special0Flag;
constexpr uint8_t king_castle = special1Flag;
constexpr uint8_t queen_castle = special0Flag | special1Flag;
constexpr uint8_t capture = captureFlag;
constexpr uint8_t ep_capture = captureFlag | special0Flag;
constexpr uint8_t knight_promo = promotionFlag;
constexpr uint8_t bishop_promo = promotionFlag | special0Flag;
constexpr uint8_t rook_promo = promotionFlag | special1Flag;
constexpr uint8_t queen_promo = promotionFlag | special1Flag | special0Flag;
constexpr uint8_t knight_promo_capture = knight_promo | capture;
constexpr uint8_t bishop_promo_capture = bishop_promo | capture;
constexpr uint8_t rook_promo_capture = rook_promo | capture;
constexpr uint8_t queen_promo_capture = queen_promo | capture;


constexpr uint64_t WhiteKingCastleEmpty = (1ULL << f1) | (1ULL << g1);
constexpr uint64_t WhiteQueenCastleEmpty = (1ULL << d1) | (1ULL << c1) | (1ULL << b1);
constexpr uint64_t BlackKingCastleEmpty = (1ULL << f8) | (1ULL << g8);
constexpr uint64_t BlackQueenCastleEmpty = (1ULL << d8) | (1ULL << c8) | (1ULL << b8);


constexpr uint64_t WhiteQueenCastleAttack = (1ULL << d1) | (1ULL << c1);
constexpr uint64_t BlackQueenCastleAttack = (1ULL << d8) | (1ULL << c8);


constexpr int PLUS_INFINITY = 50000;
constexpr int MINUS_INFINITY = -50000;

constexpr int ExactFlag = 1;
constexpr int UpperBound = 2;
constexpr int LowerBound = 3;

static int Side_value[] = { 0, 6 };
static int Get_Whitepiece[] = { 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5 };

static inline const uint16_t  Le = 1;
static inline const bool IS_LITTLE_ENDIAN = *reinterpret_cast<const char*>(&Le) == 1;

const int WHITEWIN = 2;
const int DRAW = 1;
const int BLACKWIN = 0;


constexpr int64_t NOLIMIT = -1;
inline int get_piece(int piece, int col)
{
    //std::cout << Get_Whitepiece[piece] << "\n";
    //std::cout << Side_value[col] << "\n";
    return Get_Whitepiece[piece] + Side_value[col];


    
    //return 1;
    //bool isBlack = piece >= 6;
    //return (col == Side.White) ? (isBlack ? piece - 6 : piece) : (isBlack ? piece : piece + 6);

}
struct MoveList
{
	Move moves[256];  // Fixed-size array
	int count = 0;  // Number of moves currently stored

	void clear() { count = 0; }  // Reset move list
	void add(Move move) { if (count < 256) moves[count++] = move; } // Add move
};
template<typename IntType>
inline IntType readLittleEndian(std::istream& stream) {
	IntType result;

	if (IS_LITTLE_ENDIAN)
		stream.read(reinterpret_cast<char*>(&result), sizeof(IntType));
	else {
		std::uint8_t                  u[sizeof(IntType)];
		std::make_unsigned_t<IntType> v = 0;

		stream.read(reinterpret_cast<char*>(u), sizeof(IntType));
		for (size_t i = 0; i < sizeof(IntType); ++i)
			v = (v << 8) | u[sizeof(IntType) - i - 1];

		std::memcpy(&result, &v, sizeof(IntType));
	}

	return result;
}