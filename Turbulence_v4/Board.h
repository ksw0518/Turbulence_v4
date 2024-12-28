#pragma once

#include <string> 
#include <vector>
class Board
{
public:
    uint64_t bitboards[12];
    uint64_t occupancies[3];
    int mailbox[64];
    int side;
    int enpassent;
    uint64_t castle;
    int halfmove;
    uint64_t Zobrist_key;

    std::vector<uint64_t> history;

    int last_irreversible_ply = 0;
    //u64 Zobrist;

    // Constructor to initialize members
    Board();
};

struct Move
{
    uint8_t From;
    uint8_t To;
    uint8_t Type;
    uint8_t Piece;
    

    Move(uint8_t from, uint8_t to, uint8_t type, uint8_t piece);
    Move();


    // Equality operator
    bool operator==(const Move& other) const {
        return From == other.From &&
            To == other.To &&
            Type == other.Type &&
            Piece == other.Piece;
    }//bool operator==(const Move& other) const;
    //bool Equals(Move other);
};

//inline bool Move::operator==(const Move& other) const {
//    return std::tie(From, To, Type, Piece) == std::tie(other.From, other.To, other.Type, other.Piece);
//}

int getPieceFromChar(char pieceChar);

char getCharFromPiece(int piece);

std::string CoordinatesToChessNotation(int square);
void PrintBoards(Board board);
void print_mailbox(int mailbox[]);

void parse_fen(std::string fen, Board& board);
void PrintLegalMoves(std::vector<Move> moveList);

