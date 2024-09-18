
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


constexpr int UNIT_EVERYNODE = 8000; //check for things like time bound every 4096 nodes to balance speed and accuracy

auto start = std::chrono::steady_clock::now();

int pv_length[64];
Move pv_table[64][64];

int ply;
int negamax_nodecount;
int nodes_for_time_checking;
int Searchtime_MS;
int curr_depth;
bool is_search_stopped;
constexpr int MAX_HISTORY = 4096;

static int mvv_lva[6][6] = {
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};

static Move last_bestMove[64];
Move killer_moves[2][64];

int history_moves[12][64];



static inline int get_move_score(Move move, Board& board)
{




	/*int victim = get_piece(board.mailbox[move.To], White);
	int attacker = get_piece(move.Piece, White);*/
	

	//std::cout<<(victim >= 0 && victim < 6);
	//std::cout << (attacker >= 0 && attacker < 6);
	//if (victim > 6 || attacker > 6) std::cout << ("fucked up");
	if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{
		if (move.Type == ep_capture)
		{
			return mvv_lva[P][P];
		}

		int victim = get_piece(board.mailbox[move.To], White);
		int attacker = get_piece(move.Piece, White);
		//score moves based on mvv-lva scheme
		//return 0;

		//std::cout << board.mailbox[move.To] << "\n";
		//std::cout << attacker << "\n";
		return mvv_lva[attacker][victim];
	}
	else
	{
		//1st killer
		if (last_bestMove[curr_depth - 2] == move)
		{
			return 10000;
		}
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
			//return history_moves[move.Piece][move.To];
			/*if (last_bestMove[curr_depth - 2] == move)
			{
				return 10000;
			}
			else
			{
				
			}*/
			
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

static int Quiescence(Board& board, int alpha, int beta)
{
	
	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	if (elapsedMS > Searchtime_MS) {
		is_search_stopped = true;
		return 0; // Return a neutral score if time is exceeded
	}
	if (ply > 63)
	{
		//std::cout << "fuck";
		return Evaluate(board);
	}
		// evaluate position
		//std::cout << "fuck";
		

	int evaluation = Evaluate(board);

	if (evaluation >= beta)
	{
		return evaluation;
	}

	if (evaluation > alpha)
	{
		alpha = evaluation;
	}

	std::vector<Move> moveList;
	Generate_Legal_Moves(moveList, board, false);

	sort_moves(moveList, board);

	int bestValue = MINUS_INFINITY;
	int legal_moves = 0;
	for (Move& move : moveList)
	{
		if ((captureFlag & move.Type) == 0) continue; //skip non capture moves
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
		int score = -Quiescence(board, -beta, -alpha);

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

				//pv_table[ply][ply] = move;


				////copy move from deeper ply into a current ply's line
				//for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
				//{
				//	pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
				//}

				//pv_length[ply] = pv_length[ply + 1];
			}
		}
		if (score >= beta)
		{
			//killer_moves[1][ply] = killer_moves[0][ply];
			//killer_moves[0][ply] = move;
			return score;
		}
	}

	if (legal_moves == 0) // quiet position
	{
		return evaluation;
	}
	return alpha;
	//negamax_nodecount++;
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
	//for (int i = 0; i < board.history.size(); i++)
	//{
	//	std::cout << std::hex << board.history[i] << std::dec << "\n";
	//}
	//std::cout << "\n";

	if (is_threefold(board.history))
	{
		
		//std::cout << "rep";
		//PrintBoards(board);


			return 0;
		
		
	}
	if (depth == 0)
	{
		return Quiescence(board, alpha, beta);
		//return Evaluate(board);
	}

	
	int bestValue = MINUS_INFINITY;
	std::vector<Move> moveList;
	Generate_Legal_Moves(moveList, board, false);

	int legal_moves = 0;

	//printMoveSort(board);
	//PrintBoards(board);

	sort_moves(moveList, board);

	uint64_t last_zobrist = board.Zobrist_key;
	std::vector<uint64_t> last_history;

	last_history.clear();
	last_history.reserve(board.history.size());

	for (int i = 0; i < board.history.size(); i++)
	{
		last_history.push_back(board.history[i]);
	}

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


			//std::vector<int> copy(last_history.size()); // Pre-allocate space

			board.history.clear();
			for (int i = 0; i < last_history.size(); i++)
			{
				board.history.push_back(last_history[i]);
			}
			//board.history = last_history;
			board.Zobrist_key = last_zobrist;
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
			board.history.clear();
			for (int i = 0; i < last_history.size(); i++)
			{
				board.history.push_back(last_history[i]);
			}

			board.Zobrist_key = last_zobrist;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}

		ply--;
		UnmakeMove(board, move, captured_piece);

		board.history.clear();
		for (int i = 0; i < last_history.size(); i++)
		{
			board.history.push_back(last_history[i]);
		}
		board.Zobrist_key = last_zobrist;
		board.enpassent = lastEp;
		board.castle = lastCastle;
		board.side = lastside;
		if (score > bestValue)
		{
			//store history moves

			
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
			if ((move.Type & captureFlag) == 0) //non capture
			{
				//int clampedBonus = std::clamp(depth, MAX_HISTORY, -MAX_HISTORY);
				//history_moves[move.Piece][move.To] += clampedBonus - history_moves[move.Piece][move.To] * std::abs(clampedBonus) / MAX_HISTORY;
				
				
				
				//if (negamax_nodecount % 1000000 == 0) { // For example, after every million nodes
				//	for (int piece = 0; piece < 12; ++piece) {
				//		for (int square = 0; square < 64; ++square) {
				//			history_moves[piece][square] /= 10; // Halve the history values
				//		}
				//	}
				//}


				killer_moves[1][ply] = killer_moves[0][ply];
				killer_moves[0][ply] = move;
			}
			return score;
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
	
	for (curr_depth = 1; curr_depth <= depth; curr_depth++)
	{
		ply = 0;

		nodes_for_time_checking = 0;
		is_search_stopped = false;
		memset(last_bestMove, 0, 64 * sizeof(int));
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
			last_bestMove[curr_depth - 1] = bestmove;
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

void printMoveSort(Board board)
{
	std::vector<Move> movelist;
	Generate_Legal_Moves(movelist,board, false);

	sort_moves(movelist, board);

	for (int i = 0; i < movelist.size(); i++)
	{
		printMove(movelist[i]);
		std::cout << " "<<get_move_score(movelist[i], board);
		std::cout << "\n";
	}
	//PrintLegalMoves(movelist);
}
