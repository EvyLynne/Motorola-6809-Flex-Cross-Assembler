/* A09, 6809 Assembler -- shared header.

   created 1993,1994 by L.C. Benschop.
   copyleft (c) 1994-2014 by the sbc09 team, see AUTHORS for more details.
   license: GNU General Public License version 2, see LICENSE for more details.
   THERE IS NO WARRANTY ON THIS PROGRAM.

   Three-file split, brought up to ISO/IEC 9899:2018 (C17):
     a09.h       this header: constants, macros, types, extern globals,
                 function prototypes
     a09_data.c  opcode/register/symbol tables, global state, error messages
     a09_core.c  scanner, expression evaluator, code generators, pass driver
     a09_main.c  main()

   Build:  gcc -std=c17 -Wall -Wextra -pedantic -o a09 \
               a09_data.c a09_core.c a09_main.c

   See the revision notes in a09_core.c for the full list of C17 changes
   relative to the 1993/94 original plus the Intel Hex fork.

   Machine dependencies:
                  char is 8 bits.
                  short is 16 bits.
                  integer arithmetic is twos complement.

   syntax: a09 [-o objname] [-s objname] [-x objname] [-l listname] srcname

   Options
   -o filename  name of the binary output file
   -s filename  name of the Motorola S-record output file
   -x filename  name of the Intel Hex output file
   -l filename  list file name (default no listing)
*/

#ifndef A09_H
#define A09_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define NLABELS 2048
#define MAXIDLEN 16
#define MAXLISTBYTES 7
#define FNLEN 30
#define LINELEN 128

#define EXITEVAL {srcptr--;return t;}

/* resolve such cases as constant added to address or difference between
   two addresses in same module */
#define RESOLVECAT {if((oldcat&15)==0)oldcat=0;\
           if((exprcat&15)==0)exprcat=0;\
           if((exprcat==2&&oldcat==34)||(exprcat==34&&oldcat==2)) {\
             exprcat=0;\
             oldcat=0;}\
           exprcat|=oldcat;}

#define RESTORE {srcptr=oldsrcptr;c=*srcptr;goto dodefault;}

/* types */

struct oprecord{char * name;
                unsigned char cat;
                unsigned short code;};

/* Instruction categories:
   0 one byte oprcodes   NOP
   1 two byte opcodes    SWI2
   2 opcodes w. imm byte ANDCC
   3 LEAX etc.
   4 short branches. BGE
   5 long branches 2byte opc LBGE
   6 long branches 1byte opc LBRA
   7 accumulator instr.      ADDA
   8 double reg instr 1byte opc LDX
   9 double reg instr 2 byte opc LDY
   10 single address instrs NEG
   11 TFR, EXG
   12 push,pull
   13 pseudoops
*/

struct symrecord{char name[MAXIDLEN+1];
                 char cat;
                 unsigned short value;
                };


/* Symbol categories.
   0 Constant value (from equ).
   1 Variable value (from set)
   2 Address within program module (label).
   3 Variable containing address.
   4 Adress in other program module (extern)
   5 Variable containing external address.
   6 Unresolved address.
   7 Variable containing unresolved address.
   8 Public label.
   9 Macro definition.
  10 Public label (yet undefined).
  11 parameter name.
  12 local label.
  13 empty.
*/

struct regrecord{char *name;unsigned char tfr,psh;};

/* tables and global state (defined in a09_data.c) */

extern struct oprecord optable[];
extern int optablesize;          /* entry count; sizeof() not usable
                                    on an extern incomplete array */
extern struct symrecord symtable[NLABELS];
extern int symcounter;
extern struct regrecord regtable[12];
extern char *errormsg[];

extern FILE *listfile,*objfile;
extern char listname[FNLEN+1],objname[FNLEN+1],srcname[FNLEN+1],curname[FNLEN+1];
extern int lineno;

extern char pass;             /* Assembler pass=1 or 2 */
extern char listing;          /* flag to indicate listing */
extern char relocatable;      /* flag to indicate relocatable object. */
extern char terminate;        /* flag to indicate termination. */
extern char generating;      /* flag to indicate that we generate code */
extern unsigned short loccounter,oldlc;  /* Location counter */

extern char inpline[128];     /* Current input line (not expanded)*/
extern char srcline[128];     /* Current source line */
extern char * srcptr;         /* Pointer to line being parsed */

extern char unknown;          /* flag to indicate value unknown */
extern char certain;          /* flag to indicate value is certain at pass 1*/
extern int error;             /* flags indicating errors in current line. */
extern int errors;            /* number of errors */
extern char exprcat;          /* category of expression being parsed */

extern char namebuf[MAXIDLEN+1];

extern char mode;   /* addressing mode 0=immediate,1=direct,2=extended,
                       3=postbyte 4=pcrelative(with postbyte) 5=indirect
                       6=pcrel&indirect */
extern char opsize; /* desired operand size 0=dunno,1=5,2=8,3=16 */
extern short operand;
extern unsigned char postbyte;
extern int dpsetting;

extern unsigned char codebuf[128];
extern int codeptr;  /* byte offset within instruction */
extern int suppress; /* 0=no suppress 1=until ENDIF 2=until ELSE 3=until ENDM */
extern int ifcount;  /* count of nested IFs within suppressed text */

extern unsigned char outmode; /* 0 is binary, 1 is s-records, 2 is Intel Hex */

extern unsigned short hexaddr;
extern int hexcount;
extern unsigned char hexbuffer[32]; /* was 16; outihex() fills up to 32 */
extern unsigned int chksum;

/* forward declarations (C17 prototypes) */

struct oprecord * findop(char * nm);
struct symrecord * findsym(char * nm);
struct regrecord * findreg(char *nm);
void outsymtable(void);
void scanname(void);
void skipspace(void);
short scandecimal(void);
short scanhex(void);
short scanchar(void);
short scanbin(void);
short scanoct(void);
short scanlabel(void);
short scanfactor(void);
short scanexpr(int level);
int scanindexreg(void);
void set3(void);
void scanspecial(void);
void scanindexed(void);
void scanoperands(void);
void flushhex(void);
void flushihex(void);
void outhex(unsigned char x);
void outihex(unsigned char x);
void outbuffer(void);
void report(void);
void outlist(void);
void setlabel(struct symrecord * lp);
void putbyte(unsigned char b);
void putword(unsigned short w);
void doaddress(void);
void onebyte(int co);
void twobyte(int co);
void oneimm(int co);
void lea(int co);
void sbranch(int co);
void lbra(int co);
void lbranch(int co);
void arith(int co);
void darith(int co);
void d2arith(int co);
void oneaddr(int co);
void tfrexg(int co);
void pshpul(int co);
void pseudoop(int co,struct symrecord * lp);
void processline(void);
void suppressline(void);
void usage(char*nm);
void getoptions(int c,char*v[]);
void expandline(void);
void processfile(char *name);


#endif /* A09_H */
