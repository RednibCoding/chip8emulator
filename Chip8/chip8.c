#include "chip8.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

void chip8_init(chip8_t* chip) {
    // Initialize memory to 0
    memset(chip->mem, 0, sizeof(chip->mem));

    // Initialize registers to 0
    memset(chip->V, 0, sizeof(chip->V));

    // Initialize program counter to 0x200
    chip->pc = CHIP8_PROGRAM_OFFSET;

    // Initialize stack pointer to 0
    chip->sp = 0;

    // Initialize keypad to 0
    memset(chip->keypad, 0, sizeof(chip->keypad));

    // Initialize display to 0
    memset(chip->display, 0, sizeof(chip->display));
}

bool chip8_load_rom(chip8_t* chip, const char* filename) {
    // Open the file using fopen_s if compiled on Windows
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__)
    FILE* fp;
    errno_t err = fopen_s(&fp, filename, "rb");
    if (err != 0) {
        fprintf(stderr, "Failed to open rom file: %s\n", filename);
        return false;
    }
#else
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open rom file: %s\n", filename);
        return false;
    }
#endif
    
    if (!fp) {
        fprintf(stderr, "Failed to open rom file: %s\n", filename);
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    long rom_size = ftell(fp);
    rewind(fp);

    if (rom_size > CHIP8_MEM_SIZE - CHIP8_PROGRAM_OFFSET) {
        fprintf(stderr, "rom file too large: %s\n", filename);
        fclose(fp);
        return false;
    }

    uint8_t* rom_buffer = &chip->mem[CHIP8_PROGRAM_OFFSET];
    fread(rom_buffer, rom_size, 1, fp);
    fclose(fp);

    return true;
}

void chip8_set_key(chip8_t* chip, chip8_key key, uint8_t state) {
    if (key >= CHIP8_KEY_0 && key <= CHIP8_KEY_F) {
        chip->keypad[key] = state;
    }
}


