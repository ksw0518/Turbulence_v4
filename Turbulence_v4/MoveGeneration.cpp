#include "MoveGeneration.h"
#include "Board.h"

#include "BitManipulation.h"
#include "const.h"

#include <cstdint>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>


constexpr uint64_t NotAFile = 18374403900871474942ULL;
constexpr uint64_t NotHFile = 9187201950435737471ULL;
constexpr uint64_t NotHGFile = 4557430888798830399ULL;
constexpr uint64_t NotABFile = 18229723555195321596ULL;

static uint64_t pawn_attacks[2][64] = {};
static uint64_t Knight_attacks[64] = {};
static uint64_t King_attacks[64] = {};
static uint64_t bishop_masks[64] = {};
static uint64_t rook_masks[64] = {};
static uint64_t bishop_attacks[64][512] = {};
static uint64_t rook_attacks[64][4096] = {};
static uint64_t betweenTable[64][64] = {};
uint32_t random_state = 1804289383;

int MinorPieces[6] = { B, b, N, n, K, k };

uint64_t all_attackers_to_square(Board &board, uint64_t occupied, int sq) {

    // When performing a static exchange evaluation we need to find all
    // attacks to a given square, but we also are given an updated occupied
    // bitboard, which will likely not match the actual board, as pieces are
    // removed during the iterations in the static exchange evaluation

    return (pawn_attacks[White][sq] & board.bitboards[p]) |
        (pawn_attacks[Black][sq] & board.bitboards[P]) |
        (Knight_attacks[sq] & (board.bitboards[n] | board.bitboards[N])) |
        (get_bishop_attacks(sq, occupied) &
            ((board.bitboards[b] | board.bitboards[B]) |
                (board.bitboards[q] | board.bitboards[Q]))) |
        (get_rook_attacks(sq, occupied) &
            ((board.bitboards[r] | board.bitboards[R]) |
                (board.bitboards[q] | board.bitboards[Q]))) |
        (King_attacks[sq] & (board.bitboards[k] | board.bitboards[K]));

    return 0ULL;
}

uint32_t get_random_U32_number()
{
    // get current state
    uint32_t number = random_state;

    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // update random number state
    random_state = number;

    // return random number
    return number;
}

// generate 64-bit pseudo legal numbers
uint64_t get_random_U64_number()
{
    // define 4 random numbers
    uint64_t n1, n2, n3, n4;

    // init random numbers slicing 16 bits from MS1B side
    n1 = (uint64_t)(get_random_U32_number()) & 0xFFFF;
    n2 = (uint64_t)(get_random_U32_number()) & 0xFFFF;
    n3 = (uint64_t)(get_random_U32_number()) & 0xFFFF;
    n4 = (uint64_t)(get_random_U32_number()) & 0xFFFF;

    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}


uint64_t piece_keys[12][64];
uint64_t enpassant_keys[64];
uint64_t castle_keys[16];

uint64_t side_key;
//init random hash keys
void init_random_keys()
{
    random_state = 1804289383;

    for (int piece = P; piece <= k; piece++)
    {
        for (int square = 0; square < 64; square++)
        {
            piece_keys[piece][square] = get_random_U64_number();
            //std::cout << std::hex << piece_keys[piece][square] << "\n";
        }
    }


    for (int square = 0; square < 64; square++)
    {
        enpassant_keys[square] = get_random_U64_number();
        //std::cout << std::hex << piece_keys[piece][square] << "\n";
    }

    for (int castle = 0; castle < 16; castle++)
    {
        castle_keys[castle] = get_random_U64_number();
        //std::cout << std::hex << piece_keys[piece][square] << "\n";
    }

    side_key = get_random_U64_number();
}
static inline int get_castle(uint64_t castle)
{
    int number = 0;

    // Set bits based on the presence of each castling right
    if ((castle & WhiteKingCastle) != 0) number |= 1 << 0; // Bit 0
    if ((castle & WhiteQueenCastle) != 0) number |= 1 << 1; // Bit 1
    if ((castle & BlackKingCastle) != 0 ) number |= 1 << 2; // Bit 2
    if ((castle & BlackQueenCastle) != 0) number |= 1 << 3; // Bit 3


    return number;
}
uint64_t generate_hash_key(Board& board)
{
    uint64_t final_key = 0ULL;

    uint64_t bitboard;

    for (int piece = P; piece <= k; piece++)
    {
        bitboard = board.bitboards[piece];

        while (bitboard)
        {
            int square = get_ls1b(bitboard);

            final_key ^= piece_keys[piece][square];
            Pop_bit(bitboard, square);
        }
    }


    if (board.enpassent != no_sq)
    {
        final_key ^= enpassant_keys[board.enpassent];
    }
    final_key ^= castle_keys[get_castle(board.castle)];
    //std::cout << get_castle(board.castle);

    if (board.side == Black)
    {
        //std::cout << std::hex << side_key << std::hex << "\n";
        final_key ^= side_key;
    }
    return final_key;
}
uint64_t generate_Pawn_Hash(Board &board)
{
    uint64_t final_key = 0ULL;
    uint64_t bitboard;

    bitboard = board.bitboards[P];

    while (bitboard)
    {
        int square = get_ls1b(bitboard);

        final_key ^= piece_keys[P][square];
        Pop_bit(bitboard, square);
    }

    bitboard = board.bitboards[p];

    while (bitboard)
    {
        int square = get_ls1b(bitboard);

        final_key ^= piece_keys[p][square];
        Pop_bit(bitboard, square);
    }
    return final_key;
}
uint64_t generate_Minor_Hash(Board& board)
{
	uint64_t final_key = 0ULL;
	uint64_t bitboard;

	
	for (int i = 0; i < 6; i++)
	{
		int piece = MinorPieces[i];
		bitboard = board.bitboards[piece];

		while (bitboard)
		{
			int square = get_ls1b(bitboard);

			final_key ^= piece_keys[piece][square];
			Pop_bit(bitboard, square);
		}
	}


	return final_key;
}
static int bishop_relevant_bits[] =
{
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
};

static int rook_relevant_bits[] =
{
12, 11, 11, 11, 11, 11, 11, 12,
11, 10, 10, 10, 10, 10, 10, 11,
11, 10, 10, 10, 10, 10, 10, 11,
11, 10, 10, 10, 10, 10, 10, 11,
11, 10, 10, 10, 10, 10, 10, 11,
11, 10, 10, 10, 10, 10, 10, 11,
11, 10, 10, 10, 10, 10, 10, 11,
12, 11, 11, 11, 11, 11, 11, 12,
};

static uint64_t rook_magic_numbers[64] =
{ 0x8A80104000800020ULL,
0x140002000100040ULL,
0x2801880A0017001ULL,
0x100081001000420ULL,
0x200020010080420ULL,
0x3001C0002010008ULL,
0x8480008002000100ULL,
0x2080088004402900ULL,
0x800098204000ULL,
0x2024401000200040ULL,
0x100802000801000ULL,
0x120800800801000ULL,
0x208808088000400ULL,
0x2802200800400ULL,
0x2200800100020080ULL,
0x801000060821100ULL,
0x80044006422000ULL,
0x100808020004000ULL,
0x12108A0010204200ULL,
0x140848010000802ULL,
0x481828014002800ULL,
0x8094004002004100ULL,
0x4010040010010802ULL,
0x20008806104ULL,
0x100400080208000ULL,
0x2040002120081000ULL,
0x21200680100081ULL,
0x20100080080080ULL,
0x2000A00200410ULL,
0x20080800400ULL,
0x80088400100102ULL,
0x80004600042881ULL,
0x4040008040800020ULL,
0x440003000200801ULL,
0x4200011004500ULL,
0x188020010100100ULL,
0x14800401802800ULL,
0x2080040080800200ULL,
0x124080204001001ULL,
0x200046502000484ULL,
0x480400080088020ULL,
0x1000422010034000ULL,
0x30200100110040ULL,
0x100021010009ULL,
0x2002080100110004ULL,
0x202008004008002ULL,
0x20020004010100ULL,
0x2048440040820001ULL,
0x101002200408200ULL,
0x40802000401080ULL,
0x4008142004410100ULL,
0x2060820C0120200ULL,
0x1001004080100ULL,
0x20C020080040080ULL,
0x2935610830022400ULL,
0x44440041009200ULL,
0x280001040802101ULL,
0x2100190040002085ULL,
0x80C0084100102001ULL,
0x4024081001000421ULL,
0x20030A0244872ULL,
0x12001008414402ULL,
0x2006104900A0804ULL,
0x1004081002402ULL };
static uint64_t bishop_magic_numbers[64] =
{
     0x40040844404084ULL,
0x2004208A004208ULL,
0x10190041080202ULL,
0x108060845042010ULL,
0x581104180800210ULL,
0x2112080446200010ULL,
0x1080820820060210ULL,
0x3C0808410220200ULL,
0x4050404440404ULL,
0x21001420088ULL,
0x24D0080801082102ULL,
0x1020A0A020400ULL,
0x40308200402ULL,
0x4011002100800ULL,
0x401484104104005ULL,
0x801010402020200ULL,
0x400210C3880100ULL,
0x404022024108200ULL,
0x810018200204102ULL,
0x4002801A02003ULL,
0x85040820080400ULL,
0x810102C808880400ULL,
0xE900410884800ULL,
0x8002020480840102ULL,
0x220200865090201ULL,
0x2010100A02021202ULL,
0x152048408022401ULL,
0x20080002081110ULL,
0x4001001021004000ULL,
0x800040400A011002ULL,
0xE4004081011002ULL,
0x1C004001012080ULL,
0x8004200962A00220ULL,
0x8422100208500202ULL,
0x2000402200300C08ULL,
0x8646020080080080ULL,
0x80020A0200100808ULL,
0x2010004880111000ULL,
0x623000A080011400ULL,
0x42008C0340209202ULL,
0x209188240001000ULL,
0x400408A884001800ULL,
0x110400A6080400ULL,
0x1840060A44020800ULL,
0x90080104000041ULL,
0x201011000808101ULL,
0x1A2208080504F080ULL,
0x8012020600211212ULL,
0x500861011240000ULL,
0x180806108200800ULL,
0x4000020E01040044ULL,
0x300000261044000AULL,
0x802241102020002ULL,
0x20906061210001ULL,
0x5A84841004010310ULL,
0x4010801011C04ULL,
0xA010109502200ULL,
0x04A02012000ULL,
0x500201010098B028ULL,
0x8040002811040900ULL,
0x28000010020204ULL,
0x6000020202D0240ULL,
0x8918844842082200ULL,
0x4010011029020020ULL
};


