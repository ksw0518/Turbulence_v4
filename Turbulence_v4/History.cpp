#include "History.h"
#include "Search.h"
#include "const.h"
#include "BitManipulation.h"
#include <algorithm>


int scaledBonus(int score, int bonus)
{
	return std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY) - (score * abs(bonus) / MAX_HISTORY);
}
void updateMinorCorrHist(Board& board, const int depth, const int diff, ThreadData& data)
{
	uint64_t minorKey = board.MinorKey;
	int& entry = data.minorCorrHist[board.side][minorKey % CORRHIST_SIZE];
	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);
	entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	entry = std::clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
}
void updatePawnCorrHist(Board& board, const int depth, const int diff, ThreadData& data)
{
	uint64_t pawnKey = board.PawnKey;
	int& entry = data.pawnCorrHist[board.side][pawnKey % CORRHIST_SIZE];
	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);
	entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	entry = std::clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
}
void updateCounterCorrHist(Move prevMove, const int depth, const int diff, ThreadData& data)
{
	//uint64_t pawnKey = board.PawnKey;
	int& entry = data.counterMoveCorrHist[prevMove.Piece][prevMove.To];
	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);
	entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	entry = std::clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
}
void updateNonPawnCorrHist(Board& board, const int depth, const int diff, ThreadData& data)
{
	uint64_t whiteKey = board.WhiteNonPawnKey;
	uint64_t blackKey = board.BlackNonPawnKey;

	const int scaledDiff = diff * CORRHIST_GRAIN;
	const int newWeight = std::min(depth + 1, 16);

	int& whiteEntry = data.nonPawnCorrHist[White][board.side][whiteKey % CORRHIST_SIZE];

	whiteEntry = (whiteEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	whiteEntry = std::clamp(whiteEntry, -CORRHIST_MAX, CORRHIST_MAX);

	int& blackEntry = data.nonPawnCorrHist[Black][board.side][blackKey % CORRHIST_SIZE];

	blackEntry = (blackEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
	blackEntry = std::clamp(blackEntry, -CORRHIST_MAX, CORRHIST_MAX);
}
int adjustEvalWithCorrHist(Board& board, const int rawEval, Move prevMove, ThreadData& data)
{
	uint64_t pawnKey = board.PawnKey;
	const int& pawnEntry = data.pawnCorrHist[board.side][pawnKey % CORRHIST_SIZE];


	uint64_t minorKey = board.MinorKey;
	const int& minorEntry = data.minorCorrHist[board.side][minorKey % CORRHIST_SIZE];

	uint64_t whiteNPKey = board.WhiteNonPawnKey;
	const int& whiteNPEntry = data.nonPawnCorrHist[White][board.side][whiteNPKey % CORRHIST_SIZE];
	uint64_t blackNPKey = board.BlackNonPawnKey;
	const int& blackNPEntry = data.nonPawnCorrHist[Black][board.side][blackNPKey % CORRHIST_SIZE];

	const int& contEntry = data.counterMoveCorrHist[prevMove.Piece][prevMove.To];

	int mate_found = 49000 - 99;

	int adjust = 0;

	adjust += pawnEntry * PAWN_CORRHIST_MULTIPLIER;
	adjust += minorEntry * MINOR_CORRHIST_MULTIPLIER;
	adjust += contEntry * COUNTERMOVE_CORRHIST_MULTIPLIER;
	adjust += (whiteNPEntry + blackNPEntry) * NONPAWN_CORRHIST_MULTIPLIER;
	adjust /= 128;

	return std::clamp(rawEval + adjust / CORRHIST_GRAIN, -mate_found + 1, mate_found - 1);
}
void updateCaptureHistory(int piece_attacking, int to, int piece_captured, int bonus, ThreadData& data)
{
	int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
	data.CaptureHistory[piece_attacking][to][piece_captured] += clampedBonus - data.CaptureHistory[piece_attacking][to][piece_captured] * abs(clampedBonus) / MAX_HISTORY;
}
void updateHistory(int stm, int from, int to, int bonus, uint64_t opp_threat, ThreadData& data)
{
	int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
	data.mainHistory[stm][from][to][Get_bit(opp_threat, from)][Get_bit(opp_threat, to)] += clampedBonus - data.mainHistory[stm][from][to][Get_bit(opp_threat, from)][Get_bit(opp_threat, to)] * abs(clampedBonus) / MAX_HISTORY;
}
int getSingleContinuationHistoryScore(Move move, const int offSet, ThreadData& data)
{
	if (data.ply >= offSet)
	{
		Move previousMove = data.searchStack[data.ply - offSet].move;

		if (offSet == 1)
		{
			return data.onePlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To];
		}
		else if (offSet == 2)
		{
			return data.twoPlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To];
		}
	}
	return 0;
}
int getContinuationHistoryScore(Move& move, ThreadData& data)
{
	if (data.ply >= 1)
	{
		int onePly = getSingleContinuationHistoryScore(move, 1, data);
		int twoPly = getSingleContinuationHistoryScore(move, 2, data);


		int finalScore = onePly + twoPly;
		return finalScore;
	}
	return 0;
}
void updateSingleContinuationHistoryScore(Move& move, const int bonus, const int offSet, ThreadData& data)
{
	if (data.ply >= offSet) {
		Move previousMove = data.searchStack[data.ply - offSet].move;

		int clampedBonus = std::clamp(bonus, -MAX_CONTHIST, MAX_CONTHIST);
		const int scaledBonus = clampedBonus - getSingleContinuationHistoryScore(move, offSet, data) * abs(clampedBonus) / MAX_CONTHIST;
		//std::cout << scaledBonus;

		if (offSet == 1)
		{
			data.onePlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To] += scaledBonus;
		}
		else if (offSet == 2)
		{
			data.twoPlyContHist[previousMove.Piece][previousMove.To][move.Piece][move.To] += scaledBonus;
		}
	}
}
void updateContinuationHistoryScore(Move& move, const int bonus, ThreadData& data)
{
	updateSingleContinuationHistoryScore(move, bonus, 1, data);
	updateSingleContinuationHistoryScore(move, bonus, 2, data);
}