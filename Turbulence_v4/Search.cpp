#include "MoveGeneration.h"
#include "Evaluation.h"
#include "Board.h"
#include "Search.h"
#include "BitManipulation.h"
#include "const.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <tuple>
#include <vector>
#include <string>
#include <random>
#include <utility>  // for std::pair
#include <fstream>
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX

#include <windows.h>

#undef NOMINMAX
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN

#endif

#ifndef EVALFILE
#define EVALFILE "./nnue.bin"
#endif


#if defined(_WIN32) || defined(_WIN64)
// Windows platform
constexpr bool isOnWindow = true;
#elif defined(__linux__)
// Linux platform
constexpr bool isOnWindow = false;
#else
// Unknown platform
constexpr bool isOnWindow = false;  // Default to false (likely macOS)
#endif
#define NULLMOVE Move(0,0,0,0)

enum class ConsoleColor {
	Black = 0,
	Blue = 1,
	Green = 2,
	Cyan = 3,
	Red = 4,
	Magenta = 5,
	Yellow = 6,
	White = 7,
	Gray = 8,
	BrightBlue = 9,
	BrightGreen = 10,
	BrightCyan = 11,
	BrightRed = 12,
	BrightMagenta = 13,
	BrightYellow = 14,
	BrightWhite = 15
};

std::string benchFens[] = { // fens from alexandria, ultimately from bitgenie
	"r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 14",
	"4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 2 24",
	"r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 16 42",
	"6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 4 44",
	"8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 3 54",
	"7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 36",
	"r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 2 10",
	"3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 3 87",
	"2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 42",
	"4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 37",
	"2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 2 34",
	"1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 1 37",
	"r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 17",
	"8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 16 42",
	"1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 8 29",
	"8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 29 68",
	"3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 2 20",
	"5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 1 49",
	"1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 1 51",
	"q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 2 34",
	"r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 22",
	"r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 5",
	"r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 1 12",
	"r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 2 19",
	"r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 2 25",
	"r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 32",
	"r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 1 5",
	"3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 12",
	"5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 2 20",
	"8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 3 27",
	"8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 1 33",
	"8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 1 38",
	"8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 15 45",
	"8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 5 49",
	"8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 6 54",
	"8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 6 59",
	"8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 14 63",
	"8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 67",
	"1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 4 23",
	"4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 3 22",
	"r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 4 12",
	"2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 55",
	"6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 1 44",
	"2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 3 25",
	"r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 2 14",
	"6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 1 36",
	"rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
	"2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 20",
	"3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 23",
	"2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 23 93"
};
int RFP_MULTIPLIER = 89;
int RFP_IMPROVING_MULTIPLIER = 60;
int RFP_BASE = -42;
int RFP_IMPROVING_BASE = -35;

int LMP_BASE = 1;
int LMP_MULTIPLIER = 2;

int PVS_QUIET_BASE = -3;
int PVS_QUIET_MULTIPLIER = 62;

int PVS_NOISY_BASE = -11;
int PVS_NOISY_MULTIPLIER = 19;

int HISTORY_BASE = 130;
int HISTORY_MULTIPLIER = 87;
int CONTHIST_BASE = 64; 
int CONTHIST_MULTIPLIER = 48;

int ASP_WINDOW_INITIAL = 25;
int ASP_WINDOW_MAX = 306;

int PAWN_CORRHIST_MULTIPLIER = 177;// divide by 5 later
int MINOR_CORRHIST_MULTIPLIER = 156;// divide by 5 later
int NONPAWN_CORRHIST_MULTIPLIER = 183;// divide by 5 later
int COUNTERMOVE_CORRHIST_MULTIPLIER = 150;// divide by 5 later

int QS_SEE_PRUNING_MARGIN = -2;
int HISTORY_PRUNING_MULTIPLIER = 1320;
int HISTORY_PRUNING_BASE = 62;
int HISTORY_LMR_MULTIPLIER = 772;
int HISTORY_LMR_BASE = 86;
int NMP_EVAL_DIVISER = 399;
int NMP_DEPTH_DIVISER = 3;
int MAX_NMP_EVAL_R = 3;

int DEXT_MARGIN = 21;

int pvLengths[99];
Move pvTable[99][99];

int SEEPieceValues[] = { 98, 280, 295, 479, 1064, 0, 0 };
static Move lastBestMoves[99];

size_t TTSize = 699050;
TranspositionEntry* TranspositionTable = nullptr;

constexpr int MAX_HISTORY = 16384;
constexpr int MAX_CONTHIST = 16384;
constexpr int MAX_CAPTHIST = 1024;

constexpr int MIN_LMR_DEPTH = 3;

constexpr int MAX_PVS_SEE_DEPTH = 8;
int lmrTable[99][256];

constexpr int HFLOWER = 0;
constexpr int HFEXACT = 1;
constexpr int HFUPPER = 2;

bool isPrettyPrinting = true;

double DEF_TIME_MULTIPLIER = 0.054;
double DEF_INC_MULTIPLIER = 0.85;
double MAX_TIME_MULTIPLIER = 0.76;
double HARD_LIMIT_MULTIPLIER = 3.04;
double SOFT_LIMIT_MULTIPLIER = 0.76;

uint64_t hardNodeBound;

