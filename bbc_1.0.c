#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

// bitboard data type
#define U64 unsigned long long

// FEN dedug positions
#define empty_board     "8/8/8/8/8/8/8/8 b - - "
#define start_position  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position    "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define repetitions     "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 "

// board squares
enum
{
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1,
  no_sq
};

// encode pieces
enum
{
  P, N, B, R, Q, K, p, n, b, r, q, k
};

// sides to move (colors)
enum
{
  white,
  black,
  both
};

// bishop and rook
enum
{
  rook,
  bishop
};

//   Binary Encoded Castling Rights 
// 
//   bin     dec   description
//   0001    1     white king can castle to the king side
//   0010    2     white king can castle to the queen side
//   0100    4     black king can castle to the king side
//   1000    8     black king can castle to the queen side
//
//   examples:
//   1111          both sides can castle both directions
//   1001          black king => queen side; white king => king side

enum
{
  wk = 1,
  wq = 2,
  bk = 4,
  bq = 8
};

// convert squares to coordinates
const char* square_to_coordinates[] = {
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

char  ascii_pieces[12]   = "PNBRQKpnbrqk";                                                                             // ASCII pieces
char* unicode_pieces[12] = { "♙", "♘", "♗", "♖", "♕", "♔",                                                             // unicode pieces
                             "♟︎", "♞", "♝", "♜", "♛", "♚" }; 

int char_pieces[] = {                                                                                                  // convert ASCII character pieces to encoded constants
  ['P'] = P, ['N'] = N, ['B'] = B, ['R'] = R, ['Q'] = Q, ['K'] = K,
  ['p'] = p, ['n'] = n, ['b'] = b, ['r'] = r, ['q'] = q, ['k'] = k
};

char promoted_pieces[] = { [Q] = 'q', [R] = 'r', [B] = 'b', [N] = 'n',                                                 // promoted pieces
                           [q] = 'q', [r] = 'r', [b] = 'b', [n] = 'n' };

//                            Chess Board
//
//                            WHITE PIECES
//
//        Pawns                  Knights              Bishops
//
//  8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 0 0 0
//  7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0
//  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
//  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
//  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
//  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
//  2  1 1 1 1 1 1 1 1    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0
//  1  0 0 0 0 0 0 0 0    1  0 1 0 0 0 0 1 0    1  0 0 1 0 0 1 0 0
//
//     a b c d e f g h       a b c d e f g h       a b c d e f g h
//
//         Rooks                 Queens                 King
//
//  8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 0 0 0
//  7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0
//  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
//  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
//  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
//  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
//  2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0
//  1  1 0 0 0 0 0 0 1    1  0 0 0 1 0 0 0 0    1  0 0 0 0 1 0 0 0
//
//     a b c d e f g h       a b c d e f g h       a b c d e f g h
//
//                            BLACK PIECES
//
//        Pawns                  Knights              Bishops
//
//  8  0 0 0 0 0 0 0 0    8  0 1 0 0 0 0 1 0    8  0 0 1 0 0 1 0 0
//  7  1 1 1 1 1 1 1 1    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0
//  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
//  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
//  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
//  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
//  2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0
//  1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 0 0 0
//
//     a b c d e f g h       a b c d e f g h       a b c d e f g h
//
//         Rooks                 Queens                 King
//
//  8  1 0 0 0 0 0 0 1    8  0 0 0 1 0 0 0 0    8  0 0 0 0 1 0 0 0
//  7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0
//  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
//  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
//  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
//  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
//  2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0
//  1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 0 0 0
//
//     a b c d e f g h       a b c d e f g h       a b c d e f g h
//
//                             OCCUPANCIES
//
//     White occupancy       Black occupancy       All occupancies
//
//  8  0 0 0 0 0 0 0 0    8  1 1 1 1 1 1 1 1    8  1 1 1 1 1 1 1 1
//  7  0 0 0 0 0 0 0 0    7  1 1 1 1 1 1 1 1    7  1 1 1 1 1 1 1 1
//  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
//  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
//  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
//  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
//  2  1 1 1 1 1 1 1 1    2  0 0 0 0 0 0 0 0    2  1 1 1 1 1 1 1 1
//  1  1 1 1 1 1 1 1 1    1  0 0 0 0 0 0 0 0    1  1 1 1 1 1 1 1 1
//
//                            ALL TOGETHER
//
//                        8  ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
//                        7  ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ 
//                        6  . . . . . . . . 
//                        5  . . . . . . . . 
//                        4  . . . . . . . . 
//                        3  . . . . . .  . . 
//                        2  ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙ 
//                        1  ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
//
//                           a b c d e f g h

U64 bitboards[12];                                                                                                     // piece bitboards
U64 occupancies[3];                                                                                                    // occupancy bitboards
int side;                                                                                                              // side to move
int enpassant               = no_sq;                                                                                   // enpassant square
int castle;                                                                                                            // castling rights
U64 hash_key;                                                                                                          // "almost" unique position identifier aka hash key or position key
U64 repetition_table[1000];                                                                                            // 1000 is a number of plies (500 moves) in the entire game positions repetition table
int repetition_index;                                                                                                  // repetition index
int ply;                                                                                                               // half move counter

// Time controls variables
int quit      =  0;                                                                                                    // exit from engine flag
int movestogo = 30;                                                                                                    // UCI "movestogo" command moves counter
int movetime  = -1;                                                                                                    // UCI "movetime" command time counter
int time      = -1;                                                                                                    // UCI "time" command holder (ms)
int inc       =  0;                                                                                                    // UCI "inc" command's time increment holder
int starttime =  0;                                                                                                    // UCI "starttime" command time holder
int stoptime  =  0;                                                                                                    // UCI "stoptime" command time holder
int timeset   =  0;                                                                                                    // variable to flag time control availability
int stopped   =  0;                                                                                                    // variable to flag when the time is up

//  Miscellaneous functions
//     forked from VICE
//    by Richard Allbert

// get time in milliseconds
int
get_time_ms()
{
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

//  Function to "listen" to GUI's input during search.
//  It's waiting for the user input from STDIN.
//  OS dependent.
//
//  First Richard Allbert aka BluefeverSoftware grabbed it from somewhere...
//  And then Code Monkey King has grabbed it from VICE)

int
input_waiting()
{
  fd_set readfds;
  struct timeval tv;

  FD_ZERO (&readfds);
  FD_SET (fileno(stdin), &readfds);
  tv.tv_sec  = 0;
  tv.tv_usec = 0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
}

// read GUI/user input
void
read_input()
{
  int  bytes;                                                                                                          // bytes to read holder
  char input[256] = "", *endc;                                                                                         // GUI/user input

  if (input_waiting()) {                                                                                               // "listen" to STDIN
    stopped = 1;                                                                                                       // tell engine to stop calculating

    do {                                                                                                               // loop to read bytes from STDIN
      bytes = read(fileno(stdin), input, 256);                                                                         // read bytes from STDIN
    }
    while (bytes < 0);                                                                                                 // until bytes available

    endc = strchr(input, '\n');                                                                                        // searches for the first occurrence of '\n'
    if (endc) *endc = 0;                                                                                               // if found new line set value at pointer to 0
    if (strlen(input) > 0) {                                                                                           // if input is available
      if      (!strncmp(input, "quit", 4)) quit = 1;                                                                   // tell engine to terminate execution
      else if (!strncmp(input, "stop", 4)) quit = 1;                                                                   // tell engine to terminate execution
    }
  }
}

// a bridge function to interact between search and GUI input
static void
communicate()
{
  if (timeset == 1 && get_time_ms() > stoptime) {                                                                      // if time is up break here
    stopped = 1;                                                                                                       // tell engine to stop calculating
  }
  read_input();                                                                                                        // read GUI input
}

// Random numbers
unsigned int random_state = 1804289383;                                                                                // pseudo random number state

// generate 32-bit pseudo legal numbers
unsigned int
get_random_U32_number()
{
  unsigned int number  = random_state;                                                                                 // get current state
  number              ^= number << 13;                                                                                 // XOR shift algorithm
  number              ^= number >> 17;
  number              ^= number << 5;
  random_state         = number;                                                                                       // update random number state
  return number;
}

// generate 64-bit pseudo legal numbers
U64
get_random_U64_number()
{
  U64 n1, n2, n3, n4;                                                                                                  // define 4 random numbers
  n1 = (U64)(get_random_U32_number()) & 0xFFFF;                                                                        // init random numbers slicing 16 bits from MS1B side
  n2 = (U64)(get_random_U32_number()) & 0xFFFF;
  n3 = (U64)(get_random_U32_number()) & 0xFFFF;
  n4 = (U64)(get_random_U32_number()) & 0xFFFF;
  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
U64
generate_magic_number()
{
  return get_random_U64_number() & 
	     get_random_U64_number() &
         get_random_U64_number();
}

// Bit manipulations

// set/get/pop bit macros
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |=  (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) &   (1ULL << (square)))

// count bits within a bitboard (Brian Kernighan's way)
static inline int
count_bits(U64 bitboard)
{
  int count = 0;                                                                                                       // bit counter
  while (bitboard) {                                                                                                   // consecutively reset least significant 1st bit
    count++;
    bitboard &= bitboard - 1;                                                                                          // reset least significant 1st bit
  }
  return count;
}

// get least significant 1st bit index
static inline int
get_ls1b_index(U64 bitboard)
{
  if   (bitboard) return count_bits((bitboard & -bitboard) - 1);
  else            return -1;
}

// Zobrist keys
U64 piece_keys[12][64];                                                                                                // random piece keys [piece][square]
U64 enpassant_keys[64];                                                                                                // random enpassant keys [square]
U64 castle_keys[16];                                                                                                   // random castling keys
U64 side_key;                                                                                                          // random side key

// init random hash keys
void
init_random_keys()
{
  random_state = 1804289383;                                                                                           // update pseudo random number state
  for   (int piece  = P; piece  <= k; piece++) {                                                                       // loop over piece codes
    for (int square = 0; square < 64; square++)                                                                        // loop over board squares
      piece_keys[piece][square] = get_random_U64_number();                                                             // init random piece keys
  }
  for (int square = 0; square < 64; square++)                                                                          // loop over board squares
    enpassant_keys[square] = get_random_U64_number();                                                                  // init random enpassant keys
  for (int index = 0;  index  < 16; index++)                                                                           // loop over castling keys
    castle_keys[index] = get_random_U64_number();                                                                      // init castling keys
  side_key = get_random_U64_number();                                                                                  // ???: init random side key
}

// generate "almost" unique position ID aka hash key from scratch
U64
generate_hash_key()
{
  U64 final_key = 0ULL;                                                                                                // final hash key
  U64 bitboard;                                                                                                        // temp piece bitboard copy

  for (int piece = P; piece <= k; piece++) {                                                                           // loop over piece bitboards
    bitboard = bitboards[piece];                                                                                       // init piece bitboard copy
    while (bitboard) {                                                                                                 // loop over the pieces within a bitboard
      int square = get_ls1b_index(bitboard);                                                                           // init square occupied by the piece
      final_key ^= piece_keys[piece][square];                                                                          // hash piece
      pop_bit(bitboard, square);                                                                                       // pop LS1B
    }
  }
  if (enpassant != no_sq) final_key ^= enpassant_keys[enpassant];                                                      // hash enpassant
                          final_key ^= castle_keys[castle];                                                            // hash the side only if black is to move
  if (side == black)      final_key ^= side_key;                                                                       // hash castling rights

  return final_key;
}

// Input & Output

void
print_bitboard(U64 bitboard)
{
  printf("\n");

  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;                                                                                    // convert file & rank into square index
      if (!file) printf("  %d ", 8 - rank);                                                                            // print ranks
      printf(" %d", get_bit(bitboard, square) ? 1 : 0);                                                                // print bit state (either 1 or 0)
    }
    printf("\n");
  }
  printf("\n     a b c d e f g h\n\n");
  printf("     Bitboard: %llud\n\n", bitboard);                                                                        // print bitboard as unsigned decimal number
}

