#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int(*opcode_function_t)(unsigned char, unsigned char);

#define OPCODE_ADD   0
#define OPCODE_SUB   1
#define OPCODE_MUL   2
#define OPCODE_DIV   3
#define OPCODE_MOD   4
#define OPCODE_STP   5
#define OPCODE_LDI   6
#define OPCODE_ADR   7
#define OPCODE_ADRR  8
#define OPCODE_AND   9
#define OPCODE_OR    10
#define OPCODE_XOR   11
#define OPCODE_SUBR  12
#define OPCODE_MULR  13
#define OPCODE_DIVR  14
#define OPCODE_JMP   15
#define OPCODE_NOT   16

#define OPCODE_JE    17
#define OPCODE_JZ    17

#define OPCODE_JLT   18
#define OPCODE_JGT   19
#define OPCODE_CMP   20
#define OPCODE_INC   21
#define OPCODE_DEC   22
#define OPCODE_JLE   23
#define OPCODE_JGE   24
#define OPCODE_JNE   25
#define OPCODE_STI   26
#define OPCODE_LDM   27
#define OPCODE_STR   28 


#define PROGRAM_SIZE          sizeof(program2)
#define INSTRUCTIONS_COUNT    29
#define INSTRUCTION_SIZE      3

#define LEFT_OPERAND  IP + 1
#define RIGHT_OPERAND IP + 2

#define RX_COUNT 4
#define base 10

typedef enum Registers {
    R0,
    R1,
    R2, 
    R3,
}Registers;

static unsigned char program1 [] = {
    6, 0, 19, /* LDI, R0, 11 */
    6, 1, 22, /* LDI, R1, 22 */
    15, 12, 0, /* JMP, 18     */
    //6, 0, 34, /* LDI, R0, 34 */
    21, 0, 0, /* INC, R0     */
    //22, 0, 0, /* DEC, R0     */
    20, 0, 1, /* CMP, R0, R1 */
    //18, 9, 0, /* JLT, 9      */
    //24, 9, 0,  /* JGE, 9     */
    23, 9, 0,  /* JLE, 9     */

};

static unsigned char program2 [] = {
    0,0,0,0,0,0,0,0,0,0,
    26, 0, 50, /* STI, M[0], 50*/
    27, 0, 3,  /* LDM, R0, M[3]*/
    28, 3, 1,  /* STR, M[3], R1*/
    6, 0, 19, /* LDI, R0, 11 */
    6, 1, 22, /* LDI, R1, 22 */
    15, 31, 0, /* JMP, 18     */
    21, 0, 0, /* INC, R0     */
    20, 0, 1, /* CMP, R0, R1 */
    23, 28, 0,  /* JLE, 9     */
    0, 126, 20,  /* ADD, 126, 0 */
  };

static unsigned char program [] = {
    0, 9, 7,    /* ADD 9, 7   */
    1, 8, 4,    /* SUB 8, 4   */
    6, 2, 77,   /* LDI R2, 77 */
    //15, 200, 0,  /* JMP 200    */
    7, 2, 23,   /* ADR R2, 23 */
    2, 3, 6,    /* MUL 3, 6   */
    3, 9, 3,    /* DIV 9, 3   */
    4, 7, 2,    /* MOD 7, 2   */
    6, 1, 9,    /* LDI R1, 9  */
    8, 2, 1,    /* ADRR R2, R1*/
    9, 1, 2,    /* AND R1, R2 */
    10, 1, 2,   /* OR  R1, R2 */
    11, 1, 2,   /* XOR R1, R2 */
    12, 1, 5,   /* SUBR, R1, 5*/
    13, 1, 6,   /* MULR, R1, 6*/
    14, 1, 7,   /* DIVR, R1, 7*/
    16, 1, 0,   /* NOT, R1    */
    17, 9, 0,   /* JE, 9      */
    //18, 9, 0,   /* JLT, 9     */
    //19, 9, 0,   /*JGT, 9      */
};

// R1 = 5, R2 = 77

/* Registers */
static int IP = 0;
static unsigned char IR[INSTRUCTION_SIZE] = {0, 0, 0};
static int OUTPUT = 0;

typedef struct FLAG{
    unsigned char positive:1; 
    unsigned char zero:1;
    unsigned char negative:1;
    unsigned char divisible_by_3:1;
}Flag;

