/* A09, 6809 Assembler -- scanner, expression evaluator, code
   generators, pseudo-ops, and the two-pass driver.

   created 1993,1994 by L.C. Benschop.
   copyleft (c) 1994-2014 by the sbc09 team, see AUTHORS for more details.
   license: GNU General Public License version 2.
   THERE IS NO WARRANTY ON THIS PROGRAM.

   (fork)  Intel Hex output support added (outmode 2, -x option,
           flushihex/outihex).
   2026-07-09 brought up to ISO/IEC 9899:2018 (C17):
   - full prototypes in a09.h (fixes implicit declarations for the
     recursive scanexpr and mutually recursive processfile/pseudoop);
   - empty parameter lists () changed to (void);
   - processfile changed from int (never returned a value, undefined
     behavior under C99+) to void;
   - ctype.h calls given (unsigned char) casts to avoid undefined
     behavior on negative char arguments;
   - RESOLVECAT macro rewritten as a brace block (the original relied
     on a comment spliced into the macro body);
   - hexbuffer enlarged 16 -> 32 bytes: outihex() fills up to 32 bytes
     before flushing, which overflowed the original 16-byte buffer;
   - (suppress==1|suppress==2) changed to logical || in suppressline
     (identical truth value, silences warning);
   - findop uses optablesize instead of sizeof(optable) because the
     table is now an extern incomplete array (split build only).
   Behavior otherwise preserved bug-for-bug, including the missing
   break fall-through after case | in scanexpr (also present in the
   1993/94 original).
*/

#include "a09.h"

struct oprecord * findop(char * nm)
/* Find operation (mnemonic) in table using binary search */
{
 int lo,hi,i,s;
 lo=0;hi=optablesize-1;
 do {
  i=(lo+hi)/2;
  s=strcmp(optable[i].name,nm);
  if(s<0) lo=i+1;
  else if(s>0) hi=i-1;
  else break;
 } while(hi>=lo);
 if (s) return NULL;
 return optable+i;
}

struct symrecord * findsym(char * nm)
/* finds symbol table record; inserts if not found
   uses binary search, maintains sorted table */
{
 int lo,hi,i,j,s;
 lo=0;hi=symcounter-1;
 s=1;i=0;
 while (hi>=lo) {
  i=(lo+hi)/2;
  s=strcmp(symtable[i].name,nm);
  if(s<0) lo=i+1;
  else if(s>0) hi=i-1;
  else break;
 }
 if(s) {
  i=(s<0?i+1:i);
  if(symcounter==NLABELS) {
   fprintf(stderr,"Sorry, no storage for symbols!!!");
   exit(4);
  }
  for(j=symcounter;j>i;j--) symtable[j]=symtable[j-1];
  symcounter++;
  strcpy(symtable[i].name,nm);
  symtable[i].cat=13;
 }
 return symtable+i;
}

void outsymtable(void)
{
 int i,j=0;
 fprintf(listfile,"\nSYMBOL TABLE");
 for(i=0;i<symcounter;i++)
 if(symtable[i].cat!=13) {
  if(j%4==0)fprintf(listfile,"\n");
  fprintf(listfile,"%10s %02d %04x",symtable[i].name,symtable[i].cat,
                       symtable[i].value);
  j++;
 }
 fprintf(listfile,"\n");
}

struct regrecord * findreg(char *nm)
{
 int i;
 for(i=0;i<12;i++) {
  if(strcmp(regtable[i].name,nm)==0) return regtable+i;
 }
 return 0;
}

void scanname(void)
{
 int i=0;
 char c;
 while(1) {
   c=*srcptr++;
   if(c>='a'&&c<='z')c-=32;
   if((c<'0'||c>'9')&&(c<'A'||c>'Z'))break;
   if(i<MAXIDLEN)namebuf[i++]=c;
 }
 namebuf[i]=0;
 srcptr--;
}

void skipspace(void)
{
 char c;
 do {
  c=*srcptr++;
 } while(c==' '||c=='\t');
 srcptr--;
}

short scandecimal(void)
{
 char c;
 short t=0;
 c=*srcptr++;
 while(isdigit((unsigned char)c)) {
  t=t*10+c-'0';
  c=*srcptr++;
 }
 srcptr--;
 return t;
}

