

#include "MoveGeneration.h"
#include "Evaluation.h"
#include "Board.h"
#include "Search.h"
#include "const.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <fstream> 
#include <bit>
std::vector<std::string> option_name = {
    "RFP_MULTIPLIER",
    "RFP_IMPROVING_MULTIPLIER",
    "RFP_BASE",
    "RFP_IMPROVING_BASE",
    "LMP_BASE",
    "LMP_MULTIPLIER",
    "PVS_QUIET_BASE",
    "PVS_QUIET_MULTIPLIER",
    "PVS_NOISY_BASE",
    "PVS_NOISY_MULTIPLIER",
    "HISTORY_BASE",
    "HISTORY_MULTIPLIER",
    "ASP_WINDOW_INITIAL",
    "ASP_WINDOW_MAX",
    "PAWN_CORRHIST_MULTIPLIER",
    "MINOR_CORRHIST_MULTIPLIER",
    "NONPAWN_CORRHIST_MULTIPLIER",
    "HISTORY_PRUNING_MULTIPLIER",
    "HISTORY_PRUNING_BASE",
    "HISTORY_LMR_MULTIPLIER",
    "HISTORY_LMR_BASE",
    "NMP_EVAL_DIVISER",
    "NMP_DEPTH_DIVISER",
    "MAX_NMP_EVAL_R",
	"DEXT_MARGIN"
};

std::vector<int> option_base = {
	89,           // RFP_MULTIPLIER
	65,           // RFP_IMPROVING_MULTIPLIER
	-36,          // RFP_BASE
	-39,          // RFP_IMPROVING_BASE
	0,            // LMP_BASE
	1,            // LMP_MULTIPLIER
	4,            // PVS_QUIET_BASE
	57,           // PVS_QUIET_MULTIPLIER
	-3,           // PVS_NOISY_BASE
	14,           // PVS_NOISY_MULTIPLIER
	4 * 32,       // HISTORY_BASE
	3 * 32,       // HISTORY_MULTIPLIER
	38,           // ASP_WINDOW_INITIAL
	311,          // ASP_WINDOW_MAX
	179,          // PAWN_CORRHIST_MULTIPLIER
	154,          // MINOR_CORRHIST_MULTIPLIER
	179,          // NONPAWN_CORRHIST_MULTIPLIER
	41 * 32,      // HISTORY_PRUNING_MULTIPLIER
	2 * 32,       // HISTORY_PRUNING_BASE
	24 * 32,      // HISTORY_LMR_MULTIPLIER
	3 * 32,       // HISTORY_LMR_BASE
	418,          // NMP_EVAL_DIVISER
	4,            // NMP_DEPTH_DIVISER
	3,            // MAX_NMP_EVAL_R
	20            // DEXT_MARGIN
};


std::vector<int> option_min = {
	50,     // RFP_MULTIPLIER
	30,     // RFP_IMPROVING_MULTIPLIER
	-100,   // RFP_BASE
	-100,   // RFP_IMPROVING_BASE
	-1,     // LMP_BASE
	1,      // LMP_MULTIPLIER
	-70,    // PVS_QUIET_BASE
	20,     // PVS_QUIET_MULTIPLIER
	-70,    // PVS_NOISY_BASE
	5,      // PVS_NOISY_MULTIPLIER
	0,      // HISTORY_BASE
	1,      // HISTORY_MULTIPLIER
	20,     // ASP_WINDOW_INITIAL
	100,    // ASP_WINDOW_MAX
	1,      // PAWN_CORRHIST_MULTIPLIER
	1,      // MINOR_CORRHIST_MULTIPLIER
	1,      // NONPAWN_CORRHIST_MULTIPLIER
	10,     // HISTORY_PRUNING_MULTIPLIER
	-200,   // HISTORY_PRUNING_BASE
	10,     // HISTORY_LMR_MULTIPLIER
	-100,   // HISTORY_LMR_BASE
	200,    // NMP_EVAL_DIVISER
	2,      // NMP_DEPTH_DIVISER
	2,      // MAX_NMP_EVAL_R
	5       // DEXT_MARGIN
};