//enum Side
//{
//    White, Black, Both
//
//};
//static int bishop_relevant_bits[] =
//{
//        6, 5, 5, 5, 5, 5, 5, 6,
//        5, 5, 5, 5, 5, 5, 5, 5,
//        5, 5, 7, 7, 7, 7, 5, 5,
//        5, 5, 7, 9, 9, 7, 5, 5,
//        5, 5, 7, 9, 9, 7, 5, 5,
//        5, 5, 7, 7, 7, 7, 5, 5,
//        5, 5, 5, 5, 5, 5, 5, 5,
//        6, 5, 5, 5, 5, 5, 5, 6
//};
//
//static int rook_relevant_bits[] =
//{
//12, 11, 11, 11, 11, 11, 11, 12,
//11, 10, 10, 10, 10, 10, 10, 11,
//11, 10, 10, 10, 10, 10, 10, 11,
//11, 10, 10, 10, 10, 10, 10, 11,
//11, 10, 10, 10, 10, 10, 10, 11,
//11, 10, 10, 10, 10, 10, 10, 11,
//11, 10, 10, 10, 10, 10, 10, 11,
//12, 11, 11, 11, 11, 11, 11, 12,
//};
//
inline uint64_t between(int a, int b)
{
    return betweenTable[a][b];
};
int GetSquare(std::string squareName)
{
    //if (squareName.length() != 2)
    //    throw new std::invalid_argument("Invalid square name format. Must be in the format 'file rank', e.g., 'e2'.");

    char file = squareName[0];
    char rank = squareName[1];

    int fileIndex = -1;
    switch (file)
    {
    case 'a': fileIndex = 0; break;
    case 'b': fileIndex = 1; break;
    case 'c': fileIndex = 2; break;
    case 'd': fileIndex = 3; break;
    case 'e': fileIndex = 4; break;
    case 'f': fileIndex = 5; break;
    case 'g': fileIndex = 6; break;
    case 'h': fileIndex = 7; break;
        //default:
        //    throw std::invalid_argument("Invalid file character. Must be one of 'a' to 'h'.");
    }

    int rankIndex = -1;
    switch (rank)
    {
    case '1': rankIndex = 7; break;
    case '2': rankIndex = 6; break;
    case '3': rankIndex = 5; break;
    case '4': rankIndex = 4; break;
    case '5': rankIndex = 3; break;
    case '6': rankIndex = 2; break;
    case '7': rankIndex = 1; break;
    case '8': rankIndex = 0; break;
        //default:
        //    throw std::invalid_argument("Invalid rank character. Must be one of '1' to '8'.");
    }

    return rankIndex * 8 + fileIndex;
}
void printMove(Move move)
{
    std::cout << (CoordinatesToChessNotation(static_cast<uint8_t>(move.From)) + CoordinatesToChessNotation(static_cast<uint8_t>(move.To)));
    if (move.Type == queen_promo || move.Type == queen_promo_capture) std::cout << ("q");
    if (move.Type == rook_promo || move.Type == rook_promo_capture) std::cout << ("r");
    if (move.Type == bishop_promo || move.Type == bishop_promo_capture) std::cout << ("b");
    if (move.Type == knight_promo || move.Type == knight_promo_capture) std::cout << ("n");

    //std::cout << ("")
}
bool isWholeNumber(float number) {
    return fmod(number, 1.0) == 0.0;
}
static uint64_t Calcbetween(int a, int b)
{
    uint64_t between = 0;


    int xDiff = getFile(a) - getFile(b);
    int yDiff = getRank(a) - getRank(b);


    int totalSteps;
    if (xDiff == 0)
    {
        totalSteps = std::abs(yDiff);
    }
    else
    {
        totalSteps = std::abs(xDiff);
    }

    if (totalSteps == 0) return 0;

    float testx = -xDiff / (float)totalSteps;
    float testy = yDiff / (float)totalSteps;

    //Console.WriteLine(xStep + " ," + yStep);
    if (testx > 1 || testx < -1 || testy > 1 || testy < -1) return 0;
    if (testx == 0 && testy == 0) return 0;
    if (!isWholeNumber(testx) || !isWholeNumber(testy)) return 0;



    int xStep = (int)testx;
    int yStep = (int)testy;
    int pos = a;
    int howmuch = 0;
    //Console.WriteLine(pos);
    //Set_bit(ref between, pos);
    while (pos != b)
    {
        //CoordinatesToChessNotation
        pos += xStep;
        pos += yStep * 8;
        Set_bit(between, pos);

        //if (howmuch > 10) Console.WriteLine(CoordinatesToChessNotation(a) + "," + CoordinatesToChessNotation(b) + " " + xStep + "," + yStep + " " + totalSteps);
        howmuch++;
        //Console.WriteLine(pos);

    }
    between &= ~(1ULL << b);
    return between;
}

uint64_t get_bishop_attacks(int square, uint64_t occupancy)
{
    
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];
    //std::cout << square << "\n";
    return bishop_attacks[square][occupancy];
}
uint64_t get_rook_attacks(int square, uint64_t occupancy)
{
    //std::cout << "asdt";
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    return rook_attacks[square][occupancy];
}
uint64_t get_queen_attacks(int square, uint64_t occupancy)
{

    //Console.WriteLine(square);
    uint64_t queen_attacks;
    uint64_t bishop_occupancies = occupancy;
    uint64_t rook_occupancies = occupancy;

    rook_occupancies &= rook_masks[square];
    rook_occupancies *= rook_magic_numbers[square];
    rook_occupancies >>= 64 - rook_relevant_bits[square];
    queen_attacks = rook_attacks[square][rook_occupancies];

    bishop_occupancies &= bishop_masks[square];
    bishop_occupancies *= bishop_magic_numbers[square];
    bishop_occupancies >>= 64 - bishop_relevant_bits[square];
    queen_attacks |= bishop_attacks[square][bishop_occupancies];

    return queen_attacks;
}
void PrintBitboard(uint64_t bitboard)
{
    std::cout << ("\n");
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            std::cout << ((bitboard & (1ULL << square)) != 0 ? "1 " : "0 ");
        }
        std::cout << ("\n");
    }


    std::cout << ("\n");
}
//static Dictionary<int, string> MoveType = new()
//{
//    {quiet_move, "quiet_move" },
//    {double_pawn_push, "double_pawn_push" },
//    {king_castle, "king_castle" },
//    {queen_castle, "queen_castle" },
//    {capture, "capture" },
//    {ep_capture, "ep_capture" },
//    {knight_promo, "knight_promo" },
//    {bishop_promo, "bishop_promo" },
//    {rook_promo, "rook_promo" },
//    {queen_promo, "queen_promo" },
//    {knight_promo_capture, "knight_promo_capture" },
//    {bishop_promo_capture, "bishop_promo_capture" },
//    {rook_promo_capture, "rook_promo_capture" },
//    {queen_promo_capture, "queen_promo_capture" },
//
//
//};

//enum Square {
//    a8 = 0, b8, c8, d8, e8, f8, g8, h8,
//    a7 = 8, b7, c7, d7, e7, f7, g7, h7,
//    a6 = 16, b6, c6, d6, e6, f6, g6, h6,
//    a5 = 24, b5, c5, d5, e5, f5, g5, h5,
//    a4 = 32, b4, c4, d4, e4, f4, g4, h4,
//    a3 = 40, b3, c3, d3, e3, f3, g3, h3,
//    a2 = 48, b2, c2, d2, e2, f2, g2, h2,
//    a1 = 56, b1, c1, d1, e1, f1, g1, h1,
//    no_sq = 64
//};
//
//
//enum Piece {
//    P = 0, N, B, R, Q, K, p, n, b, r,q, k, NO_PIECE
//};



inline int getSide(int piece)
{
    return (piece > 5) ? Black : White;
    //Piece
};

void InitializeBetweenTable()
{
    for (int a = 0; a < 64; a++)
    {
        for (int b = 0; b < 64; b++)
        {
            betweenTable[a][b] = Calcbetween(a, b);
        }
    }
}
static uint64_t CalculatePawnAttack(int square, int side)
{
    uint64_t attacks = 0UL;
    uint64_t bitboard = 0UL;
    Set_bit(bitboard, square);
    //PrintBitboard(bitboard);
    if (side == White)//white
    {
        if (((bitboard >> 7) & NotAFile) != 0)
            attacks |= (bitboard >> 7);
        if (((bitboard >> 9) & NotHFile) != 0)
            attacks |= (bitboard >> 9);
    }
    else //black
    {
        if (((bitboard << 7) & NotHFile) != 0)
            attacks |= (bitboard << 7);
        if (((bitboard << 9) & NotAFile) != 0)
            attacks |= (bitboard << 9);
    }

    return attacks;
}