void chip8_step(chip8_t* chip) {
    // Fetch the current instruction from memory
    uint16_t opcode = (chip->mem[chip->pc] << 8) | chip->mem[chip->pc + 1];

#ifdef DEBUG
    // Print the fetched opcode in debug mode
    fprintf(stderr, "Fetched opcode: 0x%X\n", opcode);
#endif

    // Decode and execute the instruction
    switch (opcode & 0xF000) {
    case 0x0000:
        switch (opcode & 0x00FF) {
        case 0x00E0:
            chip8_CLS(chip);
            break;
        case 0x00EE:
            chip8_RET(chip);
            break;
        default:
            fprintf(stderr, "Unknown instruction: 0x%X\n", opcode);
            break;
        }
        break;
    case 0x1000:
        chip8_JP_addr(chip, opcode & 0x0FFF);
        break;
    case 0x2000:
        chip8_CALL_addr(chip, opcode & 0x0FFF);
        break;
    case 0x3000:
        chip8_SE_Vx_byte(chip, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
        break;
    case 0x4000:
        chip8_SNE_Vx_byte(chip, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
        break;
    case 0x5000:
        chip8_SE_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
        break;
    case 0x6000:
        chip8_LD_Vx_byte(chip, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
        break;
    case 0x7000:
        chip8_ADD_Vx_byte(chip, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
        break;
    case 0x8000:
        switch (opcode & 0x000F) {
        case 0x0000:
            chip8_LD_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
            break;
        case 0x0001:
            chip8_OR_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
            break;
        case 0x0002:
            chip8_AND_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
            break;
        case 0x0003:
            chip8_XOR_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
            break;
        case 0x0004:
            chip8_ADD_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
            break;
        case 0x0005:
            chip8_SUB_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
            break;
        case 0x0006:
            chip8_SHR_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x0007:
            chip8_SUBN_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
            break;
        case 0x000E:
            chip8_SHL_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        default:
            fprintf(stderr, "Unknown instruction: 0x%X\n", opcode);
            break;
        }
        break;
    case 0x9000:
        chip8_SNE_Vx_Vy(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
        break;
    case 0xA000:
        chip8_LD_I_addr(chip, opcode & 0x0FFF);
        break;
    case 0xB000:
        chip8_JP_V0_addr(chip, opcode & 0x0FFF);
        break;
    case 0xC000:
        chip8_RND_Vx_byte(chip, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
        break;
    case 0xD000:
        chip8_DRW_Vx_Vy_nibble(chip, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4, opcode & 0x000F);
        break;
    case 0xE000:
        switch (opcode & 0x00FF) {
        case 0x009E:
            chip8_SKP_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x00A1:
            chip8_SKNP_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        default:
            fprintf(stderr, "Unknown instruction: 0x%X\n", opcode);
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF) {
        case 0x0007:
            chip8_LD_Vx_DT(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x000A:
            chip8_LD_Vx_K(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x0015:
            chip8_LD_DT_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x0018:
            chip8_LD_ST_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x001E:
            chip8_ADD_I_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x0029:
            chip8_LD_F_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x0033:
            chip8_LD_B_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x0055:
            chip8_LD_I_Vx(chip, (opcode & 0x0F00) >> 8);
            break;
        case 0x0065:
            chip8_LD_Vx_I(chip, (opcode & 0x0F00) >> 8);
            break;
        default:
            fprintf(stderr, "Unknown instruction: 0x%X\n", opcode);
            break;
        }
        break;
    default:
        fprintf(stderr, "Unknown instruction: 0x%X\n", opcode);
        break;
    }

    // Decrement the delay timer if it's set
    if (chip->delay_timer > 0) {
        chip->delay_timer--;
    }

    // Decrement the sound timer if it's set
    if (chip->sound_timer > 0) {
        chip->sound_timer--;

        // Play a sound if the sound timer just reached zero
        if (chip->sound_timer == 0) {
            printf("BEEP!\n");
        }
    }
}


// ############################################################################################################
// Opcode handlers
// ############################################################################################################

// 00E0 - CLS: Clear the display.
void chip8_CLS(chip8_t* chip) {
    // Clear the display buffer by setting all pixels to 0
    memset(chip->display, 0, sizeof(chip->display));
    chip->pc += 2;
}

// 00EE - RET: Return from a subroutine.
void chip8_RET(chip8_t* chip) {
    // Return from a subroutine by setting the program counter to the address at the top of the stack
    chip->pc = chip->stack[--chip->sp];
    chip->pc += 2;
}

// 1nnn - JP addr: Jump to address `nnn`.
void chip8_JP_addr(chip8_t* chip, uint16_t addr) {
    // Jump to the address specified by the instruction by setting the program counter to that address
    chip->pc = addr;
}

// 2nnn - CALL addr: Call subroutine at address `nnn`.
void chip8_CALL_addr(chip8_t* chip, uint16_t addr) {
    // Push the current program counter onto the stack and increment the stack pointer
    chip->stack[chip->sp++] = chip->pc;
    // Set the program counter to the address specified by the instruction
    chip->pc = addr;
}

// 3xkk - SE Vx, byte: Skip next instruction if Vx == `kk`.
void chip8_SE_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk) {
    if (chip->V[x] == kk) {
        // Skip the next instruction by incrementing the program counter by 4
        chip->pc += 4;
    }
    else {
        // Simply increment the program counter by 2
        chip->pc += 2;
    }
}

// 4xkk - SNE Vx, byte: Skip next instruction if Vx != `kk`.
void chip8_SNE_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk) {
    if (chip->V[x] != kk) {
        // Skip the next instruction by incrementing the program counter by 4
        chip->pc += 4;
    }
    else {
        // Simply increment the program counter by 2
        chip->pc += 2;
    }
}

// 5xy0 - SE Vx, Vy: Skip next instruction if Vx == Vy.
void chip8_SE_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    if (chip->V[x] == chip->V[y]) {
        // Skip the next instruction by incrementing the program counter by 4
        chip->pc += 4;
    }
    else {
        // Simply increment the program counter by 2
        chip->pc += 2;
    }
}

// 6xkk - LD Vx, byte: Load `kk` into Vx.
void chip8_LD_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk) {
    chip->V[x] = kk;
    chip->pc += 2;
}

// 7xkk - ADD Vx, byte: Add `kk` to Vx.
void chip8_ADD_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk) {
    chip->V[x] += kk;
    chip->pc += 2;
}

// 8xy0 - LD Vx, Vy: Set Vx = Vy.
void chip8_LD_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    chip->V[x] = chip->V[y];
    chip->pc += 2;
}

// 8xy1 - OR Vx, Vy: Set Vx = Vx OR Vy.
void chip8_OR_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    chip->V[x] |= chip->V[y];
    chip->pc += 2;
}

// 8xy2 - AND Vx, Vy: Set Vx = Vx AND Vy.
void chip8_AND_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    chip->V[x] &= chip->V[y];
    chip->pc += 2;
}

