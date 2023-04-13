#include <GL/freeglut.h>
#include <stdio.h>
#include "chip8.h"
#include <stdbool.h>

// Constants for window size and scaling
#define SCREEN_WIDTH CHIP8_DISPLAY_WIDTH*10
#define SCREEN_HEIGHT CHIP8_DISPLAY_HEIGHT*10
#define PIXEL_SIZE 10

// Global Chip-8 emulator instance
chip8_t chip;

void timer(int value); // Declare the timer function prototype

// Define 16 colors as an array of RGB values
float colors[][3] = {
    {0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 1.0f},
    {0.5f, 0.5f, 0.5f},
    {0.8f, 0.8f, 0.8f},
    {0.8f, 0.2f, 0.2f},
    {0.2f, 0.8f, 0.2f},
    {0.2f, 0.2f, 0.8f},
    {0.8f, 0.8f, 0.2f},
    {0.8f, 0.2f, 0.8f},
    {0.2f, 0.8f, 0.8f}
};

// Display function for OpenGL window
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Run the emulator for one frame
    for (int i = 0; i < CHIP8_CYCLES_PER_FRAME; i++) {
        chip8_step(&chip);
    }

    if (chip.colorMode) {
        // Draw pixels from Chip-8 screen buffer
        for (int y = 0; y < CHIP8_DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < CHIP8_DISPLAY_WIDTH / 2; x++) {
                uint8_t pixel = chip.display[y * CHIP8_DISPLAY_WIDTH + x];
                uint8_t left_color_index = (pixel & 0xF0) >> 4;
                uint8_t right_color_index = pixel & 0x0F;

                // Draw left pixel
                glColor3f(colors[left_color_index][0], colors[left_color_index][1], colors[left_color_index][2]);
                glBegin(GL_QUADS);
                glVertex2f((2 * x * PIXEL_SIZE), (y * PIXEL_SIZE));
                glVertex2f((2 * x * PIXEL_SIZE), (y * PIXEL_SIZE) + PIXEL_SIZE);
                glVertex2f((2 * x * PIXEL_SIZE) + PIXEL_SIZE, (y * PIXEL_SIZE) + PIXEL_SIZE);
                glVertex2f((2 * x * PIXEL_SIZE) + PIXEL_SIZE, (y * PIXEL_SIZE));
                glEnd();

                // Draw right pixel
                glColor3f(colors[right_color_index][0], colors[right_color_index][1], colors[right_color_index][2]);
                glBegin(GL_QUADS);
                glVertex2f(((2 * x + 1) * PIXEL_SIZE), (y * PIXEL_SIZE));
                glVertex2f(((2 * x + 1) * PIXEL_SIZE), (y * PIXEL_SIZE) + PIXEL_SIZE);
                glVertex2f(((2 * x + 1) * PIXEL_SIZE) + PIXEL_SIZE, (y * PIXEL_SIZE) + PIXEL_SIZE);
                glVertex2f(((2 * x + 1) * PIXEL_SIZE) + PIXEL_SIZE, (y * PIXEL_SIZE));
                glEnd();
            }
        }
    }
    else {
        // Draw pixels from Chip-8 screen buffer
        for (int y = 0; y < CHIP8_DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < CHIP8_DISPLAY_WIDTH; x++) {
                uint8_t pixel = chip.display[y * CHIP8_DISPLAY_WIDTH + x];
                if (pixel) {
                    glColor3f(1.0f, 1.0f, 1.0f);
                }
                else {
                    glColor3f(0.0f, 0.0f, 0.0f);
                }
                glBegin(GL_QUADS);
                glVertex2f((x * PIXEL_SIZE), (y * PIXEL_SIZE));
                glVertex2f((x * PIXEL_SIZE), (y * PIXEL_SIZE) + PIXEL_SIZE);
                glVertex2f((x * PIXEL_SIZE) + PIXEL_SIZE, (y * PIXEL_SIZE) + PIXEL_SIZE);
                glVertex2f((x * PIXEL_SIZE) + PIXEL_SIZE, (y * PIXEL_SIZE));
                glEnd();
            }
        }
    }

    glutSwapBuffers();
    timer(0); // Call the timer function again to keep updating
}