static int MVVLVA[6][6] = {
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};
bool isMoveQuiet(int type)
{
	if (((type & captureFlag) == 0) && ((type & promotionFlag) == 0)) //noisy move = promotion or capture
	{
		return true;
	}
	else
	{
		return false;
	}

}
void initializeLMRTable(ThreadData& data)
{
	for (int depth = 1; depth < 99; depth++)
	{
		for (int move = 1; move < 256; move++)
		{
			lmrTable[depth][move] = std::floor(0.77 + log(move) * log(depth) / 2.36);
		}
	}
	for (int ply = 0; ply < 99; ply++)
	{
		data.killerMoves[0][ply] = Move();
		data.killerMoves[1][ply] = Move();
	}
	memset(data.mainHistory, 0, sizeof(data.mainHistory));
	memset(data.onePlyContHist, 0, sizeof(data.onePlyContHist));
	memset(data.twoPlyContHist, 0, sizeof(data.twoPlyContHist));
	memset(data.CaptureHistory, 0, sizeof(data.CaptureHistory));
	memset(data.pawnCorrHist, 0, sizeof(data.pawnCorrHist));
	memset(data.nonPawnCorrHist, 0, sizeof(data.nonPawnCorrHist));
	memset(data.minorCorrHist, 0, sizeof(data.minorCorrHist));
	memset(data.counterMoveCorrHist, 0, sizeof(data.counterMoveCorrHist));

	isPrettyPrinting = true;
	for (int ply = 0; ply < 99; ply++)
	{
		data.killerMoves[0][ply] = Move();
		data.killerMoves[1][ply] = Move();
	}
	memset(data.mainHistory, 0, sizeof(data.mainHistory));

	LoadNetwork(EVALFILE);
	//for (size_t i = 0; i < TTSize; i++)
	//{
	//	TranspositionTable[i] = TranspositionEntry();
	//}

}
int scaledBonus(int score, int bonus)
{
	return std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY) - (score * abs(bonus) / MAX_HISTORY);
}
void updateMinorCorrHist(Board& board, const int depth, const int diff, ThreadData& data)
{
	uint64_t minorKey = board.MinorKey;
	int& entry = data.minorCorrHist[board.side][minorKey % CORRHIST_SIZE];
	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);
	entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	entry = std::clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
}
void updatePawnCorrHist(Board& board, const int depth, const int diff, ThreadData& data)
{
	uint64_t pawnKey = board.PawnKey;
	int& entry = data.pawnCorrHist[board.side][pawnKey % CORRHIST_SIZE];
	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);
	entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	entry = std::clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
}
void updateCounterCorrHist(Move prevMove, const int depth, const int diff, ThreadData& data)
{
	//uint64_t pawnKey = board.PawnKey;
	int& entry = data.counterMoveCorrHist[prevMove.Piece][prevMove.To];
	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);
	entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	entry = std::clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
}
void updateNonPawnCorrHist(Board& board, const int depth, const int diff, ThreadData& data)
{
	uint64_t whiteKey = board.WhiteNonPawnKey;
	uint64_t blackKey = board.BlackNonPawnKey;

	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);

	int& whiteEntry = data.nonPawnCorrHist[White][board.side][whiteKey % CORRHIST_SIZE];

	whiteEntry = (whiteEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	whiteEntry = std::clamp(whiteEntry, -CORRHIST_MAX, CORRHIST_MAX);

	int& blackEntry = data.nonPawnCorrHist[Black][board.side][blackKey % CORRHIST_SIZE];

	blackEntry = (blackEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	blackEntry = std::clamp(blackEntry, -CORRHIST_MAX, CORRHIST_MAX);
}
int adjustEvalWithCorrHist(Board& board, const int rawEval, Move prevMove, ThreadData& data)
{
	uint64_t pawnKey = board.PawnKey;
	const int& pawnEntry = data.pawnCorrHist[board.side][pawnKey % CORRHIST_SIZE];


	uint64_t minorKey = board.MinorKey;
	const int& minorEntry = data.minorCorrHist[board.side][minorKey % CORRHIST_SIZE];

	uint64_t whiteNPKey = board.WhiteNonPawnKey;
	const int& whiteNPEntry = data.nonPawnCorrHist[White][board.side][whiteNPKey % CORRHIST_SIZE];
	uint64_t blackNPKey = board.BlackNonPawnKey;
	const int& blackNPEntry = data.nonPawnCorrHist[Black][board.side][blackNPKey % CORRHIST_SIZE];

	const int& contEntry = data.counterMoveCorrHist[prevMove.Piece][prevMove.To];

	int mate_found = 49000 - 99;

	int adjust = 0;

	adjust += pawnEntry * PAWN_CORRHIST_MULTIPLIER;
	adjust += minorEntry * MINOR_CORRHIST_MULTIPLIER;
	adjust += contEntry * COUNTERMOVE_CORRHIST_MULTIPLIER;
	adjust += (whiteNPEntry + blackNPEntry) * NONPAWN_CORRHIST_MULTIPLIER;

	adjust /= 128;

	return std::clamp(rawEval + adjust / CORRHIST_GRAIN, -mate_found + 1, mate_found - 1);
}
void updateHistory(int stm, int from, int to, int bonus, uint64_t opp_threat, ThreadData& data)
{
	int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
	data.mainHistory[stm][from][to][Get_bit(opp_threat, from)][Get_bit(opp_threat, to)] += clampedBonus - data.mainHistory[stm][from][to][Get_bit(opp_threat, from)][Get_bit(opp_threat, to)] * abs(clampedBonus) / MAX_HISTORY;
}
int getSingleContinuationHistoryScore(Move move, const int offSet, ThreadData& data)
{
	if (data.ply >= offSet)
	{
		Move previousMove = data.searchStack[data.ply - offSet].move;

		if (offSet == 1)
		{
			return data.onePlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To];
		}
		else if (offSet == 2)
		{
			return data.twoPlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To];
		}

	}
	return 0;
}
int getContinuationHistoryScore(Move& move, ThreadData& data)
{
	if (data.ply >= 1)
	{
		int onePly = getSingleContinuationHistoryScore(move, 1, data);
		int twoPly = getSingleContinuationHistoryScore(move, 2, data);


		int finalScore = onePly + twoPly;
		return finalScore;
	}
	return 0;
}
void updateSingleContinuationHistoryScore(Move& move, const int bonus, const int offSet, ThreadData& data)
{
	if (data.ply >= offSet) {
		Move previousMove = data.searchStack[data.ply - offSet].move;

		int clampedBonus = std::clamp(bonus, -MAX_CONTHIST, MAX_CONTHIST);
		const int scaledBonus = clampedBonus - getSingleContinuationHistoryScore(move, offSet, data) * abs(clampedBonus) / MAX_CONTHIST;
		//std::cout << scaledBonus;

		if (offSet == 1)
		{
			data.onePlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To] += scaledBonus;
		}
		else if (offSet == 2)
		{
			data.twoPlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To] += scaledBonus;
		}

	}
}
void updateContinuationHistoryScore(Move& move, const int bonus, ThreadData& data)
{
	updateSingleContinuationHistoryScore(move, bonus, 1, data);
	updateSingleContinuationHistoryScore(move, bonus, 2, data);
}
static inline int getMoveScore(Move move, Board& board, TranspositionEntry& entry, uint64_t opp_threat, ThreadData& data)
{

	// Check if the entry is valid and matches the current Zobrist key
	if (entry.bound != 0 && entry.zobristKey == board.zobristKey)
	{
		// If the best move from TT matches the current move, return a high score
		if (entry.bestMove == move)
		{
			//make sure TT move isn't included in search, because it is already searched before generating move
			return 99999999;
		}
	}
	if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{
		int victim;
		if (move.Type == ep_capture)
		{
			victim = P;
		}
		else
		{
			victim = get_piece(board.mailbox[move.To], White);
		}
		int attacker = get_piece(move.Piece, White);
		int score = MVVLVA[attacker][victim];
		//score += CaptureHistory[move.Piece][move.To][board.mailbox[move.To]] / 10;
		score += SEE(board, move, -100) ? 200000 : -10000000;
		return score;
	}
	else
	{
		if (data.killerMoves[0][data.ply] == move)
		{
			return 150000;
		}
		else if (data.killerMoves[1][data.ply] == move)
		{
			return 100000;
		}
		else
		{
			// Return history score for non-capture and non-killer moves
			int mainHistScore = data.mainHistory[board.side][move.From][move.To][Get_bit(opp_threat, move.From)][Get_bit(opp_threat, move.To)];
			int contHistScore = getContinuationHistoryScore(move, data) * 2;
			int historyTotal = mainHistScore + contHistScore - 100000;

			if (historyTotal >= 80000)
			{
				return 80000;
			}
			else
			{
				return historyTotal;
			}
		}
	}

	return 0;
}
static inline int get_move_score_capture(Move move, Board& board)
{
	if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{
		if (move.Type == ep_capture)
		{
			return MVVLVA[P][P] * 10000;
		}
		int victim = get_piece(board.mailbox[move.To], White);
		int attacker = get_piece(move.Piece, White);
		return MVVLVA[attacker][victim] * 10000;
	}
	else
	{
		return -999;
	}
}
bool is_promotion(int type)
{
	return  (type & promotionFlag) != 0;
}
int getPiecePromoting(int type, int side)
{
	int return_piece = 0;
	if ((type == queen_promo) || (type == queen_promo_capture))
	{
		return_piece = Q;
	}
	else if (type == rook_promo || (type == rook_promo_capture))
	{
		return_piece = R;
	}
	else if (type == bishop_promo || (type == bishop_promo_capture))
	{
		return_piece = B;
	}
	else if (type == knight_promo || (type == knight_promo_capture))
	{
		return_piece = N;
	}

	return get_piece(return_piece, side);
}
int move_estimated_value(Board& board, Move move)
{
	// Start with the value of the piece on the target square
	int target_piece = board.mailbox[move.To] > 5
		? board.mailbox[move.To] - 6
		: board.mailbox[move.To];

	int promoted_piece = getPiecePromoting(move.Type, White);
	int value = SEEPieceValues[target_piece];

	// Factor in the new piece's value and remove our promoted pawn
	if ((move.Type & promotionFlag) != 0)
		value += SEEPieceValues[promoted_piece] - SEEPieceValues[P];

	// Target square is encoded as empty for enpass moves
	else if (move.Type == ep_capture)
		value = SEEPieceValues[P];

	// We encode Castle moves as KxR, so the initial step is wrong
	else if (move.Type == king_castle || move.Type == queen_castle)
		value = 0;

	return value;
}

int SEE(Board& pos, Move move, int threshold)
{

	int from, to, enpassant, promotion, colour, balance, nextVictim;
	uint64_t bishops, rooks, occupied, attackers, myAttackers;

	// Unpack move information
	from = move.From;
	to = move.To;
	enpassant = move.Type == ep_capture;
	promotion = (move.Type & promotionFlag) != 0;

	// Next victim is moved piece or promotion type
	nextVictim = promotion ? promotion : pos.mailbox[from];
	nextVictim = nextVictim > 5 ? nextVictim - 6 : nextVictim;

	// Balance is the value of the move minus threshold. Function
	// call takes care for Enpass, Promotion and Castling moves.
	balance = move_estimated_value(pos, move) - threshold;

	// Best case still fails to beat the threshold
	if (balance < 0)
		return 0;

	// Worst case is losing the moved piece
	balance -= SEEPieceValues[nextVictim];

	// If the balance is positive even if losing the moved piece,
	// the exchange is guaranteed to beat the threshold.
	if (balance >= 0)
		return 1;

	// Grab sliders for updating revealed attackers
	bishops = pos.bitboards[b] | pos.bitboards[B] | pos.bitboards[q] |
		pos.bitboards[Q];
	rooks = pos.bitboards[r] | pos.bitboards[R] | pos.bitboards[q] |
		pos.bitboards[Q];

	// Let occupied suppose that the move was actually made
	occupied = pos.occupancies[Both];
	occupied = (occupied ^ (1ull << from)) | (1ull << to);
	if (enpassant)
	{
		int ep_square = (pos.side == White) ? move.To + 8 : move.To - 8;
		occupied ^= (1ull << ep_square);
	}


	// Get all pieces which attack the target square. And with occupied
	// so that we do not let the same piece attack twice
	attackers = all_attackers_to_square(pos, occupied, to) & occupied;

	// Now our opponents turn to recapture
	colour = pos.side ^ 1;

	while (1) {

		// If we have no more attackers left we lose
		myAttackers = attackers & pos.occupancies[colour];
		if (myAttackers == 0ull) {
			break;
		}

		// Find our weakest piece to attack with
		for (nextVictim = P; nextVictim <= Q; nextVictim++) {
			if (myAttackers &
				(pos.bitboards[nextVictim] | pos.bitboards[nextVictim + 6])) {
				break;
			}
		}

		// Remove this attacker from the occupied
		occupied ^=
			(1ull << get_ls1b(myAttackers & (pos.bitboards[nextVictim] |
				pos.bitboards[nextVictim + 6])));

		// A diagonal move may reveal bishop or queen attackers
		if (nextVictim == P || nextVictim == B || nextVictim == Q)
			attackers |= get_bishop_attacks(to, occupied) & bishops;

		// A vertical or horizontal move may reveal rook or queen attackers
		if (nextVictim == R || nextVictim == Q)
			attackers |= get_rook_attacks(to, occupied) & rooks;

		// Make sure we did not add any already used attacks
		attackers &= occupied;

		// Swap the turn
		colour = 1 - colour;

		// Negamax the balance and add the value of the next victim
		balance = -balance - 1 - SEEPieceValues[nextVictim];

		// If the balance is non negative after giving away our piece then we win
		if (balance >= 0) {

			// As a slide speed up for move legality checking, if our last attacking
			// piece is a king, and our opponent still has attackers, then we've
			// lost as the move we followed would be illegal
			if (nextVictim == K && (attackers & pos.occupancies[colour]))
				colour = 1 - colour;

			break;
		}
	}

	// Side to move after the loop loses
	return pos.side != colour;
}

