#include "riscv_sim.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }
    
    riscv_sim_t sim;
    sim_init(&sim);
    
    if (load_program(&sim, argv[1]) != 0) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }
    
    run_simulation(&sim, argv[1]);
    return 0;
}


