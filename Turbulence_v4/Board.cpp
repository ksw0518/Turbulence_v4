#include "Board.h"

#include "Search.h"
#include "MoveGeneration.h"
#include "BitManipulation.h"
#include "const.h"
#include <iostream>
Board::Board()
    : side(0), enpassent((no_sq)), castle(0), halfmove(0) , Zobrist_key(0), Pawn_key(0){
    // Initialize arrays
    for (int i = 0; i < 12; ++i) {
        bitboards[i] = 0;
    }
    for (int i = 0; i < 3; ++i) {
        occupancies[i] = 0;
    }
    for (int i = 0; i < 64; ++i) {
        mailbox[i] = 0;
    }

    history.clear();
    history.reserve(256);
}

// Move constructor
Move::Move(unsigned char from, unsigned char to, unsigned char type, unsigned char piece)
    : From(from), To(to), Type(type), Piece(piece){

}

Move::Move() : From(0), To(0), Type(0), Piece(0) {
}




int getPieceFromChar(char pieceChar) {
    switch (pieceChar) {
    case 'P': return 0;
    case 'N': return 1;
    case 'B': return 2;
    case 'R': return 3;
    case 'Q': return 4;
    case 'K': return 5;
    case 'p': return 6;
    case 'n': return 7;
    case 'b': return 8;
    case 'r': return 9;
    case 'q': return 10;
    case 'k': return 11;
    default: return -1; // Default or error value
    }
}

char getCharFromPiece(int piece) {
    switch (piece) {
    case 0: return 'P';
    case 1: return 'N';
    case 2: return 'B';
    case 3: return 'R';
    case 4: return 'Q';
    case 5: return 'K';
    case 6: return 'p';
    case 7: return 'n';
    case 8: return 'b';
    case 9: return 'r';
    case 10: return 'q';
    case 11: return 'k';
    default: return -1; // Default or error value
    }
}

std::string CoordinatesToChessNotation(int square)
{
    int rawFile = square % 8;
    int rawRank = square == 0 ? 8 : 8 - square / 8;
    char File = (char)('a' + rawFile); // Convert column index to letter ('a' to 'h')
    int row = rawRank; // Row number (1 to 8)

    // Validate row
    //if (row < 0 || row > 8)
    //{
    //    throw new ArgumentException("Invalid chess square.");
    //}
    std::string str(1, File);
    return str + std::to_string(row);
}
static inline int get_castle(uint64_t castle)
{
    int number = 0;

    // Set bits based on the presence of each castling right
    if ((castle & WhiteKingCastle) != 0) number |= 1 << 0; // Bit 0
    if ((castle & WhiteQueenCastle) != 0) number |= 1 << 1; // Bit 1
    if ((castle & BlackKingCastle) != 0) number |= 1 << 2; // Bit 2
    if ((castle & BlackQueenCastle) != 0) number |= 1 << 3; // Bit 3


    return number;
}


void PrintBoards(Board board)
{
    std::cout << ("\n");
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            if (file == 0)
            {
                std::cout << (" ") << (8 - rank) << (" ");
            }

            int piece = -1;

            for (int bb_piece = P; bb_piece <= k; bb_piece++)
            {
                if (Get_bit(board.bitboards[bb_piece], square))
                {
                    piece = bb_piece;
                }
            }
            //std::cout << (getCharFromPiece(piece));
            std::cout << (" ") << ((piece == -1) ? '.' : getCharFromPiece(piece));
        }
        std::cout << ("\n");
    }

    std::cout << ("\n    a b c d e f g h");
    std::cout << ("\n    Side :     ") << ((board.side == 0 ? "w" : "b"));
    std::cout << ("\n    Enpassent :     ") << (board.enpassent != no_sq ? CoordinatesToChessNotation(board.enpassent) : "no");
    std::cout << ("\n    Castling :     ") << (((board.castle & WhiteKingCastle) != 0) ? 'K' : '-') << (((board.castle & WhiteQueenCastle) != 0) ? 'Q' : '-') << (((board.castle & BlackKingCastle) != 0) ? 'k' : '-') << (((board.castle & BlackQueenCastle) != 0) ? 'q' : '-');
    
    std::cout << "\n\n";

    std::cout << std::hex << "    Zobrist key:     " << generate_hash_key(board) << std::hex << "\n";
    std::cout << std::hex << "    Zobrist key_incremental:     " << board.Zobrist_key << std::dec  << "\n";


    std::cout <<"    Castle_key:     " << get_castle(board.castle) << "\n";

    std::cout << "History" << "\n";

    
    

    for (int i = 0; i < board.history.size(); i++)
    {
        std::cout << std::hex << board.history[i] << std::dec << "\n";
    }
    std::cout << "isThreeFold:" << (is_threefold(board.history, board.last_irreversible_ply) ? "True" : "False");
    //std::cout << ("\n    Number :     ") << ;
    std::cout << ("\n");



}
void print_mailbox(int mailbox[])
{
    std::cout << ("\n MAILBOX \n");
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            if (file == 0)
            {
                std::cout << (" ") << (8 - rank) << " ";
            }
            //int piece = 0;
            if (mailbox[square] != NO_PIECE) //
            {

                std::cout << (" ") << getCharFromPiece(mailbox[square]);
            }
            else
            {
                std::cout << (" .");
            }
        }
        std::cout << ("\n");
    }

}