template<typename I, typename C>
constexpr void insertion_sort(I first, I last, C const&& comp) {
	if (first == last) return;
	for (auto i = std::next(first); i != last; ++i) {
		auto k = *i;
		auto j = i;
		while (j > first && comp(k, *(j - 1))) {
			*j = *(j - 1);
			--j;
		}
		*j = k;
	}
}

static inline void sort_moves_captures(MoveList& moveList, Board& board)
{
	// Temporary array for captures
	Move captures[256];
	int captureCount = 0;

	// Separate capture moves
	for (int i = 0; i < moveList.count; ++i)
	{
		if (!isMoveQuiet(moveList.moves[i].Type))
		{
			captures[captureCount++] = moveList.moves[i];
		}
	}

	// Sort only the capture moves
	insertion_sort(captures, captures + captureCount, [&board](const Move& move1, const Move& move2) {
		return get_move_score_capture(move1, board) > get_move_score_capture(move2, board);
		});

	// Copy sorted captures back
	int index = 0;
	for (int i = 0; i < captureCount; ++i)
	{
		moveList.moves[index++] = captures[i];
	}

	// Append non-captures in original order
	for (int i = 0; i < moveList.count; ++i)
	{
		if (isMoveQuiet(moveList.moves[i].Type))
		{
			moveList.moves[index++] = moveList.moves[i];
		}
	}

	moveList.count = index;  // Update move count
}

static inline void sort_moves(MoveList& moveList, Board& board, TranspositionEntry& tt_entry, uint64_t opp_threat, ThreadData& data)
{
	// Precompute scores for all moves
	std::pair<int, Move> scored_moves[256];  // Use fixed-size array
	for (int i = 0; i < moveList.count; ++i)
	{
		scored_moves[i] = { getMoveScore(moveList.moves[i], board, tt_entry, opp_threat, data), moveList.moves[i] };
	}

	// Sort the scored moves based on the scores
	insertion_sort(scored_moves, scored_moves + moveList.count, [](const auto& a, const auto& b)
		{
			return a.first > b.first; // Sort by score (descending)
		});

	// Copy back sorted moves
	for (int i = 0; i < moveList.count; ++i)
	{
		moveList.moves[i] = scored_moves[i].second;
	}
}


inline TranspositionEntry ttLookUp(uint64_t zobrist)
{
	int tt_index = zobrist % TTSize;
	return TranspositionTable[tt_index];
}
inline bool is_in_check(Board& board)
{
	if (is_square_attacked(get_ls1b(board.side == White ? board.bitboards[K] : board.bitboards[k]), 1 - board.side, board, board.occupancies[Both]))
	{
		return true;
	}
	return false;
}
bool is_checking(Board& board)
{
	if (is_square_attacked(get_ls1b(board.side == White ? board.bitboards[k] : board.bitboards[K]), board.side, board, board.occupancies[Both]))
	{
		return true;
	}
	return false;
}
static inline int Quiescence(Board& board, int alpha, int beta, ThreadData& data)
{
	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - data.clockStart).count();
	if (elapsedMS > data.Searchtime_MS || data.searchNodeCount > hardNodeBound)
	{
		data.isSearchStop = true;
		return 0; // Return a neutral score if time is exceeded
	}
	if (data.ply >= 99)
	{
		return Evaluate(board);
	}
	int score = 0;
	TranspositionEntry ttEntry = ttLookUp(board.zobristKey);
	bool is_ttmove_found = false;
	if (ttEntry.zobristKey == board.zobristKey && ttEntry.bound != 0)
	{
		is_ttmove_found = true;
		if ((ttEntry.bound == ExactFlag)
			|| (ttEntry.bound == UpperBound && ttEntry.score <= alpha)
			|| (ttEntry.bound == LowerBound && ttEntry.score >= beta))
		{
			return ttEntry.score;
		}
	}
	int staticEval = Evaluate(board);
	staticEval = adjustEvalWithCorrHist(board, staticEval, data.searchStack[data.ply - 1].move, data);
	bool isInCheck = is_in_check(board);
	int ttAdjustedEval = staticEval;
	uint8_t Bound = ttEntry.bound;

	if (is_ttmove_found && !isInCheck && (Bound == ExactFlag || (Bound == LowerBound && ttEntry.score >= staticEval) || (Bound == UpperBound && ttEntry.score <= staticEval)))
	{
		ttAdjustedEval = ttEntry.score;
	}

	if (ttAdjustedEval >= beta)
	{
		return ttAdjustedEval;
	}
	if (ttAdjustedEval > alpha)
	{
		alpha = ttAdjustedEval;
	}

	MoveList moveList;
	Generate_Legal_Moves(moveList, board, true);

	sort_moves_captures(moveList, board);

	int bestValue = MINUS_INFINITY;
	int legal_moves = 0;
	//PrintBoards(board);

	//int pvNode = beta - alpha > 1;
	//int futilityMargin = evaluation + 120;

	uint64_t lastZobrist = board.zobristKey;
	uint64_t lastPawnKey = board.PawnKey;
	uint64_t lastMinorKey = board.MinorKey;
	uint64_t lastWhiteNPKey = board.WhiteNonPawnKey;
	uint64_t lastBlackNPKey = board.BlackNonPawnKey;
	AccumulatorPair last_accumulator = board.accumulator;
	for (int i = 0; i < moveList.count; ++i)
	{
		Move& move = moveList.moves[i];
		if (isMoveQuiet(move.Type)) continue; //skip non capture moves

		if (!SEE(board, move, 1))
		{
			continue;
		}

		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.lastIrreversiblePly;

		data.ply++;
		if (data.selDepth < data.ply)
		{
			data.selDepth = data.ply;
		}

		MakeMove(board, move);
		data.searchStack[data.ply - 1].move = move;

		if (!isLegal(move, board))//isMoveValid(move, board)
		{
			data.ply--;
			UnmakeMove(board, move, captured_piece);

			board.accumulator = last_accumulator;
			board.history.pop_back();
			board.lastIrreversiblePly = last_irreversible;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			board.zobristKey = lastZobrist;
			board.PawnKey = lastPawnKey;
			board.MinorKey = lastMinorKey;
			board.WhiteNonPawnKey = lastWhiteNPKey;
			board.BlackNonPawnKey = lastBlackNPKey;
			continue;
		}

		data.searchNodeCount++;
		legal_moves++;

		score = -Quiescence(board, -beta, -alpha, data);

		UnmakeMove(board, move, captured_piece);
		board.accumulator = last_accumulator;
		board.history.pop_back();
		board.lastIrreversiblePly = last_irreversible;
		board.enpassent = lastEp;
		board.castle = lastCastle;
		board.side = lastside;
		board.zobristKey = lastZobrist;
		board.PawnKey = lastPawnKey;
		board.MinorKey = lastMinorKey;
		board.WhiteNonPawnKey = lastWhiteNPKey;
		board.BlackNonPawnKey = lastBlackNPKey;

		if (data.isSearchStop) {
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}
		data.ply--;
		if (score > bestValue)
		{
			bestValue = score;
			if (score > alpha)
			{
				alpha = score;
			}
		}
		if (score >= beta)
		{
			return score;
		}
	}
	if (legal_moves == 0) // quiet position
	{
		return ttAdjustedEval;
	}
	if (ttEntry.bound == 0)
	{
		int nodeType = bestValue >= beta ? UpperBound : LowerBound;
		ttEntry.score = bestValue;
		ttEntry.bound = nodeType;
		ttEntry.depth = -1;
		ttEntry.zobristKey = board.zobristKey;
		TranspositionTable[board.zobristKey % TTSize] = ttEntry;
	}
	return bestValue;
}