std::vector<int> option_max = {
	180,    // RFP_MULTIPLIER          (89 + 10% = ~98 ¡æ min 10 ¡æ 180)
	150,    // RFP_IMPROVING_MULTIPLIER
	100,    // RFP_BASE
	100,    // RFP_IMPROVING_BASE
	20,     // LMP_BASE                (6 + margin)
	20,     // LMP_MULTIPLIER          (3 + margin)
	120,    // PVS_QUIET_BASE          (4 + margin)
	200,    // PVS_QUIET_MULTIPLIER    (57 + margin)
	50,     // PVS_NOISY_BASE
	100,    // PVS_NOISY_MULTIPLIER    (14 + margin)
	200,    // HISTORY_BASE            (128 + margin)
	160,    // HISTORY_MULTIPLIER      (96 + margin)
	150,    // ASP_WINDOW_INITIAL
	1200,   // ASP_WINDOW_MAX          (311 + margin)
	250,    // PAWN_CORRHIST_MULTIPLIER
	200,    // MINOR_CORRHIST_MULTIPLIER
	250,    // NONPAWN_CORRHIST_MULTIPLIER
	1500,   // HISTORY_PRUNING_MULTIPLIER
	200,    // HISTORY_PRUNING_BASE
	900,    // HISTORY_LMR_MULTIPLIER
	150,    // HISTORY_LMR_BASE
	1000,   // NMP_EVAL_DIVISER
	10,     // NMP_DEPTH_DIVISER
	10,     // MAX_NMP_EVAL_R
	150     // DEXT_MARGIN
};



std::vector<int*> option_var = {
    &RFP_MULTIPLIER,
    &RFP_IMPROVING_MULTIPLIER,
    &RFP_BASE,
    &RFP_IMPROVING_BASE,
    &LMP_BASE,
    &LMP_MULTIPLIER,
    &PVS_QUIET_BASE,
    &PVS_QUIET_MULTIPLIER,
    &PVS_NOISY_BASE,
    &PVS_NOISY_MULTIPLIER,
    &HISTORY_BASE,
    &HISTORY_MULTIPLIER,
    &ASP_WINDOW_INITIAL,
    &ASP_WINDOW_MAX,
    &PAWN_CORRHIST_MULTIPLIER,
    &MINOR_CORRHIST_MULTIPLIER,
    &NONPAWN_CORRHIST_MULTIPLIER,
    &HISTORY_PRUNING_MULTIPLIER,
    &HISTORY_PRUNING_BASE,
    &HISTORY_LMR_MULTIPLIER,
    &HISTORY_LMR_BASE,
    &NMP_EVAL_DIVISER,
    &NMP_DEPTH_DIVISER,
    &MAX_NMP_EVAL_R,
	&DEXT_MARGIN
};



/*int RFP_MULTIPLIER = 75;
int RFP_BASE = 0;

int LMP_BASE = 1;
int LMP_MULTIPLIER = 3;

int PVS_QUIET_BASE = 0;
int PVS_QUIET_MULTIPLIER = 70;

int PVS_NOISY_BASE = 0;
int PVS_NOISY_MULTIPLIER = 20;

int HISTORY_BASE = 0;
int HISTORY_MULTIPLIER = 1;
*/

// generate 32-bit pseudo legal numbers




const std::string start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const std::string kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";

std::vector<std::string> position_commands = { "position", "startpos", "fen", "moves" };
std::vector<std::string> go_commands = { "go", "movetime", "wtime", "btime", "winc", "binc", "movestogo" };
std::vector<std::string> option_commands = { "setoption", "name",  "value" };
std::vector<std::string> datagen_commands = { "datagen", "pos", "file" };
std::vector<std::string> filter_commands = { "filter", "input", "output"};

Board main_board;

int perft_depth;
std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    const auto start = str.find_first_not_of(whitespace);
    const auto end = str.find_last_not_of(whitespace);

    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }

    return str.substr(start, end - start + 1);
}

