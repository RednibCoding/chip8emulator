#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

/*
* List of chip8 instructions:

* 00E0 - CLS: Clear the display.
*   To implement this instruction, simply set all bytes in the `display` buffer to 0.
*
* 00EE - RET: Return from a subroutine.
*   To implement this instruction, pop the address of the next instruction from the top of the stack and store it in the program counter.
*
* 1nnn - JP addr: Jump to address `nnn`.
*   To implement this instruction, set the program counter to the address `nnn`.
*
* 2nnn - CALL addr: Call subroutine at address `nnn`.
*   To implement this instruction, push the current value of the program counter onto the stack and then set the program counter to the address `nnn`.
*
* 3xkk - SE Vx, byte: Skip next instruction if Vx == `kk`.
*   To implement this instruction, check if the value in register Vx is equal to the byte `kk`, and if so, increment the program counter by 2.
*
* 4xkk - SNE Vx, byte: Skip next instruction if Vx != `kk`.
*   To implement this instruction, check if the value in register Vx is not equal to the byte `kk`, and if so, increment the program counter by 2.
*
* 5xy0 - SE Vx, Vy: Skip next instruction if Vx == Vy.
*   To implement this instruction, check if the value in register Vx is equal to the value in register Vy, and if so, increment the program counter by 2.
*
* 6xkk - LD Vx, byte: Load `kk` into Vx.
*   To implement this instruction, simply set the value of register Vx to the byte `kk`.
*
* 7xkk - ADD Vx, byte: Add `kk` to Vx.
*   To implement this instruction, add the byte `kk` to the value in register Vx and store the result in Vx.
*
* 8xy0 - LD Vx, Vy: Set Vx = Vy.
*   To implement this instruction, set the value of register Vx to the value of register Vy.
*
* 8xy1 - OR Vx, Vy: Set Vx = Vx OR Vy.
*   To implement this instruction, perform a bitwise OR operation between the values in registers Vx and Vy, and store the result in Vx.
*
* 8xy2 - AND Vx, Vy: Set Vx = Vx AND Vy.
*   To implement this instruction, perform a bitwise AND operation between the values in registers Vx and Vy, and store the result in Vx.
*
* 8xy3 - XOR Vx, Vy: Set Vx = Vx XOR Vy.
*   To implement this instruction, perform a bitwise XOR operation between the values in registers Vx and Vy, and store the result in Vx.
*
* 8xy4 - ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry.
*   To implement this instruction, add the values in registers Vx and Vy, store the result in Vx, and set VF to 1 if there is a carry, or 0 otherwise.
*
* 8xy5 - SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow.
*   To implement this instruction, subtract the value in register Vy from the value in register Vx, store the result in Vx, and set VF to 1 if there is no borrow, or 0 otherwise.
*
* 8xy6 - SHR Vx {, Vy}: Set Vx = Vx SHR 1, set VF = LSB of Vx before shift.
*   To implement this instruction, shift the value in register Vx right by 1 bit, store the result in Vx, and set VF to the least significant bit of Vx before the shift. If the second operand (Vy) is present, the shift is done with the value in register Vy instead.
*
* 8xy7 - SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow.
*   To implement this instruction, subtract the value in register Vx from the value in register Vy, store the result in Vx, and set VF to 1 if there is no borrow, or 0 otherwise.
*
* 8xyE - SHL Vx {, Vy}: Set Vx = Vx SHL 1, set VF = MSB of Vx before shift.
*   To implement this instruction, shift the value in register Vx left by 1 bit, store the result in Vx, and set VF to the most significant bit of Vx before the shift. If the second operand (Vy) is present, the shift is done with the value in register Vy instead.
*
* 9xy0 - SNE Vx, Vy: Skip next instruction if Vx != Vy.
*   To implement this instruction, check if the value in register Vx is not equal to the value in register Vy, and if so, increment the program counter by 2.
*
* Annn - LD I, addr: Set I = `nnn`.
*   To implement this instruction, set the value of the index register (I) to the address `nnn`.
*
* Bnnn - JP V0, addr: Jump to address `V0 + nnn`.
*   To implement this instruction, set the program counter to the address `V0 + nnn`.
*
* Cxkk - RND Vx, byte: Set Vx = random byte AND kk.
*   To implement this instruction, generate a random byte and AND it with the byte `kk`, and store the result in Vx.
*
* Dxyn - DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
*   To implement this instruction, read `n` bytes of sprite data from memory starting at address I, where `n` is the value of the last nibble of the instruction. Then, you XOR each byte of the sprite data with the corresponding byte in the display buffer starting at position (Vx, Vy). If any of the pixels in the display buffer are turned off as a result of the XOR operation, you set VF to 1 to indicate a collision.
*
* Ex9E - SKP Vx: Skip next instruction if key with the value of Vx is pressed.
*   To implement this instruction, check if the value in register Vx corresponds to a pressed key, and if so, increment the program counter by 2.
* 
* ExA1 - SKNP Vx: Skip next instruction if key with the value of Vx is not pressed.
* T o implement this instruction, check if the value in register Vx corresponds to a released key, and if so, increment the program counter by 2.
*
* Fx07 - LD Vx, DT: Set Vx = delay timer value.
*   To implement this instruction, set the value of register Vx to the value of the delay timer.
*
* Fx0A - LD Vx, K: Wait for a key press, store the value of the key in Vx.
*   To implement this instruction, halt execution of the interpreter until a key is pressed, and then store the value of the key in register Vx.
*
* Fx15 - LD DT, Vx: Set delay timer = Vx.
*   To implement this instruction, set the value of the delay timer to the value in register Vx.
*
* Fx18 - LD ST, Vx: Set sound timer = Vx.
*   To implement this instruction, set the value of the sound timer to the value in register Vx.
*
* Fx1E - ADD I, Vx: Set I = I + Vx.
*   To implement this instruction, add the value in register Vx to the index register (I), and store the result in I.
*
* Fx29 - LD F, Vx: Set I = location of sprite for digit Vx.
*   To implement this instruction, set the index register (I) to the memory address of the sprite data for the digit stored in register Vx. Since each digit is represented by a 5-byte sprite, just calculate the address of the sprite data by multiplying the value in Vx by 5.
*
* Fx33 - LD B, Vx: Store BCD representation of Vx in memory locations I, I+1, and I+2.
*   To implement this instruction, convert the value in register Vx to binary-coded decimal (BCD) format, and store each digit of the BCD representation in the next three memory locations starting at address I.
*
* Fx55 - LD [I], Vx: Store registers V0 through Vx in memory starting at location I.
*   To implement this instruction, copy the values of registers V0 through Vx into memory starting at the address stored in the index register (I), and then increment the index register by `x + 1`.
*
* Fx65 - LD Vx, [I]: Read registers V0 through Vx from memory starting at location I.
*    To implement this instruction, copy the values of memory starting at the address stored in the index register (I) into registers V0 through Vx, and then increment the index register by `x + 1`.
*/

