#include <iostream> 
#include <fstream>
#include<cstdlib>
#include <string>
#include <stdio.h>  
#include<time.h>  
#include<stdlib.h>
using namespace std;

char M[300][4], IR[4], GR[4]; // main memory
bool C;      
int PTR;   // page table register 
int job=0;
//rand(time(0)); // Seed the random_filled number generator

struct PCB
{
    char job[4], TTL[4], TLL[4];
    int TTC, LLC;
} pcb;

int VA, RA, TTL, TLL, EM, SI, TI, PI, ttl, tll, l = -1, IC, pte, InValid = 0;
fstream fin, fout;
string line;
int random_filled[30]; //  Tracks which memory frames are used.

void initialization();   
void load();     // Loads jobs from the input file
void Pagetable();   // Sets up the page table 
void allocate();   // Allocates memory frames for program cards and loads code into memory.
void startExecution();  // 	Sets up instruction counter and starts program execution
void executeProgram();
void AddMap();    // Maps virtual to real addresse
void Examine();   // Executes instructions
void MOS();   //  interrupt handling.
void Terminate();
void read();
void write();

void initialization()
{
    cout<<"Starting job No.: "<<++job<<endl<<endl;
    SI = TI = PI = pcb.TTC = pcb.LLC = TTL = TLL = EM = VA = RA = IC = PTR = InValid = 0;
    for (int i = 0; i < 30; i++)
    {
        random_filled[i] = 0;
    }
    for (int i = 0; i < 4; i++)  // mark them as empty or default set
    {
        IR[i] = '&';
        GR[i] = '_';
    }
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            M[i][j] = '_';
        }
    }
}

void load()
{
    while (fin)
    {
        int i, j;
        getline(fin, line);  // read one line from input file in line string 

        if (line[0] == '$' && line[1] == 'A' && line[2] == 'M' && line[3] == 'J')
        {
            initialization();
            for (i = 4, j = 0; i < 8; i++, j++)
            {
                pcb.job[j] = line[i];
            }
            for (i = 8, j = 0; i < 12; i++, j++)
            {
                pcb.TTL[j] = line[i];
            }
            for (i = 12, j = 0; i < 16; i++, j++)
            {
                pcb.TLL[j] = line[i];
            }
            ttl = stoi(pcb.TTL);  // ((pcb.TTL[0] - 48) * 1000 + (pcb.TTL[1] - 48) * 100 + (pcb.TTL[2] - 48) * 10 + (pcb.TTL[3] - 48));// convert to interger : '0' - 48
            tll = stoi(pcb.TLL); // (pcb.TLL[0] - 48) * 1000 + (pcb.TLL[1] - 48) * 100 + (pcb.TLL[2] - 48) * 10 + (pcb.TLL[3] - 48);
            Pagetable();
            allocate();
        }
        if (line[0] == '$' && line[1] == 'D' && line[2] == 'T' && line[3] == 'A')
        {
            startExecution();
        }
    }
}

void Pagetable()
{
    int i, j;
    PTR = (rand() % 29) * 10;
    random_filled[PTR / 10] = 1;  // mark this frame is used

    cout<<"PTR: "<<PTR<<endl;

    for (i = PTR; i < PTR + 10; i++)
    {
        for (j = 0; j < 4; j++)
        {
            M[i][j] = '*';
        }
    }
}

void allocate() 
{
    int check = 0; // check for 'H' intruction
    int i, j, pos = 0;
    int k = 0;
    char str[2];
    while (check != 1)
    {
        pos = (rand() % 29) * 10;
        while (random_filled[pos / 10] != 0) // check if that frame is occupied 
        {
            pos = (rand() % 29) * 10;
        }
        random_filled[pos / 10] = 1;
        sprintf(str, "%d", pos);  // convert frame to string 

        if (pos < 100)  // check for 1 digit frames 
        {
            M[PTR][2] = '0';
            M[PTR][3] = str[0];
        }
        else
        {
            M[PTR][2] = str[0];
            M[PTR][3] = str[1];
        }
        getline(fin, line);
        k = 0;
        for (i = 0; i < line.size() / 4; i++) // load program cards in main memory
        {
            for (j = 0; j < 4; j++)
            {
                M[pos + i][j] = line[k];
                k++;
                if (line[k] == 'H')
                {
                    check = 1;
                    M[pos + (i + 1)][0] = 'H';
                    M[pos + (i + 1)][1] = '0';
                    M[pos + (i + 1)][2] = '0';
                    M[pos + (i + 1)][3] = '0';
                    j=4;
                }
                    
            }
        }
    }
}