// 8xy3 - XOR Vx, Vy: Set Vx = Vx XOR Vy.
void chip8_XOR_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    chip->V[x] ^= chip->V[y];
    chip->pc += 2;
}

// 8xy4 - ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry.
void chip8_ADD_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    uint16_t sum = chip->V[x] + chip->V[y];
    chip->V[0xF] = (sum > 0xFF) ? 1 : 0;
    chip->V[x] = (uint8_t)sum;
    chip->pc += 2;
}

// 8xy5 - SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow.
void chip8_SUB_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    if (chip->V[x] >= chip->V[y]) {
        // If Vx >= Vy, set VF to 1
        chip->V[0xF] = 1;
    }
    else {
        // Otherwise, set VF to 0
        chip->V[0xF] = 0;
    }

    // Perform the subtraction
    chip->V[x] -= chip->V[y];

    chip->pc += 2;
}

// 8xy6 - SHR Vx {, Vy}: Set Vx = Vx SHR 1, set VF = LSB of Vx before shift.
void chip8_SHR_Vx(chip8_t* chip, uint8_t x) {
    // Set the carry flag (VF) to the least significant bit of the value in register Vx
    chip->V[0xF] = chip->V[x] & 0x1;

    // Shift the value in register Vx to the right by 1
    chip->V[x] >>= 1;
    chip->pc += 2;
}

// 8xy7 - SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow.
void chip8_SUBN_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    if (chip->V[y] > chip->V[x]) {
        // There is a borrow, so VF should be set to 0
        chip->V[0xF] = 0;
    }
    else {
        chip->V[0xF] = 1;
    }
    chip->V[x] = chip->V[y] - chip->V[x];
    chip->pc += 2;
}

// 8xyE - SHL Vx {, Vy}: Set Vx = Vx SHL 1, set VF = MSB of Vx before shift.
void chip8_SHL_Vx(chip8_t* chip, uint8_t x) {
    // Set the carry flag to the MSB of Vx
    chip->V[0xF] = chip->V[x] >> 7;

    // Shift Vx left by 1 bit
    chip->V[x] <<= 1;

    chip->pc += 2;
}

// 9xy0 - SNE Vx, Vy: Skip next instruction if Vx != Vy.
void chip8_SNE_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y) {
    if (chip->V[x] != chip->V[y]) {
        // Skip the next instruction by incrementing the program counter by 4
        chip->pc += 4;
    }
    else {
        // Simply increment the program counter by 2
        chip->pc += 2;
    }
}

// Annn - LD I, addr: Set I = `nnn`.
void chip8_LD_I_addr(chip8_t* chip, uint16_t addr) {
    // Set the index register (I) to the value specified by the instruction
    chip->I = addr;
    chip->pc += 2;
}

// Bnnn - JP V0, addr: Jump to address `V0 + nnn`.
void chip8_JP_V0_addr(chip8_t* chip, uint16_t addr) {
    // Jump to the address specified by V0 + nnn
    chip->pc = chip->V[0] + addr;
}

// Cxkk - RND Vx, byte: Set Vx = random byte AND kk.
void chip8_RND_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk) {
    // Generate a random byte using the rand() function from the C standard library
    uint8_t rand_byte = rand() & 0xFF;

    // Set register Vx to the result of the bitwise AND operation between the random byte and the byte kk
    chip->V[x] = rand_byte & kk;

    chip->pc += 2;
}

// Dxyn - DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
void chip8_DRW_Vx_Vy_nibble(chip8_t* chip, uint8_t x, uint8_t y, uint8_t height) {
    uint8_t pixel;
    uint16_t i, j;
    uint16_t index = chip->I;

    // Set VF to 0 (collision not detected)
    chip->V[0xF] = 0;

    // Loop through each row of the sprite
    for (i = 0; i < height; i++) {
        // Get the pixel value from memory
        pixel = chip->mem[index + i];

        // Loop through each column of the sprite
        for (j = 0; j < 8; j++) {
            // Check if the current pixel is on
            if ((pixel & (0x80 >> j)) != 0) {
                // Calculate the screen buffer index for the current pixel
                uint16_t buffer_index = (chip->V[x] + j) + ((chip->V[y] + i) * CHIP8_DISPLAY_WIDTH);

                // Check if the pixel is already set
                if (chip->display[buffer_index] == 1) {
                    // Set VF to 1 (collision detected)
                    chip->V[0xF] = 1;
                }

                // XOR the pixel onto the screen buffer
                chip->display[buffer_index] ^= 1;
            }
        }
    }
    chip->pc += 2;
}

