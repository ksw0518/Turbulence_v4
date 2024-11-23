#pragma once

#include "Board.h"

extern int pv_length[99];
extern Move pv_table[99][99];

extern uint64_t TT_size;
struct Transposition_entry
{
	uint64_t zobrist_key;
	Move best_move;
	int depth;
	int score;
	int node_type;
};
void initializeLMRTable();
extern Transposition_entry* TranspositionTable;
void IterativeDeepening(Board& board, int depth, int timeMS = -1, bool PrintRootVal = false, bool print_info = true);
int SEE(Board& pos, Move move, int threshold);
//void printMoveSort(Board board);
void bench();
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
