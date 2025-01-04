
#include "MoveGeneration.h"
#include "Evaluation.h"
#include "Board.h"
#include "Search.h"
#include "BitManipulation.h"
#include "const.h"
#define NOMINMAX
#include <windows.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>

int RFP_MULTIPLIER = 85;
int RFP_BASE = -49;

int LMP_BASE = 0;
int LMP_MULTIPLIER = 1;

int PVS_QUIET_BASE = 0;
int PVS_QUIET_MULTIPLIER = 63;

int PVS_NOISY_BASE = -1;
int PVS_NOISY_MULTIPLIER = 18;

int HISTORY_BASE = 4;
int HISTORY_MULTIPLIER = 2;

std::chrono::steady_clock::time_point SearchStartTime;
bool isSearchStopped;
int ply; 
int TotalNodes;


Move pvTable[MAXDEPTH][MAXDEPTH];
int pvLength[MAXDEPTH];
static int mvv_lva[6][6] = {
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};
static inline int get_move_score(Move move, Board& board)
{
	if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{
		int victim = get_piece(board.mailbox[move.To], White);
		if (move.Type == ep_capture)
		{
			victim = P;
		}
		int attacker = get_piece(move.Piece, White);
		int score = mvv_lva[attacker][victim];
		//score += CaptureHistory[move.Piece][move.To][board.mailbox[move.To]] / 10;
		//score += SEE(board, move, -100) ? 200000 : -10000000;
		return score;


	}
	else
	{
	//	if (killer_moves[0][ply] == move)
	//	{
	//		return 150000;
	//	}
	//	//2nd killer
	//	else if (killer_moves[1][ply] == move)
	//	{
	//		return 100000;
	//	}
	//	//else if (counter_move[Search_stack[ply - 1].move.From][Search_stack[ply - 1].move.To] == move)
	//	//{
	//	//	return 90000;
	//	//}
	//	else
	//	{
	//		// Return history score for non-capture and non-killer moves



	//		//int pieceType = get_piece(move.Piece, White); // Get piece type

	//		//int targetSquare = move.To; // Get target square
	//		int main_history = history_moves[board.side][move.From][move.To];
	//		int oneply_conthist = getContinuationHistoryScore(move);
	//		//int oneply_conthist = getContinuationHistoryScore(move);
	//		//int oneply_conthist = 0;
	//		int history = main_history + oneply_conthist - 100000;

	//		if (history >= 80000)
	//		{
	//			return 80000;
	//		}
	//		else
	//		{
	//			return history;
	//		}


	//	}
		//1st killer
		//history move
	}

	return 0;
}
static inline void sort_moves(std::vector<Move>& moves, Board& board) {
	// Precompute scores for all moves
	std::vector<std::pair<int, Move>> scored_moves;
	scored_moves.reserve(moves.size());
	for (const Move& move : moves) {
		int score = get_move_score(move, board);
		scored_moves.emplace_back(score, move);
	}

	// Sort the scored moves based on the scores
	std::sort(scored_moves.begin(), scored_moves.end(), [](const auto& a, const auto& b) {
		return a.first > b.first; // Sort by score (descending)
		});

	// Rebuild the original moves vector in sorted order
	for (size_t i = 0; i < moves.size(); ++i) {
		moves[i] = scored_moves[i].second;
	}
}


