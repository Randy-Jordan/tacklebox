#include "../include/chess.h"
#include <stdlib.h>

int main(void){
    U64 state = rnd64(2);
    ZHasher z;
    z.init_state = state;
    init_zhasher(&z);
    Gamestate g;
    parse_fen((char *)start_pos,&g);
    print_state(&g,&z,0);
    return EXIT_SUCCESS;
}