void parse_fen(std::string fen, Board& board)
{
    //start_position
    //r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
    //tricky_position
    //rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 
    for (int i = 0; i < 64; i++)
    {
        board.mailbox[i] = NO_PIECE;
    }
    //board.mailbox
    for (int i = 0; i < 12; i++)
    {
        board.bitboards[i] = 0;
    }
    for (int i = 0; i < 3; i++)
    {
        board.occupancies[i] = 0;
    }
    board.side = 0;
    board.enpassent = no_sq;
    //int index = 0;
    // Console.WriteLine(fen);
    int square = 0;
    int index = 0;
    for (int i = 0; i < fen.length(); i++)
    {
        char text = fen[i];
        //int file = square % 8;
        //int rank = square == 0 ? 0 : 8 - square / 8;
        if (text == ' ')
        {
            index = i + 1;
            break;
        }
        if (text == '/')
        {
            //i++;
            continue;

        }
        if (text >= '0' && text <= '9')
        {
            //Console.WriteLine(square);
            square += text - '0';
            //Console.WriteLine(square);
        }

        //Console.WriteLine(i);
        if (text >= 'a' && text <= 'z' || text >= 'A' && text <= 'Z')
        {
            int piece = getPieceFromChar(text);
            board.mailbox[square] = piece;
            Set_bit(board.bitboards[piece], square);
            square++;
            //Console.WriteLine(piece);
        }

        //if (square >= 64) Console.WriteLine("bug");

    }
    if (fen[index] == 'w')
        board.side = White;
    else
        board.side = Black;

    index += 2;

    board.castle = 0;
    for (int i = 0; i < 4; i++)
    {

        if (fen[index] == 'K') board.castle |= WhiteKingCastle;
        if (fen[index] == 'Q') board.castle |= WhiteQueenCastle;
        if (fen[index] == 'k') board.castle |= BlackKingCastle;
        if (fen[index] == 'q') board.castle |= BlackQueenCastle;
        if (fen[index] == ' ') break;
        if (fen[index] == '-')
        {
            board.castle = 0;
            break;
        }



        index++;
    }
    //PrintBoards(board);
    index++;
    if (fen[index] == ' ') index++;
    if (fen[index] != '-')
    {
        //Console.WriteLine(fen[index]);
        int file = fen[index] - 'a';
        int rank = 8 - (fen[index + 1] - '0');

        board.enpassent = rank * 8 + file;

    }
    else
    {
        //Console.WriteLine(fen[index]);
        board.enpassent = no_sq;
    }
    for (int piece = P; piece <= K; piece++)
    {
        board.occupancies[White] |= board.bitboards[piece];
    }
    for (int piece = p; piece <= k; piece++)
    {
        board.occupancies[Black] |= board.bitboards[piece];
    }
    board.occupancies[Both] |= board.occupancies[Black];
    board.occupancies[Both] |= board.occupancies[White];


}
void PrintLegalMoves(std::vector<Move> moveList)
{
    int num = 0;
    for (Move move : moveList)
    {
        std::cout << (CoordinatesToChessNotation(move.From)) << (CoordinatesToChessNotation(move.To));
        if (move.Type == queen_promo || move.Type == queen_promo_capture) std::cout << ("q");
        if (move.Type == rook_promo || move.Type == rook_promo_capture) std::cout << ("r");
        if (move.Type == bishop_promo || move.Type == bishop_promo_capture) std::cout << ("b");
        if (move.Type == knight_promo || move.Type == knight_promo_capture) std::cout << ("n");

        std::cout << (": 1 \n");

        num++;
    }
}