static inline int Negamax(Board& board, int depth, int alpha, int beta, bool doNMP, bool cutnode, ThreadData& data, Move excludedMove = NULLMOVE)
{
	bool isPvNode = beta - alpha > 1;


	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - data.clockStart).count();
	if (elapsedMS > data.Searchtime_MS || data.searchNodeCount > hardNodeBound) {
		data.isSearchStop = true;
		return 0; // Return a neutral score if time is exceeded
	}

	pvLengths[data.ply] = data.ply;

	//Return draw score if threefold repetition has occured
	if (data.ply != 0 && is_threefold(board.history, board.lastIrreversiblePly))
	{
		return 0;
	}
	//Immediately return the static evaluation score when we reach max ply
	if (data.ply > 99 - 1)
	{
		return Evaluate(board);
	}

	//Probe TT entry
	TranspositionEntry ttEntry = ttLookUp(board.zobristKey);

	int score = 0;
	int bestValue = MINUS_INFINITY;

	//Default assumption before search
	int ttFlag = UpperBound;

	bool is_ttmove_found = false;
	bool isSingularSearch = excludedMove != NULLMOVE;

	bool tt_pv = isPvNode;
	//Checks for collisions, or empty entry
	if (ttEntry.zobristKey == board.zobristKey && ttEntry.bound != 0)
	{
		is_ttmove_found = true;
		// Valid TT entry found
		if (!isPvNode && !isSingularSearch && data.ply != 0 && ttEntry.depth >= depth)
		{
			// Return immediately if exact score is found
			if (ttEntry.bound == ExactFlag)
			{
				return ttEntry.score;
			}
			else if (ttEntry.bound == LowerBound && ttEntry.score >= beta)
			{
				return ttEntry.score;
			}
			else if (ttEntry.bound == UpperBound && ttEntry.score <= alpha)
			{
				return ttEntry.score;
			}
		}
	}
	//Internal Iterative Reduction
	//If no hash move was found, reduce depth
	if (!isSingularSearch && depth >= 4 && (isPvNode || cutnode) && (!is_ttmove_found || ttEntry.depth < depth - 4))
	{
		depth--;
	}

	//checks if the node has been in a pv node in the past
	tt_pv |= ttEntry.ttPv;
	bool isInCheck = is_in_check(board);

	if (isInCheck)
	{
		//Increase search depth if the side to move is in check, as such positions often involve tactic
		depth = std::max(depth + 1, 1);
	}
	if (depth <= 0)
	{
		//Enter quiescence search to evaluate only "quiet" positions and avoid horizon effects
		return Quiescence(board, alpha, beta, data);
	}
	if (data.ply + 1 <= 99)
	{
		// Reset killer moves for the next ply to make the killer move more local
		data.killerMoves[0][data.ply + 1] = Move(0, 0, 0, 0);
		data.killerMoves[1][data.ply + 1] = Move(0, 0, 0, 0);
	}

	int rawEval = Evaluate(board);
	int staticEval = adjustEvalWithCorrHist(board, rawEval, data.searchStack[data.ply - 1].move, data);
	int ttAdjustedEval = staticEval;

	//Adjust static evaluation with search score on TT, to get move accurate estimaton.
	if (!isSingularSearch && is_ttmove_found && !isInCheck && (ttEntry.bound == ExactFlag || (ttEntry.bound == LowerBound && ttEntry.score >= staticEval) || (ttEntry.bound == UpperBound && ttEntry.score <= staticEval)))
	{
		ttAdjustedEval = ttEntry.score;
	}

	data.searchStack[data.ply].staticEval = staticEval;

	//If current static evaluation is greater than static evaluation from 2 plies ago
	bool improving = !isInCheck && data.ply > 1 && staticEval > data.searchStack[data.ply - 2].staticEval;

	int canPrune = !isInCheck && !isPvNode;
	//RFP
	//If static evaluation + margin still doesn't improve alpha, prune the node
	if (!isSingularSearch && depth < 5 && canPrune)
	{
		int rfpMargin;
		if (improving)
		{
			//Do more RFP when we are improving
			rfpMargin = RFP_IMPROVING_BASE + RFP_IMPROVING_MULTIPLIER * depth;
		}
		else
		{
			rfpMargin = RFP_BASE + RFP_MULTIPLIER * depth;
		}
		int rfpThreshold = rfpMargin;

		if (ttAdjustedEval - rfpThreshold >= beta)
		{
			return (ttAdjustedEval + beta) / 2;
		}
	}
	if (depth <= 3 && ttAdjustedEval + 200 * depth <= alpha)
	{
		int razor_score = Quiescence(board, alpha, alpha + 1, data);
		if (razor_score <= alpha)
		{
			return razor_score;
		}
	}
	//NMP
	//Since null move is worse than all the other moves in most situations,
	//if a reduced search on null move fails high over beta, return fail high score
	if (!isSingularSearch && !isPvNode && doNMP)
	{
		if (!isInCheck && depth >= 2 && data.ply && ttAdjustedEval >= beta)
		{
			if ((board.occupancies[Both] & ~(board.bitboards[P] | board.bitboards[p] | board.bitboards[K] | board.bitboards[k])) != 0ULL)
			{
				int lastep = board.enpassent;
				uint64_t lzob = board.zobristKey;
				data.ply++;
				Make_Nullmove(board);
				int R = 3 + depth / NMP_DEPTH_DIVISER;
				R += std::min((ttAdjustedEval - beta) / NMP_EVAL_DIVISER, MAX_NMP_EVAL_R);
				int score = -Negamax(board, depth - R, -beta, -beta + 1, false, !cutnode, data);

				Unmake_Nullmove(board);
				data.ply--;
				board.enpassent = lastep;
				board.zobristKey = lzob;

				if (score >= beta)
				{
					return score > 49000 ? beta : score;
				}
			}
		}
	}
	MoveList moveList;
	Generate_Legal_Moves(moveList, board, false);
	int searchedMoves = 0;

	//Calculate all squares opponent is controlling
	uint64_t oppThreats = get_attacked_squares(1 - board.side, board, board.occupancies[Both]);

	//Sort moves from best to worst(by approximation)
	sort_moves(moveList, board, ttEntry, oppThreats, data);

	int orgAlpha = alpha;
	int depthToSearch;
	bool skipQuiets = false;

	depth = std::min(depth, 98);

	int lmpThreshold = (LMP_BASE + (LMP_MULTIPLIER)*depth * depth) / (2 - improving);
	int quietSEEMargin = PVS_QUIET_BASE - PVS_QUIET_MULTIPLIER * depth;
	int noisySEEMargin = PVS_NOISY_BASE - PVS_NOISY_MULTIPLIER * depth * depth;
	int historyPruningMargin = HISTORY_PRUNING_BASE - HISTORY_PRUNING_MULTIPLIER * depth;

	MoveList quietsList;

	Move bestMove = Move(0, 0, 0, 0);
	int quietMoves = 0;

	uint64_t last_zobrist = board.zobristKey;
	uint64_t last_pawnKey = board.PawnKey;
	uint64_t last_minorKey = board.MinorKey;
	uint64_t last_whitenpKey = board.WhiteNonPawnKey;
	uint64_t last_blacknpKey = board.BlackNonPawnKey;
	AccumulatorPair last_accumulator = board.accumulator;
	for (int i = 0; i < moveList.count; ++i)
	{
		Move& move = moveList.moves[i];
		bool isQuiet = isMoveQuiet(move.Type);
		if (move == excludedMove)
		{
			//Skip search with an excluded move
			continue;
		}
		if (skipQuiets && isQuiet) //quiet move
		{
			continue;
		}
		int seeThreshold = isQuiet ? quietSEEMargin : noisySEEMargin;
		if (data.ply != 0 && depth <= MAX_PVS_SEE_DEPTH)
		{
			//if Static Exchange Evaluation score is lower than certain margin, assume the move is very bad and skip the move
			if (!SEE(board, move, seeThreshold))
			{
				continue;
			}
		}
		bool isNotMated = alpha > -49000 + 99;

		int main_history = data.mainHistory[board.side][move.From][move.To][Get_bit(oppThreats, move.From)][Get_bit(oppThreats, move.To)];
		int conthist = getContinuationHistoryScore(move, data) * 2;
		int historyScore = main_history + conthist;
		if (data.ply != 0 && isQuiet && isNotMated)
		{
			//Skip late moves, as good moves are typically found among the earlier moves due to move ordering
			if (searchedMoves >= lmpThreshold)
			{
				skipQuiets = true;
			}
			//If history score is very bad, skip the move
			if (quietMoves > 1 && depth <= 5 && historyScore < historyPruningMargin)
			{
				continue;
			}
		}
		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.lastIrreversiblePly;
		data.ply++;

		if (data.selDepth < data.ply)
		{
			data.selDepth = data.ply;
		}
		MakeMove(board, move);

		data.searchNodeCount++;
		if (!isLegal(move, board))
		{
			data.ply--;
			UnmakeMove(board, move, captured_piece);
			board.accumulator = last_accumulator;
			board.history.pop_back();
			board.lastIrreversiblePly = last_irreversible;
			board.zobristKey = last_zobrist;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			board.PawnKey = last_pawnKey;
			board.MinorKey = last_minorKey;
			board.WhiteNonPawnKey = last_whitenpKey;
			board.BlackNonPawnKey = last_blacknpKey;
			continue;
		}

		if (isQuiet)
		{
			quietsList.add(move);
			quietMoves++;
		}
		searchedMoves++;
		depthToSearch = depth - 1;

		int reduction = 0;
		bool is_reduced = false;
		int extensions = 0;

		//Singular Extension
		//If we have a TT move, we try to verify if it's the only good move. if the move is singular, search the move with increased depth
		if (data.ply > 1 && depth >= 7 && move == ttEntry.bestMove && excludedMove == NULLMOVE && ttEntry.depth >= depth - 3 && ttEntry.bound != UpperBound && std::abs(ttEntry.score) < 50000)
		{
			data.ply--;
			UnmakeMove(board, move, captured_piece);
			board.accumulator = last_accumulator;
			board.history.pop_back();
			board.lastIrreversiblePly = last_irreversible;
			board.zobristKey = last_zobrist;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			board.PawnKey = last_pawnKey;
			board.MinorKey = last_minorKey;
			board.WhiteNonPawnKey = last_whitenpKey;
			board.BlackNonPawnKey = last_blacknpKey;

			int s_beta = ttEntry.score - depth * 2;
			int s_depth = (depth - 1) / 2;
			int s_score = Negamax(board, s_depth, s_beta - 1, s_beta, true, cutnode, data, move);
			if (s_score < s_beta)
			{
				extensions++;
				//Double Extensions
				//TT move is very singular, increase depth by 2
				if (!isPvNode && s_score <= s_beta - DEXT_MARGIN)
				{
					extensions++;
				}
			}
			else if (s_beta >= beta)
			{
				return s_beta;
			}
			else if (ttEntry.score >= beta)
			{
				extensions--;
			}

			MakeMove(board, move);
			data.ply++;
		}
		data.searchStack[data.ply - 1].move = move;
		if (depth > MIN_LMR_DEPTH && searchedMoves > 1)
		{
			//LMR
			//Save search by reducing moves that are ordered closer to the end
			reduction = lmrTable[depth][searchedMoves];
			//reduce more if we are not in pv node
			if (!isPvNode && quietMoves >= 4)
			{
				reduction++;
			}
			//reduce less if we the move is giving check
			if (is_in_check(board))
			{
				reduction--;
			}
			//reduce less if the position is improving
			if (improving)
			{
				reduction--;
			}
			//reduce more if the history score is bad
			if (historyScore < (-HISTORY_LMR_MULTIPLIER * depth) + HISTORY_LMR_BASE)
			{
				reduction++;
			}
			//reduce less if the move is a capture
			if (!isQuiet)
			{
				reduction--;
			}
			//reduce less killer moves
			if ((move == data.killerMoves[0][data.ply - 1]) || (move == data.killerMoves[1][data.ply - 1]))
			{
				reduction--;
			}
			//if the node has been in a pv node in the past, reduce less
			if (tt_pv)
			{
				reduction--;
			}
			//reduce more cutnode
			if (cutnode)
			{
				reduction++;
			}
		}
		//Prevent from accidently extending the move
		if (reduction < 0) reduction = 0;
		is_reduced = reduction > 0;
		bool isChildCutNode;
		if (searchedMoves <= 1)
		{
			if (isPvNode)
			{
				isChildCutNode = false;
			}
			else
			{
				isChildCutNode = !cutnode;
			}
			score = -Negamax(board, depthToSearch + extensions, -beta, -alpha, true, isChildCutNode, data);
		}
		else
		{
			if (is_reduced)
			{
				score = -Negamax(board, depthToSearch - reduction, -alpha - 1, -alpha, true, true, data);
			}
			else
			{
				score = alpha + 1;
			}
			if (score > alpha)
			{
				if (score != alpha + 1)
				{
					depthToSearch += (score > (bestValue + 60 + depthToSearch * 2)); 
					depthToSearch -= (score < bestValue + depthToSearch && data.ply != 1);
				}
				score = -Negamax(board, depthToSearch, -alpha - 1, -alpha, true, !cutnode, data);
			}
			if (score > alpha && score < beta)
			{
				score = -Negamax(board, depthToSearch, -beta, -alpha, true, false, data);
			}

		}
		board.accumulator = last_accumulator;
		UnmakeMove(board, move, captured_piece);
		board.history.pop_back();
		board.lastIrreversiblePly = last_irreversible;
		board.zobristKey = last_zobrist;
		board.enpassent = lastEp;
		board.castle = lastCastle;
		board.side = lastside;
		board.PawnKey = last_pawnKey;
		board.MinorKey = last_minorKey;
		board.WhiteNonPawnKey = last_whitenpKey;
		board.BlackNonPawnKey = last_blacknpKey;
		data.ply--;

		if (data.isSearchStop) {
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}

		bestValue = std::max(score, bestValue);
		if (bestValue > alpha)
		{
			ttFlag = ExactFlag;
			alpha = score;
			bestMove = move;
			if (isPvNode)
			{
				pvTable[data.ply][data.ply] = move;
				for (int next_ply = data.ply + 1; next_ply < pvLengths[data.ply + 1]; next_ply++)
				{
					pvTable[data.ply][next_ply] = pvTable[data.ply + 1][next_ply];
				}
				pvLengths[data.ply] = pvLengths[data.ply + 1];
			}
		}
		if (alpha >= beta)
		{
			ttFlag = LowerBound;
			if ((move.Type & capture) == 0)
			{
				if (!(data.killerMoves[0][data.ply] == move))
				{
					data.killerMoves[1][data.ply] = data.killerMoves[0][data.ply];
					data.killerMoves[0][data.ply] = move;
				}
				int mainHistBonus = std::min(2400, HISTORY_BASE + HISTORY_MULTIPLIER * depth * depth);
				int contHistBonus = std::min(2400, CONTHIST_BASE + CONTHIST_MULTIPLIER * depth * depth);
				for (int i = 0; i < quietsList.count; ++i)
				{
					Move& move_quiet = quietsList.moves[i];
					if (move_quiet == move)
					{
						updateHistory(board.side, move_quiet.From, move_quiet.To, mainHistBonus, oppThreats, data);
						if (data.ply >= 1)
						{
							updateContinuationHistoryScore(move_quiet, contHistBonus, data);
						}
					}
					else
					{
						updateHistory(board.side, move_quiet.From, move_quiet.To, -mainHistBonus, oppThreats, data);
						if (data.ply >= 1)
						{
							updateContinuationHistoryScore(move_quiet, -contHistBonus, data);
						}
					}
				}
			}
			break;
		}
	}
	if (searchedMoves == 0)
	{
		if (is_square_attacked(get_ls1b(board.side == White ? board.bitboards[K] : board.bitboards[k]), 1 - board.side, board, board.occupancies[Both]))
		{
			return -49000 + data.ply;
		}
		else
		{
			return isSingularSearch ? alpha : 0;
		}
	}
	if (!isSingularSearch)
	{
		if (ttFlag == UpperBound && is_ttmove_found)
		{
			bestMove = ttEntry.bestMove;
		}
	}
	ttEntry.score = bestValue;
	ttEntry.bound = ttFlag;
	ttEntry.depth = depth;
	ttEntry.zobristKey = board.zobristKey;
	ttEntry.ttPv = tt_pv;
	ttEntry.bestMove = bestMove;

	int bound = bestValue >= beta ? HFLOWER : alpha != orgAlpha ? HFEXACT : HFUPPER;
	if (!isSingularSearch && !is_in_check(board) && (bestMove == Move(0, 0, 0, 0) || isMoveQuiet(bestMove.Type)) && !(bound == HFLOWER && bestValue <= staticEval) && !(bound == HFUPPER && bestValue >= staticEval))
	{
		//Save difference between static eval and search score, to get more accurate static eval in the future
		updatePawnCorrHist(board, depth, bestValue - staticEval, data);
		updateMinorCorrHist(board, depth, bestValue - staticEval, data);
		updateNonPawnCorrHist(board, depth, bestValue - staticEval, data);
		updateCounterCorrHist(data.searchStack[data.ply - 1].move, depth, bestValue - staticEval, data);
	}
	if (!isSingularSearch)
	{
		TranspositionTable[board.zobristKey % TTSize] = ttEntry;
	}
	return bestValue;
}
int get_hashfull()
{
	int entryCount = 0;
	for (int i = 0; i < 1000; i++)
	{
		if (TranspositionTable[i].bound != 0)
		{
			entryCount++;
		}
	}
	return entryCount;
}

