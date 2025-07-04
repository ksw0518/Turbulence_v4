#include "MoveGeneration.h"
#include "Evaluation.h"
#include "Board.h"
#include "Search.h"
#include "BitManipulation.h"
#include "const.h"
#include "History.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <tuple>
#include <vector>
#include <string>
#include <random>
#include <utility>  // for std::pair
#include <fstream>

#include <bit>
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX

#include <windows.h>

#undef NOMINMAX
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN

#endif

#ifndef EVALFILE
#define EVALFILE "./nnue.bin"
#endif


#if defined(_WIN32) || defined(_WIN64)
// Windows platform
constexpr bool isOnWindow = true;
#elif defined(__linux__)
// Linux platform
constexpr bool isOnWindow = false;
#else
// Unknown platform
constexpr bool isOnWindow = false;  // Default to false (likely macOS)
#endif
#define NULLMOVE Move(0,0,0,0)


int RFP_MULTIPLIER = 89;
int RFP_IMPROVING_MULTIPLIER = 56;
int RFP_BASE = -38;
int RFP_IMPROVING_BASE = -37;

int LMP_BASE = 46;
int LMP_MULTIPLIER = 94;
int LMP_IMPROVING_BASE = 95;
int LMP_IMPROVING_MULTIPLIER = 188;

int PVS_QUIET_BASE = 2;
int PVS_QUIET_MULTIPLIER = 57;

int PVS_NOISY_BASE = -10;
int PVS_NOISY_MULTIPLIER = 20;

int HISTORY_BASE = 136;
int HISTORY_MULTIPLIER = 87;
int CONTHIST_BASE = 66; 
int CONTHIST_MULTIPLIER = 48;
int CAPTHIST_BASE = 134;
int CAPTHIST_MULTIPLIER = 83;

int ASP_WINDOW_INITIAL = 15;
int ASP_WINDOW_MAX = 301;

int PAWN_CORRHIST_MULTIPLIER = 178;// divide by 5 later
int MINOR_CORRHIST_MULTIPLIER = 150;// divide by 5 later
int NONPAWN_CORRHIST_MULTIPLIER = 183;// divide by 5 later
int COUNTERMOVE_CORRHIST_MULTIPLIER = 150;// divide by 5 later


int HISTORY_PRUNING_MULTIPLIER = 1372;
int HISTORY_PRUNING_BASE = 69;
int HISTORY_LMR_MULTIPLIER = 753;
int HISTORY_LMR_BASE = 88;
int NMP_EVAL_DIVISER = 400;
int NMP_DEPTH_DIVISER = 3;
int MAX_NMP_EVAL_R = 3;

int DEXT_MARGIN = 20;

int RAZORING_MARGIN = 200;
int RAZORING_BASE = -6;

int LMR_NONPV_ADD = 1031;
int LMR_INCHECK_SUB = 985;
int LMR_IMPROVING_SUB = 1085;
int LMR_HISTORY_ADD = 1049;
int LMR_NOISY_SUB = 1015;
int LMR_KILLER_SUB = 1068;
int LMR_TTPV_SUB = 1174;
int LMR_CUTNODE_ADD = 1199;
int LMR_TTDEPTH_SUB = 1099;
constexpr int MIN_LMR_DEPTH = 3;
constexpr int MAX_PVS_SEE_DEPTH = 8;

int pvLengths[99];
Move pvTable[99][99];

int SEEPieceValues[] = { 98, 280, 295, 479, 1064, 0, 0 };

static Move lastBestMoves[99];

size_t TTSize = 699050;
TranspositionEntry* TranspositionTable = nullptr;

int lmrTable[99][256];

constexpr int HFLOWER = 0;
constexpr int HFEXACT = 1;
constexpr int HFUPPER = 2;


double DEF_TIME_MULTIPLIER = 0.054;
double DEF_INC_MULTIPLIER = 0.85;
double MAX_TIME_MULTIPLIER = 0.76;
double HARD_LIMIT_MULTIPLIER = 3.04;
double SOFT_LIMIT_MULTIPLIER = 0.76;

uint64_t hardNodeBound;

bool stop_signal = false;
bool isPrettyPrinting = true;