short scanhex(void)
{
 short t=0,i=0;
 srcptr++;
 scanname();
 while(namebuf[i]>='0'&&namebuf[i]<='F') {
  t=t*16+namebuf[i]-'0';
  if(namebuf[i]>'9')t-=7;
  i++;
 }
 if(i==0)error|=1;
 return t;
}

short scanchar(void)
{
 short t;
 srcptr++;
 t=*srcptr;
 if(t)srcptr++;
 if (*srcptr=='\'')srcptr++;
 return t;
}

short scanbin(void)
{
 char c;
 short t=0;
 srcptr++;
 c=*srcptr++;
 while(c=='0'||c=='1') {
  t=t*2+c-'0';
  c=*srcptr++;
 }
 srcptr--;
 return t;
}

short scanoct(void)
{
 char c;
 short t=0;
 srcptr++;
 c=*srcptr++;
 while(c>='0'&&c<='7') {
  t=t*8+c-'0';
  c=*srcptr++;
 }
 srcptr--;
 return t;
}

short scanlabel(void)
{
 struct symrecord * p;
 scanname();
 p=findsym(namebuf);
 if(p->cat==13) {
   p->cat=6;
   p->value=0;
 }
 if(p->cat==9||p->cat==11)error|=1;
 exprcat=p->cat&14;
 if(exprcat==6||exprcat==10)unknown=1;
 if(((exprcat==2||exprcat==8)
     && (unsigned short)(p->value)>(unsigned short)loccounter)||
     exprcat==4)
   certain=0;
 if(exprcat==8||exprcat==6||exprcat==10)exprcat=2;
 return p->value;
}

short scanfactor(void)
{
 char c;
 short t;
 skipspace();
 c=*srcptr;
 if(isalpha((unsigned char)c))return scanlabel();
 else if(isdigit((unsigned char)c))return scandecimal();
 else switch(c) {
  case '*' : srcptr++;exprcat|=2;return loccounter;
  case '$' : return scanhex();
  case '%' : return scanbin();
  case '@' : return scanoct();
  case '\'' : return scanchar();
  case '(' : srcptr++;t=scanexpr(0);skipspace();
             if(*srcptr==')')srcptr++;else error|=1;
             return t;
  case '-' : srcptr++;exprcat^=32;return -scanfactor();
  case '+' : srcptr++;return scanfactor();
  case '!' : srcptr++;exprcat|=16;return !scanfactor();
  case '~' : srcptr++;exprcat|=16;return ~scanfactor();
 }
 error|=1;
 return 0;
}

short scanexpr(int level) /* This is what you call _recursive_ descent!!!*/
{
 short t,u;
 char oldcat,c;
 exprcat=0;
 if(level==10)return scanfactor();
 t=scanexpr(level+1);
 while(1) {
  skipspace();
  c=*srcptr++;
  switch(c) {
  case '*':oldcat=exprcat;
           t*=scanexpr(10);
           exprcat|=oldcat|16;
           break;
  case '/':oldcat=exprcat;
           u=scanexpr(10);
           if(u)t/=u;else error|=1;
           exprcat|=oldcat|16;
           break;
  case '%':oldcat=exprcat;
           u=scanexpr(10);
           if(u)t%=u;else error|=1;
           exprcat|=oldcat|16;
           break;
  case '+':if(level==9)EXITEVAL
           oldcat=exprcat;
           t+=scanexpr(9);
           RESOLVECAT
           break;
  case '-':if(level==9)EXITEVAL
           oldcat=exprcat;
           t-=scanexpr(9);
           exprcat^=32;
           RESOLVECAT
           break;
  case '<':if(*(srcptr)=='<') {
            if(level>=8)EXITEVAL
            srcptr++;
            oldcat=exprcat;
            t<<=scanexpr(8);
            exprcat|=oldcat|16;
            break;
           } else if(*(srcptr)=='=') {
            if(level>=7)EXITEVAL
            srcptr++;
            oldcat=exprcat;
            t=t<=scanexpr(7);
            exprcat|=oldcat|16;
            break;
           } else {
            if(level>=7)EXITEVAL
            oldcat=exprcat;
            t=t<scanexpr(7);
            exprcat|=oldcat|16;
            break;
           }
  case '>':if(*(srcptr)=='>') {
            if(level>=8)EXITEVAL
            srcptr++;
            oldcat=exprcat;
            t>>=scanexpr(8);
            exprcat|=oldcat|16;
            break;
           } else if(*(srcptr)=='=') {
            if(level>=7)EXITEVAL
            srcptr++;
            oldcat=exprcat;
            t=t>=scanexpr(7);
            exprcat|=oldcat|16;
            break;
           } else {
            if(level>=7)EXITEVAL
            oldcat=exprcat;
            t=t>scanexpr(7);
            exprcat|=oldcat|16;
            break;
           }
  case '!':if(level>=6||*srcptr!='=')EXITEVAL
           srcptr++;
           oldcat=exprcat;
           t=t!=scanexpr(6);
           exprcat|=oldcat|16;
           break;
  case '=':if(level>=6)EXITEVAL
           if(*srcptr=='=')srcptr++;
           oldcat=exprcat;
           t=t==scanexpr(6);
           exprcat|=oldcat|16;
           break;
  case '&':if(level>=5)EXITEVAL
           oldcat=exprcat;
           t&=scanexpr(5);
           exprcat|=oldcat|16;
           break;
  case '^':if(level>=4)EXITEVAL
           oldcat=exprcat;
           t^=scanexpr(4);
           exprcat|=oldcat|16;
           break;
  case '|':if(level>=3)EXITEVAL
           oldcat=exprcat;
           t|=scanexpr(3);
           exprcat|=oldcat|16;
           /* no break here in the 1993/94 original; preserved
              bug-for-bug (srcptr ends up one char early after
              a '|' expression) */
           /* fall through */
  default: EXITEVAL
  }
 }
}

