# Operating-Systems-Phases
This repository contains the implementation of Phase 1 and Phase 2 of an Operating Systems (OS) project. It demonstrates core OS concepts such as process management and CPU scheduling through hands-on coding in C++.
## 📌 Project Overview
The project is divided into two major phases:
### 🔹 Phase 1: Simple Operating System Simulator
In Phase 1, we implemented a basic simulation of a single-job Operating System that can load, execute, and terminate jobs based on input files. It uses a predefined memory structure and instruction format to process simple commands like data read, write, comparison, and conditional branching.

This simulation mimics essential parts of a real OS, such as instruction handling, process execution, memory access, and interrupt-driven system calls.

**Key Features:**

**Memory Structure:** A 2D array simulating 100 blocks of memory with 4-character words (Memory[100][4]).

**Buffer:** Temporary 40-character buffer used for file I/O.

**Registers:**

_IR (Instruction Register):_ holds the current instruction.

_R (General Purpose Register):_ used to store intermediate values.

_IC (Instruction Counter):_ points to the current instruction in memory.

_C (Toggle Register):_ used for conditional branching.

_SI (Service Interrupt):_ Controls OS-level services like READ, WRITE, and TERMINATE.

**Supported Instructions:**

GD: Get Data (input from file to memory)

PD: Print Data (output from memory to file)

LR: Load Register (load memory value into R)

SR: Store Register (store R into memory)

CR: Compare Register (compare R with memory)

BT: Branch on Toggle (jump if comparison matches)

H: Halt (terminate the program)

### 🔹 Phase 2: Paged Memory Management and Multi-level Interrupt Handling
In Phase 2, we enhanced the OS simulator to support paged memory management and multi-level interrupt handling, introducing more realistic behavior of an operating system in managing user programs.

This phase simulates a virtual memory system where jobs are loaded into memory via a page table, and logical addresses are dynamically mapped to real memory using page frames. It also incorporates interrupt-driven execution to simulate system-level responses like page faults, invalid memory access, and time-bound execution.

**Key Features:**
**Paging Mechanism:**

Simulates paging with a page size of 10 words.

Each job has a Page Table Register (PTR) to manage its virtual memory.

Logical addresses (used in instructions) are translated to physical addresses using page table entries.

**Memory Structure:**

1. Expanded to a 2D array of Memory[300][4] simulating 300 blocks of 4-character memory.

2. Memory is dynamically allocated in page-sized chunks (10 blocks per page).

3. A bitmap tracks free and occupied blocks to prevent overlaps.

**Instruction Execution with Paging:**

1. Instructions reference logical addresses.

2. A addressMap() function translates virtual to physical addresses via page table lookup.

3. Page faults are raised and handled if the required page is not loaded.

**Registers and Components:**

IR – Instruction Register (current instruction)

R – General Purpose Register (intermediate storage)

IC – Instruction Counter (points to next instruction)

C – Condition flag for branching

PTR – Points to start of the page table

PI – Program Interrupt (opcode/operand errors, page faults)

SI – Supervisor Interrupt (system calls: READ, WRITE, HALT)

TI – Timer Interrupt (time limit enforcement)

**Error Handling (7 Types of Errors):**

1. Opcode Error : 	      Invalid or unrecognized operation code (e.g., "XY" instead of "GD")
   
2. Operand Error : 	      Invalid memory address format or illegal characters in operand
   
3. Valid Page Fault : 	  Page not currently loaded, but valid; system handles by allocating the page
   
4. Invalid Page Fault	:   Accessing a page outside of allocated memory; results in job termination
   
5. Time Limit Exceeded :	Instruction count exceeds TTL (Total Time Limit) defined for the job
    
6. Line Limit Exceeded :	Job tries to write more lines than TLL (Total Line Limit) allow
    
7. Out of Data :          GD instruction tries to read input after all data has been exhausted