std::string TryGetLabelledValue(const std::string& text, const std::string& label, const std::vector<std::string>& allLabels, const std::string& defaultValue = "") {
    
    // Trim leading and trailing whitespace
    std::string trimmedText = trim(text);

    // Find the position of the label in the trimmed text
    size_t labelPos = trimmedText.find(label);
    if (labelPos != std::string::npos) {
        // Determine the start position of the value
        size_t valueStart = labelPos + label.length();
        size_t valueEnd = trimmedText.length();

        // Iterate through allLabels to find the next label position
        for (const std::string& otherID : allLabels) {
            if (otherID != label) {
                size_t otherIDPos = trimmedText.find(otherID, valueStart);
                if (otherIDPos != std::string::npos && otherIDPos < valueEnd) {
                    valueEnd = otherIDPos;
                }
            }
        }

        // Extract the value and trim leading/trailing whitespace
        std::string value = trimmedText.substr(valueStart, valueEnd - valueStart);
        return trim(value);
    }

    return defaultValue;
}
int64_t TryGetLabelledValueInt(const std::string& text, const std::string& label, const std::vector<std::string>& allLabels, int64_t defaultValue = 0)
{
    // Helper function TryGetLabelledValue should be implemented as shown earlier
    std::string valueString = TryGetLabelledValue(text, label, allLabels, std::to_string(defaultValue));

    // Extract the first part of the valueString up to the first space
    std::istringstream iss(valueString);
    std::string firstPart;
    iss >> firstPart;

    // Try converting the extracted string to an integer
    try
    {
        return std::stoll(firstPart);
    }
    catch (const std::invalid_argument& e)
    {
        // If conversion fails, return the default value
        return defaultValue;
    }
}
std::vector<std::string> splitStringBySpace(const std::string& str)
{
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;

    while ((end = str.find(' ', start)) != std::string::npos)
    {
        if (end != start)
        { // Ignore multiple consecutive spaces
            tokens.push_back(str.substr(start, end - start));
        }
        start = end + 1;
    }

    // Push the last token if it's not empty
    if (start < str.size())
    {
        tokens.push_back(str.substr(start));
    }

    return tokens;
}

static uint64_t Perft(Board& board, int depth)
{

    if (depth == 0)
    {
        return 1ULL;
    }

    MoveList move_list;


    uint64_t nodes = 0;

    Generate_Legal_Moves(move_list, board, false);

    for (int i = 0; i < move_list.count; ++i)
    {
        Move& move = move_list.moves[i];
        int lastEp = board.enpassent;
        uint64_t lastCastle = board.castle;
        int lastside = board.side;
        int captured_piece = board.mailbox[move.To];

        uint64_t last_zobrist = board.zobristKey;
        uint64_t last_pawnKey = board.PawnKey;
        uint64_t last_minorKey = board.MinorKey;
        uint64_t last_whiteNPKey = board.WhiteNonPawnKey;
        uint64_t last_blackNPKey = board.BlackNonPawnKey;
        
        MakeMove(board, move);


        uint64_t blackNPKey_from_scratch = generate_BlackNP_Hash(board);
        if (board.BlackNonPawnKey != blackNPKey_from_scratch)
        {
            std::cout << "CRITICAL ERROR: black np key doesn't match\n";
            printMove(move);
            std::cout << "\n\n";
        }
        if (isLegal(move, board))
        {
            uint64_t nodes_added = Perft(board, depth - 1);
            nodes += nodes_added;
        }


        UnmakeMove(board, move, captured_piece);

        board.enpassent = lastEp;
        board.castle = lastCastle;
        board.side = lastside;
        board.zobristKey = last_zobrist;
        board.PawnKey = last_pawnKey;
        board.MinorKey = last_minorKey;
        board.WhiteNonPawnKey = last_whiteNPKey;
        board.BlackNonPawnKey = last_blackNPKey;

    }
    return nodes;
}

int Calculate_Hard_Bound(int64_t time, int64_t incre)
{
    return time / 2;
}
int Calculate_Soft_Bound(int64_t time, int64_t incre)
{
    return  0.6 * (static_cast<float>(time) / static_cast<float>(20) + static_cast<float>(incre) * static_cast<float>(3) / static_cast<float>(4));
}
void Initialize_TT(int size)
{
    //std::cout <<"size" << size << "\n";
    uint64_t bytes = static_cast<uint64_t>(size) * 1024ULL * 1024ULL;

    //std::cout << bytes<<"\n";
    TTSize = bytes / sizeof(TranspositionEntry);

    if (TTSize % 2 != 0)
    {
        TTSize -= 1;
    }

    if (TranspositionTable)
        delete [] TranspositionTable;

    TranspositionTable = new TranspositionEntry[TTSize]();
    
}
inline int get_ls1b(uint64_t bitboard) {
	return std::countr_zero(bitboard); // or return -1;
}