static uint64_t CalculateKnightAttack(int square)
{
    uint64_t attacks = 0UL;
    uint64_t bitboard = 0UL;
    Set_bit(bitboard, square);
    //PrintBitboard(bitboard);
    //17 15 10 6

    if (((bitboard >> 17) & NotHFile) != 0)
        attacks |= (bitboard >> 17);
    if (((bitboard >> 15) & NotAFile) != 0)
        attacks |= (bitboard >> 15);
    if (((bitboard >> 10) & NotHGFile) != 0)
        attacks |= (bitboard >> 10);
    if (((bitboard >> 6) & NotABFile) != 0)
        attacks |= (bitboard >> 6);

    if (((bitboard << 17) & NotAFile) != 0)
        attacks |= (bitboard << 17);
    if (((bitboard << 15) & NotHFile) != 0)
        attacks |= (bitboard << 15);
    if (((bitboard << 10) & NotABFile) != 0)
        attacks |= (bitboard << 10);
    if (((bitboard << 6) & NotHGFile) != 0)
        attacks |= (bitboard << 6);
    return attacks;
}
static uint64_t CalculateKingAttack(int square)
{
    uint64_t attacks = 0UL;
    uint64_t bitboard = 0UL;
    Set_bit(bitboard, square);
    //PrintBitboard(bitboard);
    //17 15 10 6

    if (((bitboard >> 8)) != 0)
        attacks |= (bitboard >> 8);
    if (((bitboard >> 9) & NotHFile) != 0)
        attacks |= (bitboard >> 9);
    if (((bitboard >> 7) & NotAFile) != 0)
        attacks |= (bitboard >> 7);
    if (((bitboard >> 1) & NotHFile) != 0)
        attacks |= (bitboard >> 1);

    if (((bitboard << 8)) != 0)
        attacks |= (bitboard << 8);
    if (((bitboard << 9) & NotAFile) != 0)
        attacks |= (bitboard << 9);
    if (((bitboard << 7) & NotHFile) != 0)
        attacks |= (bitboard << 7);
    if (((bitboard << 1) & NotAFile) != 0)
        attacks |= (bitboard << 1);
    return attacks;
}

void InitializeLeaper()
{
    for (int i = 0; i < 64; i++)
    {
        pawn_attacks[0][i] = CalculatePawnAttack(i, 0);
        pawn_attacks[1][i] = CalculatePawnAttack(i, 1);

        Knight_attacks[i] = CalculateKnightAttack(i);
        King_attacks[i] = CalculateKingAttack(i);


    }
}

static uint64_t MaskBishopAttack(int square)
{
    uint64_t attacks = 0UL;
    //ulong bitboard = 0UL;

    int r, f;
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
    {

        attacks |= (1ULL << (r * 8 + f));

    }
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
    {

        attacks |= (1ULL << (r * 8 + f));

    }
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
    {

        attacks |= (1ULL << (r * 8 + f));

    }
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
    {

        attacks |= (1ULL << (r * 8 + f));

    }
    //Set_bit(ref bitboard, square);
    //PrintBitboard(bitboard);
    //17 15 10 6


    return attacks;
}

static uint64_t MaskRookAttack(int square)
{
    uint64_t attacks = 0UL;
    //ulong bitboard = 0UL;

    int r, f;
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 6; r++)
    {
        attacks |= (1ULL << (r * 8 + tf));

    }
    for (r = tr - 1; r >= 1; r--)
    {
        attacks |= (1ULL << (r * 8 + tf));

    }
    for (f = tf + 1; f <= 6; f++)
    {
        attacks |= (1ULL << (tr * 8 + f));

    }
    for (f = tf - 1; f >= 1; f--)
    {
        attacks |= (1ULL << (tr * 8 + f));

    }
    //Set_bit(ref bitboard, square);
    //PrintBitboard(bitboard);
    //17 15 10 6


    return attacks;
}
static uint64_t CalculateRookAttack(int square, uint64_t block)
{
    uint64_t attacks = 0UL;
    //ulong bitboard = 0UL;

    int r, f;
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 7; r++)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf) & block) != 0)
        {
            break;
        }
    }
    for (r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf) & block) != 0)
        {
            break;
        }
    }
    for (f = tf + 1; f <= 7; f++)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f) & block) != 0)
        {
            break;
        }
    }
    for (f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f) & block) != 0)
        {
            break;
        }
    }
    //Set_bit(ref bitboard, square);
    //PrintBitboard(bitboard);
    //17 15 10 6


    return attacks;
}

static uint64_t CalculateBishopAttack(int square, uint64_t block)
{
    uint64_t attacks = 0UL;
    //ulong bitboard = 0UL;

    int r, f;
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f) & block) != 0)
        {
            break;
        }
    }
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f) & block) != 0)
        {
            break;
        }
    }
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f) & block) != 0)
        {
            break;
        }
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f) & block) != 0)
        {
            break;
        }
    }
    //Set_bit(ref bitboard, square);
    //PrintBitboard(bitboard);
    //17 15 10 6


    return attacks;
}
void init_sliders_attacks(int bishop)
{
    for (int square = 0; square < 64; square++)
    {
        bishop_masks[square] = MaskBishopAttack(square);
        rook_masks[square] = MaskRookAttack(square);

        uint64_t attack_mask = bishop != 0 ? bishop_masks[square] : rook_masks[square];
        int relevant_bits_count = count_bits(attack_mask);
        int occupancy_indicies = (1 << relevant_bits_count);

        for (int index = 0; index < occupancy_indicies; index++)
        {
            if (bishop != 0)
            {
                uint64_t occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

                int magic_index = (int)((occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]));

                if (magic_index < 512)
                {
                    bishop_attacks[square][magic_index] = CalculateBishopAttack(square, occupancy);
                }
                //else
                //{
                //    Console.WriteLine($"bishop magic_index out of range: {magic_index} for square: {square}");
                //}
            }
            else
            {
                uint64_t occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

                int magic_index = (int)((occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]));

                if (magic_index < 4096)
                {
                    rook_attacks[square][magic_index] = CalculateRookAttack(square, occupancy);
                }
                //else
                //{
                //    Console.WriteLine($"rook magic_index out of range: {magic_index} for square: {square}");
                //}
            }
        }
    }
}



