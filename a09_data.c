/* A09, 6809 Assembler -- tables and global state.

   created 1993,1994 by L.C. Benschop.
   copyleft (c) 1994-2014 by the sbc09 team, see AUTHORS for more details.
   license: GNU General Public License version 2.
*/

#include "a09.h"

struct oprecord optable[]={
  {"ABX",0,0x3a},{"ADCA",7,0x89},{"ADCB",7,0xc9},
  {"ADDA",7,0x8b},{"ADDB",7,0xcb},{"ADDD",8,0xc3},
  {"ANDA",7,0X84},{"ANDB",7,0xc4},{"ANDCC",2,0x1c},
  {"ASL",10,0x08},{"ASLA",0,0x48},{"ASLB",0,0x58},
  {"ASR",10,0x07},{"ASRA",0,0x47},{"ASRB",0,0x57},
  {"BCC",4,0x24},{"BCS",4,0x25},{"BEQ",4,0x27},
  {"BGE",4,0x2c},{"BGT",4,0x2e},{"BHI",4,0x22},
  {"BHS",4,0x24},{"BITA",7,0x85},{"BITB",7,0xc5},
  {"BLE",4,0x2f},{"BLO",4,0x25},{"BLS",4,0x23},
  {"BLT",4,0x2d},{"BMI",4,0x2b},{"BNE",4,0x26},
  {"BPL",4,0x2a},{"BRA",4,0x20},{"BRN",4,0x21},
  {"BSR",4,0x8d},
  {"BVC",4,0x28},{"BVS",4,0x29},
  {"CLC",1,0x1cfe},{"CLF",1,0x1cbf},{"CLI",1,0x1cef},
  {"CLIF",1,0x1caf},
  {"CLR",10,0x0f},{"CLRA",0,0x4f},{"CLRB",0,0x5f},
  {"CLV",1,0x1cfd},
  {"CMPA",7,0x81},{"CMPB",7,0xc1},{"CMPD",9,0x1083},
  {"CMPS",9,0x118c},{"CMPU",9,0x1183},{"CMPX",8,0x8c},
  {"CMPY",9,0x108c},
  {"COM",10,0x03},{"COMA",0,0x43},{"COMB",0,0x53},
  {"CWAI",2,0x3c},{"DAA",0,0x19},
  {"DEC",10,0x0a},{"DECA",0,0x4a},{"DECB",0,0x5a},
  {"DES",1,0x327f},{"DEU",1,0x335f},{"DEX",1,0x301f},
  {"DEY",1,0x313f},
  {"ELSE",13,1},{"END",13,2},{"ENDIF",13,3},
  {"ENDM",13,4},{"EORA",7,0x88},{"EORB",7,0xc8},
  {"EQU",13,5},{"EXG",11,0x1e},{"EXTERN",13,6},
  {"FCB",13,7},{"FCC",13,8},{"FCW",13,9},
  {"FDB",13,9},{"IF",13,10},
  {"INC",10,0x0c},{"INCA",0,0x4c},{"INCB",0,0x5c},
  {"INCLUDE",13,16},
  {"INS",1,0x3261},{"INU",1,0x3341},{"INX",1,0x3001},
  {"INY",1,0x3121},{"JMP",10,0x0e},{"JSR",8,0x8d},
  {"LBCC",5,0x1024},{"LBCS",5,0x1025},{"LBEQ",5,0x1027},
  {"LBGE",5,0x102c},{"LBGT",5,0x102e},{"LBHI",5,0x1022},
  {"LBHS",5,0x1024},
  {"LBLE",5,0x102f},{"LBLO",5,0x1025},{"LBLS",5,0x1023},
  {"LBLT",5,0x102d},{"LBMI",5,0x102b},{"LBNE",5,0x1026},
  {"LBPL",5,0x102a},{"LBRA",6,0x16},{"LBRN",5,0x1021},
  {"LBSR",6,0x17},
  {"LBVC",5,0x1028},{"LBVS",5,0x1029},
  {"LDA",7,0x86},{"LDB",7,0xc6},{"LDD",8,0xcc},
  {"LDS",9,0x10ce},{"LDU",8,0xce},{"LDX",8,0x8e},
  {"LDY",9,0x108e},{"LEAS",3,0x32},
  {"LEAU",3,0x33},{"LEAX",3,0x30},{"LEAY",3,0x31},
  {"LSL",10,0x08},{"LSLA",0,0x48},{"LSLB",0,0x58},
  {"LSR",10,0x04},{"LSRA",0,0x44},{"LSRB",0,0x54},
  {"MACRO",13,11},{"MUL",0,0x3d},
  {"NEG",10,0x00},{"NEGA",0,0x40},{"NEGB",0,0x50},
  {"NOP",0,0x12},
  {"ORA",7,0x8a},{"ORB",7,0xca},{"ORCC",2,0x1a},
  {"ORG",13,12},
  {"PSHS",12,0x34},{"PSHU",12,0x36},{"PUBLIC",13,13},
  {"PULS",12,0x35},{"PULU",12,0x37},{"RMB",13,0},
  {"ROL",10,0x09},{"ROLA",0,0x49},{"ROLB",0,0x59},
  {"ROR",10,0x06},{"RORA",0,0x46},{"RORB",0,0x56},
  {"RTI",0,0x3b},{"RTS",0,0x39},
  {"SBCA",7,0x82},{"SBCB",7,0xc2},
  {"SEC",1,0x1a01},{"SEF",1,0x1a40},{"SEI",1,0x1a10},
  {"SEIF",1,0x1a50},{"SET",13,15},
  {"SETDP",13,14},{"SEV",1,0x1a02},{"SEX",0,0x1d},
  {"STA",7,0x87},{"STB",7,0xc7},{"STD",8,0xcd},
  {"STS",9,0x10cf},{"STU",8,0xcf},{"STX",8,0x8f},
  {"STY",9,0x108f},
  {"SUBA",7,0x80},{"SUBB",7,0xc0},{"SUBD",8,0x83},
  {"SWI",0,0x3f},{"SWI2",1,0x103f},{"SWI3",1,0x113f},
  {"SYNC",0,0x13},{"TFR",11,0x1f},{"TITLE",13,18},
  {"TST",10,0x0d},{"TSTA",0,0x4d},{"TSTB",0,0x5d},
};