// print board
void
print_board()
{
  printf("\n");

  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      if (!file) printf("  %d ", 8 - rank);                                                                            // print ranks
      int piece = -1;                                                                                                  // define piece variable

      for (int bb_piece = P; bb_piece <= k; bb_piece++) {                                                              // loop over all piece bitboards
        if (get_bit(bitboards[bb_piece], square))                                                                      // if there is a piece on current square
          piece = bb_piece;                                                                                            // get piece code
      }
      printf(" %s", (piece == -1) ? "." : unicode_pieces[piece]);
    }
    printf("\n");
  }

  printf("\n     a b c d e f g h\n\n");                                                                                // print board files
  printf("     Side:     %s\n", !side ? "white" : "black");                                                            // print side to move
  // print enpassant square
  printf("     Enpassant:   %s\n", (enpassant != no_sq)
         ? square_to_coordinates[enpassant] : "no");

  printf("     Castling:  %c%c%c%c\n\n",                                                                               // print castling rights
         (castle & wk) ? 'K' : '-',
         (castle & wq) ? 'Q' : '-',
         (castle & bk) ? 'k' : '-',
         (castle & bq) ? 'q' : '-');

  printf("     Hash key:  %llx\n\n", hash_key);
}

// parse FEN string
void
parse_fen(char* fen)
{
  memset(bitboards,   0ULL, sizeof(bitboards));                                                                        // reset board position (bitboards)
  memset(occupancies, 0ULL, sizeof(occupancies));                                                                      // reset occupancies (bitboards)

  // reset game state variables
  enpassant        = no_sq;
  side             = 0;
  castle           = 0;
  repetition_index = 0;                                                                                                // reset repetition index

  memset(repetition_table, 0ULL, sizeof(repetition_table));                                                            // reset repetition table

  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      if ((*fen >= 'a' && *fen <= 'z') ||                                                                              // match ascii pieces within FEN string
		  (*fen >= 'A' && *fen <= 'Z'))   {
        int piece = char_pieces[*fen];                                                                                 // init piece type
        set_bit(bitboards[piece], square);                                                                             // set piece on corresponding bitboard
        fen++;                                                                                                         // increment pointer to FEN string
      }
      if (*fen >= '0' && *fen <= '9') {                                                                                // match empty square numbers within FEN string
        int offset = *fen - '0';                                                                                       // init offset (convert char 0 to int 0)
        int piece  = -1;                                                                                               // define piece variable

        for (int bb_piece = P; bb_piece <= k; bb_piece++) {                                                            // loop over all piece bitboards
          if (get_bit(bitboards[bb_piece], square))                                                                    // if there is a piece on current square
            piece = bb_piece;                                                                                          // get piece code
        }
        if (piece == -1) file--;                                                                                       // on empty current square
        file += offset;                                                                                                // adjust file counter
        fen++;                                                                                                         // increment pointer to FEN string
      }
      if (*fen == '/') fen++;                                                                                          // match rank separator
    }
  }

  fen++;                                                                                                               // got to parsing side to move (increment pointer to FEN string)
  (*fen == 'w') ? (side = white) : (side = black);                                                                     // parse side to move
  fen += 2;                                                                                                            // go to parsing castling rights

  while (*fen != ' ') {                                                                                                // parse castling rights
    if      (*fen == 'K') castle |= wk;
    else if (*fen == 'Q') castle |= wq;
    else if (*fen == 'k') castle |= bk;
    else if (*fen == 'q') castle |= bq;
    fen++;                                                                                                             // increment pointer to FEN string
  }
  fen++;                                                                                                               // got to parsing enpassant square (increment pointer to FEN string)
  if (*fen != '-') {                                                                                                   // parse enpassant square
    int file  = fen[0] - 'a';                                                                                          // parse enpassant file & rank
    int rank  = 8 - (fen[1] - '0');
    enpassant = rank * 8 + file;                                                                                       // init enpassant square
  }
  else enpassant = no_sq;                                                                                              // no enpassant square
  for (int piece = P; piece <= K; piece++)                                                                             // loop over white pieces bitboards
    occupancies[white] |= bitboards[piece];                                                                            // populate white occupancy bitboard
  for (int piece = p; piece <= k; piece++)                                                                             // loop over black pieces bitboards
    occupancies[black] |= bitboards[piece];                                                                            // populate white occupancy bitboard
  occupancies[both] |= occupancies[white];                                                                             // init all occupancies
  occupancies[both] |= occupancies[black];

  hash_key = generate_hash_key();
}

// Attacks

//     not A file          not H file         not HG files      not AB files
//      bitboard            bitboard            bitboard          bitboard
//
// 8  0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
// 7  0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
// 6  0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
// 5  0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
// 4  0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
// 3  0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
// 2  0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
// 1  0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
//
//    a b c d e f g h    a b c d e f g h    a b c d e f g h    a b c d e f g h

const U64 not_a_file  = 18374403900871474942ULL;                                                                       // not A  file constant
const U64 not_h_file  = 9187201950435737471ULL;                                                                        // not H  file constant
const U64 not_hg_file = 4557430888798830399ULL;                                                                        // not HG file constant
const U64 not_ab_file = 18229723555195321596ULL;                                                                       // not AB file constant

const int bishop_relevant_bits[64] = {  6,  5,  5,  5,  5,  5,  5,  6,                                                 // bishop relevant occupancy bit count for every square on board
                                        5,  5,  5,  5,  5,  5,  5,  5,
                                        5,  5,  7,  7,  7,  7,  5,  5,
                                        5,  5,  7,  9,  9,  7,  5,  5,
                                        5,  5,  7,  9,  9,  7,  5,  5,
                                        5,  5,  7,  7,  7,  7,  5,  5,
                                        5,  5,  5,  5,  5,  5,  5,  5,
                                        6,  5,  5,  5,  5,  5,  5,  6 };

const int rook_relevant_bits[64]   = { 12, 11, 11, 11, 11, 11, 11, 12,                                                 // rook relevant occupancy bit count for every square on board
                                       11, 10, 10, 10, 10, 10, 10, 11,
                                       11, 10, 10, 10, 10, 10, 10, 11,
                                       11, 10, 10, 10, 10, 10, 10, 11,
                                       11, 10, 10, 10, 10, 10, 10, 11,
                                       11, 10, 10, 10, 10, 10, 10, 11,
                                       11, 10, 10, 10, 10, 10, 10, 11,
                                       12, 11, 11, 11, 11, 11, 11, 12 };

// rook magic numbers
U64 rook_magic_numbers[64] = {
  0x8a80104000800020ULL, 0x140002000100040ULL,  0x2801880a0017001ULL,  0x100081001000420ULL,  0x200020010080420ULL,  0x3001c0002010008ULL,
  0x8480008002000100ULL, 0x2080088004402900ULL, 0x800098204000ULL,     0x2024401000200040ULL, 0x100802000801000ULL,  0x120800800801000ULL,
  0x208808088000400ULL,  0x2802200800400ULL,    0x2200800100020080ULL, 0x801000060821100ULL,  0x80044006422000ULL,   0x100808020004000ULL,
  0x12108a0010204200ULL, 0x140848010000802ULL,  0x481828014002800ULL,  0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
  0x100400080208000ULL,  0x2040002120081000ULL, 0x21200680100081ULL,   0x20100080080080ULL,   0x2000a00200410ULL,    0x20080800400ULL,
  0x80088400100102ULL,   0x80004600042881ULL,   0x4040008040800020ULL, 0x440003000200801ULL,  0x4200011004500ULL,    0x188020010100100ULL,
  0x14800401802800ULL,   0x2080040080800200ULL, 0x124080204001001ULL,  0x200046502000484ULL,  0x480400080088020ULL,  0x1000422010034000ULL,
  0x30200100110040ULL,   0x100021010009ULL,     0x2002080100110004ULL, 0x202008004008002ULL,  0x20020004010100ULL,   0x2048440040820001ULL,
  0x101002200408200ULL,  0x40802000401080ULL,   0x4008142004410100ULL, 0x2060820c0120200ULL,  0x1001004080100ULL,    0x20c020080040080ULL,
  0x2935610830022400ULL, 0x44440041009200ULL,   0x280001040802101ULL,  0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
  0x20030a0244872ULL,    0x12001008414402ULL,   0x2006104900a0804ULL,  0x1004081002402ULL
};

// bishop magic numbers
U64 bishop_magic_numbers[64] = {
  0x40040844404084ULL,   0x2004208a004208ULL,   0x10190041080202ULL,   0x108060845042010ULL,  0x581104180800210ULL,  0x2112080446200010ULL,
  0x1080820820060210ULL, 0x3c0808410220200ULL,  0x4050404440404ULL,    0x21001420088ULL,      0x24d0080801082102ULL, 0x1020a0a020400ULL,
  0x40308200402ULL,      0x4011002100800ULL,    0x401484104104005ULL,  0x801010402020200ULL,  0x400210c3880100ULL,   0x404022024108200ULL,
  0x810018200204102ULL,  0x4002801a02003ULL,    0x85040820080400ULL,   0x810102c808880400ULL, 0xe900410884800ULL,    0x8002020480840102ULL,
  0x220200865090201ULL,  0x2010100a02021202ULL, 0x152048408022401ULL,  0x20080002081110ULL,   0x4001001021004000ULL, 0x800040400a011002ULL,
  0xe4004081011002ULL,   0x1c004001012080ULL,   0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
  0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL, 0x209188240001000ULL,  0x400408a884001800ULL,
  0x110400a6080400ULL,   0x1840060a44020800ULL, 0x90080104000041ULL,   0x201011000808101ULL,  0x1a2208080504f080ULL, 0x8012020600211212ULL,
  0x500861011240000ULL,  0x180806108200800ULL,  0x4000020e01040044ULL, 0x300000261044000aULL, 0x802241102020002ULL,  0x20906061210001ULL,
  0x5a84841004010310ULL, 0x4010801011c04ULL,    0xa010109502200ULL,    0x4a02012000ULL,       0x500201010098b028ULL, 0x8040002811040900ULL,
  0x28000010020204ULL,   0x6000020202d0240ULL,  0x8918844842082200ULL, 0x4010011029020020ULL
};

U64 pawn_attacks[2][64];                                                                                               // pawn attacks table [side][square]
U64 knight_attacks[64];                                                                                                // knight attacks table [square]
U64 king_attacks[64];                                                                                                  // king attacks table [square]
U64 bishop_masks[64];                                                                                                  // bishop attack masks
U64 rook_masks[64];                                                                                                    // rook attack masks
U64 bishop_attacks[64][512];                                                                                           // bishop attacks table [square][occupancies]
U64 rook_attacks[64][4096];                                                                                            // rook attacks rable [square][occupancies]

// generate pawn attacks
U64
mask_pawn_attacks(int side, int square)
{
  U64 attacks  = 0ULL;                                                                                                 // result attacks bitboard
  U64 bitboard = 0ULL;                                                                                                 // piece bitboard
  set_bit(bitboard, square);                                                                                           // set piece on board

  // generate pawn attacks
  if (!side) {                                                                                                         // white pawns
    if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
  }
  else {                                                                                                               // black pawns
    if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
  }
  return attacks;
}

// generate knight attacks
U64
mask_knight_attacks(int square)
{
  U64 attacks  = 0ULL;                                                                                                 // result attacks bitboard
  U64 bitboard = 0ULL;                                                                                                 // piece bitboard
  set_bit(bitboard, square);                                                                                           // set piece on board
  if ((bitboard >> 17) & not_h_file)   attacks |= (bitboard >> 17);                                                    // generate knight attacks
  if ((bitboard >> 15) & not_a_file)   attacks |= (bitboard >> 15);
  if ((bitboard >> 10) & not_hg_file)  attacks |= (bitboard >> 10);
  if ((bitboard >>  6) & not_ab_file)  attacks |= (bitboard >>  6);
  if ((bitboard << 17) & not_a_file)   attacks |= (bitboard << 17);
  if ((bitboard << 15) & not_h_file)   attacks |= (bitboard << 15);
  if ((bitboard << 10) & not_ab_file)  attacks |= (bitboard << 10);
  if ((bitboard <<  6) & not_hg_file)  attacks |= (bitboard <<  6);
  return attacks;
}

// generate king attacks
U64
mask_king_attacks(int square)
{
  U64 attacks  = 0ULL;                                                                                                 // result attacks bitboard
  U64 bitboard = 0ULL;                                                                                                 // piece bitboard
  set_bit(bitboard, square);                                                                                           // set piece on board
  if  (bitboard >> 8)                 attacks |= (bitboard >> 8);                                                      // generate king attacks
  if ((bitboard >> 9) & not_h_file)   attacks |= (bitboard >> 9);
  if ((bitboard >> 7) & not_a_file)   attacks |= (bitboard >> 7);
  if ((bitboard >> 1) & not_h_file)   attacks |= (bitboard >> 1);
  if  (bitboard << 8)                 attacks |= (bitboard << 8);
  if ((bitboard << 9) & not_a_file)   attacks |= (bitboard << 9);
  if ((bitboard << 7) & not_h_file)   attacks |= (bitboard << 7);
  if ((bitboard << 1) & not_a_file)   attacks |= (bitboard << 1);
  return attacks;
}

