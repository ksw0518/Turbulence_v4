#pragma once

#include "Board.h"

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

extern uint64_t TTSize;

extern bool isPrettyPrinting;

extern double DEF_TIME_MULTIPLIER;
extern double DEF_INC_MULTIPLIER;
extern double MAX_TIME_MULTIPLIER;
extern double HARD_LIMIT_MULTIPLIER;
extern double SOFT_LIMIT_MULTIPLIER;
struct TranspositionEntry
{
	uint64_t zobristKey;
	int32_t score;
	Move bestMove;
	uint8_t depth;
	
	uint8_t bound;
};

struct Search_data
{
	Move move;
	int staticEval;
	int doubleExtensions = 0;
};
void initializeLMRTable();
extern TranspositionEntry* TranspositionTable;
void IterativeDeepening(Board& board, int depth, int timeMS = -1, bool PrintRootVal = false, bool print_info = true, int softbound = -1, int = -1, int = -1);
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