int scanindexreg(void)
{
 char c;
 c=*srcptr;
 if(islower((unsigned char)c))c-=32;
 switch(c) {
  case 'X':return 1;
  case 'Y':postbyte|=0x20;return 1;
  case 'U':postbyte|=0x40;return 1;
  case 'S':postbyte|=0x60;return 1;
  default: return 0;
 }
}

void set3(void)
{
 if(mode<3)mode=3;
}

void scanspecial(void)
{
 set3();
 skipspace();
 if(*srcptr=='-') {
  srcptr++;
  if(*srcptr=='-') {
   srcptr++;
   postbyte=0x83;
  } else postbyte=0x82;
  if(!scanindexreg())error|=2;else srcptr++;
 } else {
  postbyte=0x80;
  if(!scanindexreg())error|=2;else srcptr++;
  if(*srcptr=='+') {
   srcptr++;
   if(*srcptr=='+') {
    srcptr++;
    postbyte+=1;
   }
  } else postbyte+=4;
 }
}

void scanindexed(void)
{
 set3();
 postbyte=0;
 if(scanindexreg()) {
   srcptr++;
   if(opsize==0){if(unknown||!certain)opsize=3;
                else if(operand>=-16&&operand<16&&mode==3)opsize=1;
                else if(operand>=-128&&operand<128)opsize=2;
                else opsize=3;}
   switch(opsize) {
   case 1:postbyte+=(operand&31);opsize=0;break;
   case 2:postbyte+=0x88;break;
   case 3:postbyte+=0x89;break;
   }
 } else { /*pc relative*/
  if(toupper((unsigned char)*srcptr)!='P')error|=2;
  else {
    srcptr++;
    if(toupper((unsigned char)*srcptr)!='C')error|=2;
    else {
     srcptr++;
     if(toupper((unsigned char)*srcptr)=='R')srcptr++;
    }
  }
  mode++;postbyte+=0x8c;
  if(opsize==1)opsize=2;
 }
}

