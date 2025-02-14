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

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX

#include <windows.h>

#undef NOMINMAX
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN

#endif

bool isOnWindow;

#define NULLMOVE Move(0,0,0,0)

void initialize_platform() {
#if defined(_WIN32) || defined(_WIN64)
	// Windows platform
	isOnWindow = true;
#elif defined(__linux__)
	// Linux platform
	isOnWindow = false;
#else
	// Unknown platform
	isOnWindow = false;  // Default to false (assuming Linux if not Windows)
#endif
}
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
int RFP_IMPROVING_MULTIPLIER = 65;
int RFP_BASE = -36;
int RFP_IMPROVING_BASE = -39;

int LMP_BASE = 0;
int LMP_MULTIPLIER = 1;

int PVS_QUIET_BASE = 4;
int PVS_QUIET_MULTIPLIER = 57;

int PVS_NOISY_BASE = -3;
int PVS_NOISY_MULTIPLIER = 14;

int HISTORY_BASE = 4 * 32;
int HISTORY_MULTIPLIER = 3 * 32;
int CONTHIST_BASE = 4;
int CONTHIST_MULTIPLIER = 3;

int ASP_WINDOW_INITIAL = 38;
int ASP_WINDOW_MAX = 311;

int PAWN_CORRHIST_MULTIPLIER = 7;// divide by 5 later
int MINOR_CORRHIST_MULTIPLIER = 6;// divide by 5 later
int NONPAWN_CORRHIST_MULTIPLIER = 7;// divide by 5 later

int QS_SEE_PRUNING_MARGIN = -2;
int HISTORY_PRUNING_MULTIPLIER = 41 * 32;
int HISTORY_PRUNING_BASE = 2 * 32;
int HISTORY_LMR_MULTIPLIER = 24 * 32;
int HISTORY_LMR_BASE = 3 * 32;
int NMP_EVAL_DIVISER = 418;
int NMP_DEPTH_DIVISER = 4;
int MAX_NMP_EVAL_R = 3;

auto clockStart = std::chrono::steady_clock::now();

int pvLengths[99];
Move pvTable[99][99];


int ply;
int searchNodeCount;
int Searchtime_MS;
int currDepth;
bool isSearchStop;



int selDepth = 0;
int SEEPieceValues[] = { 98, 280, 295, 479, 1064, 0, 0 };
static Move lastBestMoves[99];


constexpr int CORRHIST_WEIGHT_SCALE = 256;
constexpr int CORRHIST_GRAIN = 256;
constexpr int CORRHIST_SIZE = 16384;
constexpr int CORRHIST_MAX = 16384;

Move killerMoves[2][99];

int mainHistory[2][64][64][2][2];

int CaptureHistory[12][64][12];

int onePlyContHist[12][64][12][64];
int twoPlyContHist[12][64][12][64];

int pawnCorrHist[2][CORRHIST_SIZE];
int minorCorrHist[2][CORRHIST_SIZE];

/// <summary>
/// [keySide][currentBoardSide][hash]
/// </summary>
int nonPawnCorrHist[2][2][CORRHIST_SIZE];

size_t TTSize = 16;
TranspositionEntry* TranspositionTable = nullptr;


Search_data searchStack[99];

constexpr int MAX_HISTORY = 16384;
constexpr int MAX_CONTHIST = 1024;
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