void bench()
{
	ThreadData* heapAllocated = new ThreadData(); //Allocate safely on heap
	ThreadData& data = *heapAllocated;
	auto search_start = std::chrono::steady_clock::now();
	auto search_end = std::chrono::steady_clock::now();
	Board board;
	uint64_t nodecount = 0;
	int totalsearchtime = 0;
	SearchLimitations searchLimits;
	for (int i = 0; i < 50; i++)
	{
		for (int ply = 0; ply < 99; ply++)
		{
			data.killerMoves[0][ply] = Move();
			data.killerMoves[1][ply] = Move();
		}
		memset(data.mainHistory, 0, sizeof(data.mainHistory));
		for (size_t i = 0; i < TTSize; i++)
		{
			TranspositionTable[i] = TranspositionEntry();
		}
		memset(data.onePlyContHist, 0, sizeof(data.onePlyContHist));
		memset(data.twoPlyContHist, 0, sizeof(data.twoPlyContHist));
		memset(data.CaptureHistory, 0, sizeof(data.CaptureHistory));
		memset(data.pawnCorrHist, 0, sizeof(data.pawnCorrHist));
		memset(data.minorCorrHist, 0, sizeof(data.minorCorrHist));
		memset(data.counterMoveCorrHist, 0, sizeof(data.counterMoveCorrHist));

		parse_fen(benchFens[i], board);
		board.zobristKey = generate_hash_key(board);
		board.history.push_back(board.zobristKey);

		search_start = std::chrono::steady_clock::now();
		IterativeDeepening(board, 11, searchLimits, data, false);
		search_end = std::chrono::steady_clock::now();

		float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(search_end - search_start).count();

		nodecount += data.searchNodeCount;
		totalsearchtime += std::floor(elapsedMS);
	}
	std::cout << nodecount << " nodes " << nodecount / (totalsearchtime + 1) * 1000 << " nps " << "\n";
	for (int ply = 0; ply < 99; ply++)
	{
		data.killerMoves[0][ply] = Move();
		data.killerMoves[1][ply] = Move();
	}
	memset(data.mainHistory, 0, sizeof(data.mainHistory));
	for (size_t i = 0; i < TTSize; i++)
	{
		TranspositionTable[i] = TranspositionEntry();
	}
	memset(data.onePlyContHist, 0, sizeof(data.onePlyContHist));
	memset(data.CaptureHistory, 0, sizeof(data.CaptureHistory));
	memset(data.pawnCorrHist, 0, sizeof(data.pawnCorrHist));
	memset(data.minorCorrHist, 0, sizeof(data.minorCorrHist));
	memset(data.counterMoveCorrHist, 0, sizeof(data.counterMoveCorrHist));


	delete heapAllocated;
}
void setColor([[maybe_unused]] ConsoleColor color)
{
#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
#endif
}
int countDecimalPlaces(float number)
{
	// Round to a maximum of 2 decimal places
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << number; // Round to 2 decimals

	// Get the string representation of the number
	std::string str = ss.str();

	// Find the position of the decimal point
	size_t pos = str.find('.');

	// If there is no decimal point, return 0 (no decimal places)
	if (pos == std::string::npos) {
		return 0;
	}

	// Count the number of digits after the decimal point
	return str.length() - pos - 1;
}
void print_UCI(Move(&PV_line)[99][99], Move& bestmove, int score, float elapsedMS, float nps, ThreadData& data)
{
	bestmove = pvTable[0][0];
	int hashfull = get_hashfull();
	lastBestMoves[data.currDepth - 1] = bestmove;
	std::cout << "info depth " << data.currDepth;
	std::cout << " seldepth " << data.selDepth;
	if (std::abs(score) > 40000)
	{
		int mate_ply = 49000 - std::abs(score);
		int mate_fullmove = std::ceil(static_cast<double>(mate_ply) / 2);
		if (score < 0)
		{
			mate_fullmove *= -1;
		}
		std::cout << " score mate " << mate_fullmove;
	}
	else
	{
		std::cout << " score cp " << score;
	}

	std::cout << " time " << static_cast<int>(std::round(elapsedMS)) << " nodes " << data.searchNodeCount << " nps " << static_cast<int>(std::round(nps)) << " hashfull " << hashfull << " pv " << std::flush;
	for (int count = 0; count < pvLengths[0]; count++)
	{
		printMove(pvTable[0][count]);
		std::cout << " ";
	}
	std::cout << "\n";
}
void print_Pretty(Move(&PV_line)[99][99], Move& bestmove, int score, float elapsedMS, float nps, int window_change, int asp_alpha, int asp_beta, ThreadData data)
{
	setColor(ConsoleColor::White);
	std::cout << "depth ";
	setColor(ConsoleColor::BrightBlue);
	if (data.currDepth < 10)
	{
		std::cout << " ";
	}

	std::cout << data.currDepth;
	setColor(ConsoleColor::White);
	std::cout << " / ";
	setColor(ConsoleColor::BrightBlue);


	if (data.selDepth < 10)
	{
		std::cout << " ";
	}
	std::cout << data.selDepth;

	setColor(ConsoleColor::White);
	std::cout << " TT ";
	setColor(ConsoleColor::Green);
	int hashful = get_hashfull() / 10;
	if (hashful < 10)
	{
		std::cout << "  ";
	}
	else if (hashful < 100)
	{
		std::cout << " ";
	}
	std::cout << hashful;
	std::cout << "% ";

	setColor(ConsoleColor::Blue);
	std::cout << " ";
	int time = std::round((elapsedMS / 1000));
	if (time < 10)
	{
		std::cout << "   ";
	}
	else if (time < 100)
	{
		std::cout << "  ";
	}
	else if (time < 1000)
	{
		std::cout << " ";
	}
	std::cout << time;
	setColor(ConsoleColor::Gray);
	std::cout << "S ";

	setColor(ConsoleColor::BrightGreen);
	float nps_in_M = (std::round((nps / 1000000) * 100)) / 100;
	std::cout << nps_in_M;

	setColor(ConsoleColor::Gray);
	std::cout << "MN/S";
	if (std::abs(score) > 40000)
	{
		int mate_ply = 49000 - std::abs(score);
		int mate_fullmove = std::ceil(static_cast<double>(mate_ply) / 2);
		setColor(ConsoleColor::Green);
		if (score < 0)
		{
			setColor(ConsoleColor::Red);
			mate_fullmove *= -1;
		}

		std::cout << " Mate in " << mate_fullmove;
		std::cout << " ";
	}
	else
	{
		if (std::abs(score) < 50)
		{
			setColor(ConsoleColor::White);
		}
		else if (score < 0)//score <= -50
		{
			if (score <= -500)
			{
				setColor(ConsoleColor::Red);
			}
			else if (score <= -100)
			{
				setColor(ConsoleColor::BrightRed);
			}
			else
			{
				setColor(ConsoleColor::BrightYellow);
			}

		}
		else
		{
			if (score >= 500)
			{
				setColor(ConsoleColor::Green);
			}
			else if (score >= 100)
			{
				setColor(ConsoleColor::BrightGreen);
			}
			else
			{
				setColor(ConsoleColor::BrightBlue);
			}
		}
		float score_fullPawn = (std::round((static_cast<float>(score) / 100) * 100)) / 100;
		int abs_score = std::abs(std::round(score_fullPawn));
		if (abs_score < 10)
		{
			std::cout << "  ";
		}
		else if (abs_score < 100)
		{
			std::cout << " ";
		}
		else if (abs_score < 1000)
		{
			std::cout << "";
		}
		if (score > 0)
		{
			std::cout << "+";
		}
		if (score == 0)
		{
			std::cout << "+";
		}
		std::cout << std::fixed << std::setprecision(2) << score_fullPawn;
	}

	setColor(ConsoleColor::Gray);
	std::cout << " pv ";
	for (int count = 0; count < pvLengths[0]; count++)
	{
		if (count <= 2)
		{
			setColor(ConsoleColor::White);
		}
		else
		{
			setColor(ConsoleColor::Gray);
		}
		if (count >= 10) break;
		printMove(pvTable[0][count]);
		std::cout << " ";
	}

	setColor(ConsoleColor::White);
	std::cout << "\n";
}
void scaleTime(int64_t& softLimit, uint8_t bestMoveStability, int64_t baseSoft, int64_t maxTime) {
	int bestMoveScale[5] = { 2430, 1350, 1090, 880, 680 };

	softLimit = std::min((baseSoft * bestMoveScale[bestMoveStability]) / 1000, maxTime);
}
std::pair<Move, int> IterativeDeepening(Board& board, int depth, SearchLimitations& searchLimits, ThreadData& data, bool print_info, int64_t maxTime)
{
	//std::cout << "id"<<std::flush;

	if (searchLimits.HardNodeLimit == NOLIMIT)
	{
		hardNodeBound = std::numeric_limits<int64_t>::max();
	}
	else
	{
		hardNodeBound = searchLimits.HardNodeLimit;
	}

	if (searchLimits.HardTimeLimit == NOLIMIT)
	{
		data.Searchtime_MS = std::numeric_limits<int64_t>::max();
	}
	else
	{
		data.Searchtime_MS = searchLimits.HardTimeLimit;
	}

	data.searchNodeCount = 0;
	Move bestmove;
	data.clockStart = std::chrono::steady_clock::now();

	int score = 0;
	int bestScore = 0;

	memset(pvTable, 0, sizeof(pvTable));
	memset(pvLengths, 0, sizeof(pvLengths));

	int bestMoveStability = 0;
	int64_t baseSoft = searchLimits.SoftTimeLimit;

	int64_t softLimit = searchLimits.SoftTimeLimit;
	for (data.currDepth = 1; data.currDepth <= depth; data.currDepth++)
	{
		data.ply = 0;
		data.selDepth = 0;
		data.isSearchStop = false;
		for (int i = 0; i < 99; i++)
		{
			data.searchStack[i].move = Move(0, 0, 0, 0);
		}

		int delta = ASP_WINDOW_INITIAL;
		int alpha_val = std::max(MINUS_INFINITY, score - delta);
		int beta_val = std::min(PLUS_INFINITY, score + delta);
		int window_change = 1;

		int aspirationWindowDepth = data.currDepth;
		//std::cout << alpha_val << ","<<beta_val;
		while (true)
		{
			auto end = std::chrono::steady_clock::now();
			int64_t MS = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - data.clockStart).count());
			if (softLimit != NOLIMIT)
			{
				if (MS > softLimit)
				{
					break;
				}

			}
			else
			{
				if (searchLimits.HardTimeLimit != NOLIMIT && MS > searchLimits.HardTimeLimit)
				{
					break;
				}
			}

			score = Negamax(board, std::max(aspirationWindowDepth, 1), alpha_val, beta_val, true, false, data);

			delta += delta;
			if (score <= alpha_val)
			{
				alpha_val = std::max(MINUS_INFINITY, score - delta);
				aspirationWindowDepth = data.currDepth;
			}
			else if (score >= beta_val)
			{
				beta_val = std::min(PLUS_INFINITY, score + delta);
				aspirationWindowDepth = std::max(aspirationWindowDepth - 1, data.currDepth - 5);
			}
			else
			{
				break;
			}



			if (delta >= ASP_WINDOW_MAX)
			{
				delta = PLUS_INFINITY;
			}
			window_change++;
		}


		auto end = std::chrono::steady_clock::now();
		float elapsedMS = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - data.clockStart).count());
		float second = (elapsedMS + 1) / 1000;

		double nps = data.searchNodeCount / second;

		if (pvTable[0][0] == bestmove)
		{
			bestMoveStability = std::min(bestMoveStability + 1, 4);
		}
		else
		{
			bestMoveStability = 0;
		}
		if (data.currDepth >= 6 && searchLimits.SoftTimeLimit != -1 && searchLimits.HardTimeLimit != -1)
		{
			scaleTime(softLimit, bestMoveStability, baseSoft, maxTime);
		}
		if (print_info)
		{
			if (!data.isSearchStop)
			{
				if (isPrettyPrinting && isOnWindow)
				{
					print_Pretty(pvTable, bestmove, score, elapsedMS, nps, window_change, alpha_val, beta_val, data);
				}
				else
				{
					print_UCI(pvTable, bestmove, score, elapsedMS, nps, data);
				}
			}
		}
		if (searchLimits.SoftNodeLimit != NOLIMIT)
		{
			if (data.searchNodeCount > searchLimits.SoftNodeLimit)
			{
				break;
			}
		}
		if (softLimit != NOLIMIT)
		{
			if (elapsedMS > softLimit)
			{
				break;
			}
		}
		else
		{
			if (searchLimits.HardTimeLimit != NOLIMIT && elapsedMS > searchLimits.HardTimeLimit)
			{
				break;
			}
		}
		bestmove = pvTable[0][0];
		bestScore = score;
	}
	if (print_info)
	{
		std::cout << "bestmove ";
		printMove(bestmove);
		std::cout << "\n";
	}
	return std::pair<Move, int>(bestmove, bestScore);
}
inline void Initialize_TT(int size)
{
	uint64_t bytes = static_cast<uint64_t>(size) * 1024ULL * 1024ULL;

	TTSize = bytes / sizeof(TranspositionEntry);

	if (TTSize % 2 != 0)
	{
		TTSize -= 1;
	}

	if (TranspositionTable)
		delete[] TranspositionTable;

	TranspositionTable = new TranspositionEntry[TTSize]();
}
int randBool() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<int> dist(0, 1);
	return dist(gen);
}
int randBetween(int n) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, n); // Range 1 to n
	return dist(gen);
}