// mask bishop attacks
U64
mask_bishop_attacks(int square)
{
  U64 attacks = 0ULL;                                                                                                  // result attacks bitboard
  int r, f;                                                                                                            // declare ranks & files

  int tr = square / 8;                                                                                                 // init target rank & files
  int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)  attacks |= (1ULL << (r * 8 + f));                          // mask relevant bishop occupancy bits
  for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)  attacks |= (1ULL << (r * 8 + f));
  for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)  attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)  attacks |= (1ULL << (r * 8 + f));
  return attacks;
}

// mask rook attacks
U64
mask_rook_attacks(int square)
{
  U64 attacks = 0ULL;                                                                                                  // result attacks bitboard
  int r, f;                                                                                                            // declare ranks & files

  int tr = square / 8;                                                                                                 // init target rank & files
  int tf = square % 8;

  for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << ( r * 8 + tf));                                                    // mask relevant rook occupancy bits
  for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << ( r * 8 + tf));
  for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
  for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
  return attacks;
}

// generate bishop attacks on the fly
U64
bishop_attacks_on_the_fly(int square, U64 block)
{
  U64 attacks = 0ULL;                                                                                                  // result attacks bitboard
  int r, f;                                                                                                            // declare ranks & files

  int tr = square / 8;                                                                                                 // init target rank & files
  int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {                                                           // generate bishop atacks
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block) break;
  }
  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block) break;
  }
  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block) break;
  }
  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block) break;
  }
  return attacks;
}

// generate rook attacks on the fly
U64
rook_attacks_on_the_fly(int square, U64 block)
{
  U64 attacks = 0ULL;                                                                                                  // result attacks bitboard
  int r, f;                                                                                                            // declare ranks & files

  int tr = square / 8;                                                                                                 // init target rank & files
  int tf = square % 8;

  for (r = tr + 1; r <= 7; r++) {                                                                                      // generate rook attacks
    attacks |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & block) break;
  }
  for (r = tr - 1; r >= 0; r--) {
    attacks |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & block) break;
  }
  for (f = tf + 1; f <= 7; f++) {
    attacks |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & block) break;
  }
  for (f = tf - 1; f >= 0; f--) {
    attacks |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & block) break;
  }
  return attacks;
}

// init leaper pieces attacks
void
init_leapers_attacks()
{
  for (int square = 0; square < 64; square++) {
    pawn_attacks[white][square] = mask_pawn_attacks(white, square);                                                    // init pawn attacks
    pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    knight_attacks[square]      = mask_knight_attacks(square);                                                         // init knight attacks
    king_attacks[square]        = mask_king_attacks(square);                                                           // init king attacks
  }
}

// set occupancies
U64
set_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
  U64 occupancy = 0ULL;                                                                                                // occupancy map

  for (int count = 0; count < bits_in_mask; count++) {                                                                 // loop over the range of bits within attack mask
    int square = get_ls1b_index(attack_mask);                                                                          // get LS1B index of attacks mask
    pop_bit(attack_mask, square);                                                                                      // pop LS1B in attack map
    if (index & (1 << count))                                                                                          // make sure occupancy is on board
      occupancy |= (1ULL << square);                                                                                   // populate occupancy map
  }
  return occupancy;
}

// Magics

// find appropriate magic number
U64
find_magic_number(int square, int relevant_bits, int bishop)
{
  U64 occupancies[4096];                                                                                               // init occupancies
  U64 attacks[4096];                                                                                                   // init attack tables
  U64 used_attacks[4096];                                                                                              // init used attacks

  U64 attack_mask        = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);                           // init attack mask for a current piece
  int occupancy_indicies = 1 << relevant_bits;                                                                         // init occupancy indicies

  for (int index = 0; index < occupancy_indicies; index++) {                                                           // loop over occupancy indicies
    occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);                                             // init occupancies
    attacks[index] =   bishop                                                                                          // init attacks
                     ? bishop_attacks_on_the_fly(square, occupancies[index])
                     :   rook_attacks_on_the_fly(square, occupancies[index]);
  }

  for (int random_count = 0; random_count < 100000000; random_count++) {                                               // test magic numbers loop
    U64 magic_number = generate_magic_number();                                                                        // generate magic number candidate
    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;                                   // skip inappropriate magic numbers
    memset(used_attacks, 0ULL, sizeof(used_attacks));                                                                  // init used attacks

    int fail;                                                                                                          // init fail flag
    for (int index = 0, fail = 0; !fail && index < occupancy_indicies; index++) {                                      // test magic index loop
      int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));                            // init magic index
      if      (used_attacks[magic_index] == 0ULL)            used_attacks[magic_index] = attacks[index];               // init used attacks
      else if (used_attacks[magic_index] != attacks[index])  fail = 1;                                                 // magic index doesn't work
    }
    if (!fail) return magic_number;                                                                                    // if magic number works return it
  }

  printf("  Magic number fails!\n");                                                                                   // magic number doesn't work
  return 0ULL;
}

// init magic numbers
void
init_magic_numbers()
{
  for (int square = 0; square < 64; square++)                                                                          // init rook magic numbers
    rook_magic_numbers[square]   = find_magic_number(square, rook_relevant_bits[square], rook);
  for (int square = 0; square < 64; square++)                                                                          // init bishop magic numbers
    bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
}

// init slider piece's attack tables
void
init_sliders_attacks(int bishop)
{
  for (int square = 0; square < 64; square++) {
    bishop_masks[square]    = mask_bishop_attacks(square);                                                             // init bishop masks
    rook_masks[square]      = mask_rook_attacks(square);                                                               // init rook masks
    U64 attack_mask         = bishop ? bishop_masks[square] : rook_masks[square];                                      // init current mask
    int relevant_bits_count = count_bits(attack_mask);                                                                 // init relevant occupancy bit count
    int occupancy_indicies  = (1 << relevant_bits_count);                                                              // init occupancy indicies

    for (int index = 0; index < occupancy_indicies; index++) {                                                         // loop over occupancy indicies
      if (bishop) {                                                                                                    // bishop
        U64 occupancy   = set_occupancy(index, relevant_bits_count, attack_mask);                                      // init current occupancy variation
        int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);           // init magic index
        bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);                            // init bishop attacks
      }
      else {                                                                                                           // rook
        U64 occupancy   = set_occupancy(index, relevant_bits_count, attack_mask);                                      // init current occupancy variation
        int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);               // init magic index
        rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);                                // init rook attacks
      }
    }
  }
}

// get bishop attacks
static inline U64
get_bishop_attacks(int square, U64 occupancy)
{
  occupancy  &= bishop_masks[square];                                                                                  // get bishop attacks assuming current board occupancy
  occupancy  *= bishop_magic_numbers[square];
  occupancy >>= 64 - bishop_relevant_bits[square];

  return bishop_attacks[square][occupancy];                                                                            // return bishop attacks
}

// get rook attacks
static inline U64
get_rook_attacks(int square, U64 occupancy)
{
  occupancy  &= rook_masks[square];                                                                                    // get rook attacks assuming current board occupancy
  occupancy  *= rook_magic_numbers[square];
  occupancy >>= 64 - rook_relevant_bits[square];

  return rook_attacks[square][occupancy];
}

// get queen attacks
static inline U64
get_queen_attacks(int square, U64 occupancy)
{
  U64 queen_attacks      = 0ULL;                                                                                       // init result attacks bitboard
  U64 bishop_occupancy   = occupancy;                                                                                  // init bishop occupancies
  U64 rook_occupancy     = occupancy;                                                                                  // init rook occupancies

  bishop_occupancy      &= bishop_masks[square];                                                                       // get bishop attacks assuming current board occupancy
  bishop_occupancy      *= bishop_magic_numbers[square];
  bishop_occupancy     >>= 64 - bishop_relevant_bits[square];
  queen_attacks          = bishop_attacks[square][bishop_occupancy];                                                   // get bishop attacks
  rook_occupancy        &= rook_masks[square];                                                                         // get rook attacks assuming current board occupancy
  rook_occupancy        *= rook_magic_numbers[square];
  rook_occupancy       >>= 64 - rook_relevant_bits[square];
  queen_attacks         |= rook_attacks[square][rook_occupancy];                                                       // get rook attacks

  return queen_attacks;
}

// Move generator

// is square current given attacked by the current given side
static inline int
is_square_attacked(int square, int side)
{
  if ((side == white) && (pawn_attacks[black][square] & bitboards[P]))                                 return 1;       // attacked by white pawns
  if ((side == black) && (pawn_attacks[white][square] & bitboards[p]))                                 return 1;       // attacked by black pawns
  if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n]))                        return 1;       // attacked by knights
  if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;       // attacked by bishops
  if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r]))   return 1;       // attacked by rooks
  if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q]))  return 1;       // attacked by bishops
  if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k]))                          return 1;       // attacked by kings
  return 0;                                                                                                            // by default return false
}

// print attacked squares
void
print_attacked_squares(int side)
{
  printf("\n");

  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      if (!file) printf("  %d ", 8 - rank);                                                                            // print ranks
      printf(" %d", is_square_attacked(square, side) ? 1 : 0);                                                         // check whether current square is attacked or not
    }
    printf("\n");
  }
  printf("\n     a b c d e f g h\n\n");
}

//    binary move bits                                     hexidecimal constants
//
//    0000 0000 0000 0000 0011 1111    source square       0x3f
//    0000 0000 0000 1111 1100 0000    target square       0xfc0
//    0000 0000 1111 0000 0000 0000    piece               0xf000
//    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
//    0001 0000 0000 0000 0000 0000    capture flag        0x100000
//    0010 0000 0000 0000 0000 0000    double push flag    0x200000
//    0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
//    1000 0000 0000 0000 0000 0000    castling flag       0x800000

// encode move
#define encode_move(                                                           \
  source, target, piece, promoted, capture, double, enpassant, castling)       \
  (source) | (target << 6) | (piece << 12) | (promoted << 16) |                \
    (capture << 20) | (double << 21) | (enpassant << 22) | (castling << 23)

#define get_move_source(move)     (move & 0x3f)                                                                        /* extract source square */
#define get_move_target(move)    ((move & 0xfc0)   >>  6)                                                              /* extract target square */
#define get_move_piece(move)     ((move & 0xf000)  >> 12)                                                              /* extract piece */
#define get_move_promoted(move)  ((move & 0xf0000) >> 16)                                                              /* extract promoted piece */
#define get_move_capture(move)    (move & 0x100000)                                                                    /* extract capture flag */
#define get_move_double(move)     (move & 0x200000)                                                                    /* extract double pawn push flag */
#define get_move_enpassant(move)  (move & 0x400000)                                                                    /* extract enpassant flag */
#define get_move_castling(move)   (move & 0x800000)                                                                    /* extract castling flag */

// move list structure
typedef struct
{
  int moves[256];
  int count;                                                                                                           // move count
} moves;

// add move to the move list
static inline void
add_move(moves* move_list, int move)
{
  move_list->moves[move_list->count] = move;                                                                           // store move
  move_list->count++;                                                                                                  // increment move count
}

// print move (for UCI purposes)
void
print_move(int move)
{
  if (get_move_promoted(move))
    printf("%s%s%c",
           square_to_coordinates[get_move_source(move)],
           square_to_coordinates[get_move_target(move)],
           promoted_pieces[get_move_promoted(move)]);
  else
    printf("%s%s",
           square_to_coordinates[get_move_source(move)],
           square_to_coordinates[get_move_target(move)]);
}

// print move list
void
print_move_list(moves* move_list)
{
  if (!move_list->count) {                                                                                             // do nothing on empty move list
    printf("\n     No move in the move list!\n");
    return;
  }
  printf("\n     move    piece     capture   double    enpass    castling\n\n");
  for (int move_count = 0; move_count < move_list->count; move_count++) {                                              // loop over moves within a move list
    int move = move_list->moves[move_count];                                                                           // init move

  printf("     %s%s%c   %s         %d         %d         %d         %d\n",                                             // print move
         square_to_coordinates[get_move_source(move)],
         square_to_coordinates[get_move_target(move)],
         get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)] : ' ',
         unicode_pieces[get_move_piece(move)],
         get_move_capture(move)   ? 1 : 0,
         get_move_double(move)    ? 1 : 0,
         get_move_enpassant(move) ? 1 : 0,
         get_move_castling(move)  ? 1 : 0);
  }
  printf("\n\n     Total number of moves: %d\n\n", move_list->count);                                                  // print total number of moves
}