/*
* The Chip-8 is an interpreted programming language that was created in the 1970s for the COSMAC VIP and Telmac 1800 microcomputers.
* It is also used in modern emulation of vintage video game systems.
* The Chip-8 architecture includes 16 general-purpose registers, a program counter, an index register, and a stack for subroutine calls.
* It has a 4K memory space, which includes both RAM and ROM.
* The Chip-8 also has a set of 16 opcodes that can be used to perform various operations such as arithmetic, logical, and I/O operations.
* Additionally, it has a display consisting of a 64x32 pixel monochrome graphics array, and it supports input from a 16-key keypad.
*
* A brief explanation of the fields:
*
* - mem:            Memory space for the Chip-8 emulator, which consists of 4K bytes of addressable space.
*                   Programs and data are loaded into this memory space, and the interpreter uses it to store variables and data.
*
* - V:              The Chip-8 has 16 general-purpose registers named V0 to VF, each of which is 8 bits wide.
*                   These registers are used for arithmetic and logical operations, as well as for storing temporary values during program execution.
*
* - I:              The index register is a 16-bit register used to store memory addresses.
*                   It is used to point to memory locations where data or instructions are stored.
*
* - pc:             The program counter is a 16-bit register that holds the address of the current instruction being executed.
*                   When an instruction is executed, the program counter is incremented to point to the next instruction.
*
* - delay_timer:    The delay timer is an 8-bit register that counts down at a rate of 60 Hz when its value is nonzero.
*                   It is used to time events in the program, such as delays between sound effects or animations.
*
* - sound_timer:    The sound timer is similar to the delay timer, but it generates an audible tone when its value is nonzero.
*                   It is used to produce simple sound effects in Chip-8 programs.
*
* - stack:          The Chip-8 has a call stack that is used to store the return address when a subroutine is called.
*                   The stack has a fixed size of 16 levels, each of which is a 16-bit value.
*
* - sp:             The stack pointer is a 16-bit register that points to the top of the stack.
*                   When a subroutine is called, the current value of the program counter is pushed onto the stack, and the stack pointer is incremented.
*                   When the subroutine returns, the value is popped off the stack and used as the new program counter.
*
* - display:        The display field is a 1D array of bytes representing the display. Each byte corresponds to one pixel, and its value is either 0 (off) or 1 (on).
*/