void scanoperands(void)
{
 char c,*oldsrcptr;
 unknown=0;
 opsize=0;
 certain=1;
 skipspace();
 c=*srcptr;
 mode=0;
 if(c=='[') {
  srcptr++;
  c=*srcptr;
  mode=5;
 }
 switch(c) {
 case 'D': case 'd':
  oldsrcptr=srcptr;
  srcptr++;
  skipspace();
  if(*srcptr!=',')RESTORE else {
     postbyte=0x8b;
     srcptr++;
     if(!scanindexreg())RESTORE else {srcptr++;set3();}
  }
  break;
 case 'A': case 'a':
  oldsrcptr=srcptr;
  srcptr++;
  skipspace();
  if(*srcptr!=',')RESTORE else {
     postbyte=0x86;
     srcptr++;
     if(!scanindexreg())RESTORE else {srcptr++;set3();}
  }
  break;
 case 'B': case 'b':
  oldsrcptr=srcptr;
  srcptr++;
  skipspace();
  if(*srcptr!=',')RESTORE else {
     postbyte=0x85;
     srcptr++;
     if(!scanindexreg())RESTORE else {srcptr++;set3();}
  }
  break;
 case ',':
  srcptr++;
  scanspecial();
  break;
 case '#':
  if(mode==5)error|=2;else mode=0;
  srcptr++;
  operand=scanexpr(0);
  break;
 case '<':
  srcptr++;
  if(*srcptr=='<') {
   srcptr++;
   opsize=1;
  } else opsize=2;
  goto dodefault;
 case '>':
  srcptr++;
  opsize=3;
  /* fall through */
 default: dodefault:
  operand=scanexpr(0);
  skipspace();
  if(*srcptr==',') {
   srcptr++;
   scanindexed();
  } else {
   if(opsize==0) {
    if(unknown||!certain||dpsetting==-1||
         (unsigned short)(operand-dpsetting*256)>=256)
    opsize=3; else opsize=2;
   }
   if(opsize==1)opsize=2;
   if(mode==5){
    postbyte=0x8f;
    opsize=3;
   } else mode=opsize-1;
  }
 }
 if(mode>=5) {
  skipspace();
  postbyte|=0x10;
  if(*srcptr!=']')error|=2;else srcptr++;
 }
 if(pass==2&&unknown)error|=4;
}

/* Motorola s-records */
void flushhex(void)
{
 int i;
 if(hexcount){
  fprintf(objfile,"S1%02X%04X",(hexcount+3)&0xff,hexaddr&0xffff);
  for(i=0;i<hexcount;i++)fprintf(objfile,"%02X",hexbuffer[i]);
  chksum+=(hexaddr&0xff)+((hexaddr>>8)&0xff)+hexcount+3;
  fprintf(objfile,"%02X\n",0xff-(chksum&0xff));
  hexaddr+=hexcount;
  hexcount=0;
  chksum=0;
 }
}

/* Intel Hex */
void flushihex(void)
{
 int i;
 unsigned char  *j;
 if(hexcount){
    j = &hexbuffer[0];
    fprintf(objfile, ":%02X%04X00", hexcount, hexaddr&0xffff);
    chksum = hexcount + ((hexaddr >> 8) & 0xff) + (hexaddr & 0xff);
    for (i = 0; i < hexcount; i++, j++)
    {
      chksum += *j;
      fprintf(objfile, "%02X", *j);
    }
    fprintf(objfile, "%02X\n", (-chksum) & 0xff);
    hexaddr+=hexcount;
    hexcount=0;
    chksum=0;
 }
}

void outhex(unsigned char x)
{
 if(hexcount==16)flushhex();
 hexbuffer[hexcount++]=x;
 chksum+=x;
}

void outihex(unsigned char x)
{
 if(hexcount==32)flushihex();
 hexbuffer[hexcount++]=x;
 chksum+=x;
}

void outbuffer(void)
{
 int i;
 switch(outmode) {
  /* Binary */
  case 0 :
   for(i=0;i<codeptr;i++)
    fputc(codebuf[i],objfile);
   break;
  /* s-records */
  case 1 :
   for(i=0;i<codeptr;i++)
    outhex(codebuf[i]);
   break;
  /* Intel Hex */
  case 2 :
   for(i=0;i<codeptr;i++)
    outihex(codebuf[i]);
   break;
 }
}

void report(void)
{
 int i;
 fprintf(stderr,"File %s, line %d:%s\n",curname,lineno,srcline);
 for(i=0;i<16;i++) {
  if(error&1) {
   fprintf(stderr,"%s\n",errormsg[i]);
   if(pass==2&&listing)fprintf(listfile,"**** %s\n",errormsg[i]);
  }
  error>>=1;
 }
 errors++;
}

void outlist(void)
{
 int i;
 fprintf(listfile,"%04X: ",oldlc);
 for(i=0;i<codeptr&&i<MAXLISTBYTES;i++)
  fprintf(listfile,"%02X",codebuf[i]);
 for(;i<=MAXLISTBYTES;i++)
  fprintf(listfile,"  ");
 fprintf(listfile,"%s\n",srcline);
}

