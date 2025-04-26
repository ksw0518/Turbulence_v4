#pragma once
#include <string>
#include <vector>
#include <cstdint>

inline int getFile(int square)
{
    return (square) % 8;
}

inline int getRank(int square)
{
    return (square) != 0 ? 7 - (square) / 8 : 7;
}

class Board;
struct Move;
struct MoveList;

int GetSquare(std::string squareName);

//bool isWholeNumber(float number);

//static uint64_t Calcbetween(int a, int b);
//


uint64_t get_bishop_attacks(int square, uint64_t occupancy);
uint64_t get_rook_attacks(int square, uint64_t occupancy);
uint64_t get_queen_attacks(int square, uint64_t occupancy);
void PrintBitboard(uint64_t bitboard);
void printMove(Move move);

uint64_t all_attackers_to_square(Board& board, uint64_t occupied, int sq);
int getSide(int piece);

void InitializeBetweenTable();

void init_sliders_attacks(int bishop);




void InitializeLeaper();


bool is_square_attacked(int square, int side, Board &board, uint64_t occupancy);
void Generate_Legal_Moves(MoveList& MoveList, Board& board, bool isCapture);
int get_castle(uint64_t castle, int side);

void Unmake_Nullmove(Board& board);
void Make_Nullmove(Board& board);
void MakeMove(Board& board, Move move);
void UnmakeMove(Board& board, Move move, int captured_piece);
uint64_t get_attacked_squares(int side, Board& board, uint64_t occupancy);
bool isLegal(Move& move, Board& board);

void init_random_keys();

uint64_t generate_hash_key(Board& board);
uint64_t generate_Pawn_Hash(Board& board);
uint64_t generate_Minor_Hash(Board& board);

uint64_t generate_WhiteNP_Hash(Board& board);

uint64_t generate_BlackNP_Hash(Board& board);