void startExecution()
{
    IC = 0;
    executeProgram();
}

void executeProgram()
{
    int no;
    char v[3];
    v[0] = M[PTR][2];  // get the block no from page table 
    v[1] = M[PTR][3];
    v[2] = '\0';
    no = ((v[0] - 48) * 10) + (v[1] - 48);

    while (IR[0] != 'H')
    {
        for (int k = 0; k < 4; k++)
        {
            IR[k] = M[(no * 10) + IC][k];  // load instruction from memory into IR
        }
        if (!isdigit(IR[2]) || !isdigit(IR[3]) || isalpha(IR[2]) || isalpha(IR[3]))
        {
            PI = 2;  // operand error 
            if (pcb.TTC >= ttl)
                TI = 2;
            else
                TI = 0;
            MOS();
        }

        VA = ((IR[2] - 48) * 10) + (IR[3] - 48);
        AddMap();
        Examine();
    }
}

void AddMap() {
    int pos;
    // Compute the page-table entry index for VA
    pte = PTR + (VA / 10);

    // Unmapped page
    if (M[pte][3] == '*') {
        // Only GD (Get Data) may trigger a valid page-fault mapping
        if ((IR[0] == 'G' && IR[1] == 'D') || (IR[0] == 'S' && IR[1] == 'R') ) {
            PI = 3;                    // page-fault indicator
            cout << "Valid Page Fault Handled" << endl;
            EM = 7;                    // code for “Valid Page Fault”

            // Find a free frame and mark it
            pos = (rand() % 29) * 10;
            while (random_filled[pos / 10] != 0)
                pos = (rand() % 29) * 10;
            random_filled[pos / 10] = 1;

            // Write the frame number into the page table
            string str = to_string(pos);
            if (pos < 100) {
                M[pte][2] = '0';
                M[pte][3] = str[0];
            } else {
                M[pte][2] = str[0];
                M[pte][3] = str[1];
            }
        }
        else {
            // Any other unmapped access is invalid
            PI = 3;
            InValid = 1;
            MOS();    // immediate jump to error handler
            return;
        }
    }
    else {
        // Already mapped → clear any previous page-fault indicator
        PI = 0;
    }

    // Compute real address from page table
    int p = (M[pte][2] - '0') * 10 + (M[pte][3] - '0');
    RA = (p * 10) + (VA % 10);

    // Out-of-bounds check
    if (RA >= 300) {
        PI = 2; TI = 0;
        MOS();
    }
}