// preserve board state
#define copy_board()                                                           \
  U64 bitboards_copy[12], occupancies_copy[3];                                 \
  int side_copy, enpassant_copy, castle_copy;                                  \
  memcpy(bitboards_copy, bitboards, 96);                                       \
  memcpy(occupancies_copy, occupancies, 24);                                   \
  side_copy = side, enpassant_copy = enpassant, castle_copy = castle;          \
  U64 hash_key_copy = hash_key;

// restore board state
#define take_back()                                                            \
  memcpy(bitboards, bitboards_copy, 96);                                       \
  memcpy(occupancies, occupancies_copy, 24);                                   \
  side = side_copy, enpassant = enpassant_copy, castle = castle_copy;          \
  hash_key = hash_key_copy;

// move types
enum
{
  all_moves,
  only_captures
};

//  castling right                move update     binary  decimal
// 
//  king & rooks didn't move:     1111 & 1111  =  1111    15
//         white king  moved:     1111 & 1100  =  1100    12
//   white king's rook moved:     1111 & 1110  =  1110    14
//  white queen's rook moved:     1111 & 1101  =  1101    13
//          black king moved:     1111 & 0011  =  0011     3
//   black king's rook moved:     1111 & 1011  =  1011    11
//  black queen's rook moved:     1111 & 0111  =  0111     7


// castling rights update constants
const int castling_rights[64] = {  7, 15, 15, 15,  3, 15, 15, 11,
                                  15, 15, 15, 15, 15, 15, 15, 15,
                                  15, 15, 15, 15, 15, 15, 15, 15,
                                  15, 15, 15, 15, 15, 15, 15, 15,
                                  15, 15, 15, 15, 15, 15, 15, 15,
                                  15, 15, 15, 15, 15, 15, 15, 15,
                                  15, 15, 15, 15, 15, 15, 15, 15,
                                  13, 15, 15, 15, 12, 15, 15, 14 };

// make move on chess board
static inline int
make_move(int move, int move_flag)
{
  if (move_flag == all_moves) {                                                                                        // quiet moves
    copy_board();                                                                                                      // preserve board state

    int source_square  = get_move_source(move);                                                                        // parse move
    int target_square  = get_move_target(move);
    int piece          = get_move_piece(move);
    int promoted_piece = get_move_promoted(move);
    int capture        = get_move_capture(move);
    int double_push    = get_move_double(move);
    int enpass         = get_move_enpassant(move);
    int castling       = get_move_castling(move);

    pop_bit(bitboards[piece], source_square);                                                                          // move piece
    set_bit(bitboards[piece], target_square);

    hash_key ^= piece_keys[piece][source_square];                                                                      // remove piece from source square in hash key
    hash_key ^= piece_keys[piece][target_square];                                                                      // set piece to the target square in hash key

    if (capture) {                                                                                                     // handling capture moves
      int start_piece, end_piece;                                                                                      // pick up bitboard piece index ranges depending on side

      if (side == white) {                                                                                             // white to move
        start_piece = p;
        end_piece = k;
      }
      else {                                                                                                           // black to move
        start_piece = P;
        end_piece = K;
      }

      for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {                                            // loop over bitboards opposite to the current side to move
        if (get_bit(bitboards[bb_piece], target_square)) {                                                             // if there's a piece on the target square
          pop_bit(bitboards[bb_piece], target_square);                                                                 // remove it from corresponding bitboard
          hash_key ^= piece_keys[bb_piece][target_square];                                                             // remove the piece from hash key
          break;
        }
      }
    }

    if (promoted_piece) {                                                                                              // handle pawn promotions
      if (side == white) {                                                                                             // white to move
        pop_bit(bitboards[P], target_square);                                                                          // erase the pawn from the target square
        hash_key ^= piece_keys[P][target_square];                                                                      // remove pawn from hash key
      }
      else {                                                                                                           // black to move
        pop_bit(bitboards[p], target_square);                                                                          // erase the pawn from the target square
        hash_key ^= piece_keys[p][target_square];                                                                      // remove pawn from hash key
      }
      set_bit(bitboards[promoted_piece], target_square);                                                               // set up promoted piece on chess board
      hash_key ^= piece_keys[promoted_piece][target_square];                                                           // add promoted piece into the hash key
    }

    if (enpass) {                                                                                                      // handle enpassant captures
      (side == white) ? pop_bit(bitboards[p], target_square + 8)                                                       // erase the pawn depending on side to move
                      : pop_bit(bitboards[P], target_square - 8);
      if (side == white) {                                                                                             // white to move
        pop_bit(bitboards[p], target_square + 8);                                                                      // remove captured pawn
        hash_key ^= piece_keys[p][target_square + 8];                                                                  // remove pawn from hash key
      }
      else {                                                                                                           // black to move
        pop_bit(bitboards[P], target_square - 8);                                                                      // remove captured pawn
        hash_key ^= piece_keys[P][target_square - 8];                                                                  // remove pawn from hash key
      }
    }

    if (enpassant != no_sq) hash_key ^= enpassant_keys[enpassant];                                                     // hash enpassant if available (remove enpassant square from hash key )

    enpassant = no_sq;                                                                                                 // reset enpassant square

    if (double_push) {                                                                                                 // handle double pawn push
      if (side == white) {                                                                                             // white to move
        enpassant = target_square + 8;                                                                                 // set enpassant square
        hash_key ^= enpassant_keys[target_square + 8];                                                                 // hash enpassant
      }
      else {                                                                                                           // black to move
        enpassant = target_square - 8;                                                                                 // set enpassant square
        hash_key ^= enpassant_keys[target_square - 8];                                                                 // hash enpassant
      }
    }

    if (castling) {                                                                                                    // handle castling moves
      switch (target_square) {                                                                                         // switch target square
        case (g1):                                                                                                     // move H rook
          pop_bit(bitboards[R], h1);
          set_bit(bitboards[R], f1);
          hash_key ^= piece_keys[R][h1];                                                                               // remove rook from h1 from hash key
          hash_key ^= piece_keys[R][f1];                                                                               // put rook on f1 into a hash key
          break;
        case (c1):                                                                                                     // white castles queen side
          pop_bit(bitboards[R], a1);                                                                                   // move A rook
          set_bit(bitboards[R], d1);
          hash_key ^= piece_keys[R][a1];                                                                               // remove rook from a1 from hash key
          hash_key ^= piece_keys[R][d1];                                                                               // put rook on d1 into a hash key
          break;
        case (g8):                                                                                                     // black castles king side
          pop_bit(bitboards[r], h8);                                                                                   // move H rook
          set_bit(bitboards[r], f8);
          hash_key ^= piece_keys[r][h8];                                                                               // remove rook from h8 from hash key
          hash_key ^= piece_keys[r][f8];                                                                               // put rook on f8 into a hash key
          break;
        case (c8):                                                                                                     // black castles queen side
          pop_bit(bitboards[r], a8);                                                                                   // move A rook
          set_bit(bitboards[r], d8);
          hash_key ^= piece_keys[r][a8];                                                                               // remove rook from a8 from hash key
          hash_key ^= piece_keys[r][d8];                                                                               // put rook on d8 into a hash key
          break;
      }
    }

    hash_key ^= castle_keys[castle];                                                                                   // hash castling

    castle &= castling_rights[source_square];                                                                          // update castling rights
    castle &= castling_rights[target_square];

    hash_key ^= castle_keys[castle];                                                                                   // hash castling

    memset(occupancies, 0ULL, 24);                                                                                     // reset occupancies

    for (int bb_piece = P; bb_piece <= K; bb_piece++)                                                                  // loop over white pieces bitboards
      occupancies[white] |= bitboards[bb_piece];                                                                       // update white occupancies

    for (int bb_piece = p; bb_piece <= k; bb_piece++)                                                                  // loop over black pieces bitboards
      occupancies[black] |= bitboards[bb_piece];                                                                       // update black occupancies

    occupancies[both] |= occupancies[white];                                                                           // update both sides occupancies
    occupancies[both] |= occupancies[black];

    side ^= 1;                                                                                                         // change side
    hash_key ^= side_key;                                                                                              // hash side

    if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k])                                              // make sure that king has not been exposed into a check
                                           : get_ls1b_index(bitboards[K]), side)) {
      take_back();                                                                                                     // take move back
      return 0;                                                                                                        // return illegal move
    }
    else
      return 1;                                                                                                        // return legal move
  }
  else {                                                                                                               // capture moves
    if    (get_move_capture(move))  make_move(move, all_moves);                                                        // make sure move is the capture
    else                            return 0;                                                                          // don't make it otherwise; the move is not a capture
  }
}                                                                                                                      // ????

