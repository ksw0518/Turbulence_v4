#include "Datagen.h"
#include "Search.h"
#include "const.h"
#include "BitManipulation.h"
#include "Board.h"
#include "MoveGeneration.h"
#include "Evaluation.h"
#include <random>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

int randBool()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<int> dist(0, 1);
	return dist(gen);
}

int randBetween(int n)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, n); // Range 1 to n
	return dist(gen);
}


// Function to get the castling rights part of the FEN
std::string getCastlingRights(const Board& board)
{
	std::string rights = "";
	if (board.castle & WhiteKingCastle) rights += "K"; // White kingside castling
	if (board.castle & WhiteQueenCastle) rights += "Q"; // White queenside castling
	if (board.castle & BlackKingCastle) rights += "k"; // Black kingside castling
	if (board.castle & BlackQueenCastle) rights += "q"; // Black queenside castling
	return (rights.empty()) ? "-" : rights;
}
std::string boardToFEN(const Board& board)
{
	// Step 1: Construct the piece placement part (ranks 8 to 1)
	std::string piecePlacement = "";
	for (int rank = 7; rank >= 0; --rank) {  // Fix: Iterate from 7 down to 0
		int emptyCount = 0;
		for (int file = 0; file < 8; ++file) {
			int square = (7 - rank) * 8 + file;
			int piece = board.mailbox[square];

			if (piece == NO_PIECE) {
				++emptyCount; // Empty square
			}
			else {
				if (emptyCount > 0) {
					piecePlacement += std::to_string(emptyCount); // Insert the number of empty squares
					emptyCount = 0;
				}
				piecePlacement += getCharFromPiece(piece); // Add the piece (e.g., 'p', 'R')
			}
		}

		if (emptyCount > 0) {
			piecePlacement += std::to_string(emptyCount); // End of rank with empty squares
		}

		if (rank > 0) {  // Fix: Only add '/' if it's not the last rank (rank 1)
			piecePlacement += "/";
		}
	}

	// Step 2: Construct the other parts of FEN
	std::string sideToMove = (board.side == 0) ? "w" : "b"; // White or Black to move
	std::string castlingRights = getCastlingRights(board); // Castling rights
	std::string enPassant = (board.enpassent == NO_SQ) ? "-" : CoordinatesToChessNotation(board.enpassent); // En passant square
	std::string halfmove = std::to_string(board.halfmove); // Halfmove clock
	std::string fullmove = std::to_string(board.history.size() / 2 + 1); // Fullmove number

	// Step 3: Combine all parts into the final FEN string
	std::string fen = piecePlacement + " " + sideToMove + " " + castlingRights + " " + enPassant + " " + halfmove + " " + fullmove;

	return fen;
}
bool isBoardFine(Board& board)
{
	if (board.bitboards[K] != 0ULL && board.bitboards[k] != 0ULL)
	{
		if (count_bits(board.bitboards[K] | board.bitboards[k]) == 2)
		{
			if (((board.bitboards[p] | board.bitboards[P]) & 0xff000000000000ff) == 0ULL)
			{
				return true;
			}
		}

	}
	return false;
}

