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
    //u64 Zobrist;

    // Constructor to initialize members
    Board();
};

struct Move
{
    int From;
    int To;
    int Type;
    int Piece;

    Move(int from, int to, int type, int piece);
    Move();


    // Equality operator
    bool operator==(const Move& other) const;
    //bool Equals(Move other);
};

inline bool Move::operator==(const Move& other) const {
    return std::tie(From, To, Type, Piece) == std::tie(other.From, other.To, other.Type, other.Piece);
}

int getPieceFromChar(char pieceChar);

char getCharFromPiece(int piece);

std::string CoordinatesToChessNotation(int square);
void PrintBoards(Board board);
void print_mailbox(int mailbox[]);

void parse_fen(std::string fen, Board& board);
void PrintLegalMoves(std::vector<Move> moveList);

