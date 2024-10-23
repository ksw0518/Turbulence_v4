
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

int pv_length[99];
Move pv_table[99][99];

int ply;
int negamax_nodecount;
int nodes_for_time_checking;
int Searchtime_MS;
int curr_depth;
bool is_search_stopped;


bool Print_Root = false;
constexpr int MAX_HISTORY = 16384;




static Move last_bestMove[99];
Move killer_moves[2][99];

int history_moves[12][64];
//struct Transposition_entry
//{
//	uint64_t zobrist_key;
//	Move best_move;
//	int depth;
//	int score;
//	int node_type;
//};

uint64_t TT_size = 16;
Transposition_entry* TranspositionTable;

std::vector<int> move_scores;
std::vector<Move> public_movelist;


constexpr int MAX_HISTORY = 16384;
void update_history(int piece, int to, int bonus)
{
	int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);

	//std::cout<< clampedBonus - history_moves[piece][to] * std::abs(clampedBonus) / MAX_HISTORY;
	history_moves[piece][to] += clampedBonus - history_moves[piece][to] * std::abs(clampedBonus) / MAX_HISTORY;


	//history_moves[piece][to] += bonus;
}
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



	/*int victim = get_piece(board.mailbox[move.To], White);
	int attacker = get_piece(move.Piece, White);*/


	//std::cout<<(victim >= 0 && victim < 6);
	//std::cout << (attacker >= 0 && attacker < 6);
	//if (victim > 6 || attacker > 6) std::cout << ("fucked up");
	Transposition_entry& entry = TranspositionTable[board.Zobrist_key % TT_size];

	// Check if the entry is valid and matches the current Zobrist key
	if (entry.node_type != 0 && entry.zobrist_key == board.Zobrist_key)
	{
		// If the best move from TT matches the current move, return a high score
		if (entry.best_move == move)
		{
			return 10000000;
		}
	}
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
		//return mvv_lva[victim][attacker];
		return mvv_lva[attacker][victim] * 10000;
	}
	else
	{
		 if (killer_moves[0][ply] == move)
		 {
			 return 150000;
		 }
		 //2nd killer
		 else if (killer_moves[1][ply] == move)
		 {
			 return 100000;
		 }
		 else
		 {
			 // Return history score for non-capture and non-killer moves



			 //int pieceType = get_piece(move.Piece, White); // Get piece type

			 //int targetSquare = move.To; // Get target square


			 return history_moves[move.Piece][move.To];
		 }
		//1st killer
		//history move
	}

	return 0;
}


bool compareMoves(const Move& move1, const Move& move2, Board& board)
{
	return (get_move_score(move1, board)) > (get_move_score(move2, board));
}


static inline void sort_moves(std::vector <Move>& moves, Board& board)
{
	//int list_size = moves.size();
	//int move_scores[list_size];

	// Sort using a lambda that captures the board reference
	std::sort(moves.begin(), moves.end(), [&board](const Move& move1, const Move& move2) {
		return compareMoves(move1, move2, board);
		});

}