bool operator==(AccumulatorPair& a, AccumulatorPair& b) {
	for (int i = 0; i < HL_SIZE; ++i) {
		if (a.white.values[i] != b.white.values[i])
			return false;
		if (a.black.values[i] != b.black.values[i])
			return false;
	}
	return true;
}
//inline int getFile(int square)
//{
//	return (square) % 8;
//}
void ProcessUCI(std::string input, ThreadData& data, ThreadData* data_heap)
{
    //std::cout << (input) << "\n";
    //std::string input = "This  is   a  sample string";
    std::vector<std::string> Commands = splitStringBySpace(input);
    std::string main_command = Commands[0];
	
    if (main_command == "uci")
    {
        
        std::cout << "id name Turbulence_v4 v0.0.7" << "\n";;
        std::cout << "id author ksw0518" << "\n";;
        std::cout << "\n";
        std::cout << "option name Threads type spin default 1 min 1 max 1\n";
        std::cout << "option name Hash type spin default 12 min 1 max 4096\n";
        //for (int i = 0; i < option_name.size(); i++)//for spsa
        //{
        //    std::cout << "option name " << option_name[i];
        //    std::cout << " type spin ";
        //    std::cout << " default " << option_base[i];
        //    std::cout << " min " << option_min[i];
        //    std::cout << " max " << option_max[i];
        //    std::cout << "\n";
        //}
        isPrettyPrinting = false;
        
        std::cout << "uciok" << "\n";
    }
    else if (main_command == "datagen")
    {
        int pos = TryGetLabelledValueInt(input, "pos", datagen_commands);
        std::string file = TryGetLabelledValue(input, "file", datagen_commands);
        
        Datagen(pos, file);
    }
    else if (main_command == "ucinewgame")
    {
        
        Initialize_TT(16);
        initializeLMRTable(data);
        isPrettyPrinting = false;
    }
    else if (main_command == "setoption")
    {
        std::string option = TryGetLabelledValue(input, "name", option_commands);
        int value = TryGetLabelledValueInt(input, "value", option_commands);
       
        if (option == "Hash")
        {
            Initialize_TT(value);
        }
        else
        {
            for (size_t i = 0; i < option_name.size(); i++)
            {
                if (option == option_name[i])
                {
                    *option_var[i] = value;
                }
            }
        }

    }
    else if (main_command == "isready")
    {
        std::cout << "readyok"<< "\n";

    }
    else if (main_command == "quit")
    {
		//delete data_heap;
        exit(0);
    }
    else if (main_command == "position")
    {
        main_board.history.clear();
        if (Commands[1] == "startpos")
        {
            if (Commands.size() == 2)
            {

                parse_fen(start_pos, main_board);
                main_board.zobristKey = generate_hash_key(main_board);
                main_board.history.push_back(main_board.zobristKey);
            }
            else
            {
                parse_fen(start_pos, main_board);
                main_board.zobristKey = generate_hash_key(main_board);
                main_board.history.push_back(main_board.zobristKey);

                std::string moves_in_string = TryGetLabelledValue(input, "moves", position_commands);
                if (moves_in_string != "") // move is not empty
                {
                    std::vector<std::string> moves_seperated = splitStringBySpace(moves_in_string);
                    MoveList moveList;

                    for (size_t i = 0; i < moves_seperated.size(); i++)
                    {
                        std::string From = std::string(1, moves_seperated[i][0]) + std::string(1, moves_seperated[i][1]);
                        std::string To = std::string(1, moves_seperated[i][2]) + std::string(1, moves_seperated[i][3]);
                        std::string promo = "";
                        
                        if (moves_seperated[i].size() > 4)
                        {
                            promo = std::string(1, (moves_seperated[i][4]));

                        }
                        Move move_to_play;
                        move_to_play.From = GetSquare(From);
                        move_to_play.To = GetSquare(To);

                        moveList.clear();
                        Generate_Legal_Moves(moveList, main_board, false);

                        for (size_t j = 0; j < moveList.count; j++)
                        {
   

                            if ((move_to_play.From == moveList.moves[j].From) && (move_to_play.To == moveList.moves[j].To)) //found same move
                            {
								move_to_play = moveList.moves[j];
								if (get_piece(move_to_play.Piece, White) == K)//king has moved
								{
									if (getFile(move_to_play.From) <= 3)//king was left before
									{
										if (getFile(move_to_play.To) >= 4)//king moved to right 
										{
											//fully refresh the stm accumulator, and change that to start mirroring
											if (main_board.side == White)
											{
												resetWhiteAccumulator(main_board, main_board.accumulator, true);
											}
											else
											{
												resetBlackAccumulator(main_board, main_board.accumulator, true);
											}
										}
									}
									else//king was right before
									{
										if (getFile(move_to_play.To) <= 3)//king moved to left 
										{
											//fully refresh the stm accumulator, and change that to stop mirroring
											if (main_board.side == White)
											{
												resetWhiteAccumulator(main_board, main_board.accumulator, false);
											}
											else
											{
												resetBlackAccumulator(main_board, main_board.accumulator, false);
											}
										}
									}
								}

                                if ((moveList.moves[j].Type & knight_promo) != 0) // promo
                                {
                                    //std::cout << "promo" << "\n";
                                    //std::cout << promo << "\n";
                                    if (promo == "q")
                                    {
                                        //std::cout << "qpromo";
                                        if ((moveList.moves[j].Type == queen_promo) || (moveList.moves[j].Type == queen_promo_capture))
                                        {
                                            MakeMove(main_board, moveList.moves[j]);
                                            break;

                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "r")
                                    {
                                        if ((moveList.moves[j].Type == rook_promo) || (moveList.moves[j].Type == rook_promo_capture))
                                        {
                                            MakeMove(main_board, moveList.moves[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "b")
                                    {
                                        if ((moveList.moves[j].Type == bishop_promo) || (moveList.moves[j].Type == bishop_promo_capture))
                                        {
                                            MakeMove(main_board, moveList.moves[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "n")
                                    {
                                        if ((moveList.moves[j].Type == knight_promo) || (moveList.moves[j].Type == knight_promo_capture))
                                        {
                                            MakeMove(main_board, moveList.moves[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                }
                                else
                                {

                                    MakeMove(main_board, moveList.moves[j]);

                                    


                                    break;
                                }



                            }
                        }
                    
                    }

                }
            }
           
        }
        else if (Commands[1] == "fen")
        {
            std::string fen = TryGetLabelledValue(input, "fen", position_commands);
            std::string moves = TryGetLabelledValue(input, "moves", position_commands);

            if (moves == "")
            {
                parse_fen(fen, main_board);
                main_board.zobristKey = generate_hash_key(main_board);
            
                main_board.history.push_back(main_board.zobristKey);

            }
            else
            {
                parse_fen(fen, main_board);
                main_board.zobristKey = generate_hash_key(main_board);
                main_board.history.push_back(main_board.zobristKey);
                std::string moves_in_string = TryGetLabelledValue(input, "moves", position_commands);
                if (moves_in_string != "") // move is not empty
                {
                    std::vector<std::string> moves_seperated = splitStringBySpace(moves_in_string);
                    MoveList moveList;


                    for (size_t i = 0; i < moves_seperated.size(); i++)
                    {
                        std::string From = std::string(1, moves_seperated[i][0]) + std::string(1, moves_seperated[i][1]);
                        std::string To = std::string(1, moves_seperated[i][2]) + std::string(1, moves_seperated[i][3]);
                        std::string promo = "";
                        if (moves_seperated[i].size() > 4)
                        {
                            promo = std::string(1, (moves_seperated[i][4]));
                        }
                        Move move_to_play;
                        move_to_play.From = GetSquare(From);
                        move_to_play.To = GetSquare(To);

                        //std::cout << CoordinatesToChessNotation(move_to_play.From);
                        //std::cout << CoordinatesToChessNotation(move_to_play.To);
                        //std::cout << promo;
                        moveList.clear();
                        Generate_Legal_Moves(moveList, main_board, false);

                        for (size_t j = 0; j < moveList.count; j++)
                        {
                            //Console.WriteLine("12");
                            //nodes = 0;

                            if ((move_to_play.From == moveList.moves[j].From) && (move_to_play.To == moveList.moves[j].To)) //found same move
                            {

								move_to_play = moveList.moves[j];

								if (get_piece(move_to_play.Piece, White) == K)//king has moved
								{
									if (getFile(move_to_play.From) <= 3)//king was left before
									{
										if (getFile(move_to_play.To) >= 4)//king moved to right 
										{
											//fully refresh the stm accumulator, and change that to start mirroring
											if (main_board.side == White)
											{
												resetWhiteAccumulator(main_board, main_board.accumulator, true);
											}
											else
											{
												resetBlackAccumulator(main_board, main_board.accumulator, true);
											}
										}
									}
									else//king was right before
									{
										if (getFile(move_to_play.To) <= 3)//king moved to left 
										{
											//fully refresh the stm accumulator, and change that to stop mirroring
											if (main_board.side == White)
											{
												resetWhiteAccumulator(main_board, main_board.accumulator, false);
											}
											else
											{
												resetBlackAccumulator(main_board, main_board.accumulator, false);
											}
										}
									}
								}
                                if ((moveList.moves[j].Type & knight_promo) != 0) // promo
                                {
                                    if (promo == "q")
                                    {
                                        if ((moveList.moves[j].Type == queen_promo) || (moveList.moves[j].Type == queen_promo_capture))
                                        {
                                            MakeMove(main_board, moveList.moves[j]);
                                            break;

                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "r")
                                    {
                                        if ((moveList.moves[j].Type == rook_promo) || (moveList.moves[j].Type == rook_promo_capture))
                                        {
                                            MakeMove(main_board, moveList.moves[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "b")
                                    {
                                        if ((moveList.moves[j].Type == bishop_promo) || (moveList.moves[j].Type == bishop_promo_capture))
                                        {
                                            MakeMove(main_board, moveList.moves[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "n")
                                    {
                                        if ((moveList.moves[j].Type == knight_promo) || (moveList.moves[j].Type == knight_promo_capture))
                                        {
                                            MakeMove(main_board, moveList.moves[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                }
                                else
                                {

                                    MakeMove(main_board, moveList.moves[j]);


                                    break;
                                }



                            }
                        }

                    }

                }
            }
            //std::cout << TryGetLabelledValue(input, "fen", position_commands);
        }
        else if ((Commands[1] == "kiwi"))
        {
            parse_fen(kiwipete, main_board);
            main_board.zobristKey = generate_hash_key(main_board);
            main_board.history.push_back(main_board.zobristKey);
        }
        //std::cout << generate_Pawn_Hash(main_board);

    }
    else if (main_command == "perft")
    {
        if (Commands[0] == "perft")
        {

            perft_depth = std::stoi(Commands[1]);

            //std::cout << (perft_depth);
            auto start = std::chrono::high_resolution_clock::now();

            uint64_t nodes = Perft(main_board, perft_depth);
            auto end = std::chrono::high_resolution_clock::now();

            //std::chrono::duration<double, std::milli> elapsedMS = end - start;
            //std::chrono::duration<double, std::milli> elapsedS = end - start;

            //float second = elapsedMS.count() / 1000;

            //double nps = nodes / second;

            std::cout << (nodes);
            std::cout << "\n";


        }
        }
    else if (main_command == "go")
    {
		SearchLimitations searchLimits;
        if (Commands[1] == "depth")
        {
            //Negamax_nodecount = 0;
            if (Commands.size() == 3)
            {
                int depth = std::stoi(Commands[2]);
                IterativeDeepening(main_board, depth, searchLimits, data);

            }
        }
        else if (Commands[1] == "nodes")
        {
            int64_t node = std::stoll(Commands[2]);
			searchLimits.HardNodeLimit = node;
            IterativeDeepening(main_board, 99, searchLimits, data);
        }
        else if (Commands[1] == "movetime")
        {
            int64_t movetime = std::stoll(Commands[2]);
			searchLimits.HardTimeLimit = movetime;
            IterativeDeepening(main_board, 99, searchLimits, data);
        }
        else if (Commands[1] == "wtime")
        {
            int depth = TryGetLabelledValueInt(input, "depth", go_commands);
            int64_t wtime = TryGetLabelledValueInt(input, "wtime", go_commands);
			int64_t btime = TryGetLabelledValueInt(input, "btime", go_commands);
			int64_t winc = TryGetLabelledValueInt(input, "winc", go_commands);
			int64_t binc = TryGetLabelledValueInt(input, "binc", go_commands);
           

            if (depth != 0)
            {
                //int depth = std::stoi(Commands[2]);
                IterativeDeepening(main_board, depth, searchLimits, data);
                
            }
            else
            {
                int64_t hard_bound;
				int64_t soft_bound;
				int64_t baseTime = 0;
				int64_t maxTime = 0;
                if (main_board.side == White)
                {
                    hard_bound = Calculate_Hard_Bound(wtime, winc);
                    soft_bound = Calculate_Soft_Bound(wtime, winc);
                    //baseTime = wtime * DEF_TIME_MULTIPLIER + winc * DEF_INC_MULTIPLIER;
                    maxTime = std::max(1.00, wtime * MAX_TIME_MULTIPLIER);
                }
                else
                {
                    hard_bound = Calculate_Hard_Bound(btime, binc);
                    soft_bound = Calculate_Soft_Bound(btime, binc);
                    //baseTime = btime * DEF_TIME_MULTIPLIER + binc * DEF_INC_MULTIPLIER;
                    maxTime = std::max(1.00, btime * MAX_TIME_MULTIPLIER);
                }
				searchLimits.HardTimeLimit = hard_bound;
				searchLimits.SoftTimeLimit = soft_bound;
                IterativeDeepening(main_board, 99, searchLimits, data, true, maxTime);
            }
        }
        else
        {
            IterativeDeepening(main_board, 99, searchLimits, data);
        }
        //else if (Commands[1] == "perft")
        //{

        //}
    }
    else if (main_command == "show")
    {
        PrintBoards(main_board);
        print_mailbox(main_board.mailbox);
    }
    else if (main_command == "eval")
    {
        std::cout << ("evaluation: ") << Evaluate(main_board) << "cp ";
        if (main_board.side == White)
        {
            std::cout << ("(White's perspective)\n");
        }
        else
        {
            std::cout << ("(Black's perspective)\n");


            std::cout << ("White's perspective: ") << -Evaluate(main_board) << "cp \n";
        }



    }
    else if(main_command == "move")
    {
        std::string From = std::string(1, Commands[1][0]) + std::string(1, Commands[1][1]);
        std::string To = std::string(1, Commands[1][2]) + std::string(1, Commands[1][3]);
        std::string promo = "";
        if (Commands[1].size() > 4)
        {
            promo = std::string(1, (Commands[1][4]));
        }
        Move move_to_play;
        move_to_play.From = GetSquare(From);
        move_to_play.To = GetSquare(To);

        //std::cout << CoordinatesToChessNotation(move_to_play.From);
        //std::cout << CoordinatesToChessNotation(move_to_play.To);
        //std::cout << promo;

        MoveList moveList;
        moveList.clear();
        Generate_Legal_Moves(moveList, main_board, false);

        for (size_t j = 0; j < moveList.count; j++)
        {
            //Console.WriteLine("12");
            //nodes = 0;

            if ((move_to_play.From == moveList.moves[j].From) && (move_to_play.To == moveList.moves[j].To)) //found same move
            {
				move_to_play = moveList.moves[j];

				if (get_piece(move_to_play.Piece, White) == K)//king has moved
				{
					std::cout << "king move";
					if (getFile(move_to_play.From) <= 3)//king was left before
					{
						if (getFile(move_to_play.To) >= 4)//king moved to right 
						{
							//fully refresh the stm accumulator, and change that to start mirroring
							if (main_board.side == White)
							{
								resetWhiteAccumulator(main_board, main_board.accumulator, true);
								std::cout << "mirror white";
							}
							else
							{
								resetBlackAccumulator(main_board, main_board.accumulator, true);
								std::cout << "mirror black";
							}
						}
					}
					else//king was right before
					{
						if (getFile(move_to_play.To) <= 3)//king moved to left 
						{
							//fully refresh the stm accumulator, and change that to stop mirroring
							if (main_board.side == White)
							{
								resetWhiteAccumulator(main_board, main_board.accumulator, false);
								std::cout << "unmirror white";
							}
							else
							{
								resetBlackAccumulator(main_board, main_board.accumulator, false);
								std::cout << "unmirror black";

							}
						}
					}
				}
                if ((moveList.moves[j].Type & knight_promo) != 0) // promo
                {
                    if (promo == "q")
                    {
                        if ((moveList.moves[j].Type == queen_promo) || (moveList.moves[j].Type == queen_promo_capture))
                        {
                            MakeMove(main_board, moveList.moves[j]);
                            break;

                            //Move_to_do.Add(moveList[j]);
                        }
                    }
                    else if (promo == "r")
                    {
                        if ((moveList.moves[j].Type == rook_promo) || (moveList.moves[j].Type == rook_promo_capture))
                        {
                            MakeMove(main_board, moveList.moves[j]);
                            break;
                            //Move_to_do.Add(moveList[j]);
                        }
                    }
                    else if (promo == "b")
                    {
                        if ((moveList.moves[j].Type == bishop_promo) || (moveList.moves[j].Type == bishop_promo_capture))
                        {
                            MakeMove(main_board, moveList.moves[j]);
                            break;
                            //Move_to_do.Add(moveList[j]);
                        }
                    }
                    else if (promo == "n")
                    {
                        if ((moveList.moves[j].Type == knight_promo) || (moveList.moves[j].Type == knight_promo_capture))
                        {
                            MakeMove(main_board, moveList.moves[j]);
                            break;
                            //Move_to_do.Add(moveList[j]);
                        }
                    }
                }
                else
                {
                    //Console.WriteLine(MoveType[moveList[j].Type]);
                    //Console.WriteLine(ascii_pieces[moveList[j].Piece]);
                   // printMove(moveList[j]);
                    //Console.Write(" ");
                    //Move_to_do.Add(moveList[j]); 
                    MakeMove(main_board, moveList.moves[j]);
                    //if (isMoveIrreversible(moveList[j]))
                    //{
                    //    //Console.WriteLine("aaa");
                    //    Repetition_table.Clear();
                    //    main_board.halfmove = 0;
                    //}
                    //Repetition_table.Add(main_Zobrist);
                   


                    break;
                }



            }
        }

		AccumulatorPair correctacc;
		int whiteKingFile = getFile(get_ls1b(main_board.bitboards[K]));
		if (whiteKingFile >= 4)//king is on right now, have to flip
		{
			resetWhiteAccumulator(main_board, correctacc, true);
		}
		if (whiteKingFile <= 3)//king is on left now, have to flip
		{
			resetWhiteAccumulator(main_board, correctacc, false);
		}



		int blackKingFile = getFile(get_ls1b(main_board.bitboards[k]));
		if (blackKingFile >= 4)//king is on right now, have to flip
		{
			resetBlackAccumulator(main_board, correctacc, true);
		}
		if (blackKingFile <= 3)//king is on left now, have to flip
		{
			resetBlackAccumulator(main_board, correctacc, false);
		}
		if (correctacc != main_board.accumulator)
		{
			std::cout << "accumulator doesn't match";
		}
        //main_board.history.push_back(main_board.)

        uint64_t hash_debug = generate_hash_key(main_board);

        PrintBoards(main_board);
        if (hash_debug != main_board.zobristKey)
        {
            std::cout << "warning:zobrist key doesn't match";
        }
    }
    else if(main_command == "bench")
    {
        bench();
    }

}


static void InitAll()
{
    InitializeBetweenTable();
    InitializeLeaper();
    init_sliders_attacks(1);
    init_sliders_attacks(0);
    init_tables();
    init_random_keys();
    
}
int main(int argc, char* argv[])
{
	ThreadData* heapAllocated = new ThreadData(); // Allocate safely on heap
	ThreadData& data = *heapAllocated;

    initializeLMRTable(data);
    InitAll();
    
    parse_fen(start_pos, main_board);
    main_board.zobristKey = generate_hash_key(main_board);
    main_board.history.push_back(main_board.zobristKey);

    Initialize_TT(16);

    if (argc > 1) {
        std::string command = argv[1]; // First argument (after program name)

        if (command == "bench") {

            bench();
            return EXIT_SUCCESS;
        }
    }
	std::ofstream logFile("debug_turbulence.txt", std::ios::app);
    while (true)
    {
        std::string input;

        std::getline(std::cin, input);
        if (input != "")
        {
			logFile << input << '\n';
			logFile.flush();
            ProcessUCI(input, data, heapAllocated);
        }

    }

    

}