void Examine()
{
    if (IR[0] == 'G')
    {
        IC = IC + 1;
        if (IR[1] == 'D')
        {
            SI = 1;
            pcb.TTC = pcb.TTC + 2;
            if (pcb.TTC < ttl)
                TI = 0;
            else
                TI = 2;
            MOS();
        }
        else
        {
            PI = 1;
            if (pcb.TTC > ttl)
                TI = 2;
            else
            TI = 0;
            MOS();
        }
    }
    else if (IR[0] == 'P')
    {
        IC = IC + 1;
        if (IR[1] == 'D')
        {
            pcb.LLC = pcb.LLC + 1;
            pcb.TTC = pcb.TTC + 1;
            SI = 2;
            if (pcb.TTC < ttl)
            {
                TI = 0;
                if (PI == 3)
                {
                    InValid = 1;
                }
            }
            else
                TI = 2;
        }
        else
        {
            PI = 1;
            if (pcb.TTC >= ttl)
                TI = 2;
            else
                TI = 0;
        }
        MOS();
    }
    else if (IR[0] == 'L')
    {
        IC = IC + 1;
        if (IR[1] == 'R')
        {
            if (PI == 3)
            {
                InValid = 1;
                TI = 0;
                MOS();
            }
            else
            {
                for (int j = 0; j < 4; j++)
                    GR[j] = M[RA][j];
                pcb.TTC++;
            }
            if (pcb.TTC > ttl)
            {
                PI = 3;
                TI = 2;
                MOS();
            }
        }
        else
        {
            PI = 1;
            if (pcb.TTC >= ttl)
                TI = 2;
            TI = 0;
            MOS();
        }
    }
    else if (IR[0] == 'S')
    {
        IC = IC + 1;
        if (IR[1] == 'R')
        {
            for (int j = 0; j < 4; j++)
                M[RA][j] = GR[j];
            pcb.TTC = pcb.TTC + 2;

            if (pcb.TTC > ttl)
            {
                TI = 2;
                PI = 3;
                MOS();
            }
        }
        else
        {
            PI = 1;
            if (pcb.TTC > ttl)
                TI = 2;
            TI = 0;
            MOS();
        }
    }
    else if (IR[0] == 'C')
    {
        IC = IC + 1;
        if (IR[1] == 'R')
        {
            if (PI == 3)
            {
                InValid = 1;
                TI = 0;
                MOS();
            }
            else
            {
                if (M[RA][1] == GR[1] && M[RA][2] == GR[2] && M[RA][3] == GR[3] && M[RA][0] == GR[0])
                    C = true;
                else
                    C = false;
                pcb.TTC++;
            }
            if (pcb.TTC > ttl)
            {
                TI = 2;
                PI = 3;
                MOS();
            }
        }
        else
        {
            PI = 1;
            if (pcb.TTC > ttl)
                TI = 2;
            TI = 0;
            MOS();
        }
    }
    else if (IR[0] == 'B')
    {
        IC = IC + 1;
        if (IR[1] == 'T')
        {
            if (PI == 3)
            {
                InValid = 1;
                TI = 0;
                MOS();
            }
            else
            {
                if (C == true)
                    IC = VA;
                pcb.TTC++;
            }
            if (pcb.TTC > ttl)
            {
                TI = 2;
                PI = 3;
                MOS();
            }
        }
        else
        {
            PI = 1;
            if (pcb.TTC > ttl)
                TI = 2;
            TI = 0;
            MOS();
        }
    }
    else if (IR[0] == 'H')
    {
        IC = IC + 1;
        pcb.TTC++;
        if (pcb.TTC > ttl)
        {
            TI = 2;
            PI = 3;
            MOS();
        }
        else
        {
            SI = 3;
            MOS();
        }
    }
    else
    {
        PI = 1;
        if (pcb.TTC > ttl)
            TI = 2;
        TI = 0;
        MOS();
    }
}

void MOS()
{
    if (PI == 1)
    {
        if (TI == 0)
        {
            EM = 4;
            cout << "Opcode Error" << endl;
            Terminate();
        }
        else if (TI == 2)
        {
            EM = 3;
            cout << "Time Limit Exceeded" << endl;
            EM = 4;
            cout << "Opcode Error" << endl;
            Terminate();
        }
    }
    else if (PI == 2)
    {
        if (TI == 0)
        {
            EM = 5;
            cout << "Operand Error" << endl;
            Terminate();
        }
        else if (TI == 2)
        {
            EM = 3;
            cout << "Time Limit Exceeded" << endl;
            EM = 5;
            cout << "Operand Error" << endl;
            Terminate();
        }
    }
    else if (PI == 3)
    {
        if (TI == 0)
        {
            if (InValid == 1)
            {
                EM = 6;
                cout << "Invalid Page Fault" << endl;
                Terminate();
            }
        }
        else if (TI == 2)
        {
            EM = 3;
            cout << "Time Limit Exceeded" << endl;
            Terminate();
        }
    }
    if (SI == 1)
    {
        if (TI == 0)
            read();
        else if (TI == 2)
        {
            EM = 3;
            cout << "Time Limit Exceeded" << endl;
            Terminate();
        }
    }
    if (SI == 2)
    {
        if (TI == 0)
            write();
        else if (TI == 2)
        {
            write();
            EM = 3;
            cout << "Time Limit Exceeded" << endl;
            Terminate();
        }
    }
    if (SI == 3)
    {
        EM = 0;
        cout << "No Error" << endl;
        Terminate();
    }
}

void Terminate()
{
    cout << "\n\nMEMORY (After Execution):" << endl;
    for (int i = 0; i < 300; i++)
    {
        if ( i < 100 )
            cout << "M [" << i << "] \t\t\t |";
        else
            cout << "M [" << i << "] \t\t |";
        for (int j = 0; j < 4; j++)
        {
            cout << M[i][j] << "|";
        }
        cout << endl;
    }
    cout << endl;
    cout << "Job ID : " << pcb.job[0] << pcb.job[1] << pcb.job[2] << pcb.job[3] << " \tTTL =" << ttl << "\t\tTLL =" << tll << "\t\tTTC = " << pcb.TTC << "\tLLC =" << pcb.LLC << endl;
    cout << "PTR = " << PTR << "\tIC = " << IC << "\t\tEM = " << EM << "\t\tIR = ";
    for (int i = 0; i < 4; i++)
        cout << IR[i];

    fout << "Job ID : " << pcb.job[0] << pcb.job[1] << pcb.job[2] << pcb.job[3] << " \tTTL =" << ttl << "\t\tTLL =" << tll << "\t\tTTC = " << pcb.TTC << "\tLLC =" << pcb.LLC << endl;
    fout << "PTR = " << PTR << "\tIC = " << IC << "\t\tEM = " << EM << "\t\tIR = ";
    for (int i = 0; i < 4; i++)
        fout << IR[i];

    cout<<endl;

    if(EM==0){
        fout<<endl<<"No Errors"<<endl;
    }
    else if(EM==1){
        fout<<endl<<"Out of Data"<<endl;
    }
    else if(EM==2){
        fout<<endl<<"Line limit exceeded"<<endl;
    }
    else if(EM==3){
        fout<<endl<<"Time Limit Exceeded"<<endl;
    }
    else if(EM==4){
        fout<<endl<<"Opcode Error"<<endl;
    }
    else if(EM==5){
        fout<<endl<<"Oprand Error"<<endl;
    }
    else if(EM==6){
        fout<<endl<<"Invalid Page Fault"<<endl;
    }


    cout<<endl<<"Ending of job No.: "<<job<<endl;
    cout<<"\n\n" << endl;
    fout << "-----------------------------------------------------------------------------------" << endl;
    fout << "-----------------------------------------------------------------------------------" << endl;

    load();
    exit(0);
}

void read()
{
    getline(fin, line);
    // cout << line << endl;
    if (line[0] == '$' && line[1] == 'E' && line[2] == 'N' && line[3] == 'D')
    {
        EM = 1;
        cout << "Out of Data" << endl;
        Terminate();
    }
    int i, j, k;
    k = 0;
    for (i = 0; k <= line.size(); i++)
    {
        for (j = 0; j < 4 && k <= line.size(); j++)
        {
            M[RA + i][j] = line[k];
            k++;
        }
    }
}

void write()
{
    char buff[40];
    int ra = 0, i, k;
    ra = RA;
    k = 0;
    if (pcb.LLC > tll)
    {
        EM = 2;
        cout << "Line Limit Exceeded" << endl;
        Terminate();
    }
    while (true)
    {
        for (i = 0; i < 4; i++)
        {
            if (M[ra][i] == '_')
                break;
            buff[k] = M[ra][i];
            k++;
        }
        if (M[ra][i] == '_')
            break;
        ra++;
    }
    buff[k] = '\0';
    fout << buff << endl;
    cout<<buff<<endl;
}

int main()
{
    fin.open("input_2.txt", ios::in);
    fout.open("output_2.txt", ios::out);
    load();
    fin.close();
    fout.close();
    return 0;
}

