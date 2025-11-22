#include "riscv_sim.h"

void sim_init(riscv_sim_t* sim) {
    memset(sim, 0, sizeof(riscv_sim_t));
    sim->pc = PC_START;
    sim->running = 1;
    sim->registers[2] = MEMORY_SIZE; // sp = 1MB
}

int load_program(riscv_sim_t* sim, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return -1;
    
    fread(sim->memory, 1, MEMORY_SIZE, file);
    fclose(file);
    return 0;
}

int32_t sign_extend(uint32_t value, int bits) {
    int32_t sign_bit = 1 << (bits - 1);
    if (value & sign_bit) 
        return value | (~0U << bits);
    return value;
}

uint32_t fetch_instruction(riscv_sim_t* sim) {
    if (sim->pc >= MEMORY_SIZE - 3) {
        sim->running = 0;
        return 0;
    }
    
    return *(uint32_t*)(sim->memory + sim->pc);
}

void execute_instruction(riscv_sim_t* sim, uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    int32_t rs1_val = sim->registers[rs1];
    int32_t rs2_val = sim->registers[rs2];
    int32_t imm, result, branch_taken;
    uint32_t address;

    switch (opcode) {
        case 0x33: // R-type
            switch (funct3) {
                case 0x0: result = (funct7 == 0x20) ? rs1_val - rs2_val : rs1_val + rs2_val; break;
                case 0x1: result = rs1_val << (rs2_val & 0x1F); break;
                case 0x2: result = (rs1_val < rs2_val) ? 1 : 0; break;
                case 0x3: result = ((uint32_t)rs1_val < (uint32_t)rs2_val) ? 1 : 0; break;
                case 0x4: result = rs1_val ^ rs2_val; break;
                case 0x5: result = (funct7 == 0x20) ? rs1_val >> (rs2_val & 0x1F) : (uint32_t)rs1_val >> (rs2_val & 0x1F); break;
                case 0x6: result = rs1_val | rs2_val; break;
                case 0x7: result = rs1_val & rs2_val; break;
                default: return;
            }
            if (rd != 0) sim->registers[rd] = result;
            sim->pc += 4;
            break;

        case 0x13: // I-type
            imm = sign_extend((instruction >> 20) & 0xFFF, 12);
            switch (funct3) {
                case 0x0: result = rs1_val + imm; break;
                case 0x1: result = rs1_val << (imm & 0x1F); break;
                case 0x2: result = (rs1_val < imm) ? 1 : 0; break;
                case 0x3: result = ((uint32_t)rs1_val < (uint32_t)imm) ? 1 : 0; break;
                case 0x4: result = rs1_val ^ imm; break;
                case 0x5: result = ((imm >> 5) == 0x20) ? rs1_val >> (imm & 0x1F) : (uint32_t)rs1_val >> (imm & 0x1F); break;
                case 0x6: result = rs1_val | imm; break;
                case 0x7: result = rs1_val & imm; break;
                default: return;
            }
            if (rd != 0) sim->registers[rd] = result;
            sim->pc += 4;
            break;

        case 0x03: // Load
            imm = sign_extend((instruction >> 20) & 0xFFF, 12);
            address = rs1_val + imm;
            if (address >= MEMORY_SIZE) {
                sim->running = 0;
                return;
            }
            switch (funct3) {
                case 0x0: result = sign_extend(sim->memory[address], 8); break;
                case 0x1: result = sign_extend(*(uint16_t*)(sim->memory + address), 16); break;
                case 0x2: result = *(uint32_t*)(sim->memory + address); break;
                case 0x4: result = sim->memory[address]; break;
                case 0x5: result = *(uint16_t*)(sim->memory + address); break;
                default: return;
            }
            if (rd != 0) sim->registers[rd] = result;
            sim->pc += 4;
            break;

        case 0x23: // Store
            imm = sign_extend(((instruction >> 25) & 0x7F) << 5 | ((instruction >> 7) & 0x1F), 12);
            address = rs1_val + imm;
            if (address >= MEMORY_SIZE) {
                sim->running = 0;
                return;
            }
            switch (funct3) {
                case 0x0: sim->memory[address] = rs2_val & 0xFF; break;
                case 0x1: *(uint16_t*)(sim->memory + address) = rs2_val & 0xFFFF; break;
                case 0x2: *(uint32_t*)(sim->memory + address) = rs2_val; break;
                default: return;
            }
            sim->pc += 4;
            break;

        case 0x63: // Branch
            imm = sign_extend(
                ((instruction >> 31) & 0x1) << 12 |
                ((instruction >> 25) & 0x3F) << 5 |
                ((instruction >> 8) & 0xF) << 1 |
                ((instruction >> 7) & 0x1) << 11, 13);
            
            switch (funct3) {
                case 0x0: branch_taken = (rs1_val == rs2_val); break;
                case 0x1: branch_taken = (rs1_val != rs2_val); break;
                case 0x4: branch_taken = (rs1_val < rs2_val); break;
                case 0x5: branch_taken = (rs1_val >= rs2_val); break;
                case 0x6: branch_taken = ((uint32_t)rs1_val < (uint32_t)rs2_val); break;
                case 0x7: branch_taken = ((uint32_t)rs1_val >= (uint32_t)rs2_val); break;
                default: return;
            }
            sim->pc = branch_taken ? sim->pc + imm : sim->pc + 4;
            break;

        case 0x6F: // JAL
            imm = sign_extend(
                ((instruction >> 31) & 0x1) << 20 |
                ((instruction >> 21) & 0x3FF) << 1 |
                ((instruction >> 20) & 0x1) << 11 |
                ((instruction >> 12) & 0xFF) << 12, 21);
            
            if (rd != 0) sim->registers[rd] = sim->pc + 4;
            sim->pc += imm;
            break;

        case 0x67: // JALR
            imm = sign_extend((instruction >> 20) & 0xFFF, 12);
            if (rd != 0) sim->registers[rd] = sim->pc + 4;
            sim->pc = (rs1_val + imm) & ~1;
            break;

        case 0x37: // LUI
            if (rd != 0) sim->registers[rd] = instruction & 0xFFFFF000;
            sim->pc += 4;
            break;

        case 0x17: // AUIPC
            if (rd != 0) sim->registers[rd] = sim->pc + (instruction & 0xFFFFF000);
            sim->pc += 4;
            break;

        case 0x73: // ECALL
            if (funct3 == 0x0 && sim->registers[17] == 10) {
                sim->running = 0;
            }
            sim->pc += 4;
            break;

        default:
            printf("Unknown instruction: 0x%08X\n", instruction);
            sim->running = 0;
            break;
    }
}

void dump_registers(riscv_sim_t* sim) {
    printf("Register Dump:\n");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("x%-2d: 0x%08X (%d)\n", i, sim->registers[i], sim->registers[i]);
    }
    
    printf("\nBinary Register Dump:\n");
    unsigned char regData[MEMORY_SIZE];
    for (int i = 0; i < NUM_REGISTERS; i++) {
        fwrite(&sim->registers[i], sizeof(int32_t), 1, stdout);
        regData[i] = sim->registers[i];
    }
    printf("\n");

    printf("\nCreating binary dump file");
    printf("\n %d", regData[2]);
}

void run_simulation(riscv_sim_t* sim) {
    while (sim->running) {
        uint32_t instruction = fetch_instruction(sim);
        if (!sim->running) break;
        execute_instruction(sim, instruction);
    }
    dump_registers(sim);
}