#define CHIP8_MEM_SIZE 4096
#define CHIP8_NUM_REGS 16
#define CHIP8_STACK_SIZE 16
#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_DISPLAY_HEIGHT 32
// Chip 8 programs are loaded at memory address 0x200
#define CHIP8_PROGRAM_OFFSET 0x200
// This value can be adjusted based on the desired speed of the emulator. A higher value will result in slower emulation, while a lower value will result in faster emulation.
#define CHIP8_CYCLES_PER_FRAME 20

typedef struct {
    uint8_t mem[CHIP8_MEM_SIZE];        // Memory (4K)
    uint8_t V[CHIP8_NUM_REGS];          // Registers (V0-VF)
    uint16_t I;                         // Index register (16-bit)
    uint16_t pc;                        // Program counter (16-bit)
    uint8_t delay_timer;                // Delay timer (8-bit)
    uint8_t sound_timer;                // Sound timer (8-bit)
    uint16_t stack[CHIP8_STACK_SIZE];   // Stack (16 levels)
    uint16_t sp;                        // Stack pointer (16-bit)
    uint8_t keypad[16];                 // Keypad state (16 keys)
    uint8_t display[CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT]; // Internal screen buffer (64x32 pixels)
} chip8_t;

// The chip8_key enum is a list of constants that represent the keys of a Chip-8 keypad.
// In the context of a Chip-8 emulator, the keypad is typically represented as an array of 16 elements,
// with each element corresponding to a specific key. The values of the chip8_key constants are simply indices into this array,
// allowing the emulator to easily determine which key is being pressed or released.
typedef enum {
    CHIP8_KEY_0,
    CHIP8_KEY_1,
    CHIP8_KEY_2,
    CHIP8_KEY_3,
    CHIP8_KEY_4,
    CHIP8_KEY_5,
    CHIP8_KEY_6,
    CHIP8_KEY_7,
    CHIP8_KEY_8,
    CHIP8_KEY_9,
    CHIP8_KEY_A,
    CHIP8_KEY_B,
    CHIP8_KEY_C,
    CHIP8_KEY_D,
    CHIP8_KEY_E,
    CHIP8_KEY_F
} chip8_key;


// The chip8_init function is used to initialize the state of the CHIP-8 emulator. It sets the following initial values:
// - It sets all the memory locations of the CHIP - 8 to 0 using the memset function.
// - It sets all the registers of the CHIP - 8 to 0 using the memset function.
// - It sets the program counter(pc) to the memory location 0x200 (512 in decimal).This is the location where the CHIP - 8 program starts executing.
// - It sets the stack pointer(sp) to 0.
// - It sets all the keys of the CHIP - 8's keypad to 0 using the memset function.
// - It sets all the pixels of the CHIP - 8's display to 0 using the memset function.
// These initial values prepare the CHIP - 8 emulator to execute a program.
void chip8_init(chip8_t* chip);