static int MVVLVA[6][6] = {
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};
bool is_quiet(int type)
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
void initializeLMRTable()
{
	initialize_platform();
	for (int depth = 1; depth < 99; depth++)
	{
		for (int move = 1; move < 256; move++)
		{
			lmrTable[depth][move] = std::floor(0.77 + log(move) * log(depth) / 2.36);
		}
	}
	for (int ply = 0; ply < 99; ply++)
	{
		killerMoves[0][ply] = Move();
		killerMoves[1][ply] = Move();
	}
	memset(mainHistory, 0, sizeof(mainHistory));
	memset(onePlyContHist, 0, sizeof(onePlyContHist));
	memset(twoPlyContHist, 0, sizeof(twoPlyContHist));
	memset(CaptureHistory, 0, sizeof(CaptureHistory));
	memset(pawnCorrHist, 0, sizeof(pawnCorrHist));
	memset(nonPawnCorrHist, 0, sizeof(nonPawnCorrHist));
	memset(minorCorrHist, 0, sizeof(minorCorrHist));

	isPrettyPrinting = true;
}
int scaledBonus(int score, int bonus)
{
	return std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY) - (score * abs(bonus) / MAX_HISTORY);
}
void updateMinorCorrHist(Board& board, const int depth, const int diff)
{
	uint64_t minorKey = board.MinorKey;
	int& entry = minorCorrHist[board.side][minorKey % CORRHIST_SIZE];
	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);
	entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	entry = std::clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
}
void updatePawnCorrHist(Board& board, const int depth, const int diff)
{
	uint64_t pawnKey = board.PawnKey;
	int& entry = pawnCorrHist[board.side][pawnKey % CORRHIST_SIZE];
	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);
	entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	entry = std::clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
}
void updateNonPawnCorrHist(Board& board, const int depth, const int diff)
{
	uint64_t whiteKey = board.WhiteNonPawnKey;
	uint64_t blackKey = board.BlackNonPawnKey;

	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);

	int& whiteEntry = nonPawnCorrHist[White][board.side][whiteKey % CORRHIST_SIZE];

	whiteEntry = (whiteEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	whiteEntry = std::clamp(whiteEntry, -CORRHIST_MAX, CORRHIST_MAX);

	int& blackEntry = nonPawnCorrHist[Black][board.side][blackKey % CORRHIST_SIZE];

	blackEntry = (blackEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	blackEntry = std::clamp(blackEntry, -CORRHIST_MAX, CORRHIST_MAX);
}
int adjustEvalWithCorrHist(Board& board, const int rawEval)
{
	uint64_t pawnKey = board.PawnKey;
	const int& pawnEntry = pawnCorrHist[board.side][pawnKey % CORRHIST_SIZE];
	

	uint64_t minorKey = board.MinorKey;
	const int& minorEntry = minorCorrHist[board.side][minorKey % CORRHIST_SIZE];

	uint64_t whiteNPKey = board.WhiteNonPawnKey;
	const int& whiteNPEntry = nonPawnCorrHist[White][board.side][whiteNPKey % CORRHIST_SIZE];
	uint64_t blackNPKey = board.BlackNonPawnKey;
	const int& blackNPEntry = nonPawnCorrHist[Black][board.side][blackNPKey % CORRHIST_SIZE];
	int mate_found = 49000 - 99;

	int adjust = (pawnEntry * (static_cast<float>(PAWN_CORRHIST_MULTIPLIER)/5)) + (minorEntry * (static_cast<float>(MINOR_CORRHIST_MULTIPLIER) / 5)) + ((whiteNPEntry + blackNPEntry) * (static_cast<float>(NONPAWN_CORRHIST_MULTIPLIER) / 5));
	return std::clamp(rawEval + adjust / CORRHIST_GRAIN, -mate_found + 1, mate_found - 1);
}
void updateHistory(int stm, int from, int to, int bonus, uint64_t opp_threat)
{
	int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
	mainHistory[stm][from][to][Get_bit(opp_threat, from)][Get_bit(opp_threat, to)] += clampedBonus - mainHistory[stm][from][to][Get_bit(opp_threat, from)][Get_bit(opp_threat, to)] * abs(clampedBonus) / MAX_HISTORY;
}
void update_capthist(int attacker, int to, int victim, int bonus)
{
	int clampedBonus = std::clamp(bonus, -MAX_CAPTHIST, MAX_CAPTHIST);
	CaptureHistory[attacker][to][victim] += clampedBonus - CaptureHistory[attacker][to][victim] * abs(clampedBonus) / MAX_CAPTHIST;
}
int getSingleContinuationHistoryScore(Move move, const int offSet) 
{
	if (ply >= offSet)
	{
		Move previousMove = searchStack[ply - offSet].move;

		if (offSet == 1)
		{
			return onePlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To];
		}
		else if(offSet == 2)
		{
			return twoPlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To];
		}
		
	}
	return 0;
}
int getContinuationHistoryScore(Move& move) 
{
	if (ply >= 1)
	{
		int onePly = getSingleContinuationHistoryScore(move, 1);
		int twoPly = getSingleContinuationHistoryScore(move, 2);


		int finalScore = onePly + twoPly;
		return finalScore;
	}
	return 0;
}
void updateSingleContinuationHistoryScore(Move& move, const int bonus, const int offSet)
{
	if (ply >= offSet) {
		Move previousMove = searchStack[ply - offSet].move;

		int clampedBonus = std::clamp(bonus, -MAX_CONTHIST, MAX_CONTHIST);
		const int scaledBonus = clampedBonus - getSingleContinuationHistoryScore(move, offSet) * abs(clampedBonus) / MAX_CONTHIST;
		//std::cout << scaledBonus;

		if (offSet == 1)
		{
			onePlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To] += scaledBonus;
		}
		else if(offSet == 2)
		{
			twoPlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To] += scaledBonus;
		}
		
	}
}
void updateContinuationHistoryScore(Move& move, const int bonus) 
{
	updateSingleContinuationHistoryScore(move, bonus, 1);
	updateSingleContinuationHistoryScore(move, bonus, 2);
}