void setlabel(struct symrecord * lp)
{
 if(lp) {
  if(lp->cat!=13&&lp->cat!=6) {
   if(lp->cat!=2||lp->value!=loccounter)
    error|=8;
  } else {
   lp->cat=2;
   lp->value=loccounter;
  }
 }
}

void putbyte(unsigned char b)
{
 codebuf[codeptr++]=b;
}

void putword(unsigned short w)
{
 codebuf[codeptr++]=w>>8;
 codebuf[codeptr++]=w&0x0ff;
}

void doaddress(void) /* assemble the right addressing bytes for an instruction */
{
 int offs;
 switch(mode) {
 case 0: if(opsize==2)putbyte(operand);else putword(operand);break;
 case 1: putbyte(operand);break;
 case 2: putword(operand);break;
 case 3: case 5: putbyte(postbyte);
    switch(opsize) {
     case 2: putbyte(operand);break;
     case 3: putword(operand);
    }
    break;
 case 4: case 6: offs=(unsigned short)operand-loccounter-codeptr-2;
                if(offs<-128||offs>=128||opsize==3||unknown||!certain) {
                 if((!unknown)&&opsize==2)error|=16;
                 offs--;
                 opsize=3;
                 postbyte++;
                }
                putbyte(postbyte);
                if(opsize==3)putword(offs);
                else putbyte(offs);
 }
}

void onebyte(int co)
{
 putbyte(co);
}

void twobyte(int co)
{
 putword(co);
}

void oneimm(int co)
{
 scanoperands();
 if(mode>=3)error|=2;
 putbyte(co);
 putbyte(operand);
}

void lea(int co)
{
 putbyte(co);
 scanoperands();
 if(mode==0) error|=2;
 if(mode<3) {
   opsize=3;
   postbyte=0x8f;
   mode=3;
 }
 doaddress();
}

void sbranch(int co)
{
 int offs;
 scanoperands();
 if(mode!=1&&mode!=2)error|=2;
 offs=(unsigned short)operand-loccounter-2;
 if(!unknown&&(offs<-128||offs>=128))error|=16;
 if(pass==2&&unknown)error|=4;
 putbyte(co);
 putbyte(offs);
}

void lbra(int co)
{
 scanoperands();
 if(mode!=1&&mode!=2)error|=2;
 putbyte(co);
 putword(operand-loccounter-3);
}

void lbranch(int co)
{
 scanoperands();
 if(mode!=1&&mode!=2)error|=2;
 putword(co);
 putword(operand-loccounter-4);
}

void arith(int co)
{
 scanoperands();
 switch(mode) {
 case 0:opsize=2;putbyte(co);break;
 case 1:putbyte(co+0x010);break;
 case 2:putbyte(co+0x030);break;
 default:putbyte(co+0x020);
 }
  doaddress();
}

void darith(int co)
{
 scanoperands();
 switch(mode) {
 case 0:opsize=3;putbyte(co);break;
 case 1:putbyte(co+0x010);break;
 case 2:putbyte(co+0x030);break;
 default:putbyte(co+0x020);
 }
 doaddress();
}

void d2arith(int co)
{
 scanoperands();
 switch(mode) {
 case 0:opsize=3;putword(co);break;
 case 1:putword(co+0x010);break;
 case 2:putword(co+0x030);break;
 default:putword(co+0x020);
 }
 doaddress();
}

void oneaddr(int co)
{
 scanoperands();
 switch(mode) {
 case 0: error|=2;break;
 case 1: putbyte(co);break;
 case 2: putbyte(co+0x70);break;
 default: putbyte(co+0x60);break;
 }
 doaddress();
}

void tfrexg(int co)
{
 struct regrecord * p;
 putbyte(co);
 skipspace();
 scanname();
 if((p=findreg(namebuf))==0)error|=2;
 else postbyte=(p->tfr)<<4;
 skipspace();
 if(*srcptr==',')srcptr++;else error|=2;
 skipspace();
 scanname();
 if((p=findreg(namebuf))==0)error|=2;
 else postbyte|=p->tfr;
 putbyte(postbyte);
}

void pshpul(int co)
{
 struct regrecord *p;
 putbyte(co);
 postbyte=0;
 do {
  if(*srcptr==',')srcptr++;
  skipspace();
  scanname();
  if((p=findreg(namebuf))==0)error|=2;
  else postbyte|=p->psh;
  skipspace();
 }while (*srcptr==',');
 putbyte(postbyte);
}

