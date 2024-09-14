
#include "MoveGeneration.h"
#include "Evaluation.h"
#include "Board.h"
#include "Search.h"
#include "BitManipulation.h"
#include "const.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>

constexpr int UNIT_EVERYNODE = 4096; //check for things like time bound every 4096 nodes to balance speed and accuracy

auto start = std::chrono::steady_clock::now();

int pv_length[64];
Move pv_table[64][64];

int ply;
int negamax_nodecount;
int nodes_for_time_checking;
int Searchtime_MS;

bool is_search_stopped;

static int mvv_lva[6][6] = {
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};

Move killer_moves[2][64];



static inline int get_move_score(Move move, Board& board)
{

	if (move.Type == ep_capture)
	{
		return mvv_lva[P][P];
	}

	/*int victim = get_piece(board.mailbox[move.To], White);
	int attacker = get_piece(move.Piece, White);*/


	//std::cout<<(victim >= 0 && victim < 6);
	//std::cout << (attacker >= 0 && attacker < 6);
	//if (victim > 6 || attacker > 6) std::cout << ("fucked up");
	if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{
		int victim = get_piece(board.mailbox[move.To], White);
		int attacker = get_piece(move.Piece, White);
		//score moves based on mvv-lva scheme
		//return 0;

		//std::cout << board.mailbox[move.To] << "\n";
		//std::cout << attacker << "\n";
		return mvv_lva[victim][attacker];
	}
	else
	{
		//1st killer
		if (killer_moves[0][ply] == move)
		{
			return 9000;
		}
		//2nd killer
		else if (killer_moves[1][ply] == move)
		{
			return 8000;
		}
		else
		{
			//return history score
		}
		// 
		//history move
	}

	return 0;
}
bool compareMoves(const Move& move1, const Move& move2, Board &board)
{
	return (get_move_score(move1, board)) > (get_move_score(move2, board));
}


static inline void sort_moves(std::vector <Move> &moves, Board& board)
{
	//int list_size = moves.size();
	//int move_scores[list_size];

	// Sort using a lambda that captures the board reference
	std::sort(moves.begin(), moves.end(), [&board](const Move& move1, const Move& move2) {
		return compareMoves(move1, move2, board);
		});

}



static int Negamax(Board& board, int depth, int alpha, int beta)
{
	nodes_for_time_checking++;
	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	if (elapsedMS > Searchtime_MS) {
		is_search_stopped = true;
		return 0; // Return a neutral score if time is exceeded
	}

	pv_length[ply] = ply;
	if (depth == 0)
	{
		return Evaluate(board);
	}

	
	int bestValue = MINUS_INFINITY;
	std::vector<Move> moveList;
	Generate_Legal_Moves(moveList, board, false);

	int legal_moves = 0;
	sort_moves(moveList, board);
	for (Move& move : moveList)
	{
		nodes_for_time_checking++;

		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];

		ply++;

		MakeMove(board, move);
		//u64 nodes_added
		if (!isMoveValid(move, board))//isMoveValid(move, board)
		{
			
			ply--;
			UnmakeMove(board, move, captured_piece);

			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			continue;
		}

		negamax_nodecount++;
		







		legal_moves++;
		int score = -Negamax(board, depth - 1, -beta, -alpha);

		if (is_search_stopped) {
			UnmakeMove(board, move, captured_piece);
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}

		ply--;
		UnmakeMove(board, move, captured_piece);

		board.enpassent = lastEp;
		board.castle = lastCastle;
		board.side = lastside;
		if (score > bestValue)
		{
			bestValue = score;
			if (score > alpha)
			{
				alpha = score;

				pv_table[ply][ply] = move;


				//copy move from deeper ply into a current ply's line
				for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
				{
					pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
				}

				pv_length[ply] = pv_length[ply + 1];
			}
		}
		if (score >= beta)
		{
			killer_moves[1][ply] = killer_moves[0][ply];
			killer_moves[0][ply] = move;
			return beta;
		}

	}
	if (legal_moves == 0)
	{
		if (is_square_attacked(get_ls1b(board.side == White ? board.bitboards[K] : board.bitboards[k]), 1 - board.side, board, board.occupancies[Both]))
		{
			return -49000 + ply;
		}
		else
		{
			return 0;
		}
	}
	return bestValue;
}
void IterativeDeepening(Board& board, int depth, int timeMS)
{
	if (timeMS == -1)
	{
		Searchtime_MS = INT_MAX;
	}
	else
	{
		Searchtime_MS = timeMS;
	}
	
	negamax_nodecount = 0;
	Move bestmove;
	start = std::chrono::steady_clock::now();
	for (int curr_depth = 1; curr_depth <= depth; curr_depth++)
	{
		ply = 0;

		nodes_for_time_checking = 0;
		is_search_stopped = false;

		int score = Negamax(board, curr_depth, MINUS_INFINITY, PLUS_INFINITY);

		auto end = std::chrono::steady_clock::now();

		float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		//std::chrono::duration<double, std::milli> elapsedS = end - start;

		float second = (elapsedMS + 1) / 1000;

		double nps = negamax_nodecount / second;
		double nps_in_millions = nps / 1000000.0;

		if (!is_search_stopped)
		{
			bestmove = pv_table[0][0];
			std::cout << "info depth " << curr_depth << " score cp " << score << " time " << static_cast<int>(std::round(elapsedMS)) << " nodes " << negamax_nodecount << " nps " << static_cast<int>(std::round(nps)) << " pv ";
			for (int count = 0; count < pv_length[0]; count++)
			{
				printMove(pv_table[0][count]);
				std::cout << " ";
			}
		}


		std::cout << "\n";

		if (elapsedMS > Searchtime_MS)
		{
			//std::cout << elapsedMS << "\n";
			//std::cout << Searchtime_MS << "\n";
			break;
		}
	}
	//auto end = std::chrono::high_resolution_clock::now();

	std::cout << "bestmove ";
	printMove(bestmove);
	std::cout << "\n";


}