void printTopCapthist(int side) {
	std::vector<std::tuple<int, int, int>> moves; // Stores <score, from, to>

	// Populate the vector with scores and moves
	for (int from = 0; from < 64; ++from) {
		for (int to = 0; to < 64; ++to) {
			int score = CaptureHistory[side][from][to];
			moves.emplace_back(score, from, to);
		}
	}

	// Sort moves by score in descending order
	std::stable_sort(moves.rbegin(), moves.rend(), [](auto& a, auto& b) {
		return std::get<0>(a) < std::get<0>(b);
		});

	// Print the top 10 moves
	std::cout << "Top 10 moves for history[" << side << "]:" << std::endl;
	for (int i = 0; i < std::min(10, (int)moves.size()); ++i) {
		auto [score, from, to] = moves[i];
		std::cout << "from: " << CoordinatesToChessNotation(from)
			<< " to: " << CoordinatesToChessNotation(to)
			<< " score = " << score
			<< " side_to_move = " << side
			<< std::endl;
	}
}
void printTopOneplyContHist() {
	std::vector<std::tuple<int, int, int, int, int>> moves; // Stores <score, piece_from, from, piece_to, to>

	// Populate the vector with scores and moves
	for (int piece_from = 0; piece_from < 12; ++piece_from) {
		for (int from = 0; from < 64; ++from) {
			for (int piece_to = 0; piece_to < 12; ++piece_to) {
				for (int to = 0; to < 64; ++to) {
					int score = onePlyContHist[piece_from][from][piece_to][to];
					moves.emplace_back(score, piece_from, from, piece_to, to); // 5 values pushed
				}
			}
		}
	}

	// Sort moves by score in descending order
	std::stable_sort(moves.rbegin(), moves.rend(), [](auto& a, auto& b) {
		return std::get<0>(a) < std::get<0>(b);
		});

	// Print the top 10 moves
	std::cout << "Top 10 moves from Oneply_ContHist:" << std::endl;
	for (int i = 0; i < std::min(50, (int)moves.size()); ++i) {
		auto [score, piece_from, from, piece_to, to] = moves[i];

		// Print the pieces and coordinates
		std::cout << "PieceA: " << getCharFromPiece(piece_from)
			<< " toA: " << CoordinatesToChessNotation(from)
			<< " PieceB: " << getCharFromPiece(piece_to)
			<< " toB: " << CoordinatesToChessNotation(to)
			<< " score = " << score
			<< std::endl;
	}
}


