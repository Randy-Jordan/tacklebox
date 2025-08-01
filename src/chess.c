#include "../include/chess.h"
#include <sys/random.h>
#include <stdio.h>
#include <stdlib.h>

#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
static inline int count_bits(U64 bitboard){
    // bit counter
    int count = 0;

    // consecutively reset least significant 1st bit
    while (bitboard)
    {
        // increment count
        count++;

        // reset least significant 1st bit
        bitboard &= bitboard - 1;
    }

    // return bit count
    return count;
}
static inline int get_ls1b_index(U64 bitboard){
    // make sure bitboard is not 0
    if (bitboard){
        // count trailing bits before LS1B
        return count_bits((bitboard & -bitboard) - 1);
    }
    //otherwise
    else
        // return illegal index
        return -1;
}

const char *start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";

const char *squares_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

static char ascii_pieces[12] = "PNBRQKpnbrqk";
static char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};
int char_pieces[256] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};
U64 rnd64(U64 n){
    const U64 z = 0x9FB21C651E98DF25;

    n ^= ((n << 49) | (n >> 15)) ^ ((n << 24) | (n >> 40));
    n *= z;
    n ^= n >> 35;
    n *= z;
    n ^= n >> 28;

    return n;
}
void init_zhasher(ZHasher *ht){
    // loop over piece codes
    for (int piece = P; piece <= k; piece++){
        // loop over board squares
        for (int square = 0; square < 64; square++){
            ht->piece_keys[piece][square] = rnd64(ht->init_state++);
            // printf("%s:%c:%llu\n",squares_to_coordinates[square], ascii_pieces[piece] ,ht->piece_keys[piece][square]);
        }

        // loop over board squares
        for (int square = 0; square < 64; square++){
            ht->enpassant_keys[square] = rnd64(ht->init_state++);
        }

        // loop over castling keys
        for (int index = 0; index < 16; index++){
            ht->castle_keys[index] = rnd64(ht->init_state++);
        }

        // init random side key
        ht->side_key = rnd64(ht->init_state++);
    }

}
U64 generate_hash(Gamestate *gs, ZHasher *h){
    U64 final_key = 0ULL;
    U64 bitboard;

    // loop over piece bitboards
    for (int piece = P; piece <= k; piece++)
    {
        // init piece bitboard copy
        bitboard = gs->piece_bb[piece];

        // loop over the pieces within a bitboard
        while (bitboard){
            // init square occupied by the piece
            int square = get_ls1b_index(bitboard);

            // hash piece
            final_key ^= h->piece_keys[piece][square];

            // pop LS1B
            pop_bit(bitboard, square);
        }
    }

    // if enpassant square is on board
    if (gs->enpassant != no_sq)
        // hash enpassant
        final_key ^= h->enpassant_keys[gs->enpassant];

    // hash castling rights
    final_key ^= h->castle_keys[gs->castle];

    // hash the side only if black is to move
    if (gs->side == black) final_key ^= h->side_key;

    // return generated hash key
    return final_key;
}
void parse_fen(char *fen, Gamestate *gs){
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++){
        // loop over board files
        for (int file = 0; file < 8; file++){
            // init current square
            int square = rank * 8 + file;

            // match ascii pieces within FEN string
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')){
                // init piece type
                int piece = char_pieces[(unsigned char)*fen];

                // set piece on corresponding bitboard
                set_bit(gs->piece_bb[piece], square);

                // increment pointer to FEN string
                fen++;
            }

            // match empty square numbers within FEN string
            if (*fen >= '0' && *fen <= '9'){
                // init offset (convert char 0 to int 0)
                int offset = *fen - '0';

                // define piece variable
                int piece = -1;

                // loop over all piece bitboards
                for (int bb_piece = P; bb_piece <= k; bb_piece++){
                    // if there is a piece on current square
                    if (get_bit(gs->piece_bb[bb_piece], square))
                        // get piece code
                        piece = bb_piece;
                }

                // on empty current square
                if (piece == -1)
                    // decrement file
                    file--;

                // adjust file counter
                file += offset;

                // increment pointer to FEN string
                fen++;
            }

            // match rank separator
            if (*fen == '/')
                // increment pointer to FEN string
                fen++;
        }
    }

    // got to parsing side to move (increment pointer to FEN string)
    fen++;

    // parse side to move
    (*fen == 'w') ? (gs->side = white) : (gs->side = black);

    // go to parsing castling rights
    fen += 2;

    // parse castling rights
    while (*fen != ' '){
        switch (*fen){
            case 'K': gs->castle |= wk; break;
            case 'Q': gs->castle |= wq; break;
            case 'k': gs->castle |= bk; break;
            case 'q': gs->castle |= bq; break;
            case '-': break;
        }

        // increment pointer to FEN string
        fen++;
    }

    // go to parsing enpassant square (increment pointer to FEN string)
    fen++;

    // parse enpassant square
    if (*fen != '-'){
        // parse enpassant file & rank
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');

        // init enpassant square
        gs->enpassant = rank * 8 + file;
    }

    // no enpassant square
    else
        gs->enpassant = no_sq;

    // go to parsing half move counter (increment pointer to FEN string)
    fen++;

    // parse half move counter to init fifty move counter
    gs->fifty = atoi(fen);
    fen+=2;
    gs->moves = atoi(fen);

    // loop over white pieces bitboards
    for (int piece = P; piece <= K; piece++)
        // populate white occupancy bitboard
        gs->occupancies[white] |= gs->piece_bb[piece];

    // loop over black pieces bitboards
    for (int piece = p; piece <= k; piece++)
        // populate white occupancy bitboard
        gs->occupancies[black] |= gs->piece_bb[piece];

    // init all occupancies
    gs->occupancies[both] |= gs->occupancies[white];
    gs->occupancies[both] |= gs->occupancies[black];


} 
void print_state(Gamestate *g, ZHasher *z, int flags){
    // print offset
    printf("\n");
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++){
        // loop over board files
        for (int file = 0; file < 8; file++){
            // init square
            int square = rank * 8 + file;

            // print ranks
            if (!file){
                if(flags & PRINT_CORDS){
                    printf("  %d ", 8 - rank);
                }
            }
            // define piece variable
            int piece = -1;

            // loop over all piece bitboards
            for (int bb_piece = P; bb_piece <= k; bb_piece++){
                // if there is a piece on current square
                if (get_bit(g->piece_bb[bb_piece], square))
                    // get piece code
                    piece = bb_piece;
            }

            // print different piece set depending on OS
            print_piece(piece, flags);           
        }

        // print new line every rank
        printf("\n");
    }
    // print board files
    if(flags & PRINT_CORDS){
        printf("\n     a b c d e f g h\n\n");
    }
    if(flags & PRINT_INFO){
        print_info(g,z);
    }
}
void print_info(Gamestate *g, ZHasher *z){
        // print side to move
        printf("     Side:     %s\n", !g->side ? "white" : "black");

        // print enpassant square
        printf("     Enpassant:   %s\n", (g->enpassant != no_sq) ? squares_to_coordinates[g->enpassant] : "no");

        // print castling rights
        printf("     Castling:  %c%c%c%c\n", (g->castle & wk) ? 'K' : '-',
                (g->castle & wq) ? 'Q' : '-',
                (g->castle & bk) ? 'k' : '-',
                (g->castle & bq) ? 'q' : '-');

        // fifty move rule counter
        printf("     Fifty move: %d\n", g->fifty);

        // fifty move rule counter
        printf("     Full moves: %d\n", g->moves);

        // print hash key
        printf("     Hash key:  %llu\n", generate_hash(g,z));

}
void print_piece(int piece, int flags){
    if(flags & ASCII){

        printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
    }            
    if(flags & UNICODE){
        printf(" %s", (piece == -1) ? "." : unicode_pieces[piece]);
    }
    // Fallback for unsupplied options
    printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
}