bool isNoLegalMoves(Board& board, MoveList& moveList)
{
	int searchedMoves = 0;

	uint64_t lastZobrist = board.zobristKey;
	uint64_t lastPawnKey = board.PawnKey;
	uint64_t lastMinorKey = board.MinorKey;
	uint64_t lastWhiteNPKey = board.WhiteNonPawnKey;
	uint64_t lastBlackNPKey = board.BlackNonPawnKey;
	AccumulatorPair last_accumulator = board.accumulator;
	for (int i = 0; i < moveList.count; ++i)
	{
		Move& move = moveList.moves[i];
		int lastEp = board.enpassent;
		uint64_t lastCastle = board.castle;
		int lastside = board.side;
		int captured_piece = board.mailbox[move.To];
		int last_irreversible = board.lastIrreversiblePly;
		int last_halfmove = board.halfmove;

		refresh_if_cross(move, board);
		MakeMove(board, move);
		if (!isLegal(move, board))
		{
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

		searchedMoves++;
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
		break;

	}
	if (searchedMoves == 0)
	{
		return true;
	}
	return false;

}

void PickRandomPos(Board& board, ThreadData& data)
{
	const std::string start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
randomPos:int randomMovesNum = 8 + randBool();
	board.history.clear();
	parse_fen(start_pos, board);
	board.zobristKey = generate_hash_key(board);
	board.history.push_back(board.zobristKey);
	initializeLMRTable(data);
	//Initialize_TT(16);




	//return;

	//std::cout << randomMovesNum;
	for (int i = 0; i < randomMovesNum; i++)
	{
		MoveList moveList;
		moveList.count = 0;
		Generate_Legal_Moves(moveList, board, false);
		AccumulatorPair last_accumulator = board.accumulator;
		//goto randomPos;
		if (moveList.count == 0 || isNoLegalMoves(board, moveList))//game is already over, restart the pos generation
		{
			std::cout << "b";
			goto randomPos;
		}
		int randomN;
		while (true)
		{
			randomN = randBetween(moveList.count) - 1;
			Move move = moveList.moves[randomN];
			uint64_t last_zobrist = board.zobristKey;
			uint64_t last_pawnKey = board.PawnKey;
			uint64_t last_minorKey = board.MinorKey;
			uint64_t last_whitenpKey = board.WhiteNonPawnKey;
			uint64_t last_blacknpKey = board.BlackNonPawnKey;
			int lastEp = board.enpassent;
			uint64_t lastCastle = board.castle;
			int lastside = board.side;
			int captured_piece = board.mailbox[move.To];
			int last_irreversible = board.lastIrreversiblePly;
			int last_halfmove = board.halfmove;

			refresh_if_cross(move, board);
			MakeMove(board, move);
			if (((move.Type & captureFlag) != 0) || move.Piece == p || move.Piece == P)
			{
				board.lastIrreversiblePly = board.history.size();
			}
			if (isLegal(move, board))
			{
				break;
			}
			else
			{
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
				board.accumulator = last_accumulator;
			}
		}
	}
	if (!isBoardFine(board))
	{
		//std::cout << "a";
		goto randomPos;
	}
	resetAccumulators(board, board.accumulator);

	int whiteKingFile = getFile(get_ls1b(board.bitboards[K]));
	if (whiteKingFile >= 4)//king is on right now, have to flip
	{
		resetWhiteAccumulator(board, board.accumulator, true);
	}
	if (whiteKingFile <= 3)//king is on left now, have to flip
	{
		resetWhiteAccumulator(board, board.accumulator, false);
	}



	int blackKingFile = getFile(get_ls1b(board.bitboards[k]));
	if (blackKingFile >= 4)//king is on right now, have to flip
	{
		resetBlackAccumulator(board, board.accumulator, true);
	}
	if (blackKingFile <= 3)//king is on left now, have to flip
	{
		resetBlackAccumulator(board, board.accumulator, false);
	}

}
bool isDecisive(int score)
{
	if (score > 48000 || score < -48000)
	{
		return true;
	}
	return false;
}
struct GameData
{
	Board board;
	int eval;
	int result;
	GameData(Board b, int e, int r) : board(b), eval(e), result(r) {}
};

int flipResult(int res)
{
	return 2 - res;
}

void appendToFile(const std::string& filename, const std::string& data)
{
	std::ofstream file(filename, std::ios::app | std::ios::out); // Open file in append mode, create if not exists
	if (!file)
	{
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}
	file << data << std::endl; // Write data to file
	file.close();
}
std::string convertWDL(int wdl)
{
	if (wdl == WHITEWIN)
	{
		return "1.0";
	}
	else if (wdl == DRAW)
	{
		return "0.5";
	}
	else
	{
		return "0.0";
	}
}
void estimate_time_remaining(uint64_t remaining_positions, int pps)
{
	if (pps <= 0) {
		std::cout << "Invalid PPS value!" << std::endl;
		return;
	}

	double seconds_remaining = double(remaining_positions) / pps;

	int hours = static_cast<int>(seconds_remaining) / 3600;
	int minutes = (static_cast<int>(seconds_remaining) % 3600) / 60;
	int seconds = static_cast<int>(seconds_remaining) % 60;

	std::cout << "Estimated remaining time: "
		<< hours << "h " << minutes << "m " << seconds << "s"
		<< "\n";
}
void print_progress_bar(double percentage)
{
	int barWidth = 50;  // Width of the progress bar
	int progress = static_cast<int>(percentage / 2);  // Calculate the number of '#' to print

	std::cout << "\n[";
	for (int i = 0; i < barWidth; ++i)
	{
		if (i < progress)
		{
			std::cout << "#";  // Print filled part of the progress bar
		}
		else
		{
			std::cout << "-";  // Print empty part of the progress bar
		}
	}
	std::cout << "] " << static_cast<int>(percentage) << "%";
}
bool fileExists(const std::string& filename)
{
	std::ifstream file(filename);
	return file.good();
}
std::vector<std::string> splitByPipe(const std::string& input)
{
	std::vector<std::string> tokens;
	std::stringstream ss(input);
	std::string token;

	while (std::getline(ss, token, '|')) {
		tokens.push_back(token);
	}

	return tokens;
}

void Datagen(int targetPos, std::string output_name)
{
	ThreadData* heapAllocated = new ThreadData(); // Allocate safely on heap
	ThreadData& data = *heapAllocated;

	const uint64_t targetPositions = static_cast<uint64_t>(targetPos);
	uint64_t totalPositions = 0;
	std::vector<GameData> gameData;
	gameData.reserve(256);

	std::ofstream file(output_name + ".txt", std::ios::app | std::ios::out); // Open file once
	if (!file)
	{
		std::cerr << "Error opening file: " << output_name << std::endl;
		return;
	}

	auto start_time = std::chrono::high_resolution_clock::now();
	SearchLimitations searchLimits;
	searchLimits.SoftNodeLimit = 5000;
	searchLimits.HardNodeLimit = 10000;
	Board board;
	PickRandomPos(board, data);
	Initialize_TT(16);


	while (totalPositions < targetPositions)
	{

		gameData.clear();
		//Board board;


		PickRandomPos(board, data);
		while (!isBoardFine(board))
		{
			std::cout << "fuck";
			Initialize_TT(16);
			PickRandomPos(board, data);
		}



		bool isGameOver = false;
		int result = -1;

		//int moves = 0;
		while (!isGameOver)
		{
			//std::cout << "asdf";
			//resetAccumulators(board, board.accumulator);

			//int whiteKingFile = getFile(get_ls1b(board.bitboards[K]));
			//if (whiteKingFile >= 4)//king is on right now, have to flip
			//{
			//	resetWhiteAccumulator(board, board.accumulator, true);
			//}
			//if (whiteKingFile <= 3)//king is on left now, have to flip
			//{
			//	resetWhiteAccumulator(board, board.accumulator, false);
			//}



			//int blackKingFile = getFile(get_ls1b(board.bitboards[k]));
			//if (blackKingFile >= 4)//king is on right now, have to flip
			//{
			//	resetBlackAccumulator(board, board.accumulator, true);
			//}
			//if (blackKingFile <= 3)//king is on left now, have to flip
			//{
			//	resetBlackAccumulator(board, board.accumulator, false);
			//}
			auto searchResult = IterativeDeepening(board, 99, searchLimits, data, false);
			Move bestMove;
			//bestMove.To = -1;
			if (searchResult.first.From == 0 && searchResult.first.To == 0)
			{
				searchLimits.SoftNodeLimit = 999999999;
				searchLimits.HardNodeLimit = 999999999;
				searchResult = IterativeDeepening(board, 1, searchLimits, data, false);
				searchLimits.SoftNodeLimit = 5000;
				searchLimits.HardNodeLimit = 10000;
			}
			bestMove = searchResult.first;
			//Move bestMove;
			int eval = searchResult.second;
			if (board.side == Black) eval = -eval;



			Board prevBoard = board;
			if (!isBoardFine(board))
			{
				std::cout << "fuckasdf";
				Initialize_TT(16);
				break;
			}
			refresh_if_cross(bestMove, board);
			MakeMove(board, bestMove);


			MoveList moveList;
			Generate_Legal_Moves(moveList, board, false);
			if (!isBoardFine(board))
			{
				std::cout << "fuckksw";
				Initialize_TT(16);
				break;
			}
			if (isNoLegalMoves(board, moveList))
			{
				result = is_in_check(board) ? (board.side == White ? BLACKWIN : WHITEWIN) : DRAW;
				break;
			}
			if (is_threefold(board.history, board.lastIrreversiblePly) || isInsufficientMaterial(board) || board.halfmove >= 100)
			{
				result = DRAW;
				break;
			}
			if (isDecisive(eval))
			{
				result = (eval > 48000) ? (board.side == White ? WHITEWIN : BLACKWIN) : (board.side == White ? BLACKWIN : WHITEWIN);
			}


			//moves++;
			if (!is_in_check(prevBoard) && (bestMove.Type & captureFlag) == 0 && !isDecisive(eval))
			{
				gameData.push_back(GameData(prevBoard, eval, -1));
				totalPositions++;
			}
		}

		// **Batch write game data to file instead of writing each line separately**
		std::ostringstream buffer;
		for (size_t i = 0; i < gameData.size(); i++)
		{
			gameData[i].result = result;
			buffer << boardToFEN(gameData[i].board) << " | " << gameData[i].eval << " | " << convertWDL(gameData[i].result) << "\n";
		}
		file << buffer.str(); // **Write all at once**

		auto end_time = std::chrono::high_resolution_clock::now();
		double elapsed_seconds = std::chrono::duration<double>(end_time - start_time).count();
		double positions_per_second = totalPositions / elapsed_seconds;
		double percentage = (static_cast<double>(totalPositions) / targetPos) * 100;

		setColor(ConsoleColor::BrightGreen);
		std::cout << "Positions/s: " << std::fixed << std::setprecision(2) << positions_per_second;

		setColor(ConsoleColor::BrightCyan);
		std::cout << " | Total: " << totalPositions << " ("
			<< std::fixed << std::setprecision(2) << (totalPositions / 1'000'000.0) << "M)";

		setColor(ConsoleColor::BrightYellow);
		std::cout << " | Progress: " << std::fixed << std::setprecision(4) << percentage << "% ";

		setColor(ConsoleColor::White); // Reset to default
		estimate_time_remaining(targetPositions - totalPositions, positions_per_second);
		print_progress_bar(percentage);
		std::cout << "\n\n" << std::flush;
	}

	file.close(); // **Close file only once after everything is written**
	delete heapAllocated;
}