static inline int getMoveScore(Move move, Board& board, TranspositionEntry &entry, uint64_t opp_threat)
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
		if (killerMoves[0][ply] == move)
		{
			return 150000;
		}
		else if (killerMoves[1][ply] == move)
		{
			return 100000;
		}
		else
		{
			// Return history score for non-capture and non-killer moves
			int mainHistScore = mainHistory[board.side][move.From][move.To][Get_bit(opp_threat, move.From)][Get_bit(opp_threat, move.To)]/32;
			int contHistScore = getContinuationHistoryScore(move);
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
int move_estimated_value(Board &board, Move move) 
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

static inline void sort_moves_captures(std::vector<Move>& moves, Board& board) 
{
	// Partition to segregate capture moves
	auto capture_end = std::stable_partition(moves.begin(), moves.end(), [](const Move& move) {
		return !is_quiet(move.Type);
		});

	// Sort only the capture moves
	std::stable_sort(moves.begin(), capture_end, [&board](const Move& move1, const Move& move2) {
		return get_move_score_capture(move1, board) > get_move_score_capture(move2, board);
		});

}



static inline void sort_moves(std::vector<Move>& moves, Board& board, TranspositionEntry &tt_entry, uint64_t opp_threat) 
{
    // Precompute scores for all moves
    std::vector<std::pair<int, Move>> scored_moves;
    scored_moves.reserve(moves.size());
    for (const Move& move : moves) 
	{
        int score = getMoveScore(move, board, tt_entry, opp_threat);
        scored_moves.emplace_back(score, move);
    }

    // Sort the scored moves based on the scores
    std::stable_sort(scored_moves.begin(), scored_moves.end(), [](const auto& a, const auto& b)
	{
        return a.first > b.first; // Sort by score (descending)
    });

    // Rebuild the original moves vector in sorted order
    for (size_t i = 0; i < moves.size(); ++i) 
	{
        moves[i] = scored_moves[i].second;
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
static inline int Quiescence(Board& board, int alpha, int beta)
{

	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - clockStart).count();
	if (elapsedMS > Searchtime_MS) {
		isSearchStop = true;
		return 0; // Return a neutral score if time is exceeded
	}
	if (ply > 99)
	{
		return Evaluate(board);
	}
	int score = 0;
	 
	int staticEval = Evaluate(board);
	staticEval = adjustEvalWithCorrHist(board, staticEval);
	

	if (staticEval >= beta)
	{
		return staticEval;
	}

	if (staticEval > alpha)
	{
		alpha = staticEval;
	}

	TranspositionEntry ttEntry = ttLookUp(board.zobristKey);
	if (ttEntry.zobristKey == board.zobristKey && ttEntry.bound != 0)
	{
		if ((ttEntry.bound == ExactFlag)
			|| (ttEntry.bound == UpperBound && ttEntry.score <= alpha)
			|| (ttEntry.bound == LowerBound && ttEntry.score >= beta))
		{
			return ttEntry.score;
		}
	}

	std::vector<Move> moveList;
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
	for (Move& move : moveList)
	{
		if (is_quiet(move.Type)) continue; //skip non capture moves

		if (!SEE(board, move, 0))
		{
			continue;
		}


		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.lastIrreversiblePly;


		ply++;
		if (selDepth < ply)
		{
			selDepth = ply;
		}
		MakeMove(board, move);
		
		if (!isLegal(move, board))//isMoveValid(move, board)
		{

			ply--;
			UnmakeMove(board, move, captured_piece);

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


		searchNodeCount++;








		legal_moves++;
		score = -Quiescence(board, -beta, -alpha);

		if (isSearchStop) {
			UnmakeMove(board, move, captured_piece);
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
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}

		ply--;
		UnmakeMove(board, move, captured_piece);
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
		return staticEval;
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


static inline int Negamax(Board& board, int depth, int alpha, int beta, bool doNMP, bool cutnode, Move excludedMove = NULLMOVE)
{
	bool isPvNode = beta - alpha > 1;

	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - clockStart).count();
	if (elapsedMS > Searchtime_MS) {
		isSearchStop = true;
		return 0; // Return a neutral score if time is exceeded
	}

	pvLengths[ply] = ply;


	if (ply != 0 && is_threefold(board.history, board.lastIrreversiblePly))
	{
		return 0;
	}

	if (ply > 99 - 1) //safety reason
	{
		return Evaluate(board);
	}
	TranspositionEntry ttEntry = ttLookUp(board.zobristKey);
	int score = 0;
	int ttFlag = UpperBound;

	int bestValue = MINUS_INFINITY;
	bool is_ttmove_found = false;
	bool isSingularSearch = excludedMove != NULLMOVE;
	// Only check TT for depths greater than zero (ply != 0)
	if (ttEntry.zobristKey == board.zobristKey && ttEntry.bound != 0)
	{
		is_ttmove_found = true;
		// Valid TT entry found
		if (!isSingularSearch && ply != 0 && ttEntry.depth >= depth)
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

	bool isInCheck = is_in_check(board);

	if (isInCheck)
	{
		depth = std::max(depth + 1, 1);
	}
	if (depth <= 0)
	{
		return Quiescence(board, alpha, beta);
		//return Evaluate(board);
	}
	if (ply + 1 <= 99)
	{
		killerMoves[0][ply + 1] = Move(0, 0, 0, 0);
		killerMoves[1][ply + 1] = Move(0, 0, 0, 0);
	}

	int rawEval = Evaluate(board);

	int staticEval = adjustEvalWithCorrHist(board, rawEval);

	int ttAdjustedEval = staticEval;
	uint8_t Bound = ttEntry.bound;
	if (!isSingularSearch && is_ttmove_found && !isInCheck && (Bound == ExactFlag || (Bound == LowerBound && ttEntry.score >= staticEval) || (Bound == UpperBound && ttEntry.score <= staticEval)))
	{
		ttAdjustedEval = ttEntry.score;
	}
	searchStack[ply].staticEval = staticEval;

	bool improving = !isInCheck && ply > 1 && staticEval > searchStack[ply - 2].staticEval;

	int canPrune = !isInCheck && !isPvNode;
	if (!isSingularSearch && depth < 4 && canPrune)//rfp
	{
		int rfpMargin;
		if (improving)
		{
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

	if (!isSingularSearch && !isPvNode && doNMP)
	{
		if (!isInCheck && depth >= 2 && ply && ttAdjustedEval >= beta)
		{
			if ((board.occupancies[Both] & ~(board.bitboards[P] | board.bitboards[p] | board.bitboards[K] | board.bitboards[k])) != 0ULL)
			{

				int lastep = board.enpassent;
				uint64_t lzob = board.zobristKey;
				ply++;
				Make_Nullmove(board);
				int R = 3 + depth / NMP_DEPTH_DIVISER;
				R += std::min((ttAdjustedEval - beta) / NMP_EVAL_DIVISER, MAX_NMP_EVAL_R);
				int score = -Negamax(board, depth - R, -beta, -beta + 1, false, !cutnode);

				Unmake_Nullmove(board);
				ply--;
				board.enpassent = lastep;
				board.zobristKey = lzob;

				if (score >= beta)
				{
					return score > 49000 ? beta : score;
				}

			}
		}
	}




	std::vector<Move> moveList;
	Generate_Legal_Moves(moveList, board, false);


	int searchedMoves = 0;

	uint64_t oppThreats = get_attacked_squares(1 - board.side, board, board.occupancies[Both]);
	sort_moves(moveList, board, ttEntry, oppThreats);

	int orgAlpha = alpha;

	int depthToSearch;

	bool skipQuiets = false;

	int lmpThreshold = LMP_BASE + LMP_MULTIPLIER * depth * depth;

	int quietSEEMargin = PVS_QUIET_BASE + (-PVS_QUIET_MULTIPLIER * depth);
	int noisySEEMargin = PVS_NOISY_BASE + (-PVS_NOISY_MULTIPLIER * depth * depth);

	std::vector<Move> quietsList;
	quietsList.reserve(50);

	Move bestMove = Move(0, 0, 0, 0);
	int quietMoves = 0;

	
	uint64_t last_zobrist = board.zobristKey;
	uint64_t last_pawnKey = board.PawnKey;
	uint64_t last_minorKey = board.MinorKey;
	uint64_t last_whitenpKey = board.WhiteNonPawnKey;
	uint64_t last_blacknpKey = board.BlackNonPawnKey;
	/*if (ply > 0)
	{
		searchStack[ply].doubleExtensions = searchStack[ply - 1].doubleExtensions;
	}*/
	for (Move& move : moveList)
	{

		bool isQuiet = is_quiet(move.Type);
		if(move == excludedMove)
		{
			continue;
		}
		if (skipQuiets && isQuiet) //quiet move
		{
			continue;
		}
		int seeThreshold = isQuiet ? quietSEEMargin : noisySEEMargin;
		if (depth <= MAX_PVS_SEE_DEPTH)
		{
			if (!SEE(board, move, seeThreshold))
			{ 
				continue;
			}
		}

		bool isNotMated = alpha > -49000 + 99;

		int main_history = mainHistory[board.side][move.From][move.To][Get_bit(oppThreats, move.From)][Get_bit(oppThreats, move.To)];
		int conthist = getContinuationHistoryScore(move) * 32;
		int historyScore = main_history + conthist;
		if (ply != 0 && isQuiet && isNotMated)
		{
			if (searchedMoves >= lmpThreshold)
			{
				skipQuiets = true;
			}
			if (quietMoves > 1 && depth <= 5 && historyScore < (-HISTORY_PRUNING_MULTIPLIER * depth) + HISTORY_PRUNING_BASE)
			{
				break;
			}

		}




		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.lastIrreversiblePly;
		

		

		ply++;

		if (selDepth < ply)
		{
			selDepth = ply;
		}
		MakeMove(board, move);

		searchNodeCount++;
		if (!isLegal(move, board))
		{

			ply--;
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

		if (isQuiet)
		{
			quietsList.push_back(move);
			quietMoves++;
		}



		searchedMoves++;
		depthToSearch = depth - 1;

		int reduction = 0;
		bool is_reduced = false;


		int extensions = 0;

		if(ply > 1 && depth >= 7 && move == ttEntry.bestMove && excludedMove == NULLMOVE && ttEntry.depth >= depth - 3 && ttEntry.bound != UpperBound && std::abs(ttEntry.score) < 50000)
		{
			ply--;
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


			int s_beta = ttEntry.score - depth * 2;
			int s_depth = (depth - 1) / 2;
			int s_score = Negamax(board, s_depth, s_beta - 1, s_beta, true, cutnode, move);
			if(s_score < s_beta)
			{
				if (!isPvNode && score <= s_beta - 20)
				{
					extensions = 2;
					//searchStack[ply].doubleExtensions++;
				}
    else
    {
     extensions = 1;
    }
			
			}


			MakeMove(board, move);
			ply++;
		}
		searchStack[ply-1].move = move;
		if (depth > MIN_LMR_DEPTH && searchedMoves > 1)
		{
			reduction = lmrTable[depth][searchedMoves];

			if (!isPvNode && quietMoves >= 4)
			{
				reduction++;
			}
			if (is_in_check(board))
			{
				reduction--;
			}
			if (improving)
			{
				reduction--;
			}
			if (historyScore < (- HISTORY_LMR_MULTIPLIER * depth) + HISTORY_LMR_BASE)
			{
				reduction++;
			}
			if (!isQuiet)
			{
				reduction--;
			}
			if((move == killerMoves[0][ply - 1]) || (move == killerMoves[1][ply - 1]))
			{
				reduction--;
			}
		}

		if (reduction < 0) reduction = 0;
		is_reduced = reduction > 0;

		if (searchedMoves <= 1)
		{
			score = -Negamax(board, depthToSearch + extensions, -beta, -alpha, true, false);
		}
		else
		{
			if (is_reduced)
			{
				if (isPvNode)
				{
					score = -Negamax(board, depthToSearch - reduction, -alpha - 1, -alpha, true, false);
				}
				else
				{
					score = -Negamax(board, depthToSearch - reduction, -alpha - 1, -alpha, true, !cutnode);
				}



			}
			else
			{
				score = alpha + 1;
			}
			if (score > alpha)
			{
				score = -Negamax(board, depthToSearch, -alpha-1, -alpha, true, false);
			}
			if (score > alpha && score < beta )
			{
				score = -Negamax(board, depthToSearch, -beta, -alpha, true, false);
			}

		}


		if (isSearchStop) {
			ply--;
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
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}

		ply--;
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

		bestValue = std::max(score, bestValue);


		if (bestValue > alpha)
		{
			ttFlag = ExactFlag;
			alpha = score;
			bestMove = move;
			if(isPvNode)
			{
				pvTable[ply][ply] = move;
				for (int next_ply = ply + 1; next_ply < pvLengths[ply + 1]; next_ply++)
				{
					pvTable[ply][next_ply] = pvTable[ply + 1][next_ply];
				}
				pvLengths[ply] = pvLengths[ply + 1];
			}
			



		}
		if (alpha >= beta)
		{
			ttFlag = LowerBound;
			if ((move.Type & capture) == 0)
			{
				if (!(killerMoves[0][ply] == move))
				{
					killerMoves[1][ply] = killerMoves[0][ply];
					killerMoves[0][ply] = move;
				}
				int mainHistBonus = HISTORY_BASE + HISTORY_MULTIPLIER * depth * depth;
				int contHistBonus = CONTHIST_BASE + CONTHIST_MULTIPLIER * depth * depth;
				for (auto& move_quiet : quietsList) {
					if (move_quiet == move)
					{
						updateHistory(board.side, move_quiet.From, move_quiet.To, mainHistBonus, oppThreats);
						if (ply >= 1)
						{
							updateContinuationHistoryScore(move_quiet, contHistBonus);
						}

					}
					else
					{
						updateHistory(board.side, move_quiet.From, move_quiet.To, -mainHistBonus, oppThreats);
						if (ply >= 1)
						{
							updateContinuationHistoryScore(move_quiet, -contHistBonus);
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
			return -49000 + ply;
		}
		else
		{
			return isSingularSearch ? alpha : 0;
		}
	}
	ttEntry.score = bestValue;
	ttEntry.bound = ttFlag;
	ttEntry.depth = depth;
	ttEntry.zobristKey = board.zobristKey;
	if (bestMove != Move(0, 0, 0, 0))
	{
		ttEntry.bestMove = bestMove;
	}

	int bound = bestValue >= beta ? HFLOWER : alpha != orgAlpha ? HFEXACT : HFUPPER;
	if (!isSingularSearch && !is_in_check(board) && (bestMove == Move(0, 0, 0, 0) || is_quiet(bestMove.Type)) && !(bound == HFLOWER && bestValue <= staticEval) && !(bound == HFUPPER && bestValue >= staticEval))
	{
		updatePawnCorrHist(board, depth, bestValue - staticEval);
		updateMinorCorrHist(board, depth, bestValue - staticEval);
		updateNonPawnCorrHist(board, depth, bestValue - staticEval);
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
	auto search_start = std::chrono::steady_clock::now();
	auto search_end = std::chrono::steady_clock::now();
	Board board;
	uint64_t nodecount = 0; 
	int totalsearchtime = 0;
	for (int i = 0; i < 50; i++)
	{
		for (int ply = 0; ply < 99; ply++)
		{
			killerMoves[0][ply] = Move();
			killerMoves[1][ply] = Move();
		}
		memset(mainHistory, 0, sizeof(mainHistory));
		for (size_t i = 0; i < TTSize; i++)
		{
			TranspositionTable[i] = TranspositionEntry();
		}
		memset(onePlyContHist, 0, sizeof(onePlyContHist));
		memset(twoPlyContHist, 0, sizeof(twoPlyContHist));
		memset(CaptureHistory, 0, sizeof(CaptureHistory));
		memset(pawnCorrHist, 0, sizeof(pawnCorrHist));
		memset(minorCorrHist, 0, sizeof(minorCorrHist));

		parse_fen(benchFens[i], board);
		board.zobristKey = generate_hash_key(board);
		board.history.push_back(board.zobristKey);

		search_start = std::chrono::steady_clock::now();
		IterativeDeepening(board, 11, -1, false, false);
		search_end = std::chrono::steady_clock::now();

		float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(search_end - search_start).count();

		nodecount += searchNodeCount;
		totalsearchtime += std::floor(elapsedMS);



	}
	std::cout << nodecount << " nodes "  << nodecount / (totalsearchtime + 1) * 1000 << " nps "  << "\n";
	for (int ply = 0; ply < 99; ply++)
	{
		killerMoves[0][ply] = Move();
		killerMoves[1][ply] = Move();
	}
	memset(mainHistory, 0, sizeof(mainHistory));
	for (size_t i = 0; i < TTSize; i++)
	{
		TranspositionTable[i] = TranspositionEntry();
	}
	memset(onePlyContHist, 0, sizeof(onePlyContHist));
	memset(CaptureHistory, 0, sizeof(CaptureHistory));
	memset(pawnCorrHist, 0, sizeof(pawnCorrHist));
	memset(minorCorrHist, 0, sizeof(minorCorrHist));

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
void print_UCI(Move (&PV_line)[99][99], Move& bestmove, int score, float elapsedMS, float nps)
{
	bestmove = pvTable[0][0];
	int hashfull = get_hashfull();
	lastBestMoves[currDepth - 1] = bestmove;
	std::cout << "info depth " << currDepth;
	std::cout << " seldepth " << selDepth;
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

	std::cout << " time " << static_cast<int>(std::round(elapsedMS)) << " nodes " << searchNodeCount << " nps " << static_cast<int>(std::round(nps)) << " hashfull " << hashfull << " pv " << std::flush;
	for (int count = 0; count < pvLengths[0]; count++)
	{
		printMove(pvTable[0][count]);
		std::cout << " ";
	}
	std::cout << "\n";
}
void print_Pretty(Move(&PV_line)[99][99], Move& bestmove, int score, float elapsedMS, float nps, int window_change, int asp_alpha, int asp_beta)
{
	setColor(ConsoleColor::White);
	std::cout << "depth ";
	setColor(ConsoleColor::BrightBlue);
	if (currDepth < 10)
	{
		std::cout << " ";
	}

	std::cout<< currDepth;
	setColor(ConsoleColor::White);
	std::cout << " / ";
	setColor(ConsoleColor::BrightBlue);


	if (selDepth < 10)
	{
		std::cout << " ";
	}
	std::cout<< selDepth;

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
	std::cout<< nps_in_M;

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
		std::cout << std::fixed << std::setprecision(2) << score_fullPawn;
		
		std::setprecision(1);
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
void scaleTime(int& softLimit, uint8_t bestMoveStability, int baseSoft, int maxTime) {
	double bestMoveScale[5] = { 2.43, 1.35, 1.09, 0.88, 0.68 };
	softLimit = std::min(static_cast<int>(baseSoft * bestMoveScale[bestMoveStability]), maxTime);
}
void IterativeDeepening(Board& board, int depth, int timeMS, bool PrintRootVal, bool print_info, int softbound, int baseTime, int maxTime)
{

	if (timeMS == -1)
	{
		Searchtime_MS = std::numeric_limits<int>::max();
	}
	else
	{
		Searchtime_MS = timeMS;
	}


	searchNodeCount = 0;
	Move bestmove;
	clockStart = std::chrono::steady_clock::now();

	int score = 0;


	
	memset(pvTable, 0, sizeof(pvTable));
	memset(pvLengths, 0, sizeof(pvLengths));

	int bestMoveStability = 0;
	int baseSoft = softbound;
	for (currDepth = 1; currDepth <= depth; currDepth++)
	{
		
		ply = 0;
		selDepth = 0;


		isSearchStop = false;
		for (int i = 0; i < 99; i++)
		{
			searchStack[i].move = Move(0, 0, 0, 0);
		}
		
		int delta = ASP_WINDOW_INITIAL;
		int alpha_val = std::max(MINUS_INFINITY ,score - delta);
		int beta_val = std::min(PLUS_INFINITY, score + delta);
		int aspDepth = currDepth;
		int window_change = 1;
		//std::cout << alpha_val << ","<<beta_val;
		while (true)
		{
			auto end = std::chrono::steady_clock::now();
			float MS = std::chrono::duration_cast<std::chrono::milliseconds>(end - clockStart).count();
			if (softbound != -1)
			{
				if (MS > softbound)
				{
					break;
				}

			}
			else
			{
				if (timeMS != -1 && MS > timeMS)
				{
					break;
				}
			}
			
			score = Negamax(board, std::max(aspDepth, 1), alpha_val, beta_val, true, false);
			
			delta += delta;
			if (score <= alpha_val)
			{
				alpha_val = std::max(MINUS_INFINITY, score - delta);
				aspDepth = depth;
			}
			else if (score >= beta_val)
			{
				beta_val = std::min(PLUS_INFINITY, score + delta);
				aspDepth = std::max(aspDepth - 1, depth - 5);
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

		float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - clockStart).count();
		

		float second = (elapsedMS + 1) / 1000;

		double nps = searchNodeCount / second;
		
		if (pvTable[0][0] == bestmove)
		{
			bestMoveStability = std::min(bestMoveStability + 1, 4);
		}
		else
		{
			bestMoveStability = 0;
		}
		if (currDepth >= 6 && softbound != -1 && baseTime != -1) {
			scaleTime(softbound, bestMoveStability, baseSoft, maxTime);
		}
		if (print_info)
		{
			if (!isSearchStop)
			{
				bestmove = pvTable[0][0];
				if (isPrettyPrinting && isOnWindow)
				{
					print_Pretty(pvTable, bestmove, score, elapsedMS, nps, window_change, alpha_val, beta_val);
				}
				else
				{
					print_UCI(pvTable, bestmove, score, elapsedMS, nps);
				}

			}


			
		}




		if (softbound != -1)
		{
			if (elapsedMS > softbound)
			{
				break;
			}

		}
		else
		{
			if (timeMS != -1 && elapsedMS > timeMS)
			{
				break;
			}
		}


	}
	if (print_info)
	{
		std::cout << "bestmove ";
		printMove(bestmove);
		std::cout << "\n";
	}

}
