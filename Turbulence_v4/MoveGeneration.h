#pragma once
#include <string>
#include <vector>

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

int GetSquare(std::string squareName);

//bool isWholeNumber(float number);

//static uint64_t Calcbetween(int a, int b);
//


uint64_t get_bishop_attacks(int square, uint64_t occupancy);
uint64_t get_rook_attacks(int square, uint64_t occupancy);
uint64_t get_queen_attacks(int square, uint64_t occupancy);
void PrintBitboard(uint64_t bitboard);
void printMove(Move move);
//static Dictionary<int, string> MoveType = new()
//{
//    {quiet_move, "quiet_move" },
//    {double_pawn_push, "double_pawn_push" },
//    {king_castle, "king_castle" },
//    {queen_castle, "queen_castle" },
//    {capture, "capture" },
//    {ep_capture, "ep_capture" },
//    {knight_promo, "knight_promo" },
//    {bishop_promo, "bishop_promo" },
//    {rook_promo, "rook_promo" },
//    {queen_promo, "queen_promo" },
//    {knight_promo_capture, "knight_promo_capture" },
//    {bishop_promo_capture, "bishop_promo_capture" },
//    {rook_promo_capture, "rook_promo_capture" },
//    {queen_promo_capture, "queen_promo_capture" },
//
//
//};

//enum Square {
//    a8 = 0, b8, c8, d8, e8, f8, g8, h8,
//    a7 = 8, b7, c7, d7, e7, f7, g7, h7,
//    a6 = 16, b6, c6, d6, e6, f6, g6, h6,
//    a5 = 24, b5, c5, d5, e5, f5, g5, h5,
//    a4 = 32, b4, c4, d4, e4, f4, g4, h4,
//    a3 = 40, b3, c3, d3, e3, f3, g3, h3,
//    a2 = 48, b2, c2, d2, e2, f2, g2, h2,
//    a1 = 56, b1, c1, d1, e1, f1, g1, h1,
//    no_sq = 64
//};
//
//
//enum Piece {
//    P = 0, N, B, R, Q, K, p, n, b, r,q, k, NO_PIECE
//};



//inline uint64_t between(int a, int b);


uint64_t all_attackers_to_square(Board& board, uint64_t occupied, int sq);
int getSide(int piece);

void InitializeBetweenTable();
//uint64_t CalculatePawnAttack(int square, int side);
//
//uint64_t CalculateKnightAttack(int square);
//uint64_t CalculateKingAttack(int square);
//
//void InitializeLeaper();
//
//uint64_t MaskBishopAttack(int square);
//
//uint64_t MaskRookAttack(int square);
//uint64_t CalculateRookAttack(int square, uint64_t block);
//
//uint64_t CalculateBishopAttack(int square, uint64_t block);
void init_sliders_attacks(int bishop);




void InitializeLeaper();

//static void Generate_Pawn_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask);
//static void Generate_Knight_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask);
//static void Generate_Bishop_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask);
//static void Generate_Rook_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask);
//static void Generate_Queen_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask);
//static void Generate_King_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask);

bool is_square_attacked(int square, int side, Board &board, uint64_t occupancy);
void Generate_Legal_Moves(std::vector<Move>& MoveList, Board& board, bool isCapture);
int get_castle(uint64_t castle, int side);

void Unmake_Nullmove(Board& board);
void Make_Nullmove(Board& board);
void MakeMove(Board& board, Move move);
void UnmakeMove(Board& board, Move move, int captured_piece);
uint64_t get_attacked_squares(int side, Board& board, uint64_t occupancy);
bool isMoveValid(Move& move, Board& board);

void init_random_keys();

uint64_t generate_hash_key(Board& board);
uint64_t generate_Pawn_Hash(Board& board);
uint64_t generate_Minor_Hash(Board& board);
uint64_t generate_Major_Hash(Board& board);