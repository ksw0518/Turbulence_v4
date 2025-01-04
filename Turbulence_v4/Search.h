#pragma once

#include "Board.h"
constexpr int INF = 50000;
constexpr int MATE = 49000;
constexpr int NOSCORE = 2000000;
constexpr int MAXDEPTH = 256;

extern int RFP_MULTIPLIER;
extern int RFP_BASE;

extern int LMP_BASE;
extern int LMP_MULTIPLIER;

extern int PVS_QUIET_BASE;
extern int PVS_QUIET_MULTIPLIER;

extern int PVS_NOISY_BASE;
extern int PVS_NOISY_MULTIPLIER;

extern int HISTORY_BASE;
extern int HISTORY_MULTIPLIER;
//extern int pv_length[99];
//extern Move pv_table[99][99];
//
//
//extern int RFP_MULTIPLIER;
//extern int RFP_BASE;
//
//extern int LMP_BASE;
//extern int LMP_MULTIPLIER;
//
//extern int PVS_QUIET_BASE;
//extern int PVS_QUIET_MULTIPLIER;
//
//extern int PVS_NOISY_BASE;
//extern int PVS_NOISY_MULTIPLIER;
//
//extern int HISTORY_BASE;
//extern int HISTORY_MULTIPLIER;
//
//extern uint64_t TT_size;
// 
// 
void startSearch(const Board& pos, int hardBound = -1, int softBound = -1, int maxDepth = 256);
//
//extern bool is_Pretty_Printing;
//struct Transposition_entry
//{
//	uint64_t zobrist_key;
//	int32_t score;
//	Move best_move;
//	uint8_t depth;
//	
//	uint8_t node_type;
//};
//
//struct Search_data
//{
//	Move move;
//
//};
//void initializeLMRTable();
//extern Transposition_entry* TranspositionTable;
////void IterativeDeepening(Board& board, int depth, int timeMS = -1, bool PrintRootVal = false, bool print_info = true, int softbound = -1);
//int SEE(Board& pos, Move move, int threshold);
//
//void bench();
//inline bool is_threefold(std::vector<uint64_t> history_table, int last_irreversible)
//{
//	//has to be called after allocating new position move
//
//	uint64_t lastmove = history_table[history_table.size() - 1];
//
//	//std::cout << lastmove; 
//	//printMove(lastmove);
//	int repetition_count = 1;
//	for (int i = history_table.size() - 2; i > last_irreversible; i--)
//	{
//		if (i < 0) break;
//
//		if (history_table[i] == lastmove)
//		{
//			repetition_count++;
//		}
//
//		if (repetition_count >= 2)
//		{
//			return true;
//		}
//	}
//	return false;
//}
inline bool is_threefold(std::vector<uint64_t> history_table, int last_irreversible)
{
	//has to be called after allocating new position move

	uint64_t lastmove = history_table[history_table.size() - 1];

	//std::cout << lastmove; 
	//printMove(lastmove);
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