int optablesize = sizeof(optable)/sizeof(optable[0]);

int symcounter=0;

struct symrecord symtable[NLABELS];

struct regrecord regtable[]=
                 {{"D",0x00,0x06},{"X",0x01,0x10},{"Y",0x02,0x20},
                  {"U",0x03,0x40},{"S",0x04,0x40},{"PC",0x05,0x80},
                  {"A",0x08,0x02},{"B",0x09,0x04},{"CC",0x0a,0x01},
                  {"CCR",0x0a,0x01},{"DP",0x0b,0x08},{"DPR",0x0b,0x08}};

/* global state */

FILE *listfile,*objfile;
char listname[FNLEN+1],objname[FNLEN+1],srcname[FNLEN+1],curname[FNLEN+1];
int lineno;

char pass;             /* Assembler pass=1 or 2 */
char listing;          /* flag to indicate listing */
char relocatable;      /* flag to indicate relocatable object. */
char terminate;        /* flag to indicate termination. */
char generating;       /* flag to indicate that we generate code */
unsigned short loccounter,oldlc;  /* Location counter */

char inpline[128];     /* Current input line (not expanded)*/
char srcline[128];     /* Current source line */
char * srcptr;         /* Pointer to line being parsed */

char unknown;          /* flag to indicate value unknown */
char certain;          /* flag to indicate value is certain at pass 1*/
int error;             /* flags indicating errors in current line. */
int errors;            /* number of errors */
char exprcat;          /* category of expression being parsed, eg.
                          label or constant, this is important when
                          generating relocatable object code. */


char namebuf[MAXIDLEN+1];

char mode; /* addressing mode 0=immediate,1=direct,2=extended,3=postbyte
               4=pcrelative(with postbyte) 5=indirect 6=pcrel&indirect*/
char opsize; /*desired operand size 0=dunno,1=5,2=8,3=16*/
short operand;
unsigned char postbyte;

int dpsetting;

unsigned char codebuf[128];
int codeptr; /* byte offset within instruction */
int suppress; /* 0=no suppress 1=until ENDIF 2=until ELSE 3=until ENDM */
int ifcount;  /* count of nested IFs within suppressed text */

unsigned char outmode; /* 0 is binary, 1 is s-records, 2 is Intel Hex */

unsigned short hexaddr;
int hexcount;
unsigned char hexbuffer[32]; /* was 16; outihex() fills up to 32 before flushing */
unsigned int chksum;

char *errormsg[]={"Error in expression",
                "Illegal addressing mode",
                "Undefined label",
                "Multiple definitions of label",
                "Relative branch out of range",
                "Missing label",
                "","","","","","","","","",
                "Illegal mnemonic"
               };

