#ifndef CHESS_INCLUDED
#define CHESS_INCLUDED

#include <stdint.h>

#define U64 unsigned long long
extern const char *start_pos;  


enum { white, black, both };
enum {
    UNICODE     = 1 << 0, // 1
    ASCII       = 1 << 1, // 2
    PRINT_CORDS = 1 << 2, // 4
    PRINT_INFO  = 1 << 3  // 8

};
// Squares 
enum Squares {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

extern const char *squares_to_coordinates[];  

// Pieces 
enum Pieces { P, N, B, R, Q, K, p, n, b, r, q, k };
enum { wk = 1, wq = 2, bk = 4, bq = 8 };

extern int char_pieces[256];  
extern char promoted_pieces[256];  


// Gamestate 
struct Gamestate {
    U64 piece_bb[12];
    U64 occupancies[3];
    int side;
    int castle;
    int enpassant;
    int fifty;
    int moves;
};
typedef struct Gamestate Gamestate;

// Hasher 
struct ZHasher {
    U64 piece_keys[12][64];
    U64 enpassant_keys[64];
    U64 castle_keys[16];
    U64 side_key;
    U64 init_state;
};
typedef struct ZHasher ZHasher;

U64 rnd64(U64 n);
void init_zhasher(ZHasher *ht);
U64 generate_hash(Gamestate *gs, ZHasher *h);
void parse_fen(char *fen, Gamestate *gs);
void print_state(Gamestate *g, ZHasher *z, int flags);
void print_info(Gamestate *g, ZHasher *z);
void print_piece(int piece, int flags);
#endif