// Timer function to update the screen
void timer(int value) {
    glutPostRedisplay(); // Schedule a call to the display function
    glutTimerFunc(1000 / 60, timer, 0); // Call this function again after 1000/60 milliseconds (approx. 60 FPS)
}

// Key press callback function
void key_down(unsigned char key, int x, int y) {
    chip8_key chip_key;

    switch (key) {
    case 'x': chip_key = CHIP8_KEY_0; break;
    case '1': chip_key = CHIP8_KEY_1; break;
    case '2': chip_key = CHIP8_KEY_2; break;
    case '3': chip_key = CHIP8_KEY_3; break;
    case 'q': chip_key = CHIP8_KEY_4; break;
    case 'w': chip_key = CHIP8_KEY_5; break;
    case 'e': chip_key = CHIP8_KEY_6; break;
    case 'a': chip_key = CHIP8_KEY_7; break;
    case 's': chip_key = CHIP8_KEY_8; break;
    case 'd': chip_key = CHIP8_KEY_9; break;
    case 'z': chip_key = CHIP8_KEY_A; break;
    case 'c': chip_key = CHIP8_KEY_B; break;
    case '4': chip_key = CHIP8_KEY_C; break;
    case 'r': chip_key = CHIP8_KEY_D; break;
    case 'f': chip_key = CHIP8_KEY_E; break;
    case 'v': chip_key = CHIP8_KEY_F; break;
    default: return;
    }

    chip8_set_key(&chip, chip_key, 1);
}

// Key release callback function
void key_up(unsigned char key, int x, int y) {
    chip8_key chip_key;

    switch (key) {
    case 'x': chip_key = CHIP8_KEY_0; break;
    case '1': chip_key = CHIP8_KEY_1; break;
    case '2': chip_key = CHIP8_KEY_2; break;
    case '3': chip_key = CHIP8_KEY_3; break;
    case 'q': chip_key = CHIP8_KEY_4; break;
    case 'w': chip_key = CHIP8_KEY_5; break;
    case 'e': chip_key = CHIP8_KEY_6; break;
    case 'a': chip_key = CHIP8_KEY_7; break;
    case 's': chip_key = CHIP8_KEY_8; break;
    case 'd': chip_key = CHIP8_KEY_9; break;
    case 'z': chip_key = CHIP8_KEY_A; break;
    case 'c': chip_key = CHIP8_KEY_B; break;
    case '4': chip_key = CHIP8_KEY_C; break;
    case 'r': chip_key = CHIP8_KEY_D; break;
    case 'f': chip_key = CHIP8_KEY_E; break;
    case 'v': chip_key = CHIP8_KEY_F; break;
    default: return;
    }

    chip8_set_key(&chip, chip_key, 0);
}

// Main function
int main(int argc, char** argv) {
    // Initialize the Chip-8 emulator
    chip8_init(&chip);

    

    const char* test_program =
        "6000" // LD V0, 0x00
        "6100" // LD V1, 0x00
        "F31F" // Enable color mode
        "A202" // LD I, 0x202 (sprite data start address)
        "D015" // DRW V0, V1, 0x05 (draw the sprite with 5 rows)
        "00EE" // Return from subroutine
        "0000" // Halt execution
        // Sprite data (5 rows, 8 pixels per row, 4 bits per pixel)
        "1234" "5678" "9ABC" "DEF0" "2345"
        ;

    // Load a ROM file into memory
    bool success = chip8_load_rom(&chip, "test_opcode.ch8");
    //bool success = chip8_load_program_from_hex_string(&chip, test_program);

    if (success) {
        // Initialize freeglut and create window
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
        glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        glutCreateWindow("Chip-8 VM");

        // Set up OpenGL display function
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Set up drawing parameters
        glPointSize(PIXEL_SIZE);
        glLineWidth(PIXEL_SIZE);

        // Set the background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Set the color for drawing pixels
        glColor3f(1.0f, 1.0f, 1.0f);
        

        // Register display function and keyboard input functions
        glutDisplayFunc(display);
        glutKeyboardFunc(key_down);
        glutKeyboardUpFunc(key_up);
        // Register the timer function to update the screen continuously
        glutTimerFunc(1000 / 60, timer, 0);

        // Enter main loop
        glutMainLoop();
    }

    getchar();

    return 0;
}