// Function to get the castling rights part of the FEN
std::string getCastlingRights(const Board& board) {
	std::string rights = "";
	if (board.castle & WhiteKingCastle) rights += "K"; // White kingside castling
	if (board.castle & WhiteQueenCastle) rights += "Q"; // White queenside castling
	if (board.castle & BlackKingCastle) rights += "k"; // Black kingside castling
	if (board.castle & BlackQueenCastle) rights += "q"; // Black queenside castling
	return (rights.empty()) ? "-" : rights;
}
std::string boardToFEN(const Board& board) {
	// Step 1: Construct the piece placement part (ranks 8 to 1)
	std::string piecePlacement = "";
	for (int rank = 7; rank >= 0; --rank) {  // Fix: Iterate from 7 down to 0
		int emptyCount = 0;
		for (int file = 0; file < 8; ++file) {
			int square = (7 - rank) * 8 + file;
			int piece = board.mailbox[square];

			if (piece == NO_PIECE) {
				++emptyCount; // Empty square
			}
			else {
				if (emptyCount > 0) {
					piecePlacement += std::to_string(emptyCount); // Insert the number of empty squares
					emptyCount = 0;
				}
				piecePlacement += getCharFromPiece(piece); // Add the piece (e.g., 'p', 'R')
			}
		}

		if (emptyCount > 0) {
			piecePlacement += std::to_string(emptyCount); // End of rank with empty squares
		}

		if (rank > 0) {  // Fix: Only add '/' if it's not the last rank (rank 1)
			piecePlacement += "/";
		}
	}

	// Step 2: Construct the other parts of FEN
	std::string sideToMove = (board.side == 0) ? "w" : "b"; // White or Black to move
	std::string castlingRights = getCastlingRights(board); // Castling rights
	std::string enPassant = (board.enpassent == NO_SQ) ? "-" : CoordinatesToChessNotation(board.enpassent); // En passant square
	std::string halfmove = std::to_string(board.history.size() - board.lastIrreversiblePly); // Halfmove clock
	std::string fullmove = std::to_string(board.history.size() / 2 + 1); // Fullmove number

	// Step 3: Combine all parts into the final FEN string
	std::string fen = piecePlacement + " " + sideToMove + " " + castlingRights + " " + enPassant + " " + halfmove + " " + fullmove;

	return fen;
}