static unsigned char FLAGS = 0;

#define FLAG_POSITIVE       0 
#define FLAG_ZERO           1
#define FLAG_NEGATIVE       2
#define FLAG_DIVISIBLE_BY_3 3
#define FLAG_OVERFLOW       4

/*
static unsigned char R0 = 0;
static unsigned char R1 = 0;
static unsigned char R2 = 0;
static unsigned char R3 = 0;
*/

static unsigned char Reg[RX_COUNT] = {0};



static opcode_function_t current_instruction = NULL;

int opcode_add(unsigned char left_operand, unsigned char right_operand){
    int result = left_operand + right_operand;

    if (result > 127 || result < -128) {
        printf("flag overflow: %d\n", FLAG_OVERFLOW);
        FLAGS |= FLAG_OVERFLOW;  
    }
    unsigned char final = (unsigned char)(result & 0xFF);
    return final;
}

int opcode_sub(unsigned char left_operand, unsigned char right_operand){
    return left_operand - right_operand;
}

int opcode_mul(unsigned char left_operand, unsigned char right_operand){
    return left_operand * right_operand;
}

int opcode_div(unsigned char left_operand, unsigned char right_operand){
    if(right_operand == 0){
                    printf("Exception: divide by zero\n");
                    return false;
                }
    return left_operand / right_operand;
}

int opcode_mod(unsigned char left_operand, unsigned char right_operand){
    return left_operand % right_operand;
}

static unsigned char get_rx_value(unsigned char operand){
    Registers r = operand; 
    switch(r){
        case R0: return Reg[R0];
        case R1: return Reg[R1];
        case R2: return Reg[R2];
        case R3: return Reg[R3];
        // skip default case
    }

    return 255;
}

static void set_rx_value(unsigned char operand, unsigned char value){
     switch(operand){
        case R0: Reg[R0] = value; return;
        case R1: Reg[R1] = value; return;
        case R2: Reg[R2] = value; return;
        case R3: Reg[R3] = value; return;
        // skip default case
    }
}

int opcode_ldi(unsigned char left_operand, unsigned char right_operand){
    switch(left_operand){
        case R0: Reg[R0] = right_operand; break;
        case R1: Reg[R1] = right_operand; break;
        case R2: Reg[R2] = right_operand; break;
        case R3: Reg[R3] = right_operand; break;
        default:
            printf("Invalid register address\n");
            exit(0);
    }

    return right_operand;
}

int opcode_stp(unsigned char left_operand, unsigned char right_operand){
    OUTPUT = left_operand;
    exit(left_operand);
}

int opcode_adr(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0); // why exit(0) not exit(1)?
    }

    unsigned char Rx = get_rx_value(left_operand);

    return Rx + right_operand;
}

int opcode_subr(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0); // why exit(0) not exit(1)?
    }

    unsigned char Rx = get_rx_value(left_operand);

    return Rx - right_operand;
}

int opcode_mulr(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0); // why exit(0) not exit(1)?
    }

    unsigned char Rx = get_rx_value(left_operand);

    return Rx * right_operand;
}

int opcode_divr(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0); // why exit(0) not exit(1)?
    }

    unsigned char Rx = get_rx_value(left_operand);

    if(right_operand == 0){
        printf("Exception: divide by zero\n");
        return false;
    }

    return Rx / right_operand;
}



int opcode_adrr(unsigned char rst, unsigned char rnd){
    if(rst >= RX_COUNT || rnd >= RX_COUNT){
        printf("One or more invalid Rx register address\n");
        exit(0); 
    }
    unsigned char Rx = get_rx_value(rst);
    unsigned char Ry = get_rx_value(rnd);
    return Rx + Ry;
}

int opcode_and(unsigned char rst, unsigned char rnd){
    if(rst >= RX_COUNT || rnd >= RX_COUNT){
        printf("One or more invalid Rx register address\n");
        exit(0); 
    }
    unsigned char Rx = get_rx_value(rst);
    unsigned char Ry = get_rx_value(rnd);
    return Rx & Ry;
}

int opcode_or(unsigned char rst, unsigned char rnd){
    if(rst >= RX_COUNT || rnd >= RX_COUNT){
        printf("One or more invalid Rx register address\n");
        exit(0); 
    }
    unsigned char Rx = get_rx_value(rst);
    unsigned char Ry = get_rx_value(rnd);
    return Rx | Ry;
}