// generate all moves
static inline void
generate_moves(moves* move_list)
{
  move_list->count = 0;                                                                                                // init move count
  int source_square, target_square;

  U64 bitboard, attacks;                                                                                               // define current piece's bitboard copy & it's attacks

  for (int piece = P; piece <= k; piece++) {                                                                           // loop over all the bitboards
    bitboard = bitboards[piece];                                                                                       // init piece bitboard copy

    if (side == white) {                                                                                               // generate white pawns & white king castling moves
      if (piece == P) {                                                                                                // pick up white pawn bitboards index
        while (bitboard) {                                                                                             // loop over white pawns within white pawn bitboard
          source_square = get_ls1b_index(bitboard);                                                                    // init source square
          target_square = source_square - 8;                                                                           // init target square
          if (!(target_square < a8) &&                                                                                 // generate quiet pawn moves
              !get_bit(occupancies[both], target_square)) {
            if (source_square >= a7 && source_square <= h7) {                                                          // pawn promotion
              add_move(move_list, encode_move( source_square, target_square, piece, Q, 0, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, R, 0, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, B, 0, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, N, 0, 0, 0, 0));
            }
            else {
              add_move(move_list, encode_move( source_square, target_square, piece, 0, 0, 0, 0, 0));                   // one square ahead pawn move
              if ((source_square >= a2 && source_square <= h2) &&                                                      // two squares ahead pawn move
                  !get_bit(occupancies[both], target_square - 8))
                add_move( move_list, encode_move( source_square, target_square - 8, piece, 0, 0, 1, 0, 0));            // ?????
            }
          }

          attacks = pawn_attacks[side][source_square] & occupancies[black];                                            // init pawn attacks bitboard

          while (attacks) {                                                                                            // generate pawn captures
            target_square = get_ls1b_index(attacks);                                                                   // init target square

            if (source_square >= a7 && source_square <= h7) {                                                          // pawn promotion
              add_move(move_list, encode_move( source_square, target_square, piece, Q, 1, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, R, 1, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, B, 1, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, N, 1, 0, 0, 0));
            }
            else
              add_move(move_list, encode_move( source_square, target_square, piece, 0, 1, 0, 0, 0));                   // one square ahead pawn move

            pop_bit(attacks, target_square);                                                                           // pop ls1b of the pawn attacks
          }

          if (enpassant != no_sq) {                                                                                    // generate enpassant captures
            U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);                           // lookup pawn attacks and bitwise AND with enpassant square (bit)

            if (enpassant_attacks) {                                                                                   // make sure enpassant capture available
              int target_enpassant = get_ls1b_index(enpassant_attacks);                                                // init enpassant capture target square
              add_move( move_list, encode_move( source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
            }
          }

          pop_bit(bitboard, source_square);                                                                            // pop ls1b from piece bitboard copy
        }
      }

      if (piece == K) {                                                                                                // castling moves
        if (castle & wk) {                                                                                             // king side castling is available
          if (!get_bit(occupancies[both], f1) &&                                                                       // make sure square between king and king's rook are empty
              !get_bit(occupancies[both], g1)) {
            if (!is_square_attacked(e1, black) &&                                                                      // make sure king and the f1 squares are not under attacks
                !is_square_attacked(f1, black))
              add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
          }
        }

        if (castle & wq) {                                                                                             // queen side castling is available
          if (!get_bit(occupancies[both], d1) &&                                                                       // make sure square between king and queen's rook are empty
              !get_bit(occupancies[both], c1) &&
              !get_bit(occupancies[both], b1)) {
            if (!is_square_attacked(e1, black) &&                                                                      // make sure king and the d1 squares are not under attacks
                !is_square_attacked(d1, black))
              add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
          }
        }
      }
    }
    else {                                                                                                             // generate black pawns & black king castling moves
      if (piece == p) {                                                                                                // pick up black pawn bitboards index
        while (bitboard) {                                                                                             // loop over white pawns within white pawn bitboard
          source_square = get_ls1b_index(bitboard);                                                                    // init source square

          target_square = source_square + 8;                                                                           // init target square

          if (!(target_square > h1) &&                                                                                 // generate quiet pawn moves
              !get_bit(occupancies[both], target_square)) {
            if (source_square >= a2 && source_square <= h2) {                                                          // pawn promotion
              add_move(move_list, encode_move( source_square, target_square, piece, q, 0, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, r, 0, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, b, 0, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, n, 0, 0, 0, 0));
            }
            else {
              add_move(move_list, encode_move( source_square, target_square, piece, 0, 0, 0, 0, 0));                   // one square ahead pawn move

              if ((source_square >= a7 && source_square <= h7) &&                                                      // two squares ahead pawn move
                  !get_bit(occupancies[both], target_square + 8))
                add_move(move_list, encode_move( source_square, target_square + 8, piece, 0, 0, 1, 0, 0));             // ????
            }
          }

          attacks = pawn_attacks[side][source_square] & occupancies[white];                                            // init pawn attacks bitboard

          while (attacks) {                                                                                            // generate pawn captures
            target_square = get_ls1b_index(attacks);                                                                   // init target square

            if (source_square >= a2 && source_square <= h2) {                                                          // pawn promotion
              add_move(move_list, encode_move( source_square, target_square, piece, q, 1, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, r, 1, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, b, 1, 0, 0, 0));
              add_move(move_list, encode_move( source_square, target_square, piece, n, 1, 0, 0, 0));
            }
            else
              add_move(move_list, encode_move( source_square, target_square, piece, 0, 1, 0, 0, 0));                   // one square ahead pawn move

            pop_bit(attacks, target_square);                                                                           // pop ls1b of the pawn attacks
          }

          if (enpassant != no_sq) {                                                                                    // generate enpassant captures
            U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);                           // lookup pawn attacks and bitwise AND with enpassant square (bit)

            if (enpassant_attacks) {                                                                                   // make sure enpassant capture available
              int target_enpassant = get_ls1b_index(enpassant_attacks);                                                // init enpassant capture target square
              add_move(move_list, encode_move( source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
            }
          }

          pop_bit(bitboard, source_square);                                                                            // pop ls1b from piece bitboard copy
        }
      }

      if (piece == k) {                                                                                                // castling moves
        if (castle & bk) {                                                                                             // king side castling is available
          if (!get_bit(occupancies[both], f8) &&                                                                       // make sure square between king and king's rook are empty
              !get_bit(occupancies[both], g8)) {
            if (!is_square_attacked(e8, white) &&                                                                      // make sure king and the f8 squares are not under attacks
                !is_square_attacked(f8, white))
              add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
          }
        }

        if (castle & bq) {                                                                                             // queen side castling is available
          if (!get_bit(occupancies[both], d8) &&                                                                       // make sure square between king and queen's rook are empty
              !get_bit(occupancies[both], c8) &&
              !get_bit(occupancies[both], b8)) {
            if (!is_square_attacked(e8, white) &&                                                                      // make sure king and the d8 squares are not under attacks
                !is_square_attacked(d8, white))
              add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
          }
        }
      }
    }

    if ((side == white) ? piece == N : piece == n) {                                                                   // genarate knight moves
      while (bitboard) {                                                                                               // loop over source squares of piece bitboard copy
        source_square = get_ls1b_index(bitboard);                                                                      // init source square

        attacks = knight_attacks[source_square] &                                                                      // init piece attacks in order to get set of target squares
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {                                                                                              // loop over target squares available from generated attacks
          target_square = get_ls1b_index(attacks);                                                                     // init target square

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))                    // quiet move
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));                      // capture move

          pop_bit(attacks, target_square);                                                                             // pop ls1b in current attacks set
        }

        pop_bit(bitboard, source_square);                                                                              // pop ls1b of the current piece bitboard copy
      }
    }

    if ((side == white) ? piece == B : piece == b) {                                                                   // generate bishop moves
      while (bitboard) {                                                                                               // loop over source squares of piece bitboard copy
        source_square = get_ls1b_index(bitboard);                                                                      // init source square

        attacks = get_bishop_attacks(source_square, occupancies[both]) &                                               // init piece attacks in order to get set of target squares
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {                                                                                              // loop over target squares available from generated attacks
          target_square = get_ls1b_index(attacks);                                                                     // init target square

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))                    // quiet move
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));                      // capture move
          pop_bit(attacks, target_square);                                                                             // pop ls1b in current attacks set
        }
        pop_bit(bitboard, source_square);                                                                              // pop ls1b of the current piece bitboard copy
      }
    }

    if ((side == white) ? piece == R : piece == r) {                                                                   // generate rook moves
      while (bitboard) {                                                                                               // loop over source squares of piece bitboard copy
        source_square = get_ls1b_index(bitboard);                                                                      // init source square

        attacks = get_rook_attacks(source_square, occupancies[both]) &                                                 // init piece attacks in order to get set of target squares
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {                                                                                              // loop over target squares available from generated attacks
          target_square = get_ls1b_index(attacks);                                                                     // init target square

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))                    // quiet move
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));                      // capture move

          pop_bit(attacks, target_square);                                                                             // pop ls1b in current attacks set
        }

        pop_bit(bitboard, source_square);                                                                              // pop ls1b of the current piece bitboard copy
      }
    }

    if ((side == white) ? piece == Q : piece == q) {                                                                   // generate queen moves
      while (bitboard) {                                                                                               // loop over source squares of piece bitboard copy
        source_square = get_ls1b_index(bitboard);                                                                      // init source square

        attacks = get_queen_attacks(source_square, occupancies[both]) &                                                // init piece attacks in order to get set of target squares
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {                                                                                              // loop over target squares available from generated attacks
          target_square = get_ls1b_index(attacks);                                                                     // init target square

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))                    // quiet move
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));                      // capture move
          pop_bit(attacks, target_square);                                                                             // pop ls1b in current attacks set
        }

        pop_bit(bitboard, source_square);                                                                              // pop ls1b of the current piece bitboard copy
      }
    }

    if ((side == white) ? piece == K : piece == k) {                                                                   // generate king moves
      while (bitboard) {                                                                                               // loop over source squares of piece bitboard copy
        source_square = get_ls1b_index(bitboard);                                                                      // init source square

        attacks = king_attacks[source_square] &                                                                        // init piece attacks in order to get set of target squares
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {                                                                                              // loop over target squares available from generated attacks
          target_square = get_ls1b_index(attacks);                                                                     // init target square

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))                    // quiet move
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));                      // capture move

          pop_bit(attacks, target_square);                                                                             // pop ls1b in current attacks set
        }

        pop_bit(bitboard, source_square);                                                                              // pop ls1b of the current piece bitboard copy
      }
    }
  }
}

// Perft

// leaf nodes (number of positions reached during the test of the move generator at a given depth)
U64 nodes;

// perft driver
static inline void
perft_driver(int depth)
{
  if (depth == 0) {                                                                                                    // reccursion escape condition
    nodes++;                                                                                                           // increment nodes count (count reached positions)
    return;
  }

  moves move_list[1];                                                                                                  // create move list instance
  generate_moves(move_list);                                                                                           // generate moves

  for (int move_count = 0; move_count < move_list->count; move_count++) {                                              // loop over generated moves
    copy_board();                                                                                                      // preserve board state
    if (!make_move(move_list->moves[move_count], all_moves))                                                           // make move
      continue;                                                                                                        // skip to the next move
    perft_driver(depth - 1);                                                                                           // call perft driver recursively
    take_back();                                                                                                       // take back
  }
}

// perft test
void
perft_test(int depth)
{
  printf("\n     Performance test\n\n");

  moves move_list[1];                                                                                                  // create move list instance
  generate_moves(move_list);                                                                                           // generate moves

  long start = get_time_ms();                                                                                          // init start time

  for (int move_count = 0; move_count < move_list->count; move_count++) {                                              // loop over generated moves
    copy_board();                                                                                                      // preserve board state
    if (!make_move(move_list->moves[move_count], all_moves))                                                           // make move
      continue;                                                                                                        // skip to the next move
    long cummulative_nodes = nodes;                                                                                    // cummulative nodes
    perft_driver(depth - 1);                                                                                           // call perft driver recursively
    long old_nodes = nodes - cummulative_nodes;                                                                        // old nodes
    take_back();                                                                                                       // take back
    printf("     move: %s%s%c  nodes: %ld\n",                                                                          // print move
           square_to_coordinates[get_move_source(move_list->moves[move_count])],
           square_to_coordinates[get_move_target(move_list->moves[move_count])],
           get_move_promoted(move_list->moves[move_count])
             ? promoted_pieces[get_move_promoted(move_list->moves[move_count])]
             : ' ',
           old_nodes);
  }

  printf("\n    Depth: %d\n", depth); // print results
  printf("    Nodes: %lld\n", nodes);
  printf("     Time: %ld\n\n", get_time_ms() - start);
}

// Evaluation

// material scrore
// ♙ = 100   = ♙
// ♘ = 300   = ♙ * 3
// ♗ = 350   = ♙ * 3 + ♙ * 0.5
// ♖ = 500   = ♙ * 5
// ♕ = 1000  = ♙ * 10
// ♔ = 10000 = ♙ * 100

int material_score[12] = {
   100,   // white pawn score
   300,   // white knight scrore
   350,   // white bishop score
   500,   // white rook score
   1000,  // white queen score
   10000, // white king score
  -100,   // black pawn score
  -300,   // black knight scrore
  -350,   // black bishop score
  -500,   // black rook score
  -1000,  // black queen score
  -10000, // black king score
};

// pawn positional score
const int pawn_score[64] = { 90,  90,  90,  90,  90,  90,  90,  90,
                             30,  30,  30,  40,  40,  30,  30,  30,
                             20,  20,  20,  30,  30,  30,  20,  20,
                             10,  10,  10,  20,  20,  10,  10,  10,
                              5,   5,  10,  20,  20,   5,   5,   5,
                              0,   0,   0,   5,   5,   0,   0,   0,
                              0,   0,   0, -10, -10,   0,   0,   0,
                              0,   0,   0,   0,   0,   0,   0,   0 };

// knight positional score
const int knight_score[64] = { -5,   0,  0,  0,  0,  0,   0, -5,
                               -5,   0,  0, 10, 10,  0,   0, -5,
                               -5,   5, 20, 20, 20, 20,   5, -5,
                               -5,  10, 20, 30, 30, 20,  10, -5,
                               -5,  10, 20, 30, 30, 20,  10, -5,
                               -5,   5, 20, 10, 10, 20,   5, -5,
                               -5,   0,  0,  0,  0,  0,   0, -5,
                               -5, -10,  0,  0,  0,  0, -10, -5 };

// bishop positional score
const int bishop_score[64] = { 0,  0,   0,  0,  0,   0,  0, 0,
                               0,  0,   0,  0,  0,   0,  0, 0,
                               0, 20,   0, 10, 10,   0, 20, 0,
                               0,  0,  10, 20, 20,  10,  0, 0,
                               0,  0,  10, 20, 20,  10,  0, 0,
                               0, 10,   0,  0,  0,   0, 10, 0,
                               0, 30,   0,  0,  0,   0, 30, 0,
                               0,  0, -10,  0,  0, -10,  0, 0 };

// rook positional score
const int rook_score[64] = { 50, 50, 50, 50, 50, 50, 50, 50,
                             50, 50, 50, 50, 50, 50, 50, 50,
                              0,  0, 10, 20, 20, 10,  0,  0,
                              0,  0, 10, 20, 20, 10,  0,  0,
                              0,  0, 10, 20, 20, 10,  0,  0,
                              0,  0, 10, 20, 20, 10,  0,  0,
                              0,  0, 10, 20, 20, 10,  0,  0,
                              0,  0, 0,  20, 20,  0,  0,  0 };

// king positional score
const int king_score[64] = { 0, 0,  0,  0,   0,  0,  0, 0,
                             0, 0,  5,  5,   5,  5,  0, 0,
                             0, 5,  5, 10,  10,  5,  5, 0,
                             0, 5, 10, 20,  20, 10,  5, 0,
                             0, 5, 10, 20,  20, 10,  5, 0,
                             0, 0,  5, 10,  10,  5,  0, 0,
                             0, 5,  5, -5,  -5,  0,  5, 0,
                             0, 0,  5,  0, -15,  0, 10, 0 };