void pseudoop(int co,struct symrecord * lp)
{
 int i;
 char c;
 char fname[FNLEN+1];
 switch(co) {
 case 0:/* RMB */
        setlabel(lp);
        operand=scanexpr(0);
        if(unknown)error|=4;
        loccounter+=operand;
        if(generating&&pass==2) {
         switch(outmode) {
          /* Binary */
          case 0 :
           for(i=0;i<operand;i++)
            fputc(0,objfile);
           break;
          /* s-records */
          case 1 :
           for(i=0;i<operand;i++)
            flushhex();
           break;
          /* Intel Hex */
          case 2 :
           for(i=0;i<operand;i++)
            flushihex();
           break;
         }
        }
        hexaddr=loccounter;
        break;
 case 5:/* EQU */
        operand=scanexpr(0);
        if(!lp)error|=32;
        else {
         if(lp->cat==13||lp->cat==6||
            (lp->value==(unsigned short)operand&&pass==2)) {
          if(exprcat==2)lp->cat=2;
          else lp->cat=0;
          lp->value=operand;
         } else error|=8;
        }
        break;
 case 7:/* FCB */
        setlabel(lp);
        generating=1;
        do {
        if(*srcptr==',')srcptr++;
        skipspace();
        if(*srcptr=='\"') {
         srcptr++;
         while(*srcptr!='\"'&&*srcptr)
          putbyte(*srcptr++);
         if(*srcptr=='\"')srcptr++;
        } else {
          putbyte(scanexpr(0));
          if(unknown&&pass==2)error|=4;
        }
        skipspace();
        } while(*srcptr==',');
        break;
 case 8:/* FCC */
        setlabel(lp);
        skipspace();
        c=*srcptr++;
        while(*srcptr!=c&&*srcptr)
         putbyte(*srcptr++);
        if(*srcptr==c)srcptr++;
        break;
 case 9:/* FDB */
        setlabel(lp);
        generating=1;
        do {
         if(*srcptr==',')srcptr++;
         skipspace();
         putword(scanexpr(0));
         if(unknown&&pass==2)error|=4;
         skipspace();
        } while(*srcptr==',');
        break;
 case 1: /* ELSE */
        suppress=1;
        break;
 case 10: /* IF */
        operand=scanexpr(0);
        if(unknown)error|=4;
        if(!operand)suppress=2;
        break;
 case 12: /* ORG */
         operand=scanexpr(0);
         if(unknown)error|=4;
         if(generating&&pass==2) {
          switch(outmode) {
           /* Binary */
           case 0 :
            for(i=0;i<(unsigned short)operand-loccounter;i++)
             fputc(0,objfile);
            break;
           /* s-records */
           case 1 :
            for(i=0;i<(unsigned short)operand-loccounter;i++)
             flushhex();
            break;
           /* Intel Hex */
           case 2 :
            for(i=0;i<(unsigned short)operand-loccounter;i++)
             flushihex();
            break;
          }
	 }
         loccounter=operand;
         hexaddr=loccounter;
         break;
  case 14: /* SETDP */
         operand=scanexpr(0);
         if(unknown)error|=4;
         if(!(operand&255))operand=(unsigned short)operand>>8;
         if((unsigned)operand>255)operand=-1;
         dpsetting=operand;
         break;
  case 15: /* SET */
        operand=scanexpr(0);
        if(!lp)error|=32;
        else {
         if(lp->cat&1||lp->cat==6) {
          if(exprcat==2)lp->cat=3;
          else lp->cat=1;
          lp->value=operand;
         } else error|=8;
        }
        break;
   case 2: /* END */
   	terminate=1;
   	break;
   case 16: /* INCLUDE */
        skipspace();
        if(*srcptr=='"')srcptr++;
        for(i=0;i<FNLEN;i++) {
          if(*srcptr==0||*srcptr=='"')break;
          fname[i]=*srcptr++;
        }
        fname[i]=0;
        processfile(fname);
        codeptr=0;
        srcline[0]=0;
        break;
 }
}