// The chip8_load_rom function takes a pointer to the CHIP-8 emulator instance (chip8_t) and the filename of the ROM file to load.
// The function attempts to open the file in binary read mode ("rb") and checks if the file was opened successfully.
// If the file cannot be opened, an error message is printed and the function returns.
// The function then seeks to the end of the file to get its size, rewinds the file pointer to the beginning, and checks if the ROM file is too large to fit into the emulator's memory.
// If the ROM file is too large, an error message is printed and the function returns.

// If the ROM file is small enough to fit into memory, the function reads the contents of the file into a buffer located in the emulator's memory starting at the CHIP8_PROGRAM_OFFSET address,
// which is the beginning of the program memory region. Finally, the function closes the ROM file.
bool chip8_load_rom(chip8_t* chip, const char* filename);

// This function fetches the current opcode, increments the program counter, decodes and executes the opcode,
// decrements the delayand sound timers if they are set, and plays a sound if the sound timer just reached zero (currently only prints BEEP).
// If the opcode is not recognized, the function prints an error message.
void chip8_step(chip8_t* chip);

// This function checks if the chip8_key value passed in is a valid key value (between CHIP8_KEY_0 and CHIP8_KEY_F),
// and sets the corresponding element in the chip->keypad array to the state value passed in.
void chip8_set_key(chip8_t* chip, uint8_t key, uint8_t state);


// ############################################################################################################
// Opcode handlers
// ############################################################################################################

// 00E0 - CLS: Clear the display.
//
// This function simply sets all the pixels in the display buffer to 0, effectively clearing the screen.
// Note that the display buffer is stored as a one - dimensional array of bytes, where each byte represents a pixel(either on or off).
// The dimensions of the display are defined by the CHIP8_DISPLAY_WIDTH and CHIP8_DISPLAY_HEIGHT constants in the chip8.h header file.
// In this case, the buffer is simply being cleared by setting all of its values to 0 using the memset() function.
void chip8_CLS(chip8_t* chip);

// 00EE - RET: Return from a subroutine.
//
// This function simply pops the top address from the stack(by decrementing the stack pointer), and sets the program counter to that address.
// This causes the interpreter to resume execution from the instruction that was interrupted by the subroutine call.
void chip8_RET(chip8_t* chip);

// 1nnn - JP addr: Jump to address `nnn`.
//
// This function simply sets the program counter to the address specified by the instruction,
// causing the interpreter to jump to that address and execute the next instruction from there.
// Note that the addr argument should be the 12-bit address (i.e., the last three digits of the instruction), not the full 16-bit instruction itself.
// The full instruction is passed to the instruction decoding logic, which extracts the address and calls this function with the correct argument.
void chip8_JP_addr(chip8_t* chip, uint16_t addr);

// 2nnn - CALL addr: Call subroutine at address `nnn`.
//
// This function first pushes the current value of the program counter onto the stack by storing it at the current top of the stack(as determined by the value of the stack pointer),
// and then increments the stack pointer. It then sets the program counter to the address specified by the instruction,
// causing the interpreter to begin executing the subroutine at that address.
// The subroutine should end with a RET instruction, which will cause the interpreter to return to the instruction that was interrupted by the subroutine call.
void chip8_CALL_addr(chip8_t* chip, uint16_t addr);

// 3xkk - SE Vx, byte: Skip next instruction if Vx == `kk`.
//
// This function first checks if the value in register Vx matches the byte kk.
// If it does, it increments the program counter by 4 to skip the next instruction.
// Otherwise, it simply increments the program counter by 2, causing the interpreter to execute the next instruction as usual.
// Note that the x argument should be the lower nibble of the instruction's first byte, and the kk argument should be the second byte of the instruction.
// The full instruction is passed to the instruction decoding logic, which extracts the register index (x) and the byte value (kk) and calls this function with the correct arguments.
void chip8_SE_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk);

// 4xkk - SNE Vx, byte: Skip next instruction if Vx != `kk`.
//
// This function first checks if the value in register Vx does not match the byte `kk`.
// If it does not match, it increments the program counter by 4 to skip the next instruction.
// Otherwise, it simply increments the program counter by 2, causing the interpreter to execute the next instruction as usual.
// Note that the `x` argument should be the lower nibble of the instruction's first byte, and the `kk` argument should be the second byte of the instruction.
// The full instruction is passed to the instruction decoding logic, which extracts the register index (`x`) and the byte value (`kk`) and calls this function with the correct arguments.
void chip8_SNE_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk);

