#pragma once
#include "Search.h"
constexpr int MAX_HISTORY = 16384;
constexpr int MAX_CONTHIST = 16384;
int scaledBonus(int score, int bonus);
void updateMinorCorrHist(Board& board, const int depth, const int diff, ThreadData& data);
void updatePawnCorrHist(Board& board, const int depth, const int diff, ThreadData& data);
void updateCounterCorrHist(Move prevMove, const int depth, const int diff, ThreadData& data);
void updateNonPawnCorrHist(Board& board, const int depth, const int diff, ThreadData& data);
int adjustEvalWithCorrHist(Board& board, const int rawEval, Move prevMove, ThreadData& data);
void updateCaptureHistory(int piece_attacking, int to, int piece_captured, int bonus, ThreadData& data);
void updateHistory(int stm, int from, int to, int bonus, uint64_t opp_threat, ThreadData& data);
int getSingleContinuationHistoryScore(Move move, const int offSet, ThreadData& data);
int getContinuationHistoryScore(Move& move, ThreadData& data);
void updateSingleContinuationHistoryScore(Move& move, const int bonus, const int offSet, ThreadData& data);
void updateContinuationHistoryScore(Move& move, const int bonus, ThreadData& data);