inline bool isMoveCapture(int type)
{
	return (type & captureFlag) != 0;
}
inline bool isMovePromotion(int type)
{
	return (type & promotionFlag) != 0;
}
inline bool isMoveQuiet(int type)
{
	return !isMoveCapture(type) && !isMovePromotion(type);
}
void initializeLMRTable(ThreadData& data)
{
	for (int depth = 1; depth < 99; depth++)
	{
		for (int move = 1; move < 256; move++)
		{
			lmrTable[depth][move] = std::floor(0.77 + log(move) * log(depth) / 2.36);
		}
	}
	for (int ply = 0; ply < 99; ply++)
	{
		data.killerMoves[0][ply] = Move();
	}
	memset(data.mainHistory, 0, sizeof(data.mainHistory));
	memset(data.onePlyContHist, 0, sizeof(data.onePlyContHist));
	memset(data.twoPlyContHist, 0, sizeof(data.twoPlyContHist));
	memset(data.CaptureHistory, 0, sizeof(data.CaptureHistory));
	memset(data.pawnCorrHist, 0, sizeof(data.pawnCorrHist));
	memset(data.nonPawnCorrHist, 0, sizeof(data.nonPawnCorrHist));
	memset(data.minorCorrHist, 0, sizeof(data.minorCorrHist));
	memset(data.counterMoveCorrHist, 0, sizeof(data.counterMoveCorrHist));

	isPrettyPrinting = true;
	for (int ply = 0; ply < 99; ply++)
	{
		data.killerMoves[0][ply] = Move();
	}
	memset(data.mainHistory, 0, sizeof(data.mainHistory));

	LoadNetwork(EVALFILE);
}
bool isInsufficientMaterial(const Board& board) {

	int whiteBishops = count_bits(board.bitboards[B]); 
	int blackBishops = count_bits(board.bitboards[b]);
	int whiteKnights = count_bits(board.bitboards[N]);
	int blackKnights = count_bits(board.bitboards[n]);
	int whiteRooks = count_bits(board.bitboards[R]);
	int blackRooks = count_bits(board.bitboards[r]);
	int whiteQueens = count_bits(board.bitboards[Q]);
	int blackQueens = count_bits(board.bitboards[q]);
	int whitePawns = count_bits(board.bitboards[P]);
	int blackPawns = count_bits(board.bitboards[p]);
	if (whiteQueens == 0 && blackQueens == 0 && whiteRooks == 0 && blackRooks == 0 && whitePawns == 0 && blackPawns == 0)
	{
		if (whiteBishops == 0 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 0)
		{
			return true;
		}
		else if (whiteBishops == 1 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 0)
		{
			return true;
		}
		else if (whiteBishops == 0 && blackBishops == 1 && whiteKnights == 0 && blackKnights == 0)
		{
			return true;
		}
		else if (whiteBishops == 0 && blackBishops == 0 && whiteKnights == 1 && blackKnights == 0)
		{
			return true;
		}
		else if (whiteBishops == 0 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 1)
		{
			return true;
		}
		else if (whiteBishops == 1 && blackBishops == 1 && whiteKnights == 0 && blackKnights == 0)
		{
			return true;
		}
		return false;
	}
	return false;
}
static inline int getMoveScore(Move move, Board& board, TranspositionEntry& entry, uint64_t opp_threat, ThreadData& data)
{

	// Check if the entry is valid and matches the current Zobrist key
	if (entry.bound != 0 && entry.zobristKey == board.zobristKey)
	{
		// If the best move from TT matches the current move, return a high score
		if (entry.bestMove == move)
		{
			return 99999999;
		}
	}
	if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{
		int victim;
		if (move.Type == ep_capture)
		{
			victim = P;
		}
		else
		{
			victim = get_piece(board.mailbox[move.To], White);
		}
		int attacker = get_piece(move.Piece, White);
		int score = SEEPieceValues[victim] * 100 - SEEPieceValues[attacker];
		score += data.CaptureHistory[move.Piece][move.To][board.mailbox[move.To]];
		score += SEE(board, move, -100) ? 200000 : -10000000;
		return score;
	}
	else
	{
		if (data.killerMoves[0][data.ply] == move)
		{
			return 150000;
		}
		else
		{
			// Return history score for non-capture and non-killer moves
			int mainHistScore = data.mainHistory[board.side][move.From][move.To][Get_bit(opp_threat, move.From)][Get_bit(opp_threat, move.To)];
			int contHistScore = getContinuationHistoryScore(move, data) * 2;
			int historyTotal = mainHistScore + contHistScore - 100000;

			if (historyTotal >= 80000)
			{
				return 80000;
			}
			else
			{
				return historyTotal;
			}
		}
	}

	return 0;
}
static inline int get_move_score_capture(Move move, Board& board, ThreadData& data)
{
	if ((move.Type & captureFlag) != 0) // if a move is a capture move
	{
		int victim;
		if (move.Type == ep_capture)
		{
			victim = P;
		}
		else
		{
			victim = get_piece(board.mailbox[move.To], White);
		}
		int attacker = get_piece(move.Piece, White);
		int score = SEEPieceValues[victim] * 100 - SEEPieceValues[attacker];
		score += data.CaptureHistory[move.Piece][move.To][board.mailbox[move.To]];
		return score;
	}
	else
	{
		return -999;
	}
}
inline int getPiecePromoting(int type, int side)
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
inline int move_estimated_value(Board& board, Move move)
{
	// Start with the value of the piece on the target square
	int target_piece = board.mailbox[move.To] > 5
		? board.mailbox[move.To] - 6
		: board.mailbox[move.To];

	int promoted_piece = getPiecePromoting(move.Type, White);
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

int SEE(Board& pos, Move move, int threshold)
{

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

	while (1) 
	{

		// If we have no more attackers left we lose
		myAttackers = attackers & pos.occupancies[colour];
		if (myAttackers == 0ull) 
		{
			break;
		}

		// Find our weakest piece to attack with
		for (nextVictim = P; nextVictim <= Q; nextVictim++) 
		{
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
		if (balance >= 0)
		{

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

template<typename I, typename C>
constexpr void insertion_sort(I first, I last, C const&& comp) {
	if (first == last) return;
	for (auto i = std::next(first); i != last; ++i) {
		auto k = *i;
		auto j = i;
		while (j > first && comp(k, *(j - 1))) {
			*j = *(j - 1);
			--j;
		}
		*j = k;
	}
}

static inline void sort_moves_captures(MoveList& moveList, Board& board, ThreadData& data)
{
	// Temporary array for captures
	Move captures[256];
	int captureCount = 0;

	// Separate capture moves
	for (int i = 0; i < moveList.count; ++i)
	{
		if (!isMoveQuiet(moveList.moves[i].Type))
		{
			captures[captureCount++] = moveList.moves[i];
		}
	}

	// Sort only the capture moves
	insertion_sort(captures, captures + captureCount, [&board, &data](const Move& move1, const Move& move2) {
		return get_move_score_capture(move1, board, data) > get_move_score_capture(move2, board, data);
		});

	// Copy sorted captures back
	int index = 0;
	for (int i = 0; i < captureCount; ++i)
	{
		moveList.moves[index++] = captures[i];
	}

	// Append non-captures in original order
	for (int i = 0; i < moveList.count; ++i)
	{
		if (isMoveQuiet(moveList.moves[i].Type))
		{
			moveList.moves[index++] = moveList.moves[i];
		}
	}

	moveList.count = index;  // Update move count
}

static inline void sort_moves(MoveList& moveList, Board& board, TranspositionEntry& tt_entry, uint64_t opp_threat, ThreadData& data)
{
	// Precompute scores for all moves
	std::pair<int, Move> scored_moves[256];  // Use fixed-size array
	for (int i = 0; i < moveList.count; ++i)
	{
		scored_moves[i] = { getMoveScore(moveList.moves[i], board, tt_entry, opp_threat, data), moveList.moves[i] };
	}

	// Sort the scored moves based on the scores
	insertion_sort(scored_moves, scored_moves + moveList.count, [](const auto& a, const auto& b)
		{
			return a.first > b.first; // Sort by score (descending)
		});

	// Copy back sorted moves
	for (int i = 0; i < moveList.count; ++i)
	{
		moveList.moves[i] = scored_moves[i].second;
	}
}
inline TranspositionEntry ttLookUp(uint64_t zobrist)
{
	int tt_index = zobrist % TTSize;
	return TranspositionTable[tt_index];
}
inline bool is_in_check(Board& board)
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
void refresh_if_cross(Move& move, Board& board)
{
	if (get_piece(move.Piece, White) == K)//king has moved
	{
		if (getFile(move.From) <= 3)//king was left before
		{
			if (getFile(move.To) >= 4)//king moved to right 
			{
				//fully refresh the stm accumulator, and change that to start mirroring
				if (board.side == White)
				{
					resetWhiteAccumulator(board, board.accumulator, true);
				}
				else
				{
					resetBlackAccumulator(board, board.accumulator, true);
				}
			}
		}
		else//king was right before
		{
			if (getFile(move.To) <= 3)//king moved to left 
			{
				//fully refresh the stm accumulator, and change that to stop mirroring
				if (board.side == White)
				{
					resetWhiteAccumulator(board, board.accumulator, false);
				}
				else
				{
					resetBlackAccumulator(board, board.accumulator, false);
				}
			}
		}
	}
}
static inline int Quiescence(Board& board, int alpha, int beta, ThreadData& data)
{
	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - data.clockStart).count();
	if (elapsedMS > data.Searchtime_MS || data.searchNodeCount > hardNodeBound || stop_signal)
	{
		data.isSearchStop = true;
		return 0; // Return a neutral score if time is exceeded
	}
	if (data.ply >= 99)
	{
		return Evaluate(board);
	}
	int score = 0;
	TranspositionEntry ttEntry = ttLookUp(board.zobristKey);
	bool is_ttmove_found = false;
	if (ttEntry.zobristKey == board.zobristKey && ttEntry.bound != 0)
	{
		is_ttmove_found = true;
		if ((ttEntry.bound == ExactFlag)
			|| (ttEntry.bound == UpperBound && ttEntry.score <= alpha)
			|| (ttEntry.bound == LowerBound && ttEntry.score >= beta))
		{
			return ttEntry.score;
		}
	}
	int staticEval = Evaluate(board);
	staticEval = adjustEvalWithCorrHist(board, staticEval, data.searchStack[data.ply - 1].move, data);
	bool isInCheck = is_in_check(board);
	int ttAdjustedEval = staticEval;
	uint8_t Bound = ttEntry.bound;

	if (is_ttmove_found && !isInCheck && (Bound == ExactFlag || (Bound == LowerBound && ttEntry.score >= staticEval) || (Bound == UpperBound && ttEntry.score <= staticEval)))
	{
		ttAdjustedEval = ttEntry.score;
	}
	if (ttAdjustedEval >= beta)
	{
		return ttAdjustedEval;
	}
	if (ttAdjustedEval > alpha)
	{
		alpha = ttAdjustedEval;
	}

	MoveList moveList;
	Generate_Legal_Moves(moveList, board, true);

	sort_moves_captures(moveList, board, data);

	int bestValue = MINUS_INFINITY;
	int legal_moves = 0;

	uint64_t lastZobrist = board.zobristKey;
	uint64_t lastPawnKey = board.PawnKey;
	uint64_t lastMinorKey = board.MinorKey;
	uint64_t lastWhiteNPKey = board.WhiteNonPawnKey;
	uint64_t lastBlackNPKey = board.BlackNonPawnKey;
	AccumulatorPair last_accumulator = board.accumulator;
	for (int i = 0; i < moveList.count; ++i)
	{
		Move& move = moveList.moves[i];
		if (isMoveQuiet(move.Type)) continue; //skip non capture moves

		if (!SEE(board, move, 1))
		{
			continue;
		}

		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.lastIrreversiblePly;
		int last_halfmove = board.halfmove;

		data.ply++;
		if (data.selDepth < data.ply)
		{
			data.selDepth = data.ply;
		}

		refresh_if_cross(move, board);
		MakeMove(board, move);
		data.searchStack[data.ply - 1].move = move;

		if (!isLegal(move, board))//isMoveValid(move, board)
		{
			data.ply--;
			UnmakeMove(board, move, captured_piece);

			board.accumulator = last_accumulator;
			board.history.pop_back();
			board.lastIrreversiblePly = last_irreversible;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			board.zobristKey = lastZobrist;
			board.PawnKey = lastPawnKey;
			board.MinorKey = lastMinorKey;
			board.WhiteNonPawnKey = lastWhiteNPKey;
			board.BlackNonPawnKey = lastBlackNPKey;
			board.halfmove = last_halfmove;
			continue;
		}

		data.searchNodeCount++;
		legal_moves++;

		score = -Quiescence(board, -beta, -alpha, data);

		UnmakeMove(board, move, captured_piece);
		board.accumulator = last_accumulator;
		board.history.pop_back();
		board.lastIrreversiblePly = last_irreversible;
		board.enpassent = lastEp;
		board.castle = lastCastle;
		board.side = lastside;
		board.zobristKey = lastZobrist;
		board.PawnKey = lastPawnKey;
		board.MinorKey = lastMinorKey;
		board.WhiteNonPawnKey = lastWhiteNPKey;
		board.BlackNonPawnKey = lastBlackNPKey;
		board.halfmove = last_halfmove;

		if (data.isSearchStop) {
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}
		data.ply--;
		if (score > bestValue)
		{
			bestValue = score;
			if (score > alpha)
			{
				alpha = score;
			}
		}
		if (score >= beta)
		{
			return score;
		}
	}
	if (legal_moves == 0) // quiet position
	{
		return ttAdjustedEval;
	}
	if (ttEntry.bound == 0)
	{
		int nodeType = bestValue >= beta ? UpperBound : LowerBound;
		ttEntry.score = bestValue;
		ttEntry.bound = nodeType;
		ttEntry.depth = -1;
		ttEntry.zobristKey = board.zobristKey;
		TranspositionTable[board.zobristKey % TTSize] = ttEntry;
	}
	return bestValue;
}


static inline int Negamax(Board& board, int depth, int alpha, int beta, bool doNMP, bool cutnode, ThreadData& data, Move excludedMove = NULLMOVE)
{
	bool isPvNode = beta - alpha > 1;


	auto now = std::chrono::steady_clock::now();
	float elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - data.clockStart).count();
	if (elapsedMS > data.Searchtime_MS || data.searchNodeCount > hardNodeBound || stop_signal) {
		data.isSearchStop = true;
		return 0; // Return a neutral score if time is exceeded
	}

	pvLengths[data.ply] = data.ply;

	//Return draw score if threefold repetition has occured
	if (data.ply != 0)
	{
		if (is_threefold(board.history, board.lastIrreversiblePly))
		{
			return 0;
		}
		if (board.halfmove >= 100)
		{
			return 0;
		}
		if (isInsufficientMaterial(board))
		{
			return 0;
		}
	}
	//Immediately return the static evaluation score when we reach max ply
	if (data.ply > 99 - 1)
	{
		return Evaluate(board);
	}
	
	//Probe TT entry
	TranspositionEntry ttEntry = ttLookUp(board.zobristKey);

	int score = 0;
	int bestValue = MINUS_INFINITY;

	//Default assumption before search
	int ttFlag = UpperBound;

	bool tt_hit = false;
	bool isSingularSearch = excludedMove != NULLMOVE;

	bool tt_pv = isPvNode;
	//Checks for collisions, or empty entry
	if (ttEntry.zobristKey == board.zobristKey && ttEntry.bound != 0)
	{
		tt_hit = true;
		// Valid TT entry found
		if (!isPvNode && !isSingularSearch && data.ply != 0 && ttEntry.depth >= depth)
		{
			// Return immediately if exact score is found
			if (ttEntry.bound == ExactFlag)
			{
				return ttEntry.score;
			}
			else if (ttEntry.bound == LowerBound && ttEntry.score >= beta)
			{
				return ttEntry.score;
			}
			else if (ttEntry.bound == UpperBound && ttEntry.score <= alpha)
			{
				return ttEntry.score;
			}
		}
	}
	//Internal Iterative Reduction
	//If no hash move was found, reduce depth
	if (!isSingularSearch && depth >= 4 && (isPvNode || cutnode) && (!tt_hit))
	{
		depth--;
	}

	//checks if the node has been in a pv node in the past
	tt_pv |= ttEntry.ttPv;
	bool isInCheck = is_in_check(board);

	if (isInCheck)
	{
		//Increase search depth if the side to move is in check, as such positions often involve tactic
		depth = std::max(depth + 1, 1);
	}
	if (depth <= 0)
	{
		//Enter quiescence search to evaluate only "quiet" positions and avoid horizon effects
		return Quiescence(board, alpha, beta, data);
	}
	if (data.ply + 1 <= 99)
	{
		// Reset killer moves for the next ply to make the killer move more local
		data.killerMoves[0][data.ply + 1] = Move(0, 0, 0, 0);
	}

	int rawEval = std::clamp(Evaluate(board), -40000, 40000);
	int staticEval = adjustEvalWithCorrHist(board, rawEval, data.searchStack[data.ply - 1].move, data);
	int ttAdjustedEval = staticEval;

	//Adjust static evaluation with search score on TT, to get move accurate estimaton.
	if (!isSingularSearch && tt_hit && !isInCheck && (ttEntry.bound == ExactFlag || (ttEntry.bound == LowerBound && ttEntry.score >= staticEval) || (ttEntry.bound == UpperBound && ttEntry.score <= staticEval)))
	{
		ttAdjustedEval = ttEntry.score;
	}

	data.searchStack[data.ply].staticEval = staticEval;

	//If current static evaluation is greater than static evaluation from 2 plies ago
	bool improving = !isInCheck && data.ply > 1 && staticEval > data.searchStack[data.ply - 2].staticEval;
	bool opponent_worsening = !isInCheck && staticEval +  data.searchStack[data.ply - 1].staticEval > 1;

	int canPrune = !isInCheck && !isPvNode;
	//RFP 
	//If static evaluation + margin still doesn't improve alpha, prune the node
	if (!isSingularSearch && depth < 5 && canPrune)
	{
		int rfpMargin;
		if (improving)
		{
			//Do more RFP when we are improving
			rfpMargin = RFP_IMPROVING_BASE + RFP_IMPROVING_MULTIPLIER * depth;
		}
		else
		{
			rfpMargin = RFP_BASE + RFP_MULTIPLIER * depth;
		}
		rfpMargin -= 12 * opponent_worsening;
		int rfpThreshold = rfpMargin;

		if (ttAdjustedEval - rfpThreshold >= beta)
		{
			return (ttAdjustedEval + beta) / 2;
		}
	}
	if (depth <= 3 && ttAdjustedEval + RAZORING_MARGIN * depth + RAZORING_BASE<= alpha)
	{
		int razor_score = Quiescence(board, alpha, alpha + 1, data);
		if (razor_score <= alpha)
		{
			return razor_score;
		}
	}
	//NMP
	//Since null move is worse than all the other moves in most situations,
	//if a reduced search on null move fails high over beta, return fail high score
	if (!isSingularSearch && !isPvNode && doNMP)
	{
		if (!isInCheck && depth >= 2 && data.ply && ttAdjustedEval >= beta && data.ply >= data.minNmpPly)
		{
			if ((board.occupancies[Both] & ~(board.bitboards[P] | board.bitboards[p] | board.bitboards[K] | board.bitboards[k])) != 0ULL)
			{
				int lastep = board.enpassent;
				uint64_t lzob = board.zobristKey;
				data.ply++;
				Make_Nullmove(board);
				int R = 3 + depth / NMP_DEPTH_DIVISER;
				R += std::min((ttAdjustedEval - beta) / NMP_EVAL_DIVISER, MAX_NMP_EVAL_R);
				int score = -Negamax(board, depth - R, -beta, -beta + 1, false, !cutnode, data);

				Unmake_Nullmove(board);
				data.ply--;
				board.enpassent = lastep;
				board.zobristKey = lzob;

				if (score >= beta)
				{
					if (depth <= 14 || data.minNmpPly > 0)
					{
						return score > 49000 ? beta : score;
					}

					data.minNmpPly = data.ply + (depth - R) * 3 / 4;
					score = Negamax(board, depth - R, beta - 1, beta, false, false, data);
					data.minNmpPly = 0;

					if (score >= beta)
					{
						return score;
					}
				}
			}
		}
	}
	MoveList moveList;
	Generate_Legal_Moves(moveList, board, false);
	int searchedMoves = 0;

	//Calculate all squares opponent is controlling
	uint64_t oppThreats = get_attacked_squares(1 - board.side, board, board.occupancies[Both]);

	//Sort moves from best to worst(by approximation)
	sort_moves(moveList, board, ttEntry, oppThreats, data);

	int orgAlpha = alpha;
	int depthToSearch;
	bool skipQuiets = false;

	depth = std::min(depth, 98);

	int lmp_base = improving ? LMP_IMPROVING_BASE : LMP_BASE;
	int lmp_mult = improving ? LMP_IMPROVING_MULTIPLIER : LMP_MULTIPLIER;
	

	int lmpThreshold = (lmp_base + (lmp_mult)*depth * depth) / 100;
	int quietSEEMargin = PVS_QUIET_BASE - PVS_QUIET_MULTIPLIER * depth;
	int noisySEEMargin = PVS_NOISY_BASE - PVS_NOISY_MULTIPLIER * depth * depth;
	int historyPruningMargin = HISTORY_PRUNING_BASE - HISTORY_PRUNING_MULTIPLIER * depth;

	MoveList quietsList;
	MoveList noisyList;
	std::vector<int> capturedPiece;

	Move bestMove = Move(0, 0, 0, 0);
	int quietMoves = 0;

	uint64_t last_zobrist = board.zobristKey;
	uint64_t last_pawnKey = board.PawnKey;
	uint64_t last_minorKey = board.MinorKey;
	uint64_t last_whitenpKey = board.WhiteNonPawnKey;
	uint64_t last_blacknpKey = board.BlackNonPawnKey;
	AccumulatorPair last_accumulator = board.accumulator;
	for (int i = 0; i < moveList.count; ++i)
	{
		Move& move = moveList.moves[i];
		bool isQuiet = isMoveQuiet(move.Type);
		if (move == excludedMove)
		{
			//Skip search with an excluded move
			continue;
		}
		if (skipQuiets && isQuiet) //quiet move
		{
			continue;
		}
		int seeThreshold = isQuiet ? quietSEEMargin : noisySEEMargin; 

		int captured_piece = board.mailbox[move.To];
		int capthistScore = data.CaptureHistory[move.Piece][move.To][captured_piece];


		int main_history = data.mainHistory[board.side][move.From][move.To][Get_bit(oppThreats, move.From)][Get_bit(oppThreats, move.To)];
		int conthist = getContinuationHistoryScore(move, data) * 2;

		int historyScore = main_history + conthist;

		if (isMoveCapture(move.Type))
		{
			seeThreshold -= capthistScore / 125;
		}
		else
		{
			seeThreshold -= historyScore / 650;
		}
		
		if (data.ply != 0 && depth <= MAX_PVS_SEE_DEPTH)
		{
			//if Static Exchange Evaluation score is lower than certain margin, assume the move is very bad and skip the move
			if (!SEE(board, move, seeThreshold))
			{
				continue;
			}
		}



		bool isNotMated = bestValue > -49000 + 99;

	
		if (data.ply != 0 && isQuiet && isNotMated)
		{
			//Skip late moves, as good moves are typically found among the earlier moves due to move ordering
			if (searchedMoves >= lmpThreshold)
			{
				skipQuiets = true;
			}
			//If history score is very bad, skip the move
			if (quietMoves > 1 && depth <= 5 && historyScore < historyPruningMargin)
			{
				continue;
			}
		}
		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		
		int last_irreversible = board.lastIrreversiblePly;
		int last_halfmove = board.halfmove;
		data.ply++;

		if (data.selDepth < data.ply)
		{
			data.selDepth = data.ply;
		}

		refresh_if_cross(move, board);
		MakeMove(board, move);

		data.searchNodeCount++;
		if (!isLegal(move, board))
		{
			data.ply--;
			UnmakeMove(board, move, captured_piece);
			board.accumulator = last_accumulator;
			board.history.pop_back();
			board.lastIrreversiblePly = last_irreversible;
			board.zobristKey = last_zobrist;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			board.PawnKey = last_pawnKey;
			board.MinorKey = last_minorKey;
			board.WhiteNonPawnKey = last_whitenpKey;
			board.BlackNonPawnKey = last_blacknpKey;
			board.halfmove = last_halfmove;
			continue;
		} 

		if (isQuiet)
		{
			quietsList.add(move);
			quietMoves++;
		}
		else
		{
			noisyList.add(move);
			capturedPiece.push_back(captured_piece);
		}
		searchedMoves++;
		depthToSearch = depth - 1;

		int reduction = 0;
		bool is_reduced = false;
		int extensions = 0;

		//Singular Extension
		//If we have a TT move, we try to verify if it's the only good move. if the move is singular, search the move with increased depth
		if (data.ply > 1 && depth >= 7 && move == ttEntry.bestMove && excludedMove == NULLMOVE && ttEntry.depth >= depth - 3 && ttEntry.bound != UpperBound && std::abs(ttEntry.score) < 40000)
		{
			data.ply--;
			UnmakeMove(board, move, captured_piece);
			board.accumulator = last_accumulator;
			board.history.pop_back();
			board.lastIrreversiblePly = last_irreversible;
			board.zobristKey = last_zobrist;
			board.enpassent = lastEp;
			board.castle = lastCastle;
			board.side = lastside;
			board.PawnKey = last_pawnKey;
			board.MinorKey = last_minorKey;
			board.WhiteNonPawnKey = last_whitenpKey;
			board.BlackNonPawnKey = last_blacknpKey;
			board.halfmove = last_halfmove;

			int s_beta = ttEntry.score - depth * 2;
			int s_depth = (depth - 1) / 2;
			int s_score = Negamax(board, s_depth, s_beta - 1, s_beta, true, cutnode, data, move);
			if (s_score < s_beta)
			{
				extensions++;
				//Double Extensions
				//TT move is very singular, increase depth by 2
				if (!isPvNode && s_score <= s_beta - DEXT_MARGIN)
				{
					extensions++;
				}
			}
			else if (s_beta >= beta)
			{
				return s_beta;
			}
			else if (ttEntry.score >= beta)
			{
				extensions--;
			}
			else if (cutnode)
			{
				extensions -= 2;
			}

			refresh_if_cross(move, board);
			MakeMove(board, move);
			data.ply++;
		}
		data.searchStack[data.ply - 1].move = move;
		if (depth > MIN_LMR_DEPTH && searchedMoves > 1 + 2 * (data.ply == 1))
		{
			//LMR
			//Save search by reducing moves that are ordered closer to the end
			reduction = lmrTable[depth][searchedMoves];

			int reduction_bonus = 0;
			//reduce more if we are not in pv node
			if (!isPvNode && quietMoves >= 4)
			{
				reduction_bonus+= LMR_NONPV_ADD;
			}
			//reduce less if we the move is giving check
			if (is_in_check(board))
			{
				reduction_bonus-= LMR_INCHECK_SUB;
			}
			//reduce less if the position is improving
			if (improving)
			{
				reduction_bonus-= LMR_IMPROVING_SUB;
			}
			//reduce more if the history score is bad
			if (isQuiet)
			{
				if (historyScore < (-HISTORY_LMR_MULTIPLIER * depth) + HISTORY_LMR_BASE)
				{
					reduction_bonus += LMR_HISTORY_ADD;
				}
			}
			else
			{
				reduction_bonus -= capthistScore * 1024 / 10000;
			}
	
			//reduce less if the move is a capture
			if (!isQuiet)
			{
				reduction_bonus-= LMR_NOISY_SUB;
			}
			//reduce less killer moves
			if ((move == data.killerMoves[0][data.ply - 1]))
			{
				reduction_bonus-= LMR_KILLER_SUB;
			}
			//if the node has been in a pv node in the past, reduce less
			if (tt_pv)
			{
				reduction_bonus-= LMR_TTPV_SUB;
			}
			//reduce more cutnode
			if (cutnode)
			{
				reduction_bonus+= LMR_CUTNODE_ADD;
			}
			if (tt_hit && ttEntry.depth >= depth) 
			{
				reduction_bonus-= LMR_TTDEPTH_SUB;
			}
			reduction_bonus /= 1024;
			reduction += reduction_bonus;
		}
		//Prevent from accidently extending the move
		if (reduction < 0) reduction = 0;
		is_reduced = reduction > 0;
		bool isChildCutNode;
		uint64_t nodesBefore = data.searchNodeCount; 
		if (searchedMoves <= 1)
		{
			if (isPvNode)
			{
				isChildCutNode = false;
			}
			else
			{
				isChildCutNode = !cutnode;
			}
			score = -Negamax(board, depthToSearch + extensions, -beta, -alpha, true, isChildCutNode, data);
		}
		else
		{
			if (is_reduced)
			{
				score = -Negamax(board, depthToSearch - reduction, -alpha - 1, -alpha, true, true, data);
			}
			else
			{
				score = alpha + 1;
			}
			if (score > alpha)
			{
				if (score != alpha + 1)
				{
					depthToSearch += (score > (bestValue + 60 + depthToSearch * 2)); 
					depthToSearch -= (score < bestValue + depthToSearch && data.ply != 1);
				}
				score = -Negamax(board, depthToSearch, -alpha - 1, -alpha, true, !cutnode, data);
			}
			if (score > alpha && score < beta)
			{
				score = -Negamax(board, depthToSearch, -beta, -alpha, true, false, data);
			}

		}
		uint64_t nodesAfter = data.searchNodeCount;
		
		board.accumulator = last_accumulator;
		UnmakeMove(board, move, captured_piece);
		board.history.pop_back();
		board.lastIrreversiblePly = last_irreversible;
		board.zobristKey = last_zobrist;
		board.enpassent = lastEp;
		board.castle = lastCastle;
		board.side = lastside;
		board.PawnKey = last_pawnKey;
		board.MinorKey = last_minorKey;
		board.WhiteNonPawnKey = last_whitenpKey;
		board.BlackNonPawnKey = last_blacknpKey;
		board.halfmove = last_halfmove;
		data.ply--;

		if (data.ply == 0)
		{
			data.node_count[move.From][move.To] += nodesAfter - nodesBefore;
		}
		if (data.isSearchStop) {
			return 0; // Return a neutral score if time is exceeded during recursive calls
		}

		bestValue = std::max(score, bestValue);
		if (bestValue > alpha)
		{
			ttFlag = ExactFlag;
			alpha = score;
			bestMove = move;
			if (isPvNode)
			{
				pvTable[data.ply][data.ply] = move;
				for (int next_ply = data.ply + 1; next_ply < pvLengths[data.ply + 1]; next_ply++)
				{
					pvTable[data.ply][next_ply] = pvTable[data.ply + 1][next_ply];
				}
				pvLengths[data.ply] = pvLengths[data.ply + 1];
			}
		}
		if (alpha >= beta)
		{
			ttFlag = LowerBound;
			if ((move.Type & capture) == 0)
			{
				data.killerMoves[0][data.ply] = move;
				int mainHistBonus = std::min(2400, HISTORY_BASE + 420 * depth);
				int contHistBonus = std::min(2400, CONTHIST_BASE + 290 * depth);
				for (int i = 0; i < quietsList.count; ++i)
				{
					Move& move_quiet = quietsList.moves[i];
					if (move_quiet == move)
					{
						updateHistory(board.side, move_quiet.From, move_quiet.To, mainHistBonus, oppThreats, data);
						if (data.ply >= 1)
						{
							updateContinuationHistoryScore(move_quiet, contHistBonus, data);
						}
					}
					else
					{
						updateHistory(board.side, move_quiet.From, move_quiet.To, -mainHistBonus, oppThreats, data);
						if (data.ply >= 1)
						{
							updateContinuationHistoryScore(move_quiet, -contHistBonus, data);
						}
					}
				}
				int captHistBonus = std::min(2400, CAPTHIST_BASE + 400 * depth);
				for (int i = 0; i < noisyList.count; ++i)
				{
					Move& move_noisy = noisyList.moves[i];
					updateCaptureHistory(move_noisy.Piece, move_noisy.To, capturedPiece[i], -captHistBonus, data);
				}
			}
			else
			{
				int captHistBonus = std::min(2400, CAPTHIST_BASE + 400 * depth);
				for (int i = 0; i < noisyList.count; ++i)
				{
					Move& move_noisy = noisyList.moves[i];
					if (move_noisy == move)
					{
						updateCaptureHistory(move_noisy.Piece, move_noisy.To, capturedPiece[i], captHistBonus, data);
					}
					else
					{
						updateCaptureHistory(move_noisy.Piece, move_noisy.To, capturedPiece[i], -captHistBonus, data);
					}
				}
			}
			break;
		}
	}
	if (searchedMoves == 0)
	{
		if (is_square_attacked(get_ls1b(board.side == White ? board.bitboards[K] : board.bitboards[k]), 1 - board.side, board, board.occupancies[Both]))
		{
			return -49000 + data.ply;
		}
		else
		{
			return isSingularSearch ? alpha : 0;
		}
	}
	if (!isSingularSearch)
	{
		if (ttFlag == UpperBound && tt_hit)
		{
			bestMove = ttEntry.bestMove;
		}
	}
	ttEntry.score = bestValue;
	ttEntry.bound = ttFlag;
	ttEntry.depth = depth;
	ttEntry.zobristKey = board.zobristKey;
	ttEntry.ttPv = tt_pv;
	ttEntry.bestMove = bestMove;

	int bound = bestValue >= beta ? HFLOWER : alpha != orgAlpha ? HFEXACT : HFUPPER;
	if (!isSingularSearch && !is_in_check(board) && (bestMove == Move(0, 0, 0, 0) || isMoveQuiet(bestMove.Type)) && !(bound == HFLOWER && bestValue <= staticEval) && !(bound == HFUPPER && bestValue >= staticEval))
	{
		//Save difference between static eval and search score, to get more accurate static eval in the future
		updatePawnCorrHist(board, depth, bestValue - staticEval, data);
		updateMinorCorrHist(board, depth, bestValue - staticEval, data);
		updateNonPawnCorrHist(board, depth, bestValue - staticEval, data);
		updateCounterCorrHist(data.searchStack[data.ply - 1].move, depth, bestValue - staticEval, data);
	}
	if (!isSingularSearch)
	{
		TranspositionTable[board.zobristKey % TTSize] = ttEntry;
	}
	return bestValue;
}
int get_hashfull()
{
	int entryCount = 0;
	for (int i = 0; i < 1000; i++)
	{
		if (TranspositionTable[i].bound != 0)
		{
			entryCount++;
		}
	}
	return entryCount;
}


void setColor([[maybe_unused]] ConsoleColor color)
{
#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
#endif
}
int countDecimalPlaces(float number)
{
	// Round to a maximum of 2 decimal places
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << number; // Round to 2 decimals

	// Get the string representation of the number
	std::string str = ss.str();

	// Find the position of the decimal point
	size_t pos = str.find('.');

	// If there is no decimal point, return 0 (no decimal places)
	if (pos == std::string::npos) {
		return 0;
	}

	// Count the number of digits after the decimal point
	return str.length() - pos - 1;
}
void print_UCI(Move& bestmove, int score, float elapsedMS, float nps, ThreadData& data)
{
	bestmove = pvTable[0][0];
	int hashfull = get_hashfull();
	lastBestMoves[data.currDepth - 1] = bestmove;
	std::cout << "info depth " << data.currDepth;
	std::cout << " seldepth " << data.selDepth;
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

	std::cout << " time " << static_cast<int>(std::round(elapsedMS)) << " nodes " << data.searchNodeCount << " nps " << static_cast<int>(std::round(nps)) << " hashfull " << hashfull << " pv " << std::flush;
	for (int count = 0; count < pvLengths[0]; count++)
	{
		printMove(pvTable[0][count]);
		std::cout << " ";
	}
	std::cout << "\n" << std::flush;;
}
void print_Pretty(Move& bestmove, int score, float elapsedMS, float nps, ThreadData data)
{
	bestmove = pvTable[0][0];
	setColor(ConsoleColor::White);
	std::cout << "depth ";
	setColor(ConsoleColor::BrightBlue);
	if (data.currDepth < 10)
	{
		std::cout << " ";
	}

	std::cout << data.currDepth;
	setColor(ConsoleColor::White);
	std::cout << " / ";
	setColor(ConsoleColor::BrightBlue);


	if (data.selDepth < 10)
	{
		std::cout << " ";
	}
	std::cout << data.selDepth;

	setColor(ConsoleColor::White);
	std::cout << " TT ";
	setColor(ConsoleColor::Green);
	int hashful = get_hashfull() / 10;
	if (hashful < 10)
	{
		std::cout << "  ";
	}
	else if (hashful < 100)
	{
		std::cout << " ";
	}
	std::cout << hashful;
	std::cout << "% ";

	setColor(ConsoleColor::Blue);
	std::cout << " ";
	int time = std::round((elapsedMS / 1000));
	if (time < 10)
	{
		std::cout << "   ";
	}
	else if (time < 100)
	{
		std::cout << "  ";
	}
	else if (time < 1000)
	{
		std::cout << " ";
	}
	std::cout << time;
	setColor(ConsoleColor::Gray);
	std::cout << "S ";

	setColor(ConsoleColor::BrightGreen);
	float nps_in_M = (std::round((nps / 1000000) * 100)) / 100;
	std::cout << nps_in_M;

	setColor(ConsoleColor::Gray);
	std::cout << "MN/S";
	if (std::abs(score) > 40000)
	{
		int mate_ply = 49000 - std::abs(score);
		int mate_fullmove = std::ceil(static_cast<double>(mate_ply) / 2);
		setColor(ConsoleColor::Green);
		if (score < 0)
		{
			setColor(ConsoleColor::Red);
			mate_fullmove *= -1;
		}

		std::cout << " Mate in " << mate_fullmove;
		std::cout << " ";
	}
	else
	{
		if (std::abs(score) < 50)
		{
			setColor(ConsoleColor::White);
		}
		else if (score < 0)//score <= -50
		{
			if (score <= -500)
			{
				setColor(ConsoleColor::Red);
			}
			else if (score <= -100)
			{
				setColor(ConsoleColor::BrightRed);
			}
			else
			{
				setColor(ConsoleColor::BrightYellow);
			}

		}
		else
		{
			if (score >= 500)
			{
				setColor(ConsoleColor::Green);
			}
			else if (score >= 100)
			{
				setColor(ConsoleColor::BrightGreen);
			}
			else
			{
				setColor(ConsoleColor::BrightBlue);
			}
		}
		float score_fullPawn = (std::round((static_cast<float>(score) / 100) * 100)) / 100;
		int abs_score = std::abs(std::round(score_fullPawn));
		if (abs_score < 10)
		{
			std::cout << "  ";
		}
		else if (abs_score < 100)
		{
			std::cout << " ";
		}
		else if (abs_score < 1000)
		{
			std::cout << "";
		}
		if (score > 0)
		{
			std::cout << "+";
		}
		if (score == 0)
		{
			std::cout << "+";
		}
		std::cout << std::fixed << std::setprecision(2) << score_fullPawn;
	}

	setColor(ConsoleColor::Gray);
	std::cout << " pv ";
	for (int count = 0; count < pvLengths[0]; count++)
	{
		if (count <= 2)
		{
			setColor(ConsoleColor::White);
		}
		else
		{
			setColor(ConsoleColor::Gray);
		}
		if (count >= 10) break;
		printMove(pvTable[0][count]);
		std::cout << " ";
	}

	setColor(ConsoleColor::White);
	std::cout << "\n" << std::flush;
}
void scaleTime(int64_t& softLimit, uint8_t bestMoveStability, int64_t baseSoft, int64_t maxTime) {
	int bestMoveScale[5] = { 2430, 1350, 1090, 880, 680 };

	softLimit = std::min((baseSoft * bestMoveScale[bestMoveStability]) / 1000, maxTime);
}
std::pair<Move, int> IterativeDeepening(Board& board, int depth, SearchLimitations& searchLimits, ThreadData& data, bool print_info, int64_t maxTime)
{
	//std::cout << "id"<<std::flush;

	if (searchLimits.HardNodeLimit == NOLIMIT)
	{
		hardNodeBound = std::numeric_limits<int64_t>::max();
	}
	else
	{
		hardNodeBound = searchLimits.HardNodeLimit;
	}

	if (searchLimits.HardTimeLimit == NOLIMIT)
	{
		data.Searchtime_MS = std::numeric_limits<int64_t>::max();
	}
	else
	{
		data.Searchtime_MS = searchLimits.HardTimeLimit;
	}

	
	data.searchNodeCount = 0;
	Move bestmove = Move(0,0,0,0);
	data.clockStart = std::chrono::steady_clock::now();

	int score = 0;
	int bestScore = 0;

	memset(data.node_count, 0, sizeof(data.node_count));
	double nodesTmScale = 1.0;

	memset(pvTable, 0, sizeof(pvTable));
	memset(pvLengths, 0, sizeof(pvLengths));

	int bestMoveStability = 0;
	int64_t baseSoft = searchLimits.SoftTimeLimit;

	int64_t softLimit = searchLimits.SoftTimeLimit;
	
	for (data.currDepth = 1; data.currDepth <= depth; data.currDepth++)
	{
		data.ply = 0;
		data.selDepth = 0;
		data.isSearchStop = false;
		for (int i = 0; i < 99; i++)
		{
			data.searchStack[i].move = Move(0, 0, 0, 0);
		}

		int delta = ASP_WINDOW_INITIAL;
		int alpha_val = std::max(MINUS_INFINITY, score - delta);
		int beta_val = std::min(PLUS_INFINITY, score + delta);
		int window_change = 1;

		int aspirationWindowDepth = data.currDepth;
		//std::cout << alpha_val << ","<<beta_val;
		while (true)
		{
			auto end = std::chrono::steady_clock::now();
			int64_t MS = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - data.clockStart).count());
			if (softLimit != NOLIMIT)
			{
				if ((MS > softLimit * nodesTmScale) || stop_signal)
				{
					break;
				}

			}
			else
			{
				if ((searchLimits.HardTimeLimit != NOLIMIT && MS > searchLimits.HardTimeLimit) || stop_signal)
				{
					break;
				}
			}

			score = Negamax(board, std::max(aspirationWindowDepth, 1), alpha_val, beta_val, true, false, data);

			delta += delta;
			if (score <= alpha_val)
			{
				alpha_val = std::max(MINUS_INFINITY, score - delta);
				aspirationWindowDepth = data.currDepth;
			}
			else if (score >= beta_val)
			{
				beta_val = std::min(PLUS_INFINITY, score + delta);
				aspirationWindowDepth = std::max(aspirationWindowDepth - 1, data.currDepth - 5);
			}
			else
			{
				break;
			}



			if (delta >= ASP_WINDOW_MAX)
			{
				delta = PLUS_INFINITY;
			}
			window_change++;
		}


		auto end = std::chrono::steady_clock::now();
		float elapsedMS = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - data.clockStart).count());
		float second = (elapsedMS + 1) / 1000;

		double nps = data.searchNodeCount / second;

		if (pvTable[0][0] == bestmove)
		{
			bestMoveStability = std::min(bestMoveStability + 1, 4);
		}
		else
		{
			bestMoveStability = 0;
		}
		if (data.currDepth >= 6 && searchLimits.SoftTimeLimit != -1 && searchLimits.HardTimeLimit != -1)
		{
			nodesTmScale = (1.5 - ((double)data.node_count[bestmove.From][bestmove.To] / data.searchNodeCount)) * 1.35;
			scaleTime(softLimit, bestMoveStability, baseSoft, maxTime); 
		}
		if (!data.isSearchStop)
		{
			bestmove = pvTable[0][0];
			bestScore = score;
		}

		if (print_info)
		{
			if (!data.isSearchStop)
			{
		
				if (isPrettyPrinting && isOnWindow)
				{
					print_Pretty(bestmove, score, elapsedMS, nps, data);
				}
				else
				{
					print_UCI(bestmove, score, elapsedMS, nps, data);
				}
			}
		}
		if (searchLimits.SoftNodeLimit != NOLIMIT)
		{
			if ((data.searchNodeCount > searchLimits.SoftNodeLimit)|| stop_signal)
			{
				break;
			}
		}
		if (softLimit != NOLIMIT)
		{
			if ((elapsedMS > softLimit * nodesTmScale)|| stop_signal)
			{
				break;
			}
		}
		else
		{
			if ((searchLimits.HardTimeLimit != NOLIMIT && elapsedMS > searchLimits.HardTimeLimit)|| stop_signal)
			{
				break;
			}
		}

	}
	if (print_info)
	{
		std::cout << "bestmove ";
		printMove(bestmove);
		std::cout << "\n"<< std::flush;
	}
	
	return std::pair<Move, int>(bestmove, bestScore);
}
inline void Initialize_TT(int size)
{
	uint64_t bytes = static_cast<uint64_t>(size) * 1024ULL * 1024ULL;

	TTSize = bytes / sizeof(TranspositionEntry);

	if (TTSize % 2 != 0)
	{
		TTSize -= 1;
	}

	if (TranspositionTable)
		delete[] TranspositionTable;

	TranspositionTable = new TranspositionEntry[TTSize]();
}