static inline void Generate_Pawn_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask)
{
    //MoveList.reserve(256);
    int side = board.side;
    //uint64_t pawnBB = (side == White) ? board.bitboards[P] : board.bitboards[p];
    uint64_t pawnBB, pawn_capture_mask, pawn_capture, pawnOnePush, pawnTwoPush; uint64_t forward, doublePushSquare, promotionSquare, enpassent;
    if (side == White)
    {
        pawnBB = board.bitboards[P];
        //forward = 8;
        //opp_occupancy = board.occupancies[Black];
        promotionSquare = 0b0000000000000000000000000000000000000000000000001111111100000000;
        doublePushSquare = 0b0000000011111111000000000000000000000000000000000000000000000000;
    }
    else
    {
        pawnBB = board.bitboards[p];
        //forward = -8;
        //opp_occupancy = board.occupancies[White];
        promotionSquare = 0b0000000011111111000000000000000000000000000000000000000000000000;
        doublePushSquare = 0b0000000000000000000000000000000000000000000000001111111100000000;

    }
    //PrintBitboard(promotionSquare);
    //PrintBitboard(pawnBB);
    if (pawnBB != 0)
    {


        while (true)
        {
            int From = get_ls1b(pawnBB);
            //std::cout << (From) << ("\n");
            uint64_t currPawnBB = 1ULL << From;

            uint64_t twoForward;
            //bool isPromotion;
            if (side == White)
            {
                forward = currPawnBB >> 8;
                twoForward = currPawnBB >> 16;

                //isPromotion = (From >= a7 && From <= h7);
            }
            else
            {
                forward = currPawnBB << 8;
                twoForward = currPawnBB << 16;
                //isPromotion = (From >= a2 && From <= h2);
            }

            //PrintBitboard(currPawnBB);
            //PrintBitboard(promotionSquare);
            if ((currPawnBB & promotionSquare) != 0)
            {
                //std::cout << (From) << ("\n");
                // =======promotion======= //

                uint64_t pawnPromo = forward & ~board.occupancies[Both] & move_mask;
                //bool isPossible = pawnOnePush != 0;

                if (pawnPromo != 0)
                {
                    while (true)
                    {
                        //Console.WriteLine(pawnPromo);
                        int To = get_ls1b(pawnPromo);
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), knight_promo, static_cast<uint8_t>(get_piece(p, side))));
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), bishop_promo, static_cast<uint8_t>(get_piece(p, side))));
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), rook_promo, static_cast<uint8_t>(get_piece(p, side))));
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), queen_promo, static_cast<uint8_t>(get_piece(p, side))));
                        Pop_bit(pawnPromo, To);

                        //Console.WriteLine(pawnPromo);
                        if (pawnPromo == 0ULL) break;
                    }
                }





                // =======promo_capture======= //
                pawn_capture_mask = pawn_attacks[board.side][From];
                pawn_capture = (pawn_capture_mask & board.occupancies[1 - side]) & (move_mask | capture_mask);

                if (pawn_capture != 0)
                {
                    while (true)
                    {
                        int To = get_ls1b(pawn_capture);



                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), knight_promo_capture, static_cast<uint8_t>(get_piece(p, side))));
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), bishop_promo_capture, static_cast<uint8_t>(get_piece(p, side))));
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), rook_promo_capture, static_cast<uint8_t>(get_piece(p, side))));
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), queen_promo_capture, static_cast<uint8_t>(get_piece(p, side))));
                        Pop_bit(pawn_capture, To);
                        if (pawn_capture == 0ULL) break;
                    }
                }



            }
            else
            {
                // =======pawn one square push======= //
                pawnOnePush = (forward & ~board.occupancies[Both]) & move_mask;


                bool isPossible = pawnOnePush != 0;

                if (pawnOnePush != 0)
                {
                    while (true)
                    {
                        int To = get_ls1b(pawnOnePush);
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), quiet_move, static_cast<uint8_t>(get_piece(p, side))));
                        Pop_bit(pawnOnePush, To);
                        if (pawnOnePush == 0ULL) break;
                    }
                }


                // =======pawn two square push======= //

                pawnTwoPush = 0;
                if (isPossible)
                {
                    if ((doublePushSquare & currPawnBB) != 0)//pawn on second rank
                    {

                        pawnTwoPush = (twoForward & ~board.occupancies[Both]) & move_mask;

                    }
                }
                if (pawnTwoPush != 0)
                {
                    while (true)
                    {
                        int To = get_ls1b(pawnTwoPush);
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), double_pawn_push, static_cast<uint8_t>(get_piece(p, side))));
                        //MoveList.Add(new Move(From, To, double_pawn_push, get_piece(Piece.p, side)));
                        //PrintBitboard(pawnTwoPush);
                        Pop_bit(pawnTwoPush, To);
                        if (pawnTwoPush == 0ULL) break;
                    }
                }
                // =======pawn capture======= //
                pawn_capture_mask = pawn_attacks[board.side][From];
                pawn_capture = pawn_capture_mask & board.occupancies[1 - side];

                if (pawn_capture != 0)
                {
                    while (true)
                    {
                        int To = get_ls1b(pawn_capture);
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), capture, static_cast<uint8_t>(get_piece(p, side))));

                        Pop_bit(pawn_capture, To);

                        if (pawn_capture == 0ULL) break;
                    }
                }



                // =======pawn enpassent capture======= //

                
                if (board.enpassent != no_sq) // enpassent possible
                {

                    enpassent = (pawn_capture_mask & (1ULL << board.enpassent)) & (capture_mask);

                    if (enpassent != 0)
                    {
                        while (true)
                        {
                            int To = get_ls1b(enpassent);
                            MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), ep_capture, static_cast<uint8_t>(get_piece(p, side))));
                            Pop_bit(enpassent, To);
                            if (enpassent == 0ULL) break;
                        }
                    }



                }

            }
            Pop_bit(pawnBB, From);
            if (pawnBB == 0ULL) break;
        }
    }

}
static inline void Generate_Knight_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask)
{
    int side = board.side;
    uint64_t KnightBB;
    if (side == White)
    {
        KnightBB = board.bitboards[N];
    }
    else
    {
        KnightBB = board.bitboards[n];
    }
    if (KnightBB != 0)
    {
        while (true)
        {
            int From = get_ls1b(KnightBB);
            uint64_t KnightMove = (Knight_attacks[From] & ~board.occupancies[side]);
            if (KnightMove != 0)
            {
                while (true)
                {
                    int To = get_ls1b(KnightMove);
                    if ((board.occupancies[1 - side] & (1ULL << To)) != 0)
                    {
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), capture, static_cast<uint8_t>(get_piece(n, side))));
                    }
                    else
                    {
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), quiet_move, static_cast<uint8_t>(get_piece(n, side))));
                    }
                    Pop_bit(KnightMove, To);
                    if (KnightMove == 0) break;
                }
            }

            Pop_bit(KnightBB, From);
            if (KnightBB == 0) break;
        }
    }

}
static inline void Generate_Bishop_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask)
{
    int side = board.side;
    uint64_t BishopBB;
    if (side == White)
    {
        BishopBB = board.bitboards[B];
    }
    else
    {
        BishopBB = board.bitboards[b];
    }
    if (BishopBB != 0)
    {
        while (true)
        {
            int From = get_ls1b(BishopBB);
            uint64_t BishopMove = (get_bishop_attacks(From, board.occupancies[Both]) & ~board.occupancies[side]) & (move_mask | capture_mask);
            if (BishopMove != 0)
            {
                while (true)
                {
                    int To = get_ls1b(BishopMove);
                    if ((board.occupancies[1 - side] & (1ULL << To)) != 0)
                    {
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), capture, static_cast<uint8_t>(get_piece(b, side))));
                    }
                    else
                    {
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), quiet_move, static_cast<uint8_t>(get_piece(b, side))));
                    }
                    Pop_bit(BishopMove, To);
                    if (BishopMove == 0) break;
                }
            }
            Pop_bit(BishopBB, From);
            if (BishopBB == 0) break;
        }
    }

}
static inline void Generate_Rook_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask)
{
    int side = board.side;
    uint64_t RookBB;
    if (side == White)
    {
        RookBB = board.bitboards[R];
    }
    else
    {
        RookBB = board.bitboards[r];
    }
    //PrintBitboard(RookBB);
    if (RookBB != 0)
    {
        while (true)
        {
            int From = get_ls1b(RookBB);
            uint64_t RookMove = (get_rook_attacks(static_cast<uint8_t>(From), board.occupancies[Both]) & ~board.occupancies[side]) & (move_mask | capture_mask);

            //PrintBitboard(RookMove);
            if (RookMove != 0)
            {
                while (true)
                {
                    //PrintBitboard(RookMove);
                    int To = get_ls1b(RookMove);
                    if ((board.occupancies[1 - side] & (1ULL << To)) != 0)
                    {

                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), capture, static_cast<uint8_t>(get_piece(r, side))));
                        //std::cout << ("push");
                    }
                    else
                    {
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), quiet_move, static_cast<uint8_t>(get_piece(r, side))));
                        //std::cout << ("push");
                    }
                    Pop_bit(RookMove, To);

                    if (RookMove == 0) break;
                }
            }
            Pop_bit(RookBB, From);
            if (RookBB == 0) break;
        }
    }

}
static inline void Generate_Queen_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask)
{
    int side = board.side;
    uint64_t QueenBB;
    if (side == White)
    {
        QueenBB = board.bitboards[Q];
    }
    else
    {
        QueenBB = board.bitboards[q];
    }
    //PrintBitboard(RookBB);
    if (QueenBB != 0)
    {
        while (true)
        {
            int From = get_ls1b(QueenBB);
            uint64_t QueenMove = (get_queen_attacks(From, board.occupancies[Both]) & ~board.occupancies[side]) & (move_mask | capture_mask);
            //PrintBitboard(QueenMove);
            if (QueenMove != 0)
            {

                while (true)
                {
                    int To = get_ls1b(QueenMove);
                    if ((board.occupancies[1 - side] & (1ULL << To)) != 0)
                    {
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), capture, static_cast<uint8_t>(get_piece(q, side))));
                    }
                    else
                    {
                        MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), quiet_move, static_cast<uint8_t>(get_piece(q, side))));
                    }
                    Pop_bit(QueenMove, To);
                    if (QueenMove == 0) break;
                }
            }
            Pop_bit(QueenBB, From);
            if (QueenBB == 0) break;
        }
    }
}
static inline void Generate_King_Moves(std::vector<Move>& MoveList, Board& board, uint64_t move_mask, uint64_t capture_mask)
{
    //PrintBitboard(WhiteKingCastleEmpty);
    //PrintBitboard(WhiteQueenCastleEmpty);
    int side = board.side;
    uint64_t KingBB;
    if (side == White)
    {
        KingBB = board.bitboards[K];
    }
    else
    {
        KingBB = board.bitboards[k];
    }
    //PrintBitboard(RookBB);
    while (true)
    {
        int From = get_ls1b(KingBB);
        uint64_t KingMove = King_attacks[From] & ~board.occupancies[side];
        if (KingMove != 0)
        {
            while (true)
            {
                int To = get_ls1b(KingMove);
                if ((board.occupancies[1 - side] & (1ULL << To)) != 0)
                {
                    MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), capture, static_cast<uint8_t>(get_piece(k, side))));
                }
                else
                {
                    MoveList.push_back(Move(static_cast<uint8_t>(From), static_cast<uint8_t>(To), quiet_move, static_cast<uint8_t>(get_piece(k, side))));
                }
                Pop_bit(KingMove, To);
                if (KingMove == 0) break;
            }

        }

        Pop_bit(KingBB, From);
        if (KingBB == 0) break;
    }
    //MoveList.push_back(Move(e8, g8, king_castle, get_piece(k, side)));
    if (side == White)
    {

        if ((board.castle & WhiteKingCastle) != 0) // kingside castling
        {

            if ((board.occupancies[Both] & WhiteKingCastleEmpty) == 0)
            {
                MoveList.push_back(Move(static_cast<uint8_t>(e1), static_cast<uint8_t>(g1), king_castle, static_cast<uint8_t>(get_piece(k, side))));
            }

        }
        if ((board.castle & WhiteQueenCastle) != 0)
        {
            if ((board.occupancies[Both] & WhiteQueenCastleEmpty) == 0)
            {
                MoveList.push_back(Move(static_cast<uint8_t>(e1), static_cast<uint8_t>(c1), queen_castle, static_cast<uint8_t>(get_piece(k, side))));
            }
        }
    }
    else
    {
        if ((board.castle & BlackKingCastle) != 0) // kingside castling
        {
            //PrintBitboard(BlackKingCastleEmpty);
            //PrintBitboard((board.occupancies[Both] & BlackKingCastleEmpty));
            //Console.WriteLine("pass1");
            if ((board.occupancies[Both] & BlackKingCastleEmpty) == 0)
            {
                //std::cout<<("asdf");
                MoveList.push_back(Move(static_cast<uint8_t>(e8), static_cast<uint8_t>(g8), king_castle, static_cast<uint8_t>(get_piece(k, side))));

                //printMove(MoveList.back());
            }


        }

        if ((board.castle & BlackQueenCastle) != 0)
        {

            if ((board.occupancies[Both] & BlackQueenCastleEmpty) == 0)
            {
                MoveList.push_back(Move(static_cast<uint8_t>(e8), static_cast<uint8_t>(c8), queen_castle, static_cast<uint8_t>(get_piece(k, side))));
            }


        }
    }

}