// 5xy0 - SE Vx, Vy: Skip next instruction if Vx == Vy.
//
// This function first checks if the values in registers Vx and Vy are equal.
// If they are equal, it increments the program counter by 4 to skip the next instruction.
// Otherwise, it simply increments the program counter by 2, causing the interpreter to execute the next instruction as usual.
// Note that the `x` and `y` arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices (`x` and `y`) and calls this function with the correct arguments.
void chip8_SE_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// 6xkk - LD Vx, byte: Load `kk` into Vx.
//
// This function simply loads the byte `kk` into register Vx.
// Note that the `x` argument should be the lower nibble of the instruction's first byte, and `kk` should be the second byte of the instruction.
// The full instruction is passed to the instruction decoding logic, which extracts the register index (`x`) and the byte value (`kk`) and calls this function with the correct arguments.
void chip8_LD_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk);

// 7xkk - ADD Vx, byte: Add `kk` to Vx.
//
// This function adds the value `kk` to register Vx.
// Note that the `x` argument should be the lower nibble of the instruction's first byte, and `kk` should be the second byte of the instruction.
// The full instruction is passed to the instruction decoding logic, which extracts the register index (`x`) and the byte value (`kk`) and calls this function with the correct arguments.
void chip8_ADD_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk);

// 8xy0 - LD Vx, Vy: Set Vx = Vy.
//
// This function sets register Vx to the value of register Vy.
// Note that the `x` and `y` arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices (`x` and `y`) and calls this function with the correct arguments.
void chip8_LD_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// 8xy1 - OR Vx, Vy: Set Vx = Vx OR Vy.
//
// This function sets register Vx to the bitwise OR of its current value and the value of register Vy.
// Note that the `x` and `y` arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices (`x` and `y`) and calls this function with the correct arguments.
void chip8_OR_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// 8xy2 - AND Vx, Vy: Set Vx = Vx AND Vy.
//
// This function sets register Vx to the bitwise AND of its current value and the value of register Vy.
// Note that the `x` and `y` arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices (`x` and `y`) and calls this function with the correct arguments.
void chip8_AND_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// 8xy3 - XOR Vx, Vy: Set Vx = Vx XOR Vy.
//
// This function sets register Vx to the bitwise XOR of its current value and the value of register Vy.
// Note that the `x` and `y` arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices (`x` and `y`) and calls this function with the correct arguments.
void chip8_XOR_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// 8xy4 - ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry.
//
// This function adds the values of registers Vx and Vy together, and stores the result in register Vx.
// If the result overflows beyond 8 bits, the VF register is set to 1; otherwise it is set to 0.
// Note that the `x` and `y` arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices (`x` and `y`) and calls this function with the correct arguments.
void chip8_ADD_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// 8xy5 - SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow.
//
// This function first checks if the value in register Vx is greater than or equal to the value in register Vy.
// If it is, it sets the VF register to 1. Otherwise, it sets VF to 0.
// It then subtracts the value in register Vy from the value in register Vx, and stores the result in Vx.
// Finally, it increments the program counter by 2.
// Note that the x and y arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices and calls this function with the correct arguments.
void chip8_SUB_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// 8xy6 - SHR Vx {, Vy}: Set Vx = Vx SHR 1, set VF = LSB of Vx before shift.
//
// This function shifts the value in register Vx to the right by 1 bit, and sets the least significant bit of Vx as the carry flag (VF) before the shift.
// If the least significant bit of Vx is 1, then VF is set to 1. Otherwise, VF is set to 0.
// Note that the Vy argument is ignored in this instruction.
void chip8_SHR_Vx(chip8_t* chip, uint8_t x);

