#pragma once
#include <cstdint>
#include "Board.h"
#include <utility>
#include <chrono>

extern int pvLengths[99];
extern Move pvTable[99][99];


extern int RFP_MULTIPLIER;
extern int RFP_IMPROVING_MULTIPLIER;
extern int RFP_BASE;
extern int RFP_IMPROVING_BASE;

extern int LMP_BASE;
extern int LMP_MULTIPLIER;

extern int PVS_QUIET_BASE;
extern int PVS_QUIET_MULTIPLIER;

extern int PVS_NOISY_BASE;
extern int PVS_NOISY_MULTIPLIER;

extern int HISTORY_BASE;
extern int HISTORY_MULTIPLIER;

extern int ASP_WINDOW_INITIAL;
extern int ASP_WINDOW_MAX;

extern int PAWN_CORRHIST_MULTIPLIER; // divide by 5 later
extern int MINOR_CORRHIST_MULTIPLIER; // divide by 5 later
extern int NONPAWN_CORRHIST_MULTIPLIER; // divide by 5 later

extern int QS_SEE_PRUNING_MARGIN;
extern int HISTORY_PRUNING_MULTIPLIER;
extern int HISTORY_PRUNING_BASE;
extern int HISTORY_LMR_MULTIPLIER;
extern int HISTORY_LMR_BASE;
extern int NMP_EVAL_DIVISER;
extern int NMP_DEPTH_DIVISER;
extern int MAX_NMP_EVAL_R;
extern int DEXT_MARGIN;

extern uint64_t TTSize;

extern bool isPrettyPrinting;

extern double DEF_TIME_MULTIPLIER;
extern double DEF_INC_MULTIPLIER;
extern double MAX_TIME_MULTIPLIER;
extern double HARD_LIMIT_MULTIPLIER;
extern double SOFT_LIMIT_MULTIPLIER;

constexpr int CORRHIST_WEIGHT_SCALE = 256;
constexpr int CORRHIST_GRAIN = 256;
constexpr int CORRHIST_SIZE = 16384;
constexpr int CORRHIST_MAX = 16384;
struct TranspositionEntry
{
	uint64_t zobristKey;
	int32_t score;
	Move bestMove;
	uint8_t depth;

	uint8_t bound;
	bool ttPv;
};

struct Search_data
{
	Move move;
	int staticEval;
};
struct ThreadData
{
	std::chrono::steady_clock::time_point clockStart;

	int ply;
	int64_t searchNodeCount;
	int64_t Searchtime_MS;
	int currDepth;
	bool isSearchStop;
	int selDepth = 0;

	int mainHistory[2][64][64][2][2];

	int CaptureHistory[12][64][12];

	int onePlyContHist[12][64][12][64];
	int twoPlyContHist[12][64][12][64];

	int pawnCorrHist[2][CORRHIST_SIZE];
	int minorCorrHist[2][CORRHIST_SIZE];


	int counterMoveCorrHist[12][64];

	int nonPawnCorrHist[2][2][CORRHIST_SIZE];

	Search_data searchStack[99];
};
struct SearchLimitations
{
	int64_t HardTimeLimit = -1;
	int64_t SoftTimeLimit = -1;
	int64_t SoftNodeLimit = -1;
	int64_t HardNodeLimit = -1;
	SearchLimitations(int hardTime = -1, int softTime = -1, int64_t softNode = -1, int64_t hardNode = -1)
		: HardTimeLimit(hardTime),
		SoftTimeLimit(softTime),
		SoftNodeLimit(softNode),
		HardNodeLimit(hardNode)
	{}
};
void initializeLMRTable(ThreadData& data);
extern TranspositionEntry* TranspositionTable;
std::pair<Move, int> IterativeDeepening(Board& board, int depth, SearchLimitations& searchLimits, ThreadData& data, bool print_info = true, int64_t maxTime = -1);
int SEE(Board& pos, Move move, int threshold);

void bench();
inline bool is_threefold(std::vector<uint64_t> history_table, int last_irreversible)
{
	uint64_t lastmove = history_table[history_table.size() - 1];

	int repetition_count = 1;
	for (int i = history_table.size() - 2; i > last_irreversible; i--)
	{
		if (i < 0) break;

		if (history_table[i] == lastmove)
		{
			repetition_count++;
		}

		if (repetition_count >= 2)
		{
			return true;
		}
	}
	return false;
}
void Datagen(int targetPos, std::string output_name);