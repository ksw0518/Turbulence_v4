
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
constexpr int UNIT_EVERYNODE = 8000; //check for things like time bound every 4096 nodes to balance speed and accuracy

constexpr int asp_window_initial = 40;
constexpr int asp_window_max = 300;

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
//constexpr int MAX_HISTORY = 16384;


int seldepth = 0;
int SEEPieceValues[] = { 98, 280, 295, 479, 1064, 0, 0 };
static Move last_bestMove[99];
Move killer_moves[2][99];
Move counter_move[64][64];

int history_moves[2][64][64];
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


constexpr int MAX_HISTORY = 512;

constexpr int Minimum_lmr_depth = 3;

constexpr int Maximum_pvs_see_depth = 8;
int lmrTable[99][256];

bool is_quiet(int type)
{
	if (((type & captureFlag) == 0) && ((type & promotionFlag) == 0)) //quiet move
	{
		return true;
	}
	else
	{
		return false;
	}

}
void initializeLMRTable() {
	for (int depth = 0; depth < 99; depth++) {
		for (int move = 0; move < 256; move++) {
			lmrTable[depth][move] = std::floor(0.77 + log(move) * log(depth) / 2.36);
		}
	}
}
int scaledBonus(int score, int bonus)
{
	return std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY) - (score * abs(bonus) / MAX_HISTORY);
}
void update_history(int stm, int from, int to, int bonus)
{
	//int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);

	//std::cout<< clampedBonus - history_moves[piece][to] * std::abs(clampedBonus) / MAX_HISTORY;
	//history_moves[piece][to] += clampedBonus - history_moves[piece][to] * std::abs(clampedBonus) / MAX_HISTORY;
	int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
	history_moves[stm][from][to] += clampedBonus - history_moves[stm][from][to] * abs(clampedBonus) / MAX_HISTORY;
	//history_moves[stm][from][to] = std::clamp(history_moves[stm][from][to], -MAX_HISTORY, MAX_HISTORY);
	//history_moves[stm][from][to] += bonus;
}
static int mvv_lva[6][6] = {
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};
void printTopHistory(int side) {
	std::vector<std::tuple<int, int, int>> moves; // Stores <score, from, to>

	// Populate the vector with scores and moves
	for (int from = 0; from < 64; ++from) {
		for (int to = 0; to < 64; ++to) {
			int score = history_moves[side][from][to];
			moves.push_back({ score, from, to });
		}
	}

	// Sort moves by score in descending order
	std::sort(moves.rbegin(), moves.rend(), [](auto& a, auto& b) {
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
static inline int get_move_score(Move move, Board& board, Transposition_entry &entry)
{



	/*int victim = get_piece(board.mailbox[move.To], White);
	int attacker = get_piece(move.Piece, White);*/


	//std::cout<<(victim >= 0 && victim < 6);
	//std::cout << (attacker >= 0 && attacker < 6);
	//if (victim > 6 || attacker > 6) std::cout << ("fucked up");
	// Transposition_entry& entry = TranspositionTable[board.Zobrist_key % TT_size];

	// Check if the entry is valid and matches the current Zobrist key
	if (entry.node_type != 0 && entry.zobrist_key == board.Zobrist_key)
	{
		// If the best move from TT matches the current move, return a high score
		if (entry.best_move == move)
		{
			//make sure TT move isn't included in search, because it is already searched before generating move
			return 99999999;
		}
	}
	 if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{			 if (move.Type == ep_capture)
			 {
				 return mvv_lva[P][P] * 10000;
			 }
			 int victim = get_piece(board.mailbox[move.To], White);
			 int attacker = get_piece(move.Piece, White);
			 //score moves based on mvv-lva scheme
			 //return 0;

			 //std::cout << board.mailbox[move.To] << "\n";
			 //std::cout << attacker << "\n";
			 //return mvv_lva[victim][attacker];
			 return mvv_lva[attacker][victim] * 10000;
		 //if (SEE(board, move, -50))
		 //{

		 //}
		 //else
		 //{
			// if (move.Type == ep_capture)
			// {
			//	 return mvv_lva[P][P] * 10000 /6 -  40000;
			// }
			// int victim = get_piece(board.mailbox[move.To], White);
			// int attacker = get_piece(move.Piece, White);
			// //score moves based on mvv-lva scheme
			// //return 0;

			// //std::cout << board.mailbox[move.To] << "\n";
			// //std::cout << attacker << "\n";
			// //return mvv_lva[victim][attacker];
			// return mvv_lva[attacker][victim] * 10000 /6 - 40000;
		 //}

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

			 int history = history_moves[board.side][move.From][move.To] - 100000;

			 if (history >= 99999)
			 {
				 return 99999;
			 }
			 else
			 {
				 return history;
			 }
			 

		 }
		//1st killer
		//history move
	}

	return 0;
}
static inline int get_move_score_capture(Move move, Board& board)
{



	/*int victim = get_piece(board.mailbox[move.To], White);
	int attacker = get_piece(move.Piece, White);*/


	//std::cout<<(victim >= 0 && victim < 6);
	//std::cout << (attacker >= 0 && attacker < 6);
	//if (victim > 6 || attacker > 6) std::cout << ("fucked up");
	//Transposition_entry& entry = TranspositionTable[board.Zobrist_key % TT_size];

	// Check if the entry is valid and matches the current Zobrist key

	if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{
		if (move.Type == ep_capture)
		{
			return mvv_lva[P][P] * 10000;
		}
		int victim = get_piece(board.mailbox[move.To], White);
		int attacker = get_piece(move.Piece, White);
		//score moves based on mvv-lva scheme
		//return 0;

		//std::cout << board.mailbox[move.To] << "\n";
		//std::cout << attacker << "\n";
		//return mvv_lva[victim][attacker];
		return mvv_lva[attacker][victim] * 10000;
		//if (SEE(board, move, -50))
		//{

		//}
		//else
		//{
		   // if (move.Type == ep_capture)
		   // {
		   //	 return mvv_lva[P][P] * 10000 /6 -  40000;
		   // }
		   // int victim = get_piece(board.mailbox[move.To], White);
		   // int attacker = get_piece(move.Piece, White);
		   // //score moves based on mvv-lva scheme
		   // //return 0;

		   // //std::cout << board.mailbox[move.To] << "\n";
		   // //std::cout << attacker << "\n";
		   // //return mvv_lva[victim][attacker];
		   // return mvv_lva[attacker][victim] * 10000 /6 - 40000;
		//}

	}
	else
	{


		return -999;
	}
}
//bool compareMoves_captures(const Move& move1, const Move& move2, Board& board)
//{
//	return (get_move_score_capture(move1, board)) > (get_move_score_capture(move2, board));
//}
//
//bool compareMoves(const Move& move1, const Move& move2, Board& board)
//{
//	return (get_move_score(move1, board)) > (get_move_score(move2, board));
//}

bool is_promotion(int type)
{
	return  (type & promotionFlag) != 0;
}

int get_piece_promoting(int type, int side)
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
int move_estimated_value(Board &board, Move move) {

	// Start with the value of the piece on the target square
	int target_piece = board.mailbox[move.To] > 5
		? board.mailbox[move.To] - 6
		: board.mailbox[move.To];

	int promoted_piece = get_piece_promoting(move.Type, White);
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

int SEE(Board& pos, Move move, int threshold) {

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

static inline void sort_moves_captures(std::vector<Move>& moves, Board& board) {
	// Partition to segregate capture moves
	auto capture_end = std::partition(moves.begin(), moves.end(), [](const Move& move) {
		return move.Type & captureFlag;
		});

	// Sort only the capture moves
	std::sort(moves.begin(), capture_end, [&board](const Move& move1, const Move& move2) {
		return get_move_score_capture(move1, board) > get_move_score_capture(move2, board);
		});

	// Remove non-capture moves (optional, if you don't want them in the vector)
	moves.erase(capture_end, moves.end());
}



static inline void sort_moves(std::vector<Move>& moves, Board& board, Transposition_entry &tt_entry) {
    // Precompute scores for all moves
    std::vector<std::pair<int, Move>> scored_moves;
    scored_moves.reserve(moves.size());
    for (const Move& move : moves) {
        int score = get_move_score(move, board, tt_entry);
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
	int score = 0;

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

	sort_moves_captures(moveList, board);

	int bestValue = MINUS_INFINITY;
	int legal_moves = 0;
	//PrintBoards(board);

	int pvNode = beta - alpha > 1;
	int futilityMargin = evaluation + 120;
	for (Move& move : moveList)
	{
		if (is_quiet(move.Type)) continue; //skip non capture moves
		//if (!pvNode && futilityMargin <= alpha)
		//{

		//	//if (negamaxScore < futilityMargin)
		//	//{
		//	//	negamaxScore = futilityMargin;
		//	//}
		//	continue;
		//}
		//if ((captureFlag & move.Type) == 0) continue;
		if (!SEE(board, move, 0))
		{
			continue;
		}
		nodes_for_time_checking++;

		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.last_irreversible_ply;
		ply++;
		if (seldepth < ply)
		{
			seldepth = ply;
		}
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

		//Board boardcopy = board;


		//if (!staticExchangeEvaluation(board, move, -100))
		//{
		//	continue;
		//}

		//PrintBoards(board);
		
		negamax_nodecount++;








		legal_moves++;
		score = -Quiescence(board, -beta, -alpha);

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

static inline int Negamax(Board& board, int depth, int alpha, int beta, bool doNMP, bool cutnode)
{
	bool is_pv_node = beta - alpha > 1;
	//nodes_for_time_checking++;
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

	//if (ply > 99 - 1) {
	//	return Evaluate(board);
	//}
	//Transposition_entry ttEntry;

	Transposition_entry ttEntry = ttLookUp(board.Zobrist_key);
	int score = 0;
	int ttFlag = AlphaFlag;
	uint64_t last_zobrist = board.Zobrist_key;
	int bestValue = MINUS_INFINITY;
	// Only check TT for depths greater than zero (ply != 0)
	if (ttEntry.zobrist_key == board.Zobrist_key && ttEntry.node_type != 0)
	{
		// Valid TT entry found
		if (ply != 0 && ttEntry.depth >= depth)
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

	
	if (is_in_check(board))
	{
		depth = std::max(depth + 1, 1);
	}
	if (depth <= 0)
	{
		return Quiescence(board, alpha, beta);
		//return Evaluate(board);
	}
	

	//bool moreSafeCutNodeReduction = false;
	//if (!doNMP && depth >= 3) {
	//	moreSafeCutNodeReduction = true;
	//} 

	// IIR by Ed Schroder (~15 Elo)
	//if (depth >= 6 && (is_pv_node || cutnode) && (ttEntry.node_type = 0))
	//{
	//	//std::cout << 1 + (cutnode && !moreSafeCutNodeReduction);
	//	--depth;
	//}



	int static_eval = Evaluate(board);

	int canPrune = !is_in_check(board) && !is_pv_node;
	if (depth < 4 && canPrune)//rfp
	{
		int rfpMargin = 75 * depth;
		int rfpThreshold = rfpMargin;

		if (static_eval - rfpThreshold >= beta)
		{
			return static_eval - rfpThreshold;
		}
	}
	if (doNMP)
	{
		if (!is_in_check(board) && depth >= 3 && ply)
		{
			if ((board.occupancies[Both] & ~(board.bitboards[P] | board.bitboards[p] | board.bitboards[K] | board.bitboards[k])) != 0ULL)
			{

				int lastep = board.enpassent;
				uint64_t lzob = board.Zobrist_key;
				ply++;
				Make_Nullmove(board);
				int score = -Negamax(board, depth - 1 - 2, -beta, -beta + 1, false, !cutnode);

				Unmake_Nullmove(board);
				ply--;
				board.enpassent = lastep;
				board.Zobrist_key = lzob;

				if (score >= beta)
				{
					return score > 49000 ? beta : score;
				}

			}
		}
	}
	

	
	
	std::vector<Move> moveList;
	Generate_Legal_Moves(moveList, board, false);


	int legal_moves = 0;

	//printMoveSort(board);
	//PrintBoards(board);

	sort_moves(moveList, board, ttEntry);

	
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
	
	int depth_to_search;
	
	//bool isPV = beta - alpha > 1;
	bool skip_quiets = false;


	pv_table[ply][ply] = ttEntry.best_move;
	int lmp_threshold = 1 + 3 * depth * depth;

	int quiet_SEE_margin = -70 * depth;
	int noisy_SEE_margin = -20 * depth * depth;

	std::vector<Move> Quiet_moves_list;
	Quiet_moves_list.reserve(50);

	Move bestmove = Move(0, 0, 0, 0);
	for (Move& move : moveList)
	{
		
		bool isQuiet = is_quiet(move.Type);

		if (skip_quiets && isQuiet) //quiet move
		{
			continue;
		}

		if(depth <= Maximum_pvs_see_depth)
		{
			if (isQuiet)
			{
				if (!SEE(board, move, quiet_SEE_margin))
				{
					continue;
				}
			}
			else
			{
				if (!SEE(board, move, noisy_SEE_margin))
				{
					continue;
				}
			}
		}

		bool isNotMated = alpha > -49000 + 99;

		if (ply != 0 && isQuiet && isNotMated)
		{
			if (legal_moves >= lmp_threshold)
			{
				skip_quiets = true;
			}
			//bool is_checked = is_in_check(board);
			////int lmr_depth = std::max(1, depth - (lmrTable[depth][legal_moves]));
			//if (!is_pv_node && ply != 0 && isNotMated && depth < 4 && !is_checked && isQuiet && (static_eval + (depth * 177 + 133)) <= alpha)
			//{
			//	skip_quiets = true;
			//}
			
		}

		// 
		// 
		//int futility_margin = 60+250*depth;
		//if (canPrune && depth < 4 && abs(alpha) < 2000 && static_eval + futility_margin <= alpha)
		//{
		//	skip_quiets = true;
		//}

		//if (!is_pv_node && !is_in_check(board) && is_quiet(move.Type) && legal_moves-1 > (4 + 3 * depth * depth))
		//{
		//	skip_quiets = true;
		//}


		nodes_for_time_checking++;

		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.last_irreversible_ply;



		ply++;
		if (seldepth < ply)
		{
			seldepth = ply;
		}
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
		if (isQuiet)
		{
			Quiet_moves_list.push_back(move);
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



		if (depth > Minimum_lmr_depth && legal_moves > 1)
		{
			//reduction = 0.77 + log(legal_moves) * log(depth) / 2.36;
			
			reduction = lmrTable[depth][legal_moves];

			if (is_pv_node)
			{
				if (reduction >= 1)
				{
					reduction--;
				}
				
			}
			//asdf

			//if (beta - alpha >= 1) //reduce less on pv nodes
			//{
			//	reduction -= 1;
			//}
		}
		
		is_reduced = reduction > 0;
		
		if (legal_moves <= 1)
		{
			score = -Negamax(board, depth_to_search, -beta, -alpha, true, false);
		}
		else
		{
			if (is_reduced)
			{
				if (is_pv_node)
				{
					score = -Negamax(board, depth_to_search - reduction, -alpha - 1, -alpha, true, false);
				}
				else
				{
					score = -Negamax(board, depth_to_search - reduction, -alpha - 1, -alpha, true, !cutnode);
				}
				

				
			}
			else
			{
				score = alpha + 1;
			}
			//score = -Negamax(board, depth_to_search, -alpha-1, -alpha, true);
			
			if (score > alpha)
			{
				score = -Negamax(board, depth_to_search, -alpha-1, -alpha, true, false);
			}
			if (score > alpha && score < beta )
			{
				score = -Negamax(board, depth_to_search, -beta, -alpha, true, false);
			}
		
		}


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

		bestValue = std::max(score, bestValue);


		if (bestValue > alpha)
		{
			found_pv = true;
			ttFlag = ExactFlag;
			alpha = score;

			//if (ply < 0 || ply > 98)
			//{
			//	std::cout << ply << "\n";
			//}
			
			pv_table[ply][ply] = move;
			bestmove = move;

			
			

			//copy move from deeper ply into a current ply's line
			for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
			{
				pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
			}

			pv_length[ply] = pv_length[ply + 1];



		}
		if (alpha >= beta)
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
				if (!(killer_moves[0][ply] == move))
				{
					killer_moves[1][ply] = killer_moves[0][ply];
					killer_moves[0][ply] = move;
				} 


				//history_moves[board.side][move.From][move.To] += depth * depth;

				for (const auto& move_quiet : Quiet_moves_list) {
					if (move_quiet == move)
					{
						update_history(board.side, move_quiet.From, move_quiet.To, depth * depth);
					}
					else
					{
						update_history(board.side, move_quiet.From, move_quiet.To, -depth * depth);
					}
					
				}
				//update_history(board.side, move.From, move.To, depth*depth);
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
	ttEntry.best_move = bestmove;
	//if (!(bestmove == Move(0, 0, 0, 0)))
	//{
	//	
	//}
	//else
	//{
	//	ttEntry.best_move = Move(0, 0, 0, 0);
	//}
	


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
			killer_moves[0][ply] = Move();
			killer_moves[1][ply] = Move();
		}
		for (int from = 0; from < 64; ++from)
		{
			for (int to = 0; to < 64; ++to)
			{
				history_moves[0][from][to] = 0;
				history_moves[1][from][to] = 0;
				//std::cout << to<<"\n";
				//history_moves[piece][square] = 0;
			}
		}

		for (int i = 0; i < TT_size; i++)
		{
			TranspositionTable[i] = Transposition_entry();
		}

		parse_fen(bench_fens[i], board);


		search_start = std::chrono::steady_clock::now();
		IterativeDeepening(board, 10, -1, false, false);
		search_end = std::chrono::steady_clock::now();

		float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(search_end - search_start).count();
		//std::chrono::duration<double, std::milli> elapsedS = end - start;

		float second = (elapsedMS + 1) / 1000;

		double nps = negamax_nodecount / second;


		nodecount += negamax_nodecount;
		totalsearchtime += std::floor(elapsedMS);

		

	}
	std::cout << "nodes " << nodecount << " nps " << nodecount / (totalsearchtime + 1) * 1000 << "\n";
	
}

void IterativeDeepening(Board& board, int depth, int timeMS, bool PrintRootVal, bool print_info)
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


	int score = 0;
	Print_Root = false;
	//move_scores.clear();
	//public_movelist.clear();

	for (int from = 0; from < 64; ++from)
	{
		for (int to = 0; to < 64; ++to)
		{
			history_moves[0][from][to] = 0;
			history_moves[1][from][to] = 0;
			//std::cout << to<<"\n";
			//history_moves[piece][square] = 0;
		}
	}
	//std::cout << "white";
	//printTopHistory(0);
	//std::cout << "black";
	//printTopHistory(1);

	for (curr_depth = 1; curr_depth <= depth; curr_depth++)
	{
		//move_scores.cler();
		//public_movelist.clear();
		ply = 0;
		seldepth = 0;
		nodes_for_time_checking = 0;
		is_search_stopped = false;
		//memset(last_bestMove, 0, 64 * sizeof(int));

		//if (curr_depth >= 4)
		//{
		//	alpha_val = score - 25;
		//	beta_val = score + 25;
		//}
		//for (int i = 0; i < 99; ++i) {
		//	for (int j = 0; j < 99; ++j) {
		//		pv_table[i][j] = Move();  // Calls the default constructor
		//	}
		//}
		//if (curr_depth == depth)
		//{
		//	Print_Root = PrintRootVal;
		//}

		int delta = asp_window_initial;
		int alpha_val = std::max(MINUS_INFINITY ,score - delta);
		int beta_val = std::min(PLUS_INFINITY, score + delta);
		//std::cout << alpha_val << ","<<beta_val;
		while (true)
		{
			score = Negamax(board, curr_depth, alpha_val, beta_val, true, false);
			//std::cout << "alpha:" << alpha_val<<"\n";
			//std::cout << "beta:" << beta_val << "\n";
			//std::cout << "delta:" << delta << "\n";
			//std::cout << score;
			delta += delta;
			if (score <= alpha_val)
			{
				alpha_val = std::max(MINUS_INFINITY, score - delta);
			}
			else if (score >= beta_val)
			{
				beta_val = std::min(PLUS_INFINITY, score + delta);
			}
			else
			{
				break;
			}

			

			if (delta >= asp_window_max)
			{
				delta = PLUS_INFINITY;
			}
		}

		

		//score = Negamax(board, curr_depth, alpha_val, beta_val, true);

		//
		////int last_alpha = alpha_val;
		////int last_beta = beta_val;

		//// Continue re-searching until the search is successful
		//while (score <= alpha_val || score >= beta_val)
		//{
		//	if (score <= alpha_val) // Failed low
		//	{
		//		alpha_val -= 50; // Widen alpha by decreasing it
		//	}
		//	else if (score >= beta_val) // Failed high
		//	{
		//		beta_val += 50; // Widen beta by increasing it
		//	}

		//	// Perform the search again with the widened window
		//	//move_scores.clear();
		//	//public_movelist.clear();
		//	score = Negamax(board, curr_depth, alpha_val, beta_val, true);
		//	//std::cout << "Window widened: [" << alpha_val << ", " << beta_val << "]" << std::endl;
		//}



		//std::cout << "white";
		//printTopHistory(0);
		//std::cout << "black";
		//printTopHistory(1);
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

		if (print_info)
		{
			if (!is_search_stopped)
			{
				bestmove = pv_table[0][0];
				int hashfull = get_hashfull();
				last_bestMove[curr_depth - 1] = bestmove;
				std::cout << "info depth " << curr_depth;
				std::cout << " seldepth " << seldepth;
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

				std::cout << " time " << static_cast<int>(std::round(elapsedMS)) << " nodes " << negamax_nodecount << " nps " << static_cast<int>(std::round(nps)) << " hashfull " << hashfull << " pv " << std::flush;
				for (int count = 0; count < pv_length[0]; count++)
				{
					printMove(pv_table[0][count]);
					std::cout << " ";
				}
			}
		}



		

		if (elapsedMS > Searchtime_MS)
		{
			//std::cout << elapsedMS << "\n";
			//std::cout << Searchtime_MS << "\n";
			break;
		}
		if (print_info)
		{
			std::cout << "\n";
		}
	}
	//auto end = std::chrono::high_resolution_clock::now();
	if (print_info)
	{
		std::cout << "bestmove ";
		printMove(bestmove);
		std::cout << "\n";
	}

}

//void printMoveSort(Board board)
//{
//	std::vector<Move> movelist;
//	Generate_Legal_Moves(movelist,board, false);
//
//	sort_moves(movelist, board);
//
//	for (int i = 0; i < movelist.size(); i++)
//	{
//		printMove(movelist[i]);
//		std::cout << " "<<get_move_score(movelist[i], board);
//		std::cout << "\n";
//	}
//	//PrintLegalMoves(movelist);
//}
