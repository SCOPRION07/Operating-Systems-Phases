#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdint>

using namespace std;

#define BUFFER_SIZE 41
#define MEMORY_ROWS 100
#define MEMORY_COLS 4

char buffer[BUFFER_SIZE];
char Memory[MEMORY_ROWS][MEMORY_COLS];
char IR[MEMORY_COLS];
char R[MEMORY_COLS];
int16_t IC;  // 2-byte instruction counter/address
int SI;       // supervisor interrupt
int8_t C;     // toggle register: 0(false) or 1(true)

ifstream infile;
ofstream outfile;

// Function prototypes
void init();
void resetBuffer();
void masterMode();
void READ();
void WRITE();
void TERMINATE();
void LOAD();
void MOSstartexe();
void executeUserProgram();
void displayMemoryAndRegister();

void displayMemoryAndRegister() {
    cout << "Memory:" << endl;
    for (int i = 0; i < MEMORY_ROWS; i++) {
        for (int j = 0; j < MEMORY_COLS; j++) {
            cout << (Memory[i][j] ? Memory[i][j] : ' ') << ' ';
        }
        cout << endl;
    }
    cout << "Buffer:" << endl;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        cout << (buffer[i] ? buffer[i] : ' ') << ' ';
    }
    cout << endl;
    cout << "IR:" << endl;
    for (int i = 0; i < MEMORY_COLS; i++) cout << (IR[i] ? IR[i] : ' ') << ' ';
    cout << endl;
    cout << "R:" << endl;
    for (int i = 0; i < MEMORY_COLS; i++) cout << (R[i] ? R[i] : ' ') << ' ';
    cout << endl;
    cout << "IC: " << IC << ", C: " << static_cast<int>(C) << ", SI: " << SI << endl;
}

void init() {
    memset(buffer, '\0', BUFFER_SIZE);
    memset(Memory, '\0', MEMORY_ROWS * MEMORY_COLS);
    memset(IR, '\0', MEMORY_COLS);
    memset(R, '\0', MEMORY_COLS);
    IC = 0;
    C = 1;   // start true, as in original C
    SI = 0;
}

void resetBuffer() {
    memset(buffer, '\0', BUFFER_SIZE);
}

void masterMode() {
    switch (SI) {
        case 1: READ();     break;
        case 2: WRITE();    break;
        case 3: TERMINATE(); break;
    }
    SI = 0;
}

void READ() {
    if (infile.getline(buffer, BUFFER_SIZE)) {
        // restore newline to match fgets
        size_t len = strlen(buffer);
        if (len < BUFFER_SIZE - 1) {
            buffer[len++] = '\n';
            buffer[len] = '\0';
        }
        int buff = 0;
        int mem_ptr = (IR[2] - '0') * 10;
        while (buff < BUFFER_SIZE && buffer[buff] != '\0') {
            for (int i = 0; i < MEMORY_COLS && buffer[buff] != '\0'; i++) {
                Memory[mem_ptr][i] = buffer[buff++];
            }
            mem_ptr++;
        }
    }
    resetBuffer();
}

void WRITE() {
    outfile.open("output.txt", ios::app);
    int start = (IR[2] - '0') * 10;
    int end = start + 10;
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MEMORY_COLS; j++) {
            if (Memory[i][j] != '\0') outfile.put(Memory[i][j]);
        }
    }
    outfile.close();
}

void TERMINATE() {
    outfile.open("output.txt", ios::app);
    outfile << "\n\n";
    outfile.close();
}

void LOAD() {
    char line[BUFFER_SIZE];
    while (infile.getline(line, BUFFER_SIZE)) {
        if (strncmp(line, "$AMJ", 4) == 0) {
            init();
        }
        else if (strncmp(line, "$DTA", 4) == 0) {
            resetBuffer();
            MOSstartexe();
        }
        else if (strncmp(line, "$END", 4) == 0) {
            // continue loading
        }
        else {
            // restore newline in load
            size_t len = strlen(line);
            if (len < BUFFER_SIZE - 1) {
                line[len++] = '\n';
                line[len] = '\0';
            }
            resetBuffer();
            memcpy(buffer, line, len);
            int buff = 0;
            while (buff < BUFFER_SIZE && buffer[buff] != '\0') {
                for (int j = 0; j < MEMORY_COLS && buffer[buff] != '\0'; j++) {
                    if (buffer[buff] == 'H') {
                        Memory[IC][j] = 'H';
                        buff++;
                        break;
                    }
                    Memory[IC][j] = buffer[buff++];
                }
                IC++;
            }
            cout << "Before execution of program" << endl;
            displayMemoryAndRegister();
        }
    }
    infile.close();
}

void MOSstartexe() {
    IC = 0;
    executeUserProgram();
}

void executeUserProgram() {
    while (IC < MEMORY_ROWS && Memory[IC][0] != '\0') {
        memcpy(IR, Memory[IC], MEMORY_COLS);
        IC++;
        if (IR[0] == 'G' && IR[1] == 'D') { SI = 1; masterMode(); }
        else if (IR[0] == 'P' && IR[1] == 'D') { SI = 2; masterMode(); }
        else if (IR[0] == 'H') { SI = 3; masterMode(); return; }
        else if (IR[0] == 'L' && IR[1] == 'R') {
            int addr = (IR[2] - '0') * 10 + (IR[3] - '0');
            memcpy(R, Memory[addr], MEMORY_COLS);
        }
        else if (IR[0] == 'S' && IR[1] == 'R') {
            int addr = (IR[2] - '0') * 10 + (IR[3] - '0');
            memcpy(Memory[addr], R, MEMORY_COLS);
        }
        else if (IR[0] == 'C' && IR[1] == 'R') {
            int addr = (IR[2] - '0') * 10 + (IR[3] - '0');
            C = (memcmp(Memory[addr], R, MEMORY_COLS) == 0);
        }
        else if (IR[0] == 'B' && IR[1] == 'T') {
            if (C) IC = (IR[2] - '0') * 10 + (IR[3] - '0');
        }
    }
}

int main() {
    cout << "Operating System Phase 1" << endl;
    cout << "Group 11" << endl;
    infile.open("input.txt");
    if (!infile) { cerr << "Error opening input file" << endl; return EXIT_FAILURE; }
    init();
    LOAD();
    cout << "\nAfter execution of program" << endl;
    displayMemoryAndRegister();
    return 0;
}