void processline(void)
{
 struct symrecord * lp;
 struct oprecord * op;
 int co;
 char c;
 srcptr=srcline;
 oldlc=loccounter;
 error=0;
 unknown=0;certain=1;
 lp=0;
 codeptr=0;
 if(isalnum((unsigned char)*srcptr)) {
  scanname();lp=findsym(namebuf);
  if(*srcptr==':') srcptr++;
 }
 skipspace();
 if(isalnum((unsigned char)*srcptr)) {
  scanname();
  op=findop(namebuf);
  if(op) {
   if(op->cat!=13){
     setlabel(lp);
     generating=1;
   }
   co=op->code;
   switch(op->cat) {
   case 0:onebyte(co);break;
   case 1:twobyte(co);break;
   case 2:oneimm(co);break;
   case 3:lea(co);break;
   case 4:sbranch(co);break;
   case 5:lbranch(co);break;
   case 6:lbra(co);break;
   case 7:arith(co);break;
   case 8:darith(co);break;
   case 9:d2arith(co);break;
   case 10:oneaddr(co);break;
   case 11:tfrexg(co);break;
   case 12:pshpul(co);break;
   case 13:pseudoop(co,lp);
   }
   c=*srcptr;
   if(c!=' '&&*(srcptr-1)!=' '&&c!=0&&c!=';')error|=2;
  }
  else error|=0x8000;
 }else setlabel(lp);
 if(pass==2) {
  outbuffer();
  if(listing)outlist();
 }
 if(error)report();
 loccounter+=codeptr;
}

void suppressline(void)
{
 struct oprecord * op;
 srcptr=srcline;
 oldlc=loccounter;
 codeptr=0;
 if(isalnum((unsigned char)*srcptr)) {
  scanname();
  if(*srcptr==':')srcptr++;
 }
 skipspace();
 scanname();op=findop(namebuf);
 if(op && op->cat==13) {
  if(op->code==10) ifcount++;
  else if(op->code==3) {
   if(ifcount>0)ifcount--;else if((suppress==1)||(suppress==2))suppress=0;
  } else if(op->code==1) {
   if(ifcount==0 && suppress==2)suppress=0;
  }
 }
 if(pass==2&&listing)outlist();
}

void usage(char*nm)
{
  fprintf(stderr,"Usage: %s [-o objname] [-l listname] srcname\n",nm);
  exit(2);
}

void getoptions(int c,char*v[])
{
 int i=0;
 if(c==1)usage(v[0]);
 if(strcmp(v[1],"-o")==0) {
   if(c<4)usage(v[0]);
   strcpy(objname,v[2]);
   i+=2;
 }
 if(strcmp(v[i+1],"-s")==0) {
   if(c<4+i)usage(v[0]);
   strcpy(objname,v[i+2]);
   outmode=1;
   i+=2;
 }
 if(strcmp(v[i+1],"-x")==0) {
   if(c<4+i)usage(v[0]);
   strcpy(objname,v[i+2]);
   outmode=2;
   i+=2;
 }
 if(strcmp(v[i+1],"-l")==0) {
   if(c<4+i)usage(v[0]);
   strcpy(listname,v[2+i]);
   i+=2;
 }
 strcpy(srcname,v[1+i]);
 if(objname[0]==0) {
  for(i=0;i<=FNLEN;i++) {
   if(srcname[i]=='.')break;
   if(srcname[i]==0){strcpy(objname+i,".b");break;}
    objname[i]=srcname[i];
  }
 }
 listing=listname[0]!=0;
}

void expandline(void)
{
 int i=0,j=0,k,j1;
 for(i=0;i<128&&j<128;i++)
 {
  if(inpline[i]=='\n') {
    srcline[j]=0;break;
  }
  if(inpline[i]=='\t') {
    j1=j;
    for(k=0;k<8-j1%8 && j<128;k++)srcline[j++]=' ';
  }else srcline[j++]=inpline[i];
 }
 srcline[127]=0;
}

void processfile(char *name)
{
 char oldname[FNLEN+1];
 int oldno;
 FILE *srcfile;
 strcpy(oldname,curname);
 strcpy(curname,name);
 oldno=lineno;
 lineno=0;
 if((srcfile=fopen(name,"r"))==0) {
  fprintf(stderr,"Cannot open source file %s\n",name);
  exit(4);
 }
 while(!terminate&&fgets(inpline,128,srcfile)) {
   expandline();
   lineno++;
   srcptr=srcline;
   if(suppress)suppressline(); else processline();
 }
 fclose(srcfile);
 if(suppress) {
   fprintf(stderr,"improperly nested IF statements in %s",curname);
   errors++;
   suppress=0;
 }
 lineno=oldno;
 strcpy(curname,oldname);
}