int opcode_xor(unsigned char rst, unsigned char rnd){
    if(rst >= RX_COUNT || rnd >= RX_COUNT){
        printf("One or more invalid Rx register address\n");
        exit(0); 
    }
    unsigned char Rx = get_rx_value(rst);
    unsigned char Ry = get_rx_value(rnd);
    return Rx ^ Ry;
}

int opcode_inc(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand);
    Rx++;
    set_rx_value(left_operand, Rx);
    return Rx;
}

int opcode_dec(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand);
    Rx--;
    set_rx_value(left_operand, Rx);
    return Rx;
}

int opcode_jmp(unsigned char jump, unsigned char un_used){
    IP = jump;
    return jump;
}


int opcode_not(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0); // why exit(0) not exit(1)?
    }

    unsigned char Rx = get_rx_value(left_operand);

    unsigned int R = ~Rx & 0xFF;
    return R;
}

int opcode_cmp(unsigned char left_operand, unsigned char right_operand){
    unsigned char Rx = get_rx_value(left_operand);
    unsigned char Ry = get_rx_value(right_operand);

    OUTPUT = Rx - Ry;
    FLAGS = FLAG_POSITIVE;

    if(OUTPUT == 0){
        FLAGS = FLAG_ZERO;
    }else if (OUTPUT < 0){
        FLAGS = FLAG_NEGATIVE;
    }

    if(OUTPUT % 3 == 0){
        printf("%d divisible by 3\n", OUTPUT);
        FLAGS = FLAG_DIVISIBLE_BY_3;
    }

    return FLAGS;
}

int opcode_je(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_ZERO){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jlt(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_NEGATIVE){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jgt(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_POSITIVE){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jle(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS != FLAG_POSITIVE){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jge(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS != FLAG_NEGATIVE){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jne(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS != FLAG_ZERO){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_sti(unsigned char mem_index, unsigned char immediate){
    program2[mem_index] = immediate;

    return immediate;
}

int opcode_ldm(unsigned char reg_index, unsigned char mem_index){
    
    unsigned char R = program2[mem_index];

    set_rx_value(reg_index, program2[mem_index]);
    return R;
}

int opcode_str(unsigned char mem_index, unsigned char reg_index){
    
    unsigned char Rx = get_rx_value(reg_index);
    program2[mem_index] = Rx;

    return Rx;
}

static const opcode_function_t opcode_functions[INSTRUCTIONS_COUNT] = {
        opcode_add, opcode_sub, opcode_mul,
        opcode_div, opcode_mod, opcode_stp,
        opcode_ldi, opcode_adr, opcode_adrr,
        opcode_and, opcode_or, opcode_xor,
        opcode_subr, opcode_mulr, opcode_divr,
        opcode_jmp, opcode_not, opcode_je,
        opcode_jlt, opcode_jgt, opcode_cmp,
        opcode_inc, opcode_dec, opcode_jle,
        opcode_jge, opcode_jne, opcode_sti,
        opcode_ldm, opcode_str,
};


static bool cpu_fetch(void){
    if(IP >= PROGRAM_SIZE){
        /* HALT */
        exit(0);
    }

    /* Fetch */
    IR[0] = program2[IP];
    IR[1] = program2[LEFT_OPERAND];
    IR[2] = program2[RIGHT_OPERAND];

    /* Move to the next insturction */
    IP += INSTRUCTION_SIZE;
    
    return true;
}

static bool cpu_decode(void){
    if(IR[0] >= INSTRUCTIONS_COUNT){
        printf("Invalid instruction\n");
        // IR[0] = 10; // rewrite
        IR[0] = IR[1] = IR[2] = 0;
        exit(0);
    }

    current_instruction = opcode_functions[IR[0]];

    return true;
}

static bool cpu_execute(void){
    //OUTPUT = current_instruction(IR[1], IR[2]);
    OUTPUT = opcode_functions[IR[0]](IR[1],IR[2]);
    return true;
}

int main(void){
    // reset
    IP = 0 + base;
    IR[0] = IR[1] = IR[2] = 0;

    int counter = 0;
    while(true){
        cpu_fetch();
        cpu_decode();
        cpu_execute();
        printf("%d: output -> %d\n", counter, OUTPUT);
        counter++;
    }

    return 0;
}