bool isNoLegalMoves(Board board, MoveList& moveList)
{
	int searchedMoves = 0;

	uint64_t last_zobrist = board.zobristKey;
	uint64_t last_pawnKey = board.PawnKey;
	uint64_t last_minorKey = board.MinorKey;
	uint64_t last_whitenpKey = board.WhiteNonPawnKey;
	uint64_t last_blacknpKey = board.BlackNonPawnKey;
	for (int i = 0; i < moveList.count; ++i)
	{
		Move& move = moveList.moves[i];
		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.lastIrreversiblePly;

		MakeMove(board, move);
		if (!isLegal(move, board))
		{
			UnmakeMove(board, move, captured_piece);
			board.history.pop_back();
			board.lastIrreversiblePly = last_irreversible;
			board.zobristKey = last_zobrist;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			board.PawnKey = last_pawnKey;
			board.MinorKey = last_minorKey;
			board.WhiteNonPawnKey = last_whitenpKey;
			board.BlackNonPawnKey = last_blacknpKey;
			continue;
		}

		searchedMoves++;
		UnmakeMove(board, move, captured_piece);
		board.history.pop_back();
		board.lastIrreversiblePly = last_irreversible;
		board.zobristKey = last_zobrist;
		board.enpassent = lastEp;
		board.castle = lastCastle;
		board.side = lastside;
		board.PawnKey = last_pawnKey;
		board.MinorKey = last_minorKey;
		board.WhiteNonPawnKey = last_whitenpKey;
		board.BlackNonPawnKey = last_blacknpKey;
	}
	if (searchedMoves == 0)
	{
		return true;
	}
	return false;

}

void PickRandomPos(Board& board, ThreadData& data)
{
	const std::string start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
randomPos:int randomMovesNum = 8 + randBool();
	board.history.clear();
	parse_fen(start_pos, board);
	board.zobristKey = generate_hash_key(board);
	board.history.push_back(board.zobristKey);
	initializeLMRTable(data);
	Initialize_TT(16);
	//std::cout << randomMovesNum;
	for (int i = 0; i < randomMovesNum; i++)
	{
		MoveList moveList;
		moveList.count = 0;
		Generate_Legal_Moves(moveList, board, false);
		//goto randomPos;
		if (moveList.count == 0 || isNoLegalMoves(board, moveList))//game is already over, restart the pos generation
		{
			goto randomPos;
		}
		int randomN;
		while (true)
		{
			randomN = randBetween(moveList.count) - 1;
			Move move = moveList.moves[randomN];
			uint64_t last_zobrist = board.zobristKey;
			uint64_t last_pawnKey = board.PawnKey;
			uint64_t last_minorKey = board.MinorKey;
			uint64_t last_whitenpKey = board.WhiteNonPawnKey;
			uint64_t last_blacknpKey = board.BlackNonPawnKey;
			int lastEp = board.enpassent;
			uint64_t lastCastle = board.castle;
			int lastside = board.side;
			int captured_piece = board.mailbox[move.To];
			int last_irreversible = board.lastIrreversiblePly;
			MakeMove(board, move);
			if (((move.Type & captureFlag) != 0) || move.Piece == p || move.Piece == P)
			{
				board.lastIrreversiblePly = board.history.size();
			}
			if (isLegal(move, board))
			{
				break;
			}
			else
			{
				UnmakeMove(board, move, captured_piece);
				board.history.pop_back();
				board.lastIrreversiblePly = last_irreversible;
				board.zobristKey = last_zobrist;
				board.enpassent = lastEp;
				board.castle = lastCastle;
				board.side = lastside;
				board.PawnKey = last_pawnKey;
				board.MinorKey = last_minorKey;
				board.WhiteNonPawnKey = last_whitenpKey;
				board.BlackNonPawnKey = last_blacknpKey;
			}
		}
	}
}
bool isDecisive(int score)
{
	if (score > 48000 || score < -48000)
	{
		return true;
	}
	return false;
}
struct GameData
{
	Board board;
	int eval;
	int result;
	GameData(Board b, int e, int r) : board(b), eval(e), result(r) {}
};
bool isInsufficientMaterial(const Board& board) {

	int whiteBishops = count_bits(board.bitboards[B]);
	int blackBishops = count_bits(board.bitboards[b]);
	int whiteKnights = count_bits(board.bitboards[N]);
	int blackKnights = count_bits(board.bitboards[n]);
	int whiteRooks = count_bits(board.bitboards[R]);
	int blackRooks = count_bits(board.bitboards[r]);
	int whiteQueens = count_bits(board.bitboards[Q]);
	int blackQueens = count_bits(board.bitboards[q]);
	int whitePawns = count_bits(board.bitboards[P]);
	int blackPawns = count_bits(board.bitboards[p]);
	if (whiteQueens == 0 && blackQueens == 0 && whiteRooks == 0 && blackRooks == 0 && whitePawns == 0 && blackPawns == 0)
	{
		if (whiteBishops == 0 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 0)
		{
			return true;
		}
		else if (whiteBishops == 1 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 0)
		{
			return true;
		}
		else if (whiteBishops == 0 && blackBishops == 1 && whiteKnights == 0 && blackKnights == 0)
		{
			return true;
		}
		else if (whiteBishops == 0 && blackBishops == 0 && whiteKnights == 1 && blackKnights == 0)
		{
			return true;
		}
		else if (whiteBishops == 0 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 1)
		{
			return true;
		}
		else if (whiteBishops == 1 && blackBishops == 1 && whiteKnights == 0 && blackKnights == 0)
		{
			return true;
		}
		return false;
	}
	return false;
}
int flipResult(int res) {
	return 2 - res;
}

void appendToFile(const std::string& filename, const std::string& data) {
	std::ofstream file(filename, std::ios::app | std::ios::out); // Open file in append mode, create if not exists
	if (!file) {
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}
	file << data << std::endl; // Write data to file
	file.close();
}
std::string convertWDL(int wdl)
{
	if (wdl == WHITEWIN)
	{
		return "1.0";
	}
	else if (wdl == DRAW)
	{
		return "0.5";
	}
	else
	{
		return "0.0";
	}
}
void estimate_time_remaining(uint64_t remaining_positions, int pps) {
	if (pps <= 0) {
		std::cout << "Invalid PPS value!" << std::endl;
		return;
	}

	double seconds_remaining = double(remaining_positions) / pps;

	int hours = static_cast<int>(seconds_remaining) / 3600;
	int minutes = (static_cast<int>(seconds_remaining) % 3600) / 60;
	int seconds = static_cast<int>(seconds_remaining) % 60;

	std::cout << "Estimated remaining time: "
		<< hours << "h " << minutes << "m " << seconds << "s"
		<< "\n";
}
void print_progress_bar(double percentage) {
	int barWidth = 50;  // Width of the progress bar
	int progress = static_cast<int>(percentage / 2);  // Calculate the number of '#' to print

	std::cout << "\n[";
	for (int i = 0; i < barWidth; ++i) {
		if (i < progress) {
			std::cout << "#";  // Print filled part of the progress bar
		}
		else {
			std::cout << "-";  // Print empty part of the progress bar
		}
	}
	std::cout << "] " << static_cast<int>(percentage) << "%";
}
bool fileExists(const std::string& filename) {
	std::ifstream file(filename);
	return file.good();
}
std::vector<std::string> splitByPipe(const std::string& input) {
	std::vector<std::string> tokens;
	std::stringstream ss(input);
	std::string token;

	while (std::getline(ss, token, '|')) {
		tokens.push_back(token);
	}

	return tokens;
}
void Datagen(int targetPos, std::string output_name)
{
	ThreadData* heapAllocated = new ThreadData(); // Allocate safely on heap
	ThreadData& data = *heapAllocated;

	const uint64_t targetPositions = static_cast<uint64_t>(targetPos);
	uint64_t totalPositions = 0;
	std::vector<GameData> gameData;
	gameData.reserve(256);

	std::ofstream file(output_name + ".txt", std::ios::app | std::ios::out); // Open file once
	if (!file) {
		std::cerr << "Error opening file: " << output_name << std::endl;
		return;
	}

	auto start_time = std::chrono::high_resolution_clock::now();
	SearchLimitations searchLimits;
	searchLimits.SoftNodeLimit = 5000;
	searchLimits.HardNodeLimit = 10000;
	while (totalPositions < targetPositions) {
		gameData.clear();
		Board board;
		PickRandomPos(board, data);
		bool isGameOver = false;
		int result = -1;

		int moves = 0;
		while (!isGameOver) {
			auto searchResult = IterativeDeepening(board, 99, searchLimits, data, false);
			Move bestMove = searchResult.first;
			int eval = searchResult.second;
			if (board.side == Black) eval = -eval;

			MoveList moveList;
			Generate_Legal_Moves(moveList, board, false);

			if (isNoLegalMoves(board, moveList)) {
				result = is_in_check(board) ? (board.side == White ? BLACKWIN : WHITEWIN) : DRAW;
				break;
			}
			if (is_threefold(board.history, board.lastIrreversiblePly) || isInsufficientMaterial(board) || board.history.size() - board.lastIrreversiblePly >= 100) {
				result = DRAW;
				break;
			}
			if (isDecisive(eval)) {
				result = (eval > 48000) ? (board.side == White ? WHITEWIN : BLACKWIN) : (board.side == White ? BLACKWIN : WHITEWIN);
			}

			MakeMove(board, bestMove);
			moves++;
			if (!is_in_check(board) && (bestMove.Type & captureFlag) == 0 && !isDecisive(eval)) {
				gameData.push_back(GameData(board, eval, -1));
				totalPositions++;
			}
			//std::cout << moves << "\n";

		}

		// **Batch write game data to file instead of writing each line separately**
		std::ostringstream buffer;
		for (int i = 0; i < gameData.size(); i++) {
			gameData[i].result = result;
			buffer << boardToFEN(gameData[i].board) << " | " << gameData[i].eval << " | " << convertWDL(gameData[i].result) << "\n";
		}
		file << buffer.str(); // **Write all at once**

		auto end_time = std::chrono::high_resolution_clock::now();
		double elapsed_seconds = std::chrono::duration<double>(end_time - start_time).count();
		double positions_per_second = totalPositions / elapsed_seconds;
		double percentage = (static_cast<double>(totalPositions) / targetPos) * 100;

		setColor(ConsoleColor::BrightGreen);
		std::cout << "Positions/s: " << std::fixed << std::setprecision(2) << positions_per_second;

		setColor(ConsoleColor::BrightCyan);
		std::cout << " | Total: " << totalPositions << " ("
			<< std::fixed << std::setprecision(2) << (totalPositions / 1'000'000.0) << "M)";

		setColor(ConsoleColor::BrightYellow);
		std::cout << " | Progress: " << std::fixed << std::setprecision(4) << percentage << "% ";

		setColor(ConsoleColor::White); // Reset to default
		estimate_time_remaining(targetPositions - totalPositions, positions_per_second);
		print_progress_bar(percentage);
		std::cout << "\n\n" << std::flush;
	}

	file.close(); // **Close file only once after everything is written**
	delete heapAllocated;
}