// 8xy7 - SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow.
//
// This function subtracts the value in register Vx from the value in register Vy, and stores the result in Vx.
// It also sets the VF register to 0 if there is a borrow (i.e., if Vy is less than Vx), or to 1 otherwise.
// Note that the x and y arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices (x and y) and calls this function with the correct arguments.
void chip8_SUBN_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// 8xyE - SHL Vx {, Vy}: Set Vx = Vx SHL 1, set VF = MSB of Vx before shift.
//
// This function shifts the value in register Vx left by 1 bit and stores the result in Vx.
// The most significant bit of Vx (i.e., the bit with the highest numerical value) is stored in the VF register before the shift is performed.
// The instruction can also take a second register argument Vy, but it is not used in this implementation of the CHIP-8 interpreter.
void chip8_SHL_Vx(chip8_t* chip, uint8_t x);

// 9xy0 - SNE Vx, Vy: Skip next instruction if Vx != Vy.
//
// This function first checks if the value in register Vx is not equal to the value in register Vy.
// If they are not equal, it increments the program counter by 4 to skip the next instruction.
// Otherwise, it simply increments the program counter by 2, causing the interpreter to execute the next instruction as usual.
// Note that the x and y arguments should be the lower nibbles of the instruction's first and second bytes, respectively.
// The full instruction is passed to the instruction decoding logic, which extracts the register indices (x and y) and calls this function with the correct arguments.
void chip8_SNE_Vx_Vy(chip8_t* chip, uint8_t x, uint8_t y);

// Annn - LD I, addr: Set I = `nnn`.
//
// This function simply sets the index register (I) to the value specified by the instruction.
// Note that the addr argument should be the full 16-bit address specified by the instruction.
// The full instruction is passed to the instruction decoding logic, which extracts the address and calls this function with the correct argument.
void chip8_LD_I_addr(chip8_t* chip, uint16_t addr);

// Bnnn - JP V0, addr: Jump to address `V0 + nnn`.
//
// This function adds the value in register V0 to the 12-bit address specified by the instruction,
// and sets the program counter to the resulting address.
// This allows for a jump to a location that is relative to the current value of V0.
// Note that the addr argument should be the 12-bit address (i.e., the last three digits of the instruction), not the full 16-bit instruction itself.
// The full instruction is passed to the instruction decoding logic, which extracts the address and calls this function with the correct argument.
void chip8_JP_V0_addr(chip8_t* chip, uint16_t addr);

// Cxkk - RND Vx, byte: Set Vx = random byte AND kk.
//
// This function generates a random byte and ANDs it with the byte value kk,
// then stores the result in register Vx. The effect of this is to generate a random number between 0 and 255
// and mask it with the byte kk, resulting in a random number between 0 and kk.
// Note that the x argument should be the lower nibble of the instruction's first byte, and the kk argument should be the second byte of the instruction.
// The full instruction is passed to the instruction decoding logic, which extracts the register index (x) and the byte value (kk) and calls this function with the correct arguments.
void chip8_RND_Vx_byte(chip8_t* chip, uint8_t x, uint8_t kk);

// Dxyn - DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
//
// This function draws a sprite of height n (specified by the third argument) starting at memory location I (specified by the index register).
// The sprite is drawn at the (x, y) position specified by the first two arguments, wrapping around to the opposite side of the screen if it goes out of bounds.
// The pixels of the sprite are XORed onto the screen buffer, which is a one-dimensional array of bytes representing the display.
// If any pixels in the sprite overlap with existing pixels on the screen buffer, the VF register is set to 1 (indicating a collision).
// Note that the x and y arguments specify the register indexes, not the actual x and y positions on the screen.
void chip8_DRW_Vx_Vy_nibble(chip8_t* chip, uint8_t x, uint8_t y, uint8_t height);

// Ex9E - SKP Vx: Skip next instruction if key with the value of Vx is pressed.
//
// This function checks if the key with the value in register Vx is currently pressed.
// If it is, it increments the program counter by 4 to skip the next instruction.
// Otherwise, it simply increments the program counter by 2, causing the interpreter to execute the next instruction as usual.
// Note that the x argument should be the lower nibble of the instruction's first byte.
// The full instruction is passed to the instruction decoding logic, which extracts the register index (x) and calls this function with the correct argument.
void chip8_SKP_Vx(chip8_t* chip, uint8_t x);

