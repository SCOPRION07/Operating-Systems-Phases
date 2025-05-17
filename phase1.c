#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h> // Library to use int8_t data type and int16_t data type

#define BUFFER_SIZE 41
#define MEMORY_ROWS 100
#define MEMORY_COLS 4

char buffer[BUFFER_SIZE];
char Memory[MEMORY_ROWS][MEMORY_COLS]; // 100x4 grid for main memory
char IR[MEMORY_COLS]; // Instruction Register
char R[MEMORY_COLS]; // General Purpose Register
int16_t IC;    // Instruction Counter (2 bytes)
int SI;        // System Interrupt
int8_t C;    // Toggle Register (1 byte)
FILE *infile, *outfile;

// All functions prototype
void init();    // Initializes all memory and registers
void resetBuffer();
void masterMode();  /* Handled through SI codes (1-3) */
void READ();      /* Reads data from input file into memory */ 
void WRITE();     /* Writes data from memory to output file */
void TERMINATE(); /* Marks end of program execution */
void LOAD();      /* Load the instructions in main memory*/ 
void MOSstartexe();
void executeUserProgram();
void displayMemoryAndRegister();

void displayMemoryAndRegister(){
    int i,j;
    printf("Memory:\n");
    for(i=0;i<MEMORY_ROWS;i++)
    {
        printf(" %d :\t\t",i);
        for(j=0;j<MEMORY_COLS;j++)
        {
            printf("|%c",Memory[i][j]);
        }
            printf("|\n");
    }
    printf("Buffer:\n");
    for(i = 0;i<BUFFER_SIZE;i++){
        printf("%c ",buffer[i]);
    }
    printf("\nIR:\n");
    for(i = 0;i<MEMORY_COLS;i++){
        printf("%c ",IR[i]);
    }
    printf("\nR:\n");
    for(i = 0;i<MEMORY_COLS;i++){
        printf("%c ",R[i]);
    }
    printf("\nIC: %d, C: %d, SI: %d",IC,C,SI);
}

void init()
{
    memset(buffer, '\0', BUFFER_SIZE);
    memset(Memory, '\0', MEMORY_ROWS * MEMORY_COLS);
    memset(IR, '\0', MEMORY_COLS);
    memset(R, '\0', MEMORY_COLS);
    IC = 0;
    C = 1;
    SI = 0;
}

void resetBuffer()
{
    memset(buffer, '\0', BUFFER_SIZE);
}

void masterMode()
{
    switch (SI)
    {
    case 1:
        READ();
        break;
    case 2:
        WRITE();
        break;
    case 3:
        TERMINATE();
        break;
    }
    SI = 0;
}

void READ()
{
    if (fgets(buffer, BUFFER_SIZE, infile))  // 1. Read input 
    {
        int buff = 0, mem_ptr = (IR[2] - '0') * 10;  //Calculate starting position
        while (buff < BUFFER_SIZE && buffer[buff] != '\0')
        {
            for (int i = 0; i < MEMORY_COLS; i++)    
            {
                Memory[mem_ptr][i] = buffer[buff];   // Fill memory columns
                buff++;
            }
            mem_ptr++;  //  Move to next row
        }
    } 
    resetBuffer();   
}

void WRITE()
{
    outfile = fopen("output.txt", "a");
    for (int i = (IR[2] - '0') * 10; i < (IR[2] - '0' + 1) * 10; i++)
    {
        for (int j = 0; j < MEMORY_COLS; j++)
        {
            if (Memory[i][j] != '\0')
            {
                fputc(Memory[i][j], outfile);
            }
        }
    }
    fclose(outfile);
}

void TERMINATE()
{
    outfile = fopen("output.txt", "a");
    fputs("\n\n", outfile);
    fclose(outfile);
}

void LOAD()
{
    char line[BUFFER_SIZE];  //  Temporary input buffer
    if (infile)     //  Check if file is open
    {
        while (fgets(line, BUFFER_SIZE, infile))  //  Read file line-by-line
        {
            if (strncmp(line, "$AMJ", 4) == 0)  //  Job start marker
            {
                init();   //  Reset all memory/registers
            }
            else if (strncmp(line, "$DTA", 4) == 0)  //  Data/execution marker
            {
                resetBuffer();
                MOSstartexe();    //  Begin program execution
            }
            else if (strncmp(line, "$END", 4) == 0)  // Job End
            {
                return;
            }
            else
            {
                int len = strlen(line);
                resetBuffer();
                memcpy(buffer, line, len);
                int buff = 0;
                while (buff < BUFFER_SIZE && buffer[buff] != '\0')
                {
                    for (int j = 0; j < MEMORY_COLS; j++)
                    {
                        if (buffer[buff] == 'H')
                        {
                            Memory[IC][j] = 'H';
                            buff++;
                            break;
                        }
                        Memory[IC][j] = buffer[buff];
                        buff++;
                    }
                    IC++;
                }
                printf("Before execution of program\n");
                displayMemoryAndRegister();
            }
        }
        fclose(infile);
    }
}

void MOSstartexe()
{
    IC = 0;
    executeUserProgram();
}

void executeUserProgram()
{
    while (IC < 99 && Memory[IC][0] != '\0')
    {
        memcpy(IR, Memory[IC], MEMORY_COLS);
        IC++;
        if (IR[0] == 'G' && IR[1] == 'D')  // Get data 
        {
            SI = 1;
            masterMode();
        }
        else if (IR[0] == 'P' && IR[1] == 'D')  // write data 
        {
            SI = 2;
            masterMode();
        }
        else if (IR[0] == 'H')  // exit 
        {
            SI = 3;
            masterMode();
            return;
        }
        else if (IR[0] == 'L' && IR[1] == 'R')  // load in register 
        {
            memcpy(R, Memory[(IR[2] - '0') * 10 + (IR[3] - '0')], MEMORY_COLS);
        }
        else if (IR[0] == 'S' && IR[1] == 'R')  // load from register 
        {
            memcpy(Memory[(IR[2] - '0') * 10 + (IR[3] - '0')], R, MEMORY_COLS);
        }
        else if (IR[0] == 'C' && IR[1] == 'R')  // compare register 
        {
            C = memcmp(Memory[(IR[2] - '0') * 10 + (IR[3] - '0')], R, MEMORY_COLS) == 0;
        }
        else if (IR[0] == 'B' && IR[1] == 'T')  // flag to jump if c=1
        {
            if (C)
            {
                IC = (IR[2] - '0') * 10 + (IR[3] - '0');
            }
        }
    }
}

int main()
{
    printf("Operating System Phase 1\nGroup 11\n");
    infile = fopen("input.txt", "r");
    if (!infile)
    {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }
    init();
    LOAD();
    printf("\nAfter execution of program\n");
    displayMemoryAndRegister();
    return 0;
}