// mirror positional score tables for opposite side
const int mirror_score[128] = { a1, b1, c1, d1, e1, f1, g1, h1,
                                a2, b2, c2, d2, e2, f2, g2, h2,
                                a3, b3, c3, d3, e3, f3, g3, h3,
                                a4, b4, c4, d4, e4, f4, g4, h4,
                                a5, b5, c5, d5, e5, f5, g5, h5,
                                a6, b6, c6, d6, e6, f6, g6, h6,
                                a7, b7, c7, d7, e7, f7, g7, h7,
                                a8, b8, c8, d8, e8, f8, g8, h8 };

//          Rank mask            File mask           Isolated mask        Passed
//   pawn mask for square a6        for square f2         for square g2 for square
//   c4
//
//    8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 1 0 0    8  0 0 0 0 0 1 0 1     8  0 1 1
//   1 0 0 0 0 7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 1 0 0    7  0 0 0 0 0 1 0 1 7  0
//   1 1 1 0 0 0 0 6  1 1 1 1 1 1 1 1    6  0 0 0 0 0 1 0 0    6  0 0 0 0 0 1 0 1
//   6  0 1 1 1 0 0 0 0 5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 1 0 0    5  0 0 0 0 0 1
//   0 1     5  0 1 1 1 0 0 0 0 4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 1 0 0    4  0 0
//   0 0 0 1 0 1     4  0 0 0 0 0 0 0 0 3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 1 0 0
//   3  0 0 0 0 0 1 0 1     3  0 0 0 0 0 0 0 0 2  0 0 0 0 0 0 0 0    2  0 0 0 0 0
//   1 0 0    2  0 0 0 0 0 1 0 1     2  0 0 0 0 0 0 0 0 1  0 0 0 0 0 0 0 0    1  0
//   0 0 0 0 1 0 0    1  0 0 0 0 0 1 0 1     1  0 0 0 0 0 0 0 0
//
//       a b c d e f g h       a b c d e f g h       a b c d e f g h        a b c
//   d e f g h

U64 file_masks[64];                                                                                                    // file masks [square]
U64 rank_masks[64];                                                                                                    // rank masks [square]
U64 isolated_masks[64];                                                                                                // isolated pawn masks [square]
U64 white_passed_masks[64];                                                                                            // white passed pawn masks [square]
U64 black_passed_masks[64];                                                                                            // black passed pawn masks [square]

// extract rank from a square [square]
const int get_rank[64] = { 7, 7, 7, 7, 7, 7, 7, 7,
                           6, 6, 6, 6, 6, 6, 6, 6,
                           5, 5, 5, 5, 5, 5, 5, 5,
                           4, 4, 4, 4, 4, 4, 4, 4,
                           3, 3, 3, 3, 3, 3, 3, 3,
                           2, 2, 2, 2, 2, 2, 2, 2,
                           1, 1, 1, 1, 1, 1, 1, 1,
                           0, 0, 0, 0, 0, 0, 0, 0 };

const int double_pawn_penalty   = -10;                                                                                 // double pawns penalty
const int isolated_pawn_penalty = -10;                                                                                 // isolated pawn penalty
const int semi_open_file_score  =  10;                                                                                 // semi open file score
const int open_file_score       =  15;                                                                                 // open file score
const int king_shield_bonus     =   5;                                                                                 // king's shield bonus
const int passed_pawn_bonus[8]  = { 0, 10, 30, 50, 75, 100, 150, 200 };                                                // passed pawn bonus

// set file or rank mask
U64
set_file_rank_mask(int file_number, int rank_number)
{
  U64 mask = 0ULL; // file or rank mask

  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;

      if (file_number != -1) {
        if (file == file_number)                                                                                       // on file match
          mask |= set_bit(mask, square);                                                                               // ????; set bit on mask
      }
      else if (rank_number != -1) {
        if (rank == rank_number)                                                                                       // on rank match
          mask |= set_bit(mask, square);                                                                               // ????; set bit on mask
      }
    }
  }

  return mask;                                                                                                         // return mask
}

// init evaluation masks
void
init_evaluation_masks()
{
  /******** Init file masks ********/
  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      file_masks[square] |= set_file_rank_mask(file, -1);                                                              // init file mask for a current square
    }
  }
  /******** Init rank masks ********/
  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      rank_masks[square] |= set_file_rank_mask(-1, rank);                                                              // init rank mask for a current square
    }
  }
  /******** Init isolated masks ********/
  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      isolated_masks[square] |= set_file_rank_mask(file - 1, -1);                                                      // init isolated pawns masks for a current square
      isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
    }
  }
  /******** White passed masks ********/
  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      white_passed_masks[square] |= set_file_rank_mask(file - 1, -1);                                                  // init white passed pawns mask for a current square
      white_passed_masks[square] |= set_file_rank_mask(file, -1);
      white_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

      for (int i = 0; i < (8 - rank); i++)                                                                             // loop over redudant ranks
        white_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + file];                                                 // reset redudant bits
    }
  }
  /******** Black passed masks ********/
  for   (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      black_passed_masks[square] |= set_file_rank_mask(file - 1, -1);                                                  // init black passed pawns mask for a current square
      black_passed_masks[square] |= set_file_rank_mask(file, -1);
      black_passed_masks[square] |= set_file_rank_mask(file + 1, -1);                                                  

      for (int i = 0; i < rank + 1; i++)                                                                               // loop over redudant ranks
        black_passed_masks[square] &= ~rank_masks[i * 8 + file];                                                       // reset redudant bits
    }
  }
}

// position evaluation
static inline int
evaluate()
{
  int score = 0;                                                                                                       // static evaluation score
  U64 bitboard;                                                                                                        // current pieces bitboard copy
  int piece, square;                                                                                                   // init piece & square
  int double_pawns = 0;                                                                                                // penalties

  for (int bb_piece = P; bb_piece <= k; bb_piece++) {                                                                  // loop over piece bitboards
    bitboard = bitboards[bb_piece];                                                                                    // init piece bitboard copy

    while (bitboard) {                                                                                                 // loop over pieces within a bitboard
      piece  = bb_piece;                                                                                               // init piece
      square = get_ls1b_index(bitboard);                                                                               // init square

      score += material_score[piece];                                                                                  // score material weights

      switch (piece) {                                                                                                 // score positional piece scores
        case P:                                                                                                        // evaluate white pawns
          score        += pawn_score[square];                                                                          // positional score
          double_pawns  = count_bits(bitboards[P] & file_masks[square]);                                               // double pawn penalty
          if (double_pawns > 1)                                                                                        // on double pawns (tripple, etc)
            score += double_pawns * double_pawn_penalty;
          if ((bitboards[P] & isolated_masks[square]) == 0)                                                            // on isolated pawn
            score += isolated_pawn_penalty;                                                                            // give an isolated pawn penalty
          if ((white_passed_masks[square] & bitboards[p]) == 0)                                                        // on passed pawn
            score += passed_pawn_bonus[get_rank[square]];                                                              // give passed pawn bonus
          break;
        case N:                                                                                                        // evaluate white knights
          score += knight_score[square];                                                                               // positional score
          break;
        case B:                                                                                                        // evaluate white bishops
          score += bishop_score[square];                                                                               // positional scores
          score += count_bits(get_bishop_attacks(square, occupancies[both]));                                          // mobility
          break;
        case R:                                                                                                        // evaluate white rooks
          score += rook_score[square];                                                                                 // positional score
          if ((bitboards[P] & file_masks[square]) == 0)                                                                // semi open file
            score += semi_open_file_score;                                                                             // add semi open file bonus
          if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)                                               // semi open file
            score += open_file_score;                                                                                  // add semi open file bonus
          break;
        case Q:                                                                                                        // evaluate white queens
          score += count_bits(get_queen_attacks(square, occupancies[both]));                                           // mobility
          break;
        case K:                                                                                                        // evaluate white king
          score += king_score[square];                                                                                 // positional score
          if ((bitboards[P] & file_masks[square]) == 0)                                                                // semi open file
            score -= semi_open_file_score;                                                                             // add semi open file penalty
          if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)                                               // semi open file
            score -= open_file_score;                                                                                  // add semi open file penalty
          score += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;                          // king safety bonus
          break;
        case p:                                                                                                        // evaluate black pawns
          score -= pawn_score[mirror_score[square]];                                                                   // positional score
          double_pawns = count_bits(bitboards[p] & file_masks[square]);                                                // double pawn penalty

          if (double_pawns > 1)                                                                                        // on double pawns (tripple, etc)
            score -= double_pawns * double_pawn_penalty;

          if ((bitboards[p] & isolated_masks[square]) == 0)                                                            // on isolated pawnd
            score -= isolated_pawn_penalty;                                                                            // give an isolated pawn penalty

          if ((black_passed_masks[square] & bitboards[P]) == 0)                                                        // on passed pawn
            score -= passed_pawn_bonus[get_rank[mirror_score[square]]];                                                // give passed pawn bonus
          break;
        case n:                                                                                                        // evaluate black knights
          score -= knight_score[mirror_score[square]];                                                                 // positional score
          break;
        case b:                                                                                                        // evaluate black bishops
          score -= bishop_score[mirror_score[square]];                                                                 // positional score
          score -= count_bits(get_bishop_attacks(square, occupancies[both]));                                          // mobility
          break;
        case r:                                                                                                        // evaluate black rooks
          score -= rook_score[mirror_score[square]];                                                                   // positional score
          if ((bitboards[p] & file_masks[square]) == 0)                                                                // semi open file
            score -= semi_open_file_score;                                                                             // add semi open file bonus
          if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)                                               // semi open file
            score -= open_file_score;                                                                                  // add semi open file bonus
          break;
        case q:                                                                                                        // evaluate black queens
          score -= count_bits(get_queen_attacks(square, occupancies[both]));                                           // mobility
          break;
        case k:                                                                                                        // evaluate black king
          score -= king_score[mirror_score[square]];                                                                   // positional score
          if ((bitboards[p] & file_masks[square]) == 0)                                                                // semi open file
            score += semi_open_file_score;                                                                             // add semi open file penalty
          if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)                                               // semi open file
            score += open_file_score;                                                                                  // add semi open file penalty
          score -= count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;                          // king safety bonus
          break;
      }
      pop_bit(bitboard, square);                                                                                       // pop ls1b
    }
  }

  return (side == white) ? score : -score;                                                                             // return final evaluation based on side
}


// Search

// These are the score bounds for the range of the mating scores
// [-infinity, -mate_value ... -mate_score, ... score ... mate_score ...  mate_value, infinity]

#define infinity   50000
#define mate_value 49000
#define mate_score 48000

// most valuable victim & less valuable attacker

//    (Victims) Pawn Knight Bishop   Rook  Queen   King
//  (Attackers)
//        Pawn   105    205    305    405    505    605
//      Knight   104    204    304    404    504    604
//      Bishop   103    203    303    403    503    603
//        Rook   102    202    302    402    502    602
//       Queen   101    201    301    401    501    601
//        King   100    200    300    400    500    600

// MVV LVA [attacker][victim]
static int mvv_lva[12][12] = {
  105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605, 104, 204, 304,
  404, 504, 604, 104, 204, 304, 404, 504, 604, 103, 203, 303, 403, 503, 603,
  103, 203, 303, 403, 503, 603, 102, 202, 302, 402, 502, 602, 102, 202, 302,
  402, 502, 602, 101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
  100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600,

  105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605, 104, 204, 304,
  404, 504, 604, 104, 204, 304, 404, 504, 604, 103, 203, 303, 403, 503, 603,
  103, 203, 303, 403, 503, 603, 102, 202, 302, 402, 502, 602, 102, 202, 302,
  402, 502, 602, 101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
  100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600
};

#define max_ply 64                                                                                                     /* max ply that we can reach within a search */

int killer_moves[2][max_ply];                                                                                          // killer moves [id][ply]
int history_moves[12][64];                                                                                             // history moves [piece][square]

//      ================================
//            Triangular PV table
//      --------------------------------
//        PV line: e2e4 e7e5 g1f3 b8c6
//      ================================
//
//           0    1    2    3    4    5
//
//      0    m1   m2   m3   m4   m5   m6
//
//      1    0    m2   m3   m4   m5   m6
//
//      2    0    0    m3   m4   m5   m6
//
//      3    0    0    0    m4   m5   m6
//
//      4    0    0    0    0    m5   m6
//
//      5    0    0    0    0    0    m6

