
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
std::string bench_fens[] = { // fens from alexandria, ultimately from bitgenie
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
Move killerMoves[2][99];
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
		return score + 20000000;


	}
	else
	{
		if (killerMoves[0][ply] == move)
		{
			return 150000;
		}
		//2nd killer
		else if (killerMoves[1][ply] == move)
		{
			return 100000;
		}
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

bool isQuiet(int moveType)
{
	if ((moveType & captureFlag) == 0)
	{
		return true;
	}
	return false;
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

			if (isQuiet(CurrentMove.Type))
			{
				if (!(killerMoves[0][ply] == CurrentMove))
				{
					killerMoves[1][ply] = killerMoves[0][ply];
					killerMoves[0][ply] = CurrentMove;
				}
			}
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
void startSearch(const Board& pos, int hardBound, int softBound, int maxDepth, bool printInfo)
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
			previousBestMove = pvTable[0][0];
			if (printInfo)
			{
				auto CurrentTime = std::chrono::steady_clock::now();
				int elapsedMS = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - SearchStartTime).count());
				printSearchInfo(depth, Score, elapsedMS, TotalNodes, pvTable);
				
			}

		}
		else
		{
			break;
		}
		
	}
	if (printInfo)
	{
		std::cout << "bestmove ";
		printMove(previousBestMove);
		std::cout << "\n";
	}

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
		for (int i = 0; i < 99; i++)
		{
			killerMoves[0][i] == Move(0, 0, 0, 0);
			killerMoves[1][i] == Move(0, 0, 0, 0);
		}
		parse_fen(bench_fens[i], board);
		board.Zobrist_key = generate_hash_key(board);
		board.history.push_back(board.Zobrist_key);

		search_start = std::chrono::steady_clock::now();
		startSearch(board, -1, -1, 4, false);
		search_end = std::chrono::steady_clock::now();

		float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(search_end - search_start).count();
		//std::chrono::duration<double, std::milli> elapsedS = end - start;

		float second = (elapsedMS + 1) / 1000;

		double nps = TotalNodes / second;


		nodecount += TotalNodes;
		totalsearchtime += std::floor(elapsedMS);



	}
	std::cout << nodecount << " nodes " << nodecount / (totalsearchtime + 1) * 1000 << " nps " << "\n";


}