static inline int Quiescence(Board& board, int alpha, int beta)
{
	
	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	if (elapsedMS > Searchtime_MS) {
		is_search_stopped = true;
		return 0; // Return a neutral score if time is exceeded
	}
	if (ply > 99)
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
		int last_irreversible = board.last_irreversible_ply;
		ply++;

		MakeMove(board, move);
		//u64 nodes_added
		if (!isMoveValid(move, board))//isMoveValid(move, board)
		{

			ply--;
			UnmakeMove(board, move, captured_piece);

			board.history.pop_back();
			board.last_irreversible_ply = last_irreversible;
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
			board.history.pop_back();
			board.last_irreversible_ply = last_irreversible;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}

		ply--;
		UnmakeMove(board, move, captured_piece);
		board.history.pop_back();
		board.last_irreversible_ply = last_irreversible;
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
inline Transposition_entry ttLookUp(uint64_t zobrist)
{
	int tt_index = zobrist % TT_size;
	return TranspositionTable[tt_index];
}

inline bool is_in_check(Board &board)
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
static inline int Negamax(Board& board, int depth, int alpha, int beta, bool doNMP)
{
	bool is_pv_node = beta - alpha > 1;
	nodes_for_time_checking++;
	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	if (elapsedMS > Searchtime_MS) {
		is_search_stopped = true;
		return 0; // Return a neutral score if time is exceeded
	}

	pv_length[ply] = ply;
	bool found_pv = false;
	//for (int i = 0; i < board.history.size(); i++)
	//{
	//	std::cout << std::hex << board.history[i] << std::dec << "\n";
	//}
	//std::cout << "\n";

	/*std::cout << "\n" <<board.last_irreversible_ply<<"\n";*/


	
	//std::cout << ("\n    Number :     ") << ;
	//std::cout << ("\n");

	if (is_threefold(board.history, board.last_irreversible_ply))
	{
		
		//std::cout << "rep";
		//PrintBoards(board);

		if (ply != 0)
		{
			return 0;
		}
			
		
		
	}
	//Transposition_entry ttEntry;

	Transposition_entry ttEntry = ttLookUp(board.Zobrist_key);

	// Only check TT for depths greater than zero (ply != 0)
	if (ply != 0 && ttEntry.zobrist_key == board.Zobrist_key)
	{
		// Valid TT entry found
		if (ttEntry.node_type != 0 && ttEntry.depth >= depth)
		{
			// Return immediately if exact score is found
			if (ttEntry.node_type == ExactFlag)
			{
				return ttEntry.score;
			}
			else if (ttEntry.node_type == AlphaFlag && ttEntry.score <= alpha)
			{
				return alpha;
				//alpha = ttEntry.score;
			}
			else if (ttEntry.node_type == BetaFlag && ttEntry.score >= beta )
			{
				return beta;
			}

			//if (alpha >= beta)
			//{
			//	return ttEntry.score;
			//}
		}
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
	//std::vector<uint64_t> last_history;

	//last_history.clear();

	//std::vector<uint64_t> last_history;
	//last_history.clear();
	//last_history.reserve(board.history.size());

	//for (int i = 0; i < board.history.size(); i++)
	//{
	//	last_history.push_back(board.history[i]);
	//}

	int alpha_org = alpha;
	int score = 0;
	int depth_to_search;
	int ttFlag = AlphaFlag;
	//bool isPV = beta - alpha > 1;
	for (Move& move : moveList)
	{
		nodes_for_time_checking++;

		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.last_irreversible_ply;



		ply++;

		MakeMove(board, move);

		//uint64_t zobrist_generated_from_scratch = generate_hash_key(board);

		//if (board.Zobrist_key != zobrist_generated_from_scratch)
		//{
		//	std::cout << "CRITICAL ERROR: zobrist key doesn't match\n";
		//	printMove(move);
		//	std::cout << "ep " << CoordinatesToChessNotation(board.enpassent) << board.enpassent;
		//	std::cout << "\n\n";
		//}


		negamax_nodecount++;
		//u64 nodes_added
		if (!isMoveValid(move, board))//isMoveValid(move, board)
		{

			ply--;
			UnmakeMove(board, move, captured_piece);


			//std::vector<int> copy(last_history.size()); // Pre-allocate space




			//board.history = last_history;

			board.history.pop_back();
			board.last_irreversible_ply = last_irreversible;
			board.Zobrist_key = last_zobrist;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			continue;
		}



		//for (int i = 0; i < board.history.size(); i++)
		//{
		//	std::cout << std::hex << board.history[i] << std::dec << "\n";
		//}

		//printMove(move);
		//
		//std::cout << "\n";

		//std::cout << ply;

		//std::cout << "\n";



		legal_moves++;
		depth_to_search = depth - 1;

		int reduction = 0;
		bool is_reduced = false;

		score = -Negamax(board, depth_to_search, -beta, -alpha, true);


		if (is_search_stopped) {
			ply--;
			UnmakeMove(board, move, captured_piece);

			board.history.pop_back();
			board.last_irreversible_ply = last_irreversible;

			board.Zobrist_key = last_zobrist;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}

		ply--;
		//if (depth == curr_depth)
		//{
		//	if (Print_Root)
		//	{
		//		public_movelist.push_back(move);
		//		move_scores.push_back(score);
		//	}
		//}
//		for (int i = 0; i < board.history.size(); i++)
//{
//	std::cout << std::hex << board.history[i] << std::dec << "\n";
//}
		UnmakeMove(board, move, captured_piece);

		board.history.pop_back();
		board.last_irreversible_ply = last_irreversible;
		board.Zobrist_key = last_zobrist;
		board.enpassent = lastEp;
		board.castle = lastCastle;
		board.side = lastside;



		//std::cout << "\n";
		//printMove(move);
		//std::cout << "\n after_ \n";
		//for (int i = 0; i < board.history.size(); i++)
		//{
		//	std::cout << std::hex << board.history[i] << std::dec << "\n";
		//}
		//std::cout << "\n";
		if (score > bestValue)
		{
			//store history moves

			
			bestValue = score;

		}
		if (bestValue > alpha)
		{
			found_pv = true;
			ttFlag = ExactFlag;
			alpha = score;

			pv_table[ply][ply] = move;


			//copy move from deeper ply into a current ply's line
			for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
			{
				pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
			}

			pv_length[ply] = pv_length[ply + 1];

		}
		if (score >= beta)
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
			ttFlag = BetaFlag;
			if ((move.Type & capture) == 0)
			{
				killer_moves[1][ply] = killer_moves[0][ply];
				killer_moves[0][ply] = move;


				update_history(move.Piece, move.To, depth*depth);
			}
			break;
			//return score;
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
	ttEntry.score = bestValue;
	ttEntry.node_type = ttFlag;
	ttEntry.depth = depth;
	ttEntry.zobrist_key = board.Zobrist_key;
	ttEntry.best_move = pv_table[ply][ply];


	TranspositionTable[board.Zobrist_key % TT_size] = ttEntry;

	return bestValue;
}
int get_hashfull()
{
	int entryCount = 0;
	for (int i = 0; i < 1000; i++)
	{
		if (TranspositionTable[i].node_type != 0)
		{
			entryCount++;
		}
	}

	return entryCount;
}



void IterativeDeepening(Board& board, int depth, int timeMS, bool PrintRootVal)
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
	
	///std::vector<uint64_t> histcopy = board.history;
	//int lastirr = board.last_irreversible_ply;
	int alpha_val = MINUS_INFINITY;
	int beta_val = PLUS_INFINITY;

	int score;
	Print_Root = false;
	//move_scores.clear();
	//public_movelist.clear();

	for (int piece = 0; piece < 12; ++piece)
	{
		for (int square = 0; square < 64; ++square)
		{
			history_moves[piece][square] = 0;
			//history_moves[piece][square] = 0;
		}
	}
	for (curr_depth = 1; curr_depth <= depth; curr_depth++)
	{
		//move_scores.cler();
		//public_movelist.clear();
		ply = 0;

		nodes_for_time_checking = 0;
		is_search_stopped = false;
		//memset(last_bestMove, 0, 64 * sizeof(int));

		if (curr_depth >= 4)
		{
			alpha_val = score - 25;
			beta_val = score + 25;
		}
		//for (int i = 0; i < 99; ++i) {
		//	for (int j = 0; j < 99; ++j) {
		//		pv_table[i][j] = Move();  // Calls the default constructor
		//	}
		//}
		//if (curr_depth == depth)
		//{
		//	Print_Root = PrintRootVal;
		//}
		score = Negamax(board, curr_depth, alpha_val, beta_val, true);

		//int last_alpha = alpha_val;
		//int last_beta = beta_val;

		// Continue re-searching until the search is successful
		while (score <= alpha_val || score >= beta_val)
		{
			if (score <= alpha_val) // Failed low
			{
				alpha_val -= 50; // Widen alpha by decreasing it
			}
			else if (score >= beta_val) // Failed high
			{
				beta_val += 50; // Widen beta by increasing it
			}

			// Perform the search again with the widened window
			//move_scores.clear();
			//public_movelist.clear();
			score = Negamax(board, curr_depth, alpha_val, beta_val, true);
			//std::cout << "Window widened: [" << alpha_val << ", " << beta_val << "]" << std::endl;
		}

		//if (Print_Root)
		//{
		//	//public_movelist.clear();
		//	//Generate_Legal_Moves(public_movelist, board, false);
		//	//sort_moves_rootprint(movelist);
		//	std::cout << public_movelist.size()<<"\n";
		//	std::cout << move_scores.size() << "\n";
		//	for (int i = 0; i < public_movelist.size(); i++)
		//	{
		//		printMove(public_movelist[i]);
		//		std::cout << " : " << move_scores[i]<<"\n";
		//	}
		//}
		//else//succeed
		//{

		//}
		//board.history = histcopy;
		// 
		//board.last_irreversible_ply = lastirr;

		auto end = std::chrono::steady_clock::now();

		float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		//std::chrono::duration<double, std::milli> elapsedS = end - start;

		float second = (elapsedMS + 1) / 1000;

		double nps = negamax_nodecount / second;
		double nps_in_millions = nps / 1000000.0;

		if (!is_search_stopped)
		{
			bestmove = pv_table[0][0];
			int hashfull = get_hashfull();
			last_bestMove[curr_depth - 1] = bestmove;
			std::cout << "info depth " << curr_depth;
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
			
			std::cout << " time " << static_cast<int>(std::round(elapsedMS)) << " nodes " << negamax_nodecount << " nps " << static_cast<int>(std::round(nps)) << " hashfull " << hashfull << " pv ";
			for (int count = 0; count < pv_length[0]; count++)
			{
				printMove(pv_table[0][count]);
				std::cout << " ";
			}
		}


		

		if (elapsedMS > Searchtime_MS)
		{
			//std::cout << elapsedMS << "\n";
			//std::cout << Searchtime_MS << "\n";
			break;
		}
		std::cout << "\n";
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