int pv_length[max_ply];                                                                                                // PV length [ply]
int pv_table[max_ply][max_ply];                                                                                        // PV table [ply][ply]
int follow_pv, score_pv;                                                                                               // follow PV & score PV move

// Transposition table
#define hash_size       800000                                                                                         /* hash table size (would be around 20MB) */
#define no_hash_entry   100000                                                                                         /* no hash entry found constant */
#define hash_flag_exact 0                                                                                              /* transposition table hash flags */
#define hash_flag_alpha 1
#define hash_flag_beta  2

typedef struct                                                                                                         // transposition table data structure
{
  U64 hash_key;                                                                                                        // "almost" unique chess position identifier
  int depth;                                                                                                           // current search depth
  int flag;                                                                                                            // flag the type of node (fail-low/fail-high/PV)
  int score;                                                                                                           // score (alpha/beta/PV)
} tt;                                                                                                                  // transposition table (TT aka hash table)

tt hash_table[hash_size];                                                                                              // define TT instance

// clear TT (hash table)
void
clear_hash_table()
{
  for (int index = 0; index < hash_size; index++) {                                                                    // loop over TT elements
    hash_table[index].hash_key = 0;                                                                                    // reset TT inner fields
    hash_table[index].depth    = 0;
    hash_table[index].flag     = 0;
    hash_table[index].score    = 0;
  }
}

// read hash entry data
static inline int
read_hash_entry(int alpha, int beta, int depth)
{
  tt* hash_entry = &hash_table[hash_key % hash_size];                                                                  // create a TT instance pointer to particular hash entry storing
                                                                                                                       // the scoring data for the current board position if available
  if (hash_entry->hash_key == hash_key) {                                                                              // make sure we're dealing with the exact position we need
    if (hash_entry->depth >= depth) {                                                                                  // make sure that we match the exact depth our search is now at
      int score = hash_entry->score;                                                                                   // extract stored score from TT entry
      if (score < -mate_score) score += ply;                                                                           // retrieve score independent from the actual path
      if (score > mate_score)  score -= ply;                                                                           // from root node (position) to current node (position)
      if (hash_entry->flag  == hash_flag_exact)                                                                        // match the exact (PV node) score
        return score;                                                                                                  // return exact (PV node) score
      if ((hash_entry->flag == hash_flag_alpha) && (score <= alpha))                                                   // match alpha (fail-low node) score
        return alpha;                                                                                                  // return alpha (fail-low node) score
      if ((hash_entry->flag == hash_flag_beta) && (score >= beta))                                                     // match beta (fail-high node) score
        return beta;                                                                                                   // return beta (fail-high node) score
    }
  }
  return no_hash_entry;                                                                                                // if hash entry doesn't exist
}

// write hash entry data
static inline void
write_hash_entry(int score, int depth, int hash_flag)
{
  tt* hash_entry = &hash_table[hash_key % hash_size];                                                                  // create a TT instance pointer to particular hash entry storing
                                                                                                                       // the scoring data for the current board position if available
  if (score < -mate_score) score -= ply;                                                                               // store score independent from the actual path
  if (score > mate_score)  score += ply;                                                                               // from root node (position) to current node (position)

  hash_entry->hash_key = hash_key;                                                                                     // write hash entry data
  hash_entry->score    = score;
  hash_entry->flag     = hash_flag;
  hash_entry->depth    = depth;
}

// enable PV move scoring
static inline void
enable_pv_scoring(moves* move_list)
{
  follow_pv = 0;                                                                                                       // disable following PV

  for (int count = 0; count < move_list->count; count++) {                                                             // loop over the moves within a move list
    if (pv_table[0][ply] == move_list->moves[count]) {                                                                 // make sure we hit PV move
      score_pv = 1;                                                                                                    // enable move scoring
      follow_pv = 1;                                                                                                   // enable following PV
    }
  }
}

//  =======================
//       Move ordering
//  =======================
//  1. PV move
//  2. Captures in MVV/LVA
//  3. 1st killer move
//  4. 2nd killer move
//  5. History moves
//  6. Unsorted moves

// score moves
static inline int
score_move(int move)
{
  if (score_pv) {                                                                                                      // if PV move scoring is allowed
    if (pv_table[0][ply] == move) {                                                                                    // make sure we are dealing with PV move
      score_pv = 0;                                                                                                    // disable score PV flag
      return 20000;                                                                                                    // give PV move the highest score to search it first
    }
  }

  if (get_move_capture(move)) {                                                                                        // score capture move
    int target_piece = P;                                                                                              // init target piece

    int start_piece, end_piece;                                                                                        // pick up bitboard piece index ranges depending on side

    if (side == white) {                                                                                               // pick up side to move
      start_piece = p;
      end_piece = k;
    } else {
      start_piece = P;
      end_piece = K;
    }

    for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {                                              // loop over bitboards opposite to the current side to move
      if (get_bit(bitboards[bb_piece], get_move_target(move))) {                                                       // if there's a piece on the target square
        target_piece = bb_piece;                                                                                       // remove it from corresponding bitboard
        break;
      }
    }

    return mvv_lva[get_move_piece(move)][target_piece] + 10000;                                                        // score move by MVV LVA lookup [source piece][target piece]
  }

  else {                                                                                                               // score quiet move
    if      (killer_moves[0][ply] == move) return 9000;                                                                // score 1st killer move
    else if (killer_moves[1][ply] == move) return 8000;                                                                // score 2nd killer move
    else                                   return history_moves[get_move_piece(move)][get_move_target(move)];          // score history move
  }

  return 0;
}

// sort moves in descending order
static inline int
sort_moves(moves* move_list)
{
  int move_scores[move_list->count];                                                                                   // move scores

  for (int count = 0; count < move_list->count; count++)                                                               // score all the moves within a move list
    move_scores[count] = score_move(move_list->moves[count]);                                                          // score move

  for   (int current_move = 0; current_move < move_list->count; current_move++) {                                      // loop over current move within a move list
    for (int next_move = current_move + 1; next_move < move_list->count; next_move++) {                                // loop over next move within a move list
      if (move_scores[current_move] < move_scores[next_move]) {                                                        // compare current and next move scores
        int temp_score = move_scores[current_move];                                                                    // swap scores
        move_scores[current_move] = move_scores[next_move];
        move_scores[next_move] = temp_score;

        int temp_move = move_list->moves[current_move];                                                                // swap moves
        move_list->moves[current_move] = move_list->moves[next_move];
        move_list->moves[next_move] = temp_move;
      }
    }
  }
}                                                                                                                      // ????

// print move scores
void
print_move_scores(moves* move_list)
{
  printf("     Move scores:\n\n");

  for (int count = 0; count < move_list->count; count++) {                                                             // loop over moves within a move list
    printf("     move: ");
    print_move(move_list->moves[count]);
    printf(" score: %d\n", score_move(move_list->moves[count]));
  }
}

// position repetition detection
static inline int
is_repetition()
{
  for (int index = 0; index < repetition_index; index++)                                                               // loop over repetition indicies range
    if (repetition_table[index] == hash_key)                                                                           // if we found the hash key same with a current
      return 1;                                                                                                        // we found a repetition

  return 0;                                                                                                            // if no repetition found
}

// quiescence search
static inline int
quiescence(int alpha, int beta)
{
  if ((nodes & 2047) == 0) communicate();                                                                              // every 2047 nodes; "listen" to the GUI/user input
  nodes++;
  if (ply > max_ply - 1) return evaluate();                                                                            // we are too deep, hence there's an overflow of arrays relying on max ply constant
  int evaluation = evaluate();
  if (evaluation >= beta)  return beta;                                                                                // fail-hard beta cutoff; node (position) fails high
  if (evaluation >  alpha) alpha = evaluation;                                                                         // found a better move; PV node (position)
  moves move_list[1];                                                                                                  // create move list instance
  generate_moves(move_list);                                                                                           // generate moves
  sort_moves(move_list);                                                                                               // sort moves

  for (int count = 0; count < move_list->count; count++) {                                                             // loop over moves within a movelist
    copy_board();                                                                                                      // preserve board state
    ply++;
    repetition_index++;                                                                                                // increment repetition index & store hash key
    repetition_table[repetition_index] = hash_key;
    if (make_move(move_list->moves[count], only_captures) == 0) {                                                      // make sure to make only legal moves
      ply--;
      repetition_index--;
      continue;                                                                                                        // skip to next move
    }
    int score = -quiescence(-beta, -alpha);                                                                            // score current move
    ply--;
    repetition_index--;
    take_back();                                                                                                       // take move back
    if (stopped == 1) return 0;                                                                                        // return 0 if time is up
    if (score > alpha) {                                                                                               // found a better move
      alpha = score;                                                                                                   // PV node (position)
      if (score >= beta) return beta;                                                                                  // fail-hard beta cutoff; node (position) fails high
    }
  }
  return alpha;                                                                                                        // node (position) fails low
}

const int full_depth_moves = 4;                                                                                        // full depth moves counter
const int reduction_limit  = 3;                                                                                        // depth limit to consider reduction

