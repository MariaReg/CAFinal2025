#ifndef RISC_V_SIM_H
#define RISC_V_SIM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE (1024 * 1024) // 1MB
#define NUM_REGISTERS 32
#define PC_START 0

typedef struct {
    uint32_t pc;
    int32_t registers[NUM_REGISTERS];
    uint8_t memory[MEMORY_SIZE];
    int running;
} riscv_sim_t;

void sim_init(riscv_sim_t* sim);
int load_program(riscv_sim_t* sim, const char* filename);
void execute_instruction(riscv_sim_t* sim, uint32_t instruction);
void dump_registers(riscv_sim_t* sim);
void run_simulation(riscv_sim_t* sim);

#endif