bool is_square_attacked(int square, int side, Board &board, uint64_t occupancy)
{
    //side is the side who is checking the king
    //occupancy is both occupancy
    if ((get_bishop_attacks(square, occupancy) & ((side == White) ? board.bitboards[B] : board.bitboards[b])) != 0) return true;

    if ((get_rook_attacks(square, occupancy) & ((side == White) ? board.bitboards[R] : board.bitboards[r])) != 0) return true;
    if ((get_queen_attacks(square, occupancy) & ((side == White) ? board.bitboards[Q] : board.bitboards[q])) != 0) return true;
    if ((Knight_attacks[square] & ((side == White) ? board.bitboards[N] : board.bitboards[n])) != 0) return true;
    if ((side == White) && ((pawn_attacks[Black][square] & board.bitboards[P])) != 0) return true;
    if ((side == Black) && ((pawn_attacks[White][square] & board.bitboards[p])) != 0) return true;

    if ((King_attacks[square] & ((side == White) ? board.bitboards[K] : board.bitboards[k])) != 0) return true;



    return false;
}
void Generate_Legal_Moves(std::vector<Move>& MoveList, Board& board, bool isCapture)
{
    //int WK_Square = get_ls1b(board.bitboards[Piece.K]);
    //int BK_Square = get_ls1b(board.bitboards[Piece.k]);

    //int my_king = (board.side == Side.White) ? WK_Square : BK_Square;

    ////Console.WriteLine(my_king);
    ////ulong Attacked_square = get_attacked_squares(oppSide, board, (board.occupancies[Side.Both] & ~KingBB));
    ////Console.WriteLine((board.side == Side.White));
    //List<ulong> pin_ray = new();
    //List<ulong> pinned_piece = new();


    //ulong check_attackers = 0;
    ////ulong attacked_square = is_square_attacked



    //Detect_pinned_pieces(my_king, ref pinned_piece, ref pin_ray, board);


    //Console.WriteLine(pinned_piece.Count);
    //PrintBitboard(pin_ray[0]);


    //Detect_Check_Attackers(my_king, ref check_attackers, board);


    //Console.WriteLine(count_bits(check_attackers));

    //printBitboard(check_attackers); 
    uint64_t move_mask = 0xFFFFFFFFFFFFFFFF;



    //PrintBitboard(move_mask);
    uint64_t capture_mask = 0xFFFFFFFFFFFFFFFF;
    //print_bitboard(move_mask);
    //if (count_bits(check_attackers) == 1) // single check
    //{
    //    move_mask = between(my_king, get_ls1b(check_attackers));

    //    capture_mask = check_attackers;

    //    if (board.enpassent != (int)Square.no_sq)
    //    {
    //        int pawnToCapture;
    //        if (board.side == Side.White)
    //        {
    //            pawnToCapture = board.enpassent + 8;
    //        }
    //        else
    //        {
    //            pawnToCapture = board.enpassent - 8;
    //        }

    //        if ((check_attackers & (1UL << pawnToCapture)) != 0)
    //        {
    //            capture_mask |= (1UL << board.enpassent);
    //        }




    //    }

    //}

    //PrintBitboard(move_mask | capture_mask);

    //Console.WriteLine(count_bits(check_attackers));
    //print_bitboard(pinned_ray);
    //for (int i = 0; i < pinned_piece.Count; i++)
    //{
    //    PrintBitboard(pinned_piece[i]); 
    //    PrintBitboard(pin_ray[i]); 
    //}

    //ulong prevCastle = board.castle;
    //PrintBitboard(board.castle);



    MoveList.clear();
    MoveList.reserve(256);
    Generate_Pawn_Moves(MoveList, board, move_mask, capture_mask);

    ////if (prevCastle != board.castle) Console.WriteLine(0);
    Generate_Knight_Moves(MoveList, board, move_mask, capture_mask);
    Generate_Bishop_Moves(MoveList, board, move_mask, capture_mask);
    Generate_Rook_Moves(MoveList, board, move_mask, capture_mask);
    Generate_Queen_Moves(MoveList, board, move_mask, capture_mask);
    Generate_King_Moves(MoveList, board, move_mask, capture_mask);




    ////PrintBitboard(board.castle);
    ////if (prevCastle != board.castle) Console.WriteLine(1);
    //Generate_Bishop_Moves(ref MoveList, board, check_attackers, move_mask, capture_mask, pin_ray, pinned_piece, isCapture);
    ////PrintBitboard(board.castle);
    ////if (prevCastle != board.castle) Console.WriteLine(2);
    //Generate_Rook_Moves(ref MoveList, board, check_attackers, move_mask, capture_mask, pin_ray, pinned_piece, isCapture);
    ////PrintBitboard(board.castle);
    ////if (prevCastle != board.castle) Console.WriteLine(3);
    //Generate_Queen_Moves(ref MoveList, board, check_attackers, move_mask, capture_mask, pin_ray, pinned_piece, isCapture);
    ////PrintBitboard(board.castle);
    ////if (prevCastle != board.castle) Console.WriteLine(4);
    //Generate_King_Moves(ref MoveList, board, check_attackers, isCapture);

}
//inline int get_castle(uint64_t castle, int side)
//{
//    if (side == White)
//    {
//        if ((castle & (WhiteKingCastle | WhiteQueenCastle)) != 0)
//        {
//            return 0;
//        }
//        else if ((castle & WhiteKingCastle) != 0)
//        {
//            return 1;
//        }
//        else if ((castle & WhiteQueenCastle) != 0)
//        {
//            return 2;
//        }
//        else
//        {
//            return 3;
//        }
//    }
//    else
//    {
//        if ((castle & (BlackKingCastle | BlackQueenCastle)) != 0)
//        {
//            return 0;
//        }
//        else if ((castle & BlackKingCastle) != 0)
//        {
//            return 1;
//        }
//        else if ((castle & BlackQueenCastle) != 0)
//        {
//            return 2;
//        }
//        else
//        {
//            return 3;
//        }
//    }
//
//}
bool is_move_irreversible(Move& move)
{
    if (((move.Type & captureFlag) != 0) || move.Piece == p || move.Piece == P)
    {
        return true;
    }
    return false;
}
void Make_Nullmove(Board& board)
{
    if (board.enpassent != no_sq)
    {
        board.Zobrist_key ^= enpassant_keys[board.enpassent];
        board.enpassent = no_sq;
    }
    //Zobrist ^= PIECES[move.Piece][move.To];
    board.side = 1 - board.side;
    board.Zobrist_key ^= side_key;
}
void Unmake_Nullmove(Board& board)
{
    //if (board.enpassent != no_sq)
    //{
    //    board.Zobrist_key ^= enpassant_keys[board.enpassent];
    //    board.enpassent = no_sq;
    //}
    //Zobrist ^= PIECES[move.Piece][move.To];
    board.side = 1 - board.side;
    board.Zobrist_key ^= side_key;
}
void MakeMove(Board& board, Move move)
{
    //Console.WriteLine(board.side);

    uint64_t lastCastle = board.castle;
    int lastEp = board.enpassent;

    
    
    if (board.enpassent != no_sq)
    {
        board.Zobrist_key ^= enpassant_keys[board.enpassent];
        board.enpassent = no_sq;
    }
   
    int side = board.side;
    // change castling flag
    if (get_piece(move.Piece, White) == K) //if king moved
    {
        if (side == White)
        {
            board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
            board.castle &= ~WhiteKingCastle;
            board.castle &= ~WhiteQueenCastle;
            board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
            //PIECES

            //Zobrist ^= W_CASTLING_RIGHTS[get_castle(WhiteKingCastle | WhiteQueenCastle, side)];

        }
        else
        {
            board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
            board.castle &= ~BlackKingCastle;
            board.castle &= ~BlackQueenCastle;
            board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
            //Zobrist ^= B_CASTLING_RIGHTS[get_castle(BlackKingCastle | BlackQueenCastle, side)];

        }
    }
    else if (get_piece(move.Piece, White) == R) //if rook moved
    {
        if (side == White)
        {
            if ((board.castle & WhiteQueenCastle) != 0 && move.From == a1) // no q castle
            {
                board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                board.castle &= ~WhiteQueenCastle;
                board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                //Zobrist ^= B_CASTLING_RIGHTS[get_castle(BlackKingCastle | BlackQueenCastle, side)];
            }
            else if ((board.castle & WhiteKingCastle) != 0 && move.From == h1) // no k castle
            {
                board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                board.castle &= ~WhiteKingCastle;
                board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
            }



        }
        else
        {
            if ((board.castle & BlackQueenCastle) != 0 && move.From == a8) // no q castle
            {
                board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                board.castle &= ~BlackQueenCastle;
                board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
            }
            else if ((board.castle & BlackKingCastle) != 0 && move.From == h8) // no k castle
            {
                board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                board.castle &= ~BlackKingCastle;
                board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
            }
        }
    }

    switch (move.Type)
    {
    case double_pawn_push:
    {

        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[move.Piece] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);
        //update mailbox
        board.mailbox[move.From] = NO_PIECE;

        //remove previous place
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];

        board.mailbox[move.To] = move.Piece;

        board.Zobrist_key ^= piece_keys[move.Piece][move.To];

        //update enpassent square

        if (side == White)
        {
            board.enpassent = move.To + 8;
            board.Zobrist_key ^= enpassant_keys[board.enpassent];
        }
        else
        {
            board.enpassent = move.To - 8;
            board.Zobrist_key ^= enpassant_keys[board.enpassent];
        }



        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[move.Piece][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;

        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case quiet_move:
    {
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[move.Piece] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);
        //update mailbox

        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];

        board.mailbox[move.To] = move.Piece;
        board.Zobrist_key ^= piece_keys[move.Piece][move.To];
        //update enpassent square
        //if (move.Type == double_pawn_push)
        //{
        //    if (side == Side.White)
        //    {
        //        board.enpassent = move.To + 8;
        //    }
        //    else
        //    {
        //        board.enpassent = move.To - 8;
        //    }

        //}

        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[move.Piece][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;

        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case capture:
    {
        if (board.mailbox[move.To] == get_piece(r, 1 - side))
        {
            if (getFile(move.To) == 0) // a file rook captured; delete queen castle
            {
                if (side == White) // have to delete black queen castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                    //Console.WriteLine("here");
                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
            else if (getFile(move.To) == 7) // h file rook captured; delete king castle
            {
                //Console.WriteLine("H capture");
                if (side == White) // have to delete black king castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
        }
        //update piece bitboard
        int captured_piece = board.mailbox[move.To];
        //PrintBoards(board);
        //print_mailbox(board.mailbox);
        //printMove(move);
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[move.Piece] |= (1ULL << move.To);


        //Console.WriteLine(captured_piece);
        board.bitboards[captured_piece] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);
        //update mailbox

        board.Zobrist_key ^= piece_keys[move.Piece][move.From];
        board.mailbox[move.From] = NO_PIECE;
        
        board.Zobrist_key ^= piece_keys[board.mailbox[move.To]][move.To];
        board.Zobrist_key ^= piece_keys[move.Piece][move.To];

        board.mailbox[move.To] = move.Piece;


        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[move.Piece][move.To];
        //Zobrist ^= PIECES[captured_piece][move.To];

        //Console.WriteLine(side);


        //Console.WriteLine(get_piece(Piece.r, side));
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;
        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case king_castle: // 
    {

        //DO FROM HERE

        //    DO FROM HERE
        //    DO FROM HERE
        //    DO FROM HERE
        //    DO FROM HERE
        //    DO FROM HERE
        //    DO FROM HERE
        //    DO FROM HERE
        //    DO FROM HERE
        // 
        // 
        // 
        // 
        // 
        //update castling right & find rook square


        int rookSquare;
        if (side == White)
        {
            rookSquare = h1;
            //board.castle &= ~WhiteKingCastle;
        }
        else
        {
            rookSquare = h8;
            //board.castle &= ~BlackKingCastle;

        }


        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[move.Piece] |= (1ULL << move.To);

        board.bitboards[get_piece(r, side)] &= ~(1ULL << rookSquare);
        board.bitboards[get_piece(r, side)] |= (1ULL << (rookSquare - 2));


        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        board.occupancies[side] &= ~(1ULL << rookSquare);
        board.occupancies[side] |= (1ULL << (rookSquare - 2));

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);

        board.occupancies[Both] &= ~(1ULL << rookSquare);
        board.occupancies[Both] |= (1ULL << (rookSquare - 2));
        //update mailbox
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];
        board.mailbox[move.To] = move.Piece;
        board.Zobrist_key ^= piece_keys[move.Piece][move.To];

        board.Zobrist_key ^= piece_keys[board.mailbox[rookSquare]][rookSquare];
        board.mailbox[rookSquare] = NO_PIECE;
        
        board.mailbox[rookSquare - 2] = get_piece(r, side);
        board.Zobrist_key ^= piece_keys[get_piece(r, side)][rookSquare - 2];

        /*Zobrist ^= PIECES[move.Piece][move.From];
        Zobrist ^= PIECES[move.Piece][move.To];
        Zobrist ^= PIECES[get_piece(Piece.r, side)][rookSquare];
        Zobrist ^= PIECES[get_piece(Piece.r, side)][rookSquare - 2];*/
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;
        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case queen_castle:
    {
        //update castling right & find rook square


        int rookSquare;
        if (side == White)
        {
            rookSquare = a1;
            //board.castle &= ~WhiteKingCastle;
        }
        else
        {
            rookSquare = a8;
            //board.castle &= ~BlackKingCastle;

        }
        //Console.WriteLine(CoordinatesToChessNotation(rookSquare));

        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[move.Piece] |= (1ULL << move.To);

        board.bitboards[get_piece(r, side)] &= ~(1ULL << rookSquare);
        board.bitboards[get_piece(r, side)] |= (1ULL << (rookSquare + 3));


        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        board.occupancies[side] &= ~(1ULL << rookSquare);
        board.occupancies[side] |= (1ULL << (rookSquare + 3));

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);

        board.occupancies[Both] &= ~(1ULL << rookSquare);
        board.occupancies[Both] |= (1ULL << (rookSquare + 3));
        //update mailbox
        
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];

        board.mailbox[move.To] = move.Piece;
        board.Zobrist_key ^= piece_keys[move.Piece][move.To];

        board.Zobrist_key ^= piece_keys[board.mailbox[rookSquare]][rookSquare];
        board.mailbox[rookSquare] = NO_PIECE;
        
        board.mailbox[rookSquare + 3] = get_piece(r, side);
        board.Zobrist_key ^= piece_keys[get_piece(r, side)][rookSquare + 3];
        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[move.Piece][move.To];
        //Zobrist ^= PIECES[get_piece(Piece.r, side)][rookSquare];
        //Zobrist ^= PIECES[get_piece(Piece.r, side)][rookSquare + 3];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;

        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case queen_promo:
    {
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[get_piece(q, side)] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);
        //update mailbox
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];
        board.mailbox[move.To] = get_piece(q, side);
        board.Zobrist_key ^= piece_keys[get_piece(q, side)][move.To];
        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[get_piece(q, side)][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;
        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case rook_promo:
    {
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[get_piece(r, side)] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);
        //update mailbox
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];
        board.mailbox[move.To] = get_piece(r, side);
        board.Zobrist_key ^= piece_keys[get_piece(r, side)][move.To];
        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[get_piece(Piece.r, side)][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;
        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case bishop_promo:
    {
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[get_piece(b, side)] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);
        //update mailbox
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];
        board.mailbox[move.To] = get_piece(b, side);
        board.Zobrist_key ^= piece_keys[get_piece(b, side)][move.To];
        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[get_piece(b, side)][move.To];
        board.side = 1 - board.side;

        board.Zobrist_key ^= side_key;
        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        
        break;
    }
    case knight_promo:
    {
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[get_piece(n, side)] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);
        //update mailbox
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];
        board.mailbox[move.To] = get_piece(n, side);
        board.Zobrist_key ^= piece_keys[get_piece(n, side)][move.To];
        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[get_piece(Piece.n, side)][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;

        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case queen_promo_capture:
    {
        if (board.mailbox[move.To] == get_piece(r, 1 - side))
        {
            if (getFile(move.To) == 0) // a file rook captured; delete queen castle
            {
                if (side == White) // have to delete black queen castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                    //Console.WriteLine("here");
                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
            else if (getFile(move.To) == 7) // h file rook captured; delete king castle
            {
                //Console.WriteLine("H capture");
                if (side == White) // have to delete black king castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
        }
        int captured_piece = board.mailbox[move.To];
        //update piece bitboard
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[get_piece(q, side)] |= (1ULL << move.To);

        board.bitboards[captured_piece] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);

        //update mailbox
        board.mailbox[move.From] = NO_PIECE; 
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];

        board.Zobrist_key ^= piece_keys[board.mailbox[move.To]][move.To];
        board.mailbox[move.To] = get_piece(q, side);
        board.Zobrist_key ^= piece_keys[get_piece(q, side)][move.To];

        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[get_piece(Piece.q, side)][move.To];
        //Zobrist ^= PIECES[captured_piece][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;
        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case rook_promo_capture:
    {
        if (board.mailbox[move.To] == get_piece(r, 1 - side))
        {
            if (getFile(move.To) == 0) // a file rook captured; delete queen castle
            {
                if (side == White) // have to delete black queen castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                    //Console.WriteLine("here");
                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
            else if (getFile(move.To) == 7) // h file rook captured; delete king castle
            {
                //Console.WriteLine("H capture");
                if (side == White) // have to delete black king castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
        }
        int captured_piece = board.mailbox[move.To];
        //update piece bitboard
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[get_piece(r, side)] |= (1ULL << move.To);

        board.bitboards[captured_piece] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);

        //update mailbox
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];

        board.Zobrist_key ^= piece_keys[board.mailbox[move.To]][move.To];
        board.mailbox[move.To] = get_piece(r, side);
        board.Zobrist_key ^= piece_keys[get_piece(r, side)][move.To];

        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[get_piece(Piece.q, side)][move.To];
        //Zobrist ^= PIECES[captured_piece][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;

        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case bishop_promo_capture:
    {
        if (board.mailbox[move.To] == get_piece(r, 1 - side))
        {
            if (getFile(move.To) == 0) // a file rook captured; delete queen castle
            {
                if (side == White) // have to delete black queen castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                    //Console.WriteLine("here");
                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
            else if (getFile(move.To) == 7) // h file rook captured; delete king castle
            {
                //Console.WriteLine("H capture");
                if (side == White) // have to delete black king castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
        }
        int captured_piece = board.mailbox[move.To];
        //update piece bitboard
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[get_piece(b, side)] |= (1ULL << move.To);

        board.bitboards[captured_piece] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);

        //update mailbox
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];


        board.Zobrist_key ^= piece_keys[board.mailbox[move.To]][move.To];
        board.mailbox[move.To] = get_piece(b, side);
        board.Zobrist_key ^= piece_keys[get_piece(b, side)][move.To];

        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[get_piece(Piece.q, side)][move.To];
        //Zobrist ^= PIECES[captured_piece][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;

        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case knight_promo_capture:
    {
        if (board.mailbox[move.To] == get_piece(r, 1 - side))
        {
            if (getFile(move.To) == 0) // a file rook captured; delete queen castle
            {
                if (side == White) // have to delete black queen castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                    //Console.WriteLine("here");
                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteQueenCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
            else if (getFile(move.To) == 7) // h file rook captured; delete king castle
            {
                //Console.WriteLine("H capture");
                if (side == White) // have to delete black king castle
                {
                    if (getRank(move.To) == 7)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(BlackKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
                else
                {
                    if (getRank(move.To) == 0)
                    {
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                        board.castle &= ~(WhiteKingCastle);
                        board.Zobrist_key ^= castle_keys[get_castle(board.castle)];
                    }

                }
            }
        }
        int captured_piece = board.mailbox[move.To];
        //update piece bitboard
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[get_piece(n, side)] |= (1ULL << move.To);

        board.bitboards[captured_piece] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] |= (1ULL << move.To);

        //update mailbox
        board.mailbox[move.From] = NO_PIECE;
        board.Zobrist_key ^= piece_keys[move.Piece][move.From];

        board.Zobrist_key ^= piece_keys[board.mailbox[move.To]][move.To];
        board.mailbox[move.To] = get_piece(n, side);
        board.Zobrist_key ^= piece_keys[get_piece(n, side)][move.To];

        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[get_piece(Piece.q, side)][move.To];
        //Zobrist ^= PIECES[captured_piece][move.To];
        board.side = 1 - board.side;
        board.Zobrist_key ^= side_key;

        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }
    case ep_capture:
    {
        int capture_square;
        if (side == White)
        {
            capture_square = move.To + 8;
        }
        else
        {
            capture_square = move.To - 8;
        }


        int captured_piece = board.mailbox[capture_square];
        //update piece bitboard
        board.bitboards[move.Piece] &= ~(1ULL << move.From);
        board.bitboards[move.Piece] |= (1ULL << move.To);

        board.bitboards[captured_piece] &= ~(1ULL << capture_square);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.From);
        board.occupancies[side] |= (1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] &= ~(1ULL << capture_square);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.From);
        board.occupancies[Both] &= ~(1ULL << capture_square);
        board.occupancies[Both] |= (1ULL << move.To);

        //update mailbox

        board.Zobrist_key ^= piece_keys[board.mailbox[move.From]][move.From];
        board.mailbox[move.From] = NO_PIECE;
        
        board.mailbox[move.To] = move.Piece;
        board.Zobrist_key ^= piece_keys[move.Piece][move.To];

        board.Zobrist_key ^= piece_keys[board.mailbox[capture_square]][capture_square];
        board.mailbox[capture_square] = NO_PIECE;
        
        board.side = 1 - board.side;

        board.Zobrist_key ^= side_key;
        //Zobrist ^= PIECES[move.Piece][move.From];
        //Zobrist ^= PIECES[move.Piece][move.To];
        //Zobrist ^= PIECES[captured_piece][capture_square];
        if (is_move_irreversible(move))
        {
            board.last_irreversible_ply = board.history.size();
        }
        board.history.push_back(board.Zobrist_key);
        break;
    }

    }


    //if (board.enpassent != lastEp) //enpassent updated
    //{
    //    if (lastEp != Square.no_sq)
    //    {

    //        Zobrist ^= EN_passent[lastEp];
    //    }
    //    if (board.enpassent != Square.no_sq)
    //    {
    //        Zobrist ^= EN_passent[board.enpassent];
    //    }



    //}
    //if (board.castle != lastCastle)
    //{
    //    int lastWhite = get_castle(lastCastle, White);
    //    int lastBlack = get_castle(lastCastle, Black);

    //    //Zobrist ^= W_CASTLING_RIGHTS[lastWhite];
    //    //Zobrist ^= B_CASTLING_RIGHTS[lastBlack];

    //    //Zobrist ^= W_CASTLING_RIGHTS[get_castle(board.castle, White)];
    //    //Zobrist ^= B_CASTLING_RIGHTS[get_castle(board.castle, Black)];

    //}


    //Zobrist ^= SIDE;

    //zobrist key

    //build hashkey
    //ulong hash_from_scratch = generate_hash_key();

    //if(hash_key != hash_from_scratch)
    //{
    //    Console.WriteLine("hash doens't match");
    //}
}
void UnmakeMove(Board& board, Move move, int captured_piece)
{

    int side = 1 - board.side;
    // change castling flag

    if (move.Type == quiet_move || move.Type == double_pawn_push)
    {
        //Console.WriteLine("q");
        //Console.WriteLine(CoordinatesToChessNotation(move.From) + "," + CoordinatesToChessNotation(move.To));
        //update piece bitboard
        board.bitboards[move.Piece] &= ~(1ULL << move.To);
        board.bitboards[move.Piece] |= (1ULL << move.From);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.To);
        board.occupancies[side] |= (1ULL << move.From);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.To);
        board.occupancies[Both] |= (1ULL << move.From);
        //update mailbox
        board.mailbox[move.To] = NO_PIECE;
        board.mailbox[move.From] = move.Piece;

    }
    else if (move.Type == capture)
    {

        //update piece bitboard
        //int captured_piece = board.mailbox[move.To];
        board.bitboards[move.Piece] &= ~(1ULL << move.To);
        board.bitboards[move.Piece] |= (1ULL << move.From);

        board.bitboards[captured_piece] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.To);
        board.occupancies[side] |= (1ULL << move.From);

        //update captured piece occupancy
        board.occupancies[1 - side] |= (1ULL << move.To);

        //update both occupancy
        //board.occupancies[Side.Both] &= ~(1UL << move.To);
        board.occupancies[Both] |= (1ULL << move.From);
        //update mailbox
        board.mailbox[move.To] = captured_piece;
        board.mailbox[move.From] = move.Piece;

    }
    else if (move.Type == king_castle)
    {
        //update castling right & find rook square


        int rookSquare;
        if (side == White)
        {
            rookSquare = h1;
            //board.castle &= ~WhiteKingCastle;
        }
        else
        {
            rookSquare = h8;
            //board.castle &= ~BlackKingCastle;

        }


        board.bitboards[move.Piece] &= ~(1ULL << move.To);
        board.bitboards[move.Piece] |= (1ULL << move.From);

        board.bitboards[get_piece(r, side)] &= ~(1ULL << (rookSquare - 2));
        board.bitboards[get_piece(r, side)] |= (1ULL << (rookSquare));


        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.To);
        board.occupancies[side] |= (1ULL << move.From);

        board.occupancies[side] &= ~(1ULL << (rookSquare - 2));
        board.occupancies[side] |= (1ULL << (rookSquare));

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.To);
        board.occupancies[Both] |= (1ULL << move.From);

        board.occupancies[Both] &= ~(1ULL << (rookSquare - 2));
        board.occupancies[Both] |= (1ULL << (rookSquare));
        //update mailbox
        board.mailbox[move.To] = NO_PIECE;
        board.mailbox[move.From] = move.Piece;

        board.mailbox[rookSquare - 2] = NO_PIECE;
        board.mailbox[rookSquare] = get_piece(r, side);


    }
    else if (move.Type == queen_castle)
    {
        //update castling right & find rook square


        int rookSquare;
        if (side == White)
        {
            rookSquare = a1;
            //board.castle &= ~WhiteKingCastle;
        }
        else
        {
            rookSquare = a8;
            //board.castle &= ~BlackKingCastle;

        }
        //Console.WriteLine(CoordinatesToChessNotation(rookSquare));

        board.bitboards[move.Piece] &= ~(1ULL << move.To);
        board.bitboards[move.Piece] |= (1ULL << move.From);

        board.bitboards[get_piece(r, side)] &= ~(1ULL << (rookSquare + 3));
        board.bitboards[get_piece(r, side)] |= (1ULL << (rookSquare));


        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.To);
        board.occupancies[side] |= (1ULL << move.From);

        board.occupancies[side] &= ~(1ULL << (rookSquare + 3));
        board.occupancies[side] |= (1ULL << (rookSquare));

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.To);
        board.occupancies[Both] |= (1ULL << move.From);

        board.occupancies[Both] &= ~(1ULL << (rookSquare + 3));
        board.occupancies[Both] |= (1ULL << (rookSquare));
        //update mailbox
        board.mailbox[move.To] = NO_PIECE;
        board.mailbox[move.From] = move.Piece;

        board.mailbox[rookSquare + 3] = NO_PIECE;
        board.mailbox[rookSquare] = get_piece(r, side);
    }
    else if (move.Type == queen_promo)
    {
        //update piece bitboard
        board.bitboards[move.Piece] |= (1ULL << move.From);
        board.bitboards[get_piece(q, side)] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] |= (1ULL << move.From);
        board.occupancies[side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] |= (1ULL << move.From);
        board.occupancies[Both] &= ~(1ULL << move.To);
        //update mailbox
        board.mailbox[move.To] = NO_PIECE;
        board.mailbox[move.From] = move.Piece;

    }
    else if (move.Type == rook_promo)
    {
        board.bitboards[move.Piece] |= (1ULL << move.From);
        board.bitboards[get_piece(r, side)] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] |= (1ULL << move.From);
        board.occupancies[side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] |= (1ULL << move.From);
        board.occupancies[Both] &= ~(1ULL << move.To);
        //update mailbox
        board.mailbox[move.To] = NO_PIECE;
        board.mailbox[move.From] = move.Piece;
    }
    else if (move.Type == bishop_promo)
    {
        board.bitboards[move.Piece] |= (1ULL << move.From);
        board.bitboards[get_piece(b, side)] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] |= (1ULL << move.From);
        board.occupancies[side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] |= (1ULL << move.From);
        board.occupancies[Both] &= ~(1ULL << move.To);
        //update mailbox
        board.mailbox[move.To] = NO_PIECE;
        board.mailbox[move.From] = move.Piece;
    }
    else if (move.Type == knight_promo)
    {
        board.bitboards[move.Piece] |= (1ULL << move.From);
        board.bitboards[get_piece(n, side)] &= ~(1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] |= (1ULL << move.From);
        board.occupancies[side] &= ~(1ULL << move.To);

        //update both occupancy
        board.occupancies[Both] |= (1ULL << move.From);
        board.occupancies[Both] &= ~(1ULL << move.To);
        //update mailbox
        board.mailbox[move.To] = NO_PIECE;
        board.mailbox[move.From] = move.Piece;

    }
    else if (move.Type == queen_promo_capture)
    {
        //int captured_piece = board.mailbox[move.To];
        //update piece bitboard


        board.bitboards[move.Piece] |= (1ULL << move.From);
        board.bitboards[get_piece(q, side)] &= ~(1ULL << move.To);

        board.bitboards[captured_piece] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] |= (1ULL << move.From);
        board.occupancies[side] &= ~(1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] |= (1ULL << move.To);

        //update both occupancy
        //board.occupancies[Side.Both] &= ~(1UL << move.From);
        board.occupancies[Both] |= (1ULL << move.From);

        //update mailbox
        board.mailbox[move.To] = captured_piece;
        board.mailbox[move.From] = move.Piece;










    }
    else if (move.Type == rook_promo_capture)
    {
        board.bitboards[move.Piece] |= (1ULL << move.From);
        board.bitboards[get_piece(r, side)] &= ~(1ULL << move.To);

        board.bitboards[captured_piece] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] |= (1ULL << move.From);
        board.occupancies[side] &= ~(1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] |= (1ULL << move.To);

        //update both occupancy
        //board.occupancies[Side.Both] &= ~(1UL << move.From);
        board.occupancies[Both] |= (1ULL << move.From);

        //update mailbox
        board.mailbox[move.To] = captured_piece;
        board.mailbox[move.From] = move.Piece;
    }
    else if (move.Type == bishop_promo_capture)
    {
        board.bitboards[move.Piece] |= (1ULL << move.From);
        board.bitboards[get_piece(b, side)] &= ~(1ULL << move.To);

        board.bitboards[captured_piece] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] |= (1ULL << move.From);
        board.occupancies[side] &= ~(1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] |= (1ULL << move.To);

        //update both occupancy
        //board.occupancies[Side.Both] &= ~(1UL << move.From);
        board.occupancies[Both] |= (1ULL << move.From);

        //update mailbox
        board.mailbox[move.To] = captured_piece;
        board.mailbox[move.From] = move.Piece;
    }
    else if (move.Type == knight_promo_capture)
    {
        board.bitboards[move.Piece] |= (1ULL << move.From);
        board.bitboards[get_piece(n, side)] &= ~(1ULL << move.To);

        board.bitboards[captured_piece] |= (1ULL << move.To);

        //update moved piece occupancy
        board.occupancies[side] |= (1ULL << move.From);
        board.occupancies[side] &= ~(1ULL << move.To);

        //update captured piece occupancy
        board.occupancies[1 - side] |= (1ULL << move.To);

        //update both occupancy
        //board.occupancies[Side.Both] &= ~(1UL << move.From);
        board.occupancies[Both] |= (1ULL << move.From);

        //update mailbox
        board.mailbox[move.To] = captured_piece;
        board.mailbox[move.From] = move.Piece;
    }
    else if (move.Type == ep_capture)
    {
        int capture_square;
        int captured_pawn = get_piece(p, 1 - side);
        if (side == White)
        {
            capture_square = move.To + 8;
        }
        else
        {
            capture_square = move.To - 8;
        }


        //int captured_piece = board.mailbox[capture_square];
        //update piece bitboard
        board.bitboards[move.Piece] &= ~(1ULL << move.To);
        board.bitboards[move.Piece] |= (1ULL << move.From);

        board.bitboards[captured_pawn] |= (1ULL << capture_square);

        //update moved piece occupancy
        board.occupancies[side] &= ~(1ULL << move.To);
        board.occupancies[side] |= (1ULL << move.From);

        //update captured piece occupancy
        board.occupancies[1 - side] |= (1ULL << capture_square);

        //update both occupancy
        board.occupancies[Both] &= ~(1ULL << move.To);
        board.occupancies[Both] |= (1ULL << capture_square);
        board.occupancies[Both] |= (1ULL << move.From);

        //update mailbox
        board.mailbox[move.From] = move.Piece;
        board.mailbox[move.To] = NO_PIECE;
        board.mailbox[capture_square] = captured_pawn;
    }



    board.side = 1 - board.side;
}
uint64_t get_attacked_squares(int side, Board& board, uint64_t occupancy)
{
    uint64_t attack_map = 0;
    uint64_t bb;

    uint64_t kingBB;
    uint64_t knightBB;
    uint64_t bishopBB;
    uint64_t rookBB;
    uint64_t queenBB;
    uint64_t pawnBB;
    // Precompute piece bitboards for the given side
    if (side == White)
    {
        kingBB = board.bitboards[K];
        knightBB = board.bitboards[N];
        bishopBB = board.bitboards[B];
        rookBB = board.bitboards[R];
        queenBB = board.bitboards[Q];
        pawnBB = board.bitboards[P];

    }
    else
    {
        kingBB = board.bitboards[k];
        knightBB = board.bitboards[n];
        bishopBB = board.bitboards[b];
        rookBB = board.bitboards[r];
        queenBB = board.bitboards[q];
        pawnBB = board.bitboards[p];

    }

    // Process King
    attack_map |= King_attacks[get_ls1b(kingBB)];

    // Process Knights
    for (bb = knightBB; bb != 0;)
    {
        int loc = get_ls1b(bb);
        attack_map |= Knight_attacks[loc];
        Pop_bit(bb, loc);
    }

    // Process Bishops
    for (bb = bishopBB; bb != 0;)
    {
        int loc = get_ls1b(bb);
        attack_map |= get_bishop_attacks(loc, occupancy);
        Pop_bit(bb, loc);
    }

    // Process Rooks
    for (bb = rookBB; bb != 0;)
    {
        int loc = get_ls1b(bb);
        attack_map |= get_rook_attacks(loc, occupancy);
        Pop_bit(bb, loc);
    }

    // Process Queens
    for (bb = queenBB; bb != 0;)
    {
        int loc = get_ls1b(bb);
        attack_map |= get_queen_attacks(loc, occupancy);
        Pop_bit(bb, loc);
    }

    // Process Pawns
    for (bb = pawnBB; bb != 0;)
    {
        int loc = get_ls1b(bb);
        attack_map |= pawn_attacks[side][loc];
        Pop_bit(bb, loc);
    }

    return attack_map;
}
bool isMoveValid(Move& move, Board& board)
{

    //std::cout << (get_piece(K, White));

    //PrintBitboard(attacked_squares);

    uint64_t kingSquare = (board.bitboards[get_piece(K, 1 - board.side)]);

    //PrintBitboard(attacked_squares);
    //(board.bitboards[get_piece(K, board.side)]);
    //std::cout << get_ls1b(board.bitboards[get_piece(K, Black)]) << ("\n");
    //std::cout << << ("\n");
    //White

    //printMove(move);
    //std::cout << "\n";
    
    if (move.Type != king_castle && move.Type != queen_castle)
    {
        if (!is_square_attacked(get_ls1b(kingSquare), board.side, board, board.occupancies[Both]))
        {
            //std::cout << (1);
            return true;
        }
        return false;
    }
    else
    {
        //PrintBitboard(kingSquare);

        //return false;
        uint64_t attacked_squares = get_attacked_squares(board.side, board, board.occupancies[Both]);
        //PrintBitboard(attacked_squares);
        if (move.Type == king_castle)
        {
            uint64_t NoAttackArea;
            if (1 - board.side == White)
            {
                NoAttackArea = WhiteKingCastleEmpty | (1ULL << e1);
            }
            else
            {
                NoAttackArea = BlackKingCastleEmpty | (1ULL << e8);
            }
            if ((attacked_squares & kingSquare) == 0 && (attacked_squares & NoAttackArea) == 0)
            {
                return true;
            }
            return false;
        }
        else// queen castle
        {
            uint64_t NoAttackArea;
            if (1 - board.side == White)
            {
                NoAttackArea = WhiteQueenCastleAttack | (1ULL << e1);
            }
            else
            {
                NoAttackArea = BlackQueenCastleAttack | (1ULL << e8);
            }
            if ((attacked_squares & kingSquare) == 0 && (attacked_squares & NoAttackArea) == 0)
            {
                return true;
            }
            return false;
        }


        return false;
    }
}