// negamax alpha beta search
static inline int
negamax(int alpha, int beta, int depth)
{
  int score;                                                                                                           // variable to store current move's score (from the static evaluation perspective)
  int hash_flag = hash_flag_alpha;                                                                                     // define hash flag

  if (ply && is_repetition()) return 0;                                                                                // if position repetition occurs return draw score

  int pv_node = beta - alpha > 1;                                                                                      // a hack by Pedro Castro to figure out whether the current node is PV node or not

  if (ply && (score = read_hash_entry(alpha, beta, depth)) != no_hash_entry && pv_node == 0) return score;             // read hash entry if we're not in a root ply and hash entry is available and current node is not a PV node
                                                                                                                       // if the move has already been searched we just return the score for this move without searching it
  if ((nodes & 2047) == 0) communicate();                                                                              // every 2047 nodes "listen" to the GUI/user input
  pv_length[ply] = ply;                                                                                                // init PV length

  if (depth == 0)        return quiescence(alpha, beta);                                                               // we are too deep, hence there's an overflow of arrays relying on max ply constant; run quiescence search
  if (ply > max_ply - 1) return evaluate();                                                                            // evaluate position

  nodes++;

  int in_check = is_square_attacked((side == white) ? get_ls1b_index(bitboards[K])                                     // is king in check
                                                    : get_ls1b_index(bitboards[k]), side ^ 1);

  if (in_check) depth++;                                                                                               // increase search depth if the king has been exposed into a check
  int legal_moves = 0;                                                                                                 // legal moves counter

  if (depth >= 3 && in_check == 0 && ply) {                                                                            // null move pruning
    copy_board();                                                                                                      // preserve board state
    ply++;
    repetition_index++;                                                                                                // increment repetition index & store hash key
    repetition_table[repetition_index] = hash_key;
    if (enpassant != no_sq) hash_key ^= enpassant_keys[enpassant];                                                     // hash enpassant if available
    enpassant  = no_sq;                                                                                                // reset enpassant capture square
    side      ^= 1;                                                                                                    // switch the side, literally giving opponent an extra move to make
    hash_key  ^= side_key;                                                                                             // hash the side
    score      = -negamax(-beta, -beta + 1, depth - 1 - 2);                                                            // search moves with reduced depth to find beta cutoffs depth - 1 - R where R is a reduction limit
    ply--;
    repetition_index--;                                                                                                // decrement repetition index
    take_back();                                                                                                       // restore board state
    if (stopped == 1)    return 0;                                                                                     // return 0 if time is up
    if (score   >= beta) return beta;                                                                                  // fail-hard beta cutoff node (position) fails high
  }
  moves move_list[1];                                                                                                  // create move list instance
  generate_moves(move_list);                                                                                           // generate moves
  if (follow_pv) enable_pv_scoring(move_list);                                                                         // if we are now following PV line enable PV move scoring
  sort_moves(move_list);                                                                                               // sort moves
  int moves_searched = 0;                                                                                              // number of moves searched in a move list

  for (int count = 0; count < move_list->count; count++) {                                                             // loop over moves within a movelist
    copy_board();                                                                                                      // preserve board state
    ply++;
    repetition_index++;                                                                                                // increment repetition index & store hash key
    repetition_table[repetition_index] = hash_key;

    if (make_move(move_list->moves[count], all_moves) == 0) {                                                          // make sure to make only legal moves
      ply--;
      repetition_index--;
      continue;                                                                                                        // skip to next move
    }
    legal_moves++;

    if (moves_searched == 0) score = -negamax(-beta, -alpha, depth - 1);                                               // full depth search do normal alpha beta search
    else {                                                                                                             // late move reduction (LMR)
      if (moves_searched >= full_depth_moves && depth >= reduction_limit &&                                            // condition to consider LMR
          in_check == 0 && get_move_capture(move_list->moves[count]) == 0 &&
          get_move_promoted(move_list->moves[count]) == 0)
        score = -negamax(-alpha - 1, -alpha, depth - 2);                                                               // search current move with reduced depth:
      else                                                                                                             // hack to ensure that full-depth search is done
        score = alpha + 1;

      if (score > alpha) {                                                                                             // principle variation search PVS
        score = -negamax(-alpha - 1, -alpha, depth - 1);                                                               // Once you've found a move with a score that is between alpha and beta,
                                                                                                                       // the rest of the moves are searched with the goal of proving that they
                                                                                                                       // are all bad. It's possible to do this a bit faster than a search that
                                                                                                                       // worries that one of the remaining moves might be good.

        if ((score > alpha) && (score < beta))                                                                         // If the algorithm finds out that it was wrong, and that one of the
                                                                                                                       // subsequent moves was better than the first PV move, it has to search
                                                                                                                       // again, in the normal alpha-beta manner.  This happens sometimes, and
                                                                                                                       // it's a waste of time, but generally not often enough to counteract
                                                                                                                       // the savings gained from doing the "bad move proof" search referred to earlier.
          score = -negamax(-beta, -alpha, depth - 1);                                                                  // re-search the move that has failed to be proved to be bad with normal alpha beta score bounds
      }
    }

    ply--;
    repetition_index--;
    take_back();                                                                                                       // take move back

    if (stopped == 1) return 0;                                                                                        // return 0 if time is up

    moves_searched++;                                                                                                  // increment the counter of moves searched so far

    if (score > alpha) { 
      hash_flag = hash_flag_exact;                                                                                     // found a better move switch hash flag from storing score for fail-low node to the one storing score for PV node

      if (get_move_capture(move_list->moves[count]) == 0)                                                              // on quiet moves
        history_moves[get_move_piece (move_list->moves[count])]
                     [get_move_target(move_list->moves[count])] += depth;                                              // store history moves

      alpha = score;                                                                                                   // PV node (position)
      pv_table[ply][ply] = move_list->moves[count];                                                                    // write PV move

      for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)                                          // loop over the next ply
        pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];                                                         // copy move from deeper ply into a current ply's line

      pv_length[ply] = pv_length[ply + 1];                                                                             // adjust PV length

      if (score >= beta) {                                                                                             // fail-hard beta cutoff
        write_hash_entry(beta, depth, hash_flag_beta);                                                                 // store hash entry with the score equal to beta

        if (get_move_capture(move_list->moves[count]) == 0) {                                                          // on quiet moves
          killer_moves[1][ply] = killer_moves[0][ply];                                                                 // store killer moves
          killer_moves[0][ply] = move_list->moves[count];
        }
        return beta;                                                                                                   // node (position) fails high
      }
    }
  }

  if (legal_moves == 0) {                                                                                              // we don't have any legal moves to make in the current postion
    if (in_check)                                                                                                      // king is in check
      return -mate_value + ply;                                                                                        // return mating score (assuming closest distance to mating position)
    else                                                                                                               // king is not in check
      return 0;                                                                                                        // return stalemate score
  }

  write_hash_entry(alpha, depth, hash_flag);                                                                           // store hash entry with the score equal to alpha
  return alpha;                                                                                                        // node (position) fails low
}

// search position for the best move
void
search_position(int depth)
{
  int score = 0;                                                                                                       // define best score variable
  nodes     = 0;                                                                                                       // reset nodes counter
  stopped   = 0;                                                                                                       // reset "time is up" flag
  follow_pv = 0;                                                                                                       // reset follow PV flags
  score_pv  = 0;

  memset(killer_moves,  0, sizeof(killer_moves));                                                                      // clear helper data structures for search
  memset(history_moves, 0, sizeof(history_moves));
  memset(pv_table,      0, sizeof(pv_table));
  memset(pv_length,     0, sizeof(pv_length));

  int alpha = -infinity;                                                                                               // define initial alpha beta bounds
  int beta  = infinity;

  for (int current_depth = 1; current_depth <= depth; current_depth++) {                                               // iterative deepening
    if (stopped == 1)                                                                                                  // if time is up
      break;                                                                                                           // stop calculating and return best move so far

    follow_pv = 1;                                                                                                     // enable follow PV flag

    score = negamax(alpha, beta, current_depth);                                                                       // find best move within a given position

    if ((score <= alpha) || (score >= beta)) {                                                                         // we fell outside the window, so try again with a full-width window (and the same depth)
      alpha = -infinity;
      beta  =  infinity;
      continue;
    }

    // set up the window for the next iteration
    alpha = score - 50;
    beta  = score + 50;

    // print search info
    if      (score > -mate_value && score < -mate_score) printf("info score mate %d depth %d nodes %lld time %d pv ", -(score + mate_value) / 2 - 1, current_depth, nodes, get_time_ms() - starttime);
    else if (score >  mate_score && score <  mate_value) printf("info score mate %d depth %d nodes %lld time %d pv ",  (mate_value - score) / 2 + 1, current_depth, nodes, get_time_ms() - starttime);
    else                                                 printf("info score cp %d depth %d nodes %lld time %d pv ",    score, current_depth, nodes, get_time_ms() - starttime);

    for (int count = 0; count < pv_length[0]; count++) {                                                               // loop over the moves within a PV line
      print_move(pv_table[0][count]);                                                                                  // print PV move
      printf(" ");
    }
    printf("\n");
  }

  printf("bestmove ");
  print_move(pv_table[0][0]);
  printf("\n");
}

//        UCI
//  forked from VICE
// by Richard Allbert

// parse user/GUI move string input (e.g. "e7e8q")
int
parse_move(char* move_string)
{
  moves move_list[1];                                                                                                  // create move list instance
  generate_moves(move_list);                                                                                           // generate moves

  int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;                                       // parse source square
  int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;                                       // parse target square

  for (int move_count = 0; move_count < move_list->count; move_count++) {                                              // loop over the moves within a move list
    int move = move_list->moves[move_count];                                                                           // init move

    if (source_square == get_move_source(move) &&                                                                      // make sure source & target squares are available within the generated move
        target_square == get_move_target(move)) {
      int promoted_piece = get_move_promoted(move);                                                                    // init promoted piece

      if (promoted_piece) {                                                                                            // promoted piece is available
        if      ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q') return move;                   // promoted to queen
        else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r') return move;                   // promoted to rook
        else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b') return move;                   // promoted to bishop
        else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n') return move;                   // promoted to knight
        continue;                                                                                                      // continue the loop on possible wrong promotions (e.g. "e7e8f")
      }
      return move;                                                                                                     // return legal move
    }
  }
  return 0;                                                                                                            // return illegal move
}

// parse UCI "position" command
void
parse_position(char* command)
{
  command += 9;                                                                                                        // shift pointer to the right where next token begins
  char* current_char = command;                                                                                        // init pointer to the current character in the command string

  if (strncmp(command, "startpos", 8) == 0) parse_fen(start_position);                                                 // init chess board with start position
  else {
    current_char = strstr(command, "fen");                                                                             // make sure "fen" command is available within command string
    if (current_char == NULL) parse_fen(start_position);                                                               // if no "fen" command is available within command string init chess board with start position
    else {                                                                                                             // found "fen" substring
      current_char += 4;                                                                                               // shift pointer to the right where next token begins
      parse_fen(current_char);                                                                                         // init chess board with position from FEN string
    }
  }

  current_char = strstr(command, "moves");                                                                             // parse moves after position

  if (current_char != NULL) {                                                                                          // moves available
    current_char += 6;                                                                                                 // shift pointer to the right where next token begins

    while (*current_char) {                                                                                            // loop over moves within a move string
      int move = parse_move(current_char);                                                                             // parse next move
      if (move == 0) break;                                                                                            // if no more moves break out of the loop
      repetition_index++;
      repetition_table[repetition_index] = hash_key;                                                                   // write hash key into a repetition table
      make_move(move, all_moves);                                                                                      // make move on the chess board
      while (*current_char && *current_char != ' ') current_char++;                                                    // move current character mointer to the end of current move
      current_char++;                                                                                                  // go to the next move
    }
  }
  print_board();
}

// parse UCI command "go"
void
parse_go(char* command)
{
  int depth = -1; // init parameters
  char* argument = NULL; // init argument

  if ((argument = strstr(command, "infinite")))                { }                                                     // infinite search
  if ((argument = strstr(command, "binc"))  && side == black)  inc       = atoi(argument +  5);                        // parse black time increment
  if ((argument = strstr(command, "winc"))  && side == white)  inc       = atoi(argument +  5);                        // parse white time increment
  if ((argument = strstr(command, "wtime")) && side == white)  time      = atoi(argument +  6);                        // parse white time limit
  if ((argument = strstr(command, "btime")) && side == black)  time      = atoi(argument +  6);                        // parse black time limit
  if ((argument = strstr(command, "movestogo")))               movestogo = atoi(argument + 10);                        // parse number of moves to go
  if ((argument = strstr(command, "movetime")))                movetime  = atoi(argument +  9);                        // parse amount of time allowed to spend to make a move
  if ((argument = strstr(command, "depth")))                   depth     = atoi(argument +  6);                        // parse search depth

  if (movetime != -1) {                                                                                                // if move time is not available
    time = movetime;                                                                                                   // set time equal to move time
    movestogo = 1;                                                                                                     // set moves to go to 1
  }

  starttime = get_time_ms();                                                                                           // init start time
  depth     = depth;                                                                                                   // init search depth

  if (time != -1) {                                                                                                    // if time control is available
    timeset                = 1;                                                                                        // flag we're playing with time control
    time                  /= movestogo;                                                                                // set up timing
    if (time > 1500) time -= 50;                                                                                       // "illegal" (empty) move bug fix
    stoptime               = starttime + time + inc;                                                                   // init stoptime
  }

  if (depth == -1)                                                                                                     // if depth is not available
    depth = 64;                                                                                                        // set depth to 64 plies (takes ages to complete...)

  printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",                                                             // print debug info
         time,
         starttime,
         stoptime,
         depth,
         timeset);

  search_position(depth);                                                                                              // search position
}

// main UCI loop
void
uci_loop()
{
  setbuf(stdin, NULL);                                                                                                 // reset STDIN & STDOUT buffers
  setbuf(stdout, NULL);
  char input[2000];                                                                                                    // define user / GUI input buffer
  printf("id name BBC\n");                                                                                             // print engine info
  printf("id name Code Monkey King\n");
  printf("uciok\n");

  while (1) {
    memset(input, 0, sizeof(input));                                                                                   // reset user /GUI input
    fflush(stdout);                                                                                                    // make sure output reaches the GUI

    if (!fgets(input, 2000, stdin)) continue;                                                                          // get user / GUI input
    if (input[0] == '\n') continue;                                                                                    // make sure input is available

    if      (strncmp(input, "isready",     7) == 0) { printf("readyok\n"); continue; }
    else if (strncmp(input, "position",    8) == 0) { parse_position(input); clear_hash_table(); }
    else if (strncmp(input, "ucinewgame", 10) == 0) { parse_position("position startpos"); clear_hash_table(); }
    else if (strncmp(input, "go",          2) == 0)  parse_go(input);
    else if (strncmp(input, "quit",        4) == 0)  break; // quit from the chess engine program execution
    else if (strncmp(input, "uci",         3) == 0) { printf("id name BBC\nid name Code Monkey King\nuciok\n"); }
  }
}

// init all variables
void
init_all()
{
  init_leapers_attacks();                                                                                              // init leaper pieces attacks
  init_sliders_attacks(bishop);                                                                                        // init slider pieces attacks
  init_sliders_attacks(rook);
  init_random_keys();                                                                                                  // init random keys for hashing purposes
  clear_hash_table();                                                                                                  // clear hash table
  init_evaluation_masks();                                                                                             // init evaluation masks
}

// Main driver

int
main()
{
  init_all();
  uci_loop();                                                                                                          // connect to GUI
}