// Ex9E - SKP Vx: Skip next instruction if key with the value of Vx is pressed.
void chip8_SKP_Vx(chip8_t* chip, uint8_t x) {
    if (chip->keypad[chip->V[x]]) {
        // Skip the next instruction by incrementing the program counter by 4
        chip->pc += 4;
    }
    else {
        // Simply increment the program counter by 2
        chip->pc += 2;
    }
}

// ExA1 - SKNP Vx: Skip next instruction if key with the value of Vx is not pressed.
void chip8_SKNP_Vx(chip8_t* chip, uint8_t x) {
    // If the key with the value of Vx is not pressed, skip the next instruction
    if (!chip->keypad[chip->V[x]]) {
        // Skip the next instruction by incrementing the program counter by 4
        chip->pc += 4;
    }
    else {
        // Simply increment the program counter by 2
        chip->pc += 2;
    }
}

// Fx07 - LD Vx, DT: Set Vx = delay timer value.
void chip8_LD_Vx_DT(chip8_t* chip, uint8_t x) {
    // Set the value of register Vx to the value of the delay timer
    chip->V[x] = chip->delay_timer;
    chip->pc += 2;
}

// Fx0A - LD Vx, K: Wait for a key press, store the value of the key in Vx.
void chip8_LD_Vx_K(chip8_t* chip, uint8_t x) {
    // Check if any key is pressed
    int key_pressed = 0;
    for (int i = 0; i < 16; i++) {
        if (chip->keypad[i]) {
            // A key is pressed, store its value in register Vx
            chip->V[x] = i;
            key_pressed = 1;
            break;
        }
    }

    // If no key is pressed, do not increment the program counter
    if (!key_pressed) {
        return;
    }

    // Otherwise, increment the program counter by 2 to go to the next instruction
    chip->pc += 2;
}

// Fx15 - LD DT, Vx: Set delay timer = Vx.
void chip8_LD_DT_Vx(chip8_t* chip, uint8_t x) {
    chip->delay_timer = chip->V[x];
    chip->pc += 2;
}

// Fx18 - LD ST, Vx: Set sound timer = Vx.
void chip8_LD_ST_Vx(chip8_t* chip, uint8_t x) {
    chip->sound_timer = chip->V[x];
    chip->pc += 2;
}

// Fx1E - ADD I, Vx: Set I = I + Vx.
void chip8_ADD_I_Vx(chip8_t* chip, uint8_t x) {
    chip->I += chip->V[x];
    chip->pc += 2;
}

// Fx29 - LD F, Vx: Set I = location of sprite for digit Vx.
void chip8_LD_F_Vx(chip8_t* chip, uint8_t x) {
    // The address of the sprite for digit Vx is calculated by multiplying the value of Vx by 5 (each sprite is 5 bytes long),
    // and adding the address of the font set to the result.
    chip->I = chip->V[x] * 5;
    chip->pc += 2;
}

// Fx33 - LD B, Vx: Store BCD representation of Vx in memory locations I, I+1, and I+2.
void chip8_LD_B_Vx(chip8_t* chip, uint8_t x) {
    uint8_t value = chip->V[x];
    chip->mem[chip->I] = value / 100;        // Hundreds digit
    chip->mem[chip->I + 1] = (value / 10) % 10;  // Tens digit
    chip->mem[chip->I + 2] = value % 10;         // Ones digit
    chip->pc += 2;
}

// Fx55 - LD [I], Vx: Store registers V0 through Vx in memory starting at location I.
void chip8_LD_I_Vx(chip8_t* chip, uint8_t x) {
    for (uint8_t i = 0; i <= x; i++) {
        chip->mem[chip->I + i] = chip->V[i];
    }
    chip->I += x + 1;
    chip->pc += 2;
}

// Fx65 - LD Vx, [I]
void chip8_LD_Vx_I(chip8_t* chip, uint8_t x) {
    for (uint8_t i = 0; i <= x; i++) {
        chip->V[i] = chip->mem[chip->I + i];
    }
    chip->I += x + 1;
    chip->pc += 2;
}