int NegaMax(Board& pos, int depth, int alpha, int beta, int hardBound)
{
	pvLength[ply] = ply;
	auto CurrentTime = std::chrono::steady_clock::now();
	int elapsedMS = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - SearchStartTime).count());

	if (elapsedMS >= hardBound)
	{
		isSearchStopped = true;
	}
	if (depth <= 0)
	{
		return Evaluate(pos);
	}

	int bestScore = -INF;

	std::vector<Move> MoveList;
	MoveList.reserve(256);
	Generate_Legal_Moves(MoveList, pos, false);
	//sort_moves(MoveList, pos);


	
	for (size_t i = 0; i < MoveList.size(); i++) {
		Move CurrentMove = MoveList[i];

		//saves part of the board for unmake
		uint64_t lastZobrist = pos.Zobrist_key;
		int lastEp = pos.enpassent;
		uint64_t lastCastle = pos.castle;
		int lastSide = pos.side;
		int lastIrreversible = pos.last_irreversible_ply;
		int capturedPiece = pos.mailbox[CurrentMove.To];

		
		MakeMove(pos, CurrentMove);

		ply++;
		

		if (!isMoveValid(CurrentMove, pos))//The move we made is illegal, have to undo
		{
			ply--;
			UnmakeMove(pos, CurrentMove, capturedPiece);

			pos.history.pop_back();
			pos.last_irreversible_ply = lastIrreversible;
			pos.Zobrist_key = lastZobrist;
			pos.enpassent = lastEp;
			pos.castle = lastCastle;
			pos.side = lastSide;

			continue;
		}

		int depthToSearch = depth - 1;
		int childValue = -NegaMax(pos, depthToSearch, -beta, -alpha, hardBound);
		TotalNodes++;

		ply--;
		UnmakeMove(pos, CurrentMove, capturedPiece);

		pos.history.pop_back();
		pos.last_irreversible_ply = lastIrreversible;
		pos.Zobrist_key = lastZobrist;
		pos.enpassent = lastEp;
		pos.castle = lastCastle;
		pos.side = lastSide;
		bestScore = std::max(childValue, bestScore);
		if (bestScore > alpha)//alpha was raised
		{
			//update pv table
			pvTable[ply][ply] = CurrentMove;
			
			//printMove(pvTable[0][0]);
			for (int next_ply = ply + 1; next_ply < pvLength[ply + 1]; next_ply++)
			{
				pvTable[ply][next_ply] = pvTable[ply + 1][next_ply];
			}
			pvLength[ply] = pvLength[ply + 1];
			//std::cout << ply;

			alpha = childValue;
		}
		if (alpha >= beta)//beta cutoff
		{


			break;
		}


		

		
	}
	return bestScore;
}
void printSearchInfo(int depth, int score, int timeMS, int nodes, Move(&pvTable)[MAXDEPTH][MAXDEPTH])
{
	std::cout << "info depth ";
	std::cout << depth;
	std::cout << " seldepth ";
	std::cout << depth;
	if (std::abs(score) > MATE - 1000) // score is lower than Mate in 1000, which is impossible
	{
		int matePly = 49000 - std::abs(score);
		int mateInFullMove = std::ceil(static_cast<double>(matePly) / 2);

		if (score < 0)
		{
			mateInFullMove *= -1;
		}
		std::cout << " score mate " << mateInFullMove;

	}
	else
	{
		std::cout << " score cp " << score;
	}
	std::cout << " time " << timeMS;
	std::cout << " nodes " << nodes;
	std::cout << " nps " << static_cast<int>(nodes / (((timeMS)/1000) + 1));
	std::cout << " hashfull " << 0;
	std::cout << " pv ";
	//printMove(pvTable[0][0]);
	//std::cout << pvLength[0];
	for (int moveNum = 0; moveNum < pvLength[0]; moveNum++)
	{
		printMove(pvTable[0][moveNum]);
		std::cout << " ";
	}
	std::cout << "\n";
}
void startSearch(const Board& pos, int hardBound, int softBound, int maxDepth)
{
	if (hardBound == -1)
	{
		hardBound = INT_MAX;
	}
	if (softBound == -1)
	{
		softBound = INT_MAX;
	}

	SearchStartTime = std::chrono::steady_clock::now();
	isSearchStopped = false;

	TotalNodes = 0;
	Board currentPos = pos;
	int Score = NOSCORE;
	Move previousBestMove;
	memset(pvLength, 0, sizeof(pvLength));
	for (int depth = 1; depth < maxDepth + 1; depth++)
	{
		ply = 0;
		Score = NegaMax(currentPos, depth, -INF, INF, hardBound);
		if (!isSearchStopped)//current search was successfully finished
		{
			auto CurrentTime = std::chrono::steady_clock::now();
			int elapsedMS = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - SearchStartTime).count());
			printSearchInfo(depth, Score, elapsedMS, TotalNodes, pvTable);
			previousBestMove = pvTable[0][0];
		}

	}
}