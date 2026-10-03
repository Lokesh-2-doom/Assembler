/*
    
Name: Guguloth Lokesh
Roll no : 2401CS37
Course:CS2206
Declaration of Authorship:
I declare that this program is my own work. I have not copied
it from any other student or source except where explicitly
acknowledged. This work was completed as part of the course
requirements.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MEMORY 100000

typedef struct {
    int reg_a;
    int reg_b;
    int prog_counter;
    int stack_ptr;
    int mem[MAX_MEMORY];
    int running;
} StackMachine;

/* Sign extend 24-bit */
int extend_sign(int val) {
    if (val & 0x00800000)
        val |= 0xFF000000;
    return val;
}

/* Initialize */
void setup_machine(StackMachine *sm) {
    int i;
    sm->reg_a = 0;
    sm->reg_b = 0;
    sm->prog_counter = 0;
    sm->stack_ptr = MAX_MEMORY / 2;
    sm->running = 1;

    for (i = 0; i < MAX_MEMORY; i++)
        sm->mem[i] = 0;
}

/* Load program */
int load_program(StackMachine *sm, char *filename) {

    FILE *file;
    int count = 0;
    char *dot = strrchr(filename, '.');

    if(dot == NULL || strcmp(dot, ".obj") != 0){
        printf("Error: Input file must be a .obj file\n");
        exit(1);
    }

    file = fopen(filename, "rb");

    if (!file) {
        printf("Error: Cannot open file '%s'\n", filename);
        exit(1);
    }


    while (fread(&sm->mem[count], sizeof(int), 1, file)) {

        count++;

        if (count >= MAX_MEMORY) {
            printf("Error: Program exceeds memory capacity\n");
            fclose(file);
            exit(1);
        }
    }


    fclose(file);

    return count;
}

/* Stack bounds check */
void verify_stack_access(StackMachine *sm, int offset) {
    int addr = sm->stack_ptr + offset;

    if (addr < 0 || addr >= MAX_MEMORY) {
        printf("Error: Stack out of bounds at PC=%d\n",
                sm->prog_counter - 1);
        exit(1);
    }
}

/* Memory bounds check */
void verify_mem_access(StackMachine *sm, int addr) {
    if (addr < 0 || addr >= MAX_MEMORY) {
        printf("Error: Memory out of bounds at PC=%d\n",
                sm->prog_counter - 1);
        exit(1);
    }
}

/* Fetch instruction */
void fetch_next(StackMachine *sm, int *op, int *value, int program_size) {

    int inst;

    if (sm->prog_counter < 0 || sm->prog_counter >= program_size) {
        printf("Error: PC out of bounds\n");
        exit(1);
    }

    inst = sm->mem[sm->prog_counter++];

    *op    = inst & 0xFF;
    *value = extend_sign((inst >> 8) & 0x00FFFFFF);
}

/* Execute instruction */
void run_instruction(StackMachine *sm, int op, int value) {

    switch (op) {

        case 0:  /* ldc */
            sm->reg_b = sm->reg_a;
            sm->reg_a = value;
            break;

        case 1:  /* adc */
            sm->reg_a += value;
            break;

        case 2:  /* ldl */
            verify_stack_access(sm, value);
            sm->reg_b = sm->reg_a;
            sm->reg_a = sm->mem[sm->stack_ptr + value];
            break;

        case 3:  /* stl */
            verify_stack_access(sm, value);
            sm->mem[sm->stack_ptr + value] = sm->reg_a;
            sm->reg_a = sm->reg_b;
            break;

        case 4:  /* ldnl */
            verify_mem_access(sm, sm->reg_a + value);
            sm->reg_a = sm->mem[sm->reg_a + value];
            break;

        case 5:  /* stnl */
            verify_mem_access(sm, sm->reg_a + value);
            sm->mem[sm->reg_a + value] = sm->reg_b;
            break;

        case 6:  /* add */
            sm->reg_a = sm->reg_b + sm->reg_a;
            break;

        case 7:  /* sub */
            sm->reg_a = sm->reg_b - sm->reg_a;
            break;

        case 8:  /* shl */
            sm->reg_a = sm->reg_b << sm->reg_a;
            break;

        case 9:  /* shr */
            sm->reg_a = sm->reg_b >> sm->reg_a;
            break;

        case 10: /* adj */
            sm->stack_ptr += value;

            if (sm->stack_ptr < 0 || sm->stack_ptr >= MAX_MEMORY) {
                printf("Error: Stack pointer out of bounds\n");
                exit(1);
            }
            break;

        case 11: /* a2sp */
        if(sm->reg_a < 0 || sm->reg_a >= MAX_MEMORY){
            printf("Error: Stack pointer out of bounds\n");
            exit(1);
        }

        sm->stack_ptr = sm->reg_a;
        sm->reg_a = sm->reg_b;
        break;

        case 12: /* sp2a */
            sm->reg_b = sm->reg_a;
            sm->reg_a = sm->stack_ptr;
            break;

        case 13: /* call */
            sm->reg_b = sm->reg_a;
            sm->reg_a = sm->prog_counter;
            sm->prog_counter += value;
            break;

        case 14: /* return */
            sm->prog_counter = sm->reg_a;
            sm->reg_a = sm->reg_b;
            break;

        case 15: /* brz */
            if (sm->reg_a == 0)
                sm->prog_counter += value;
            break;

        case 16: /* brlz */
            if (sm->reg_a < 0)
                sm->prog_counter += value;
            break;

        case 17: /* br */
            sm->prog_counter += value;
            break;

        case 18: /* halt */
            sm->running = 0;
            break;

        default:
            printf("Error: Unknown opcode %d\n", op);
            exit(1);
    }
}

/* Run program */
void run(StackMachine *sm, int program_size) {

    int op, value;
    int steps = 0;
    int MAX_STEPS = 1000000;

    sm->running = 1;

    while (sm->running) {

        if (steps >= MAX_STEPS) {
            printf("Infinite loop detected\n");
            exit(0);
        }

        fetch_next(sm, &op, &value, program_size);
        run_instruction(sm, op, value);

        steps++;
    }
}

/* Print registers */
void show_registers(StackMachine *sm) {

    printf("\nFinal Register Values\n");
    printf("A  = %d (0x%08X)\n", sm->reg_a, sm->reg_a);
    printf("B  = %d (0x%08X)\n", sm->reg_b, sm->reg_b);
    printf("PC = %d (0x%08X)\n", sm->prog_counter, sm->prog_counter);
    printf("SP = %d (0x%08X)\n", sm->stack_ptr, sm->stack_ptr);
}

/* Memory dump */
void write_memory_dump(StackMachine *sm, int size, char *filename) {

    FILE *fp;
    int i;

    fp = fopen(filename, "w");

    if (!fp) {
        printf("Cannot create dump file\n");
        return;
    }

    fprintf(fp,"Address     Contents\n");
    fprintf(fp,"--------------------\n");

    for (i = 0; i < size; i++) {
        fprintf(fp,"%08X    %08X\n", i, sm->mem[i]);
    }

    fclose(fp);
}

/* Main */
int main(int argc, char *argv[]) {

    StackMachine sm;
    int program_size;

    char basename[200];
    char dumpname[200];
    char *dot;

    if (argc != 2) {
        printf("Please provide input file: program.obj\n");
        return 1;
    }

    setup_machine(&sm);

    program_size = load_program(&sm, argv[1]);

    run(&sm, program_size);

    printf("\nExecution finished\n");

    show_registers(&sm);

    strcpy(basename, argv[1]);

    dot = strrchr(basename, '.');
    if (dot)
        *dot = '\0';

    sprintf(dumpname, "%s.dump", basename);

    write_memory_dump(&sm, program_size+30, dumpname);

    printf("\nMemory dump written to %s\n", dumpname);

    return 0;
}