// ExA1 - SKNP Vx: Skip next instruction if key with the value of Vx is not pressed.
//
// This function checks if the key with the value of Vx is not currently pressed. If it is not, the interpreter skips the next instruction.
// Otherwise, it continues with the next instruction as usual.
// Note that the x argument should be the lower nibble of the instruction's first byte.
void chip8_SKNP_Vx(chip8_t* chip, uint8_t x);

// Fx07 - LD Vx, DT: Set Vx = delay timer value.
//
// This function sets the value of register Vx to the value of the delay timer.
// The delay timer is a timer that counts down at a rate of 60Hz until it reaches 0.
// It can be set to a value between 0 and 255 by writing to the special timer register (which is not directly accessible from Chip-8 instructions).
void chip8_LD_Vx_DT(chip8_t* chip, uint8_t x);

// Fx0A - LD Vx, K: Wait for a key press, store the value of the key in Vx.
//
// This function waits for a key press, and when a key is pressed, it stores its value in register Vx.
// Note that the program counter is not incremented until a key is pressed, at which point the interpreter skips the next instruction.
// The x argument should be the lower nibble of the instruction's first byte, which specifies the register to store the key value in.
void chip8_LD_Vx_K(chip8_t* chip, uint8_t x);

// Fx15 - LD DT, Vx: Set delay timer = Vx.
//
// This function sets the value of the delay timer to the value in register Vx.
void chip8_LD_DT_Vx(chip8_t* chip, uint8_t x);

// Fx18 - LD ST, Vx: Set sound timer = Vx.
//
// This function simply sets the value of the sound timer to the value of register Vx.
// Note that the x argument should be the lower nibble of the instruction's first byte.
// The full instruction is passed to the instruction decoding logic, which extracts the register index (x) and calls this function with the correct argument.
void chip8_LD_ST_Vx(chip8_t* chip, uint8_t x);

// Fx1E - ADD I, Vx: Set I = I + Vx.
//
// This function adds the value of register Vx to the index register (I).
// Note that the result is not stored in Vx; instead, it is stored in the index register itself.
// This instruction is commonly used to advance the index register to the next sprite in memory.
void chip8_ADD_I_Vx(chip8_t* chip, uint8_t x);

// Fx29 - LD F, Vx: Set I = location of sprite for digit Vx.
//
// This function sets the value of the index register (I) to the address of the sprite for the digit represented by the value in register Vx.
// Each sprite is 5 bytes long, and the font set is located in the first 80 bytes of memory, so the address is calculated by multiplying the value of Vx by 5 and adding it to the address of the font set.
void chip8_LD_F_Vx(chip8_t* chip, uint8_t x);

// Fx33 - LD B, Vx: Store BCD representation of Vx in memory locations I, I+1, and I+2.
//
// This function stores the binary-coded decimal (BCD) representation of the value in register Vx in memory locations I, I+1, and I+2.
// The most significant digit is stored in location I, the middle digit in location I+1, and the least significant digit in location I+2.
// Note that the x argument specifies the register whose value should be converted to BCD and stored in memory.
void chip8_LD_B_Vx(chip8_t* chip, uint8_t x);

// Fx55 - LD [I], Vx: Store registers V0 through Vx in memory starting at location I.
//
// This function stores the values of registers V0 through Vx (inclusive) in memory starting at the address stored in register I.
// After each value is stored, the I register is incremented to point to the next memory location.
// Note that the x argument should be the lower nibble of the instruction's first byte.
// The full instruction is passed to the instruction decoding logic, which extracts the register index (x) and calls this function with the correct argument.
void chip8_LD_I_Vx(chip8_t* chip, uint8_t x);

// Fx65 - LD Vx, [I]
//
// This function reads registers V0 through Vx from memory starting at the address specified by the I register.
// The values are then stored in the corresponding registers V0 through Vx.
// Note that the I register is incremented after the operation to point to the address immediately following the last register read.
// Therefore, if you want to read x+1 bytes from memory starting at address I, you should pass x as the argument to this function.
void chip8_LD_Vx_I(chip8_t* chip, uint8_t x);

#endif // CHIP8_H