/* LZSR Archiver Copyright (C) 2014-2026, Nania Francesco Antonio
 ******************************************************************************
 * Copyright (C) 2026  NANIA Francesco Antonio
 * * DUAL LICENSING:
 * This software is available under two different licenses:
 * * 1) GNU General Public License v3.0 (GPLv3)
 * For open-source projects and non-commercial testing.
 * See the LICENSE file in the project root for full terms.
 * * 2) Commercial License
 * For use in proprietary/commercial software where GPLv3 compliance
 * is not desired. This license offers dedicated support and removes
 * copyleft obligations.
 * * For commercial licensing inquiries, please contact: miadebora@hotmail.it
 *****************************************************************************/
#include <cstdio>
#include <malloc.h>
#include <shlobj.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <tchar.h>
#include <thread>
#include <windows.h>
#include <sys/types.h> //mode_t, size_t
#include <sys/stat.h> //stat()
#include <iostream>
#include <string>
#include <io.h>


#define DLT_CHANNEL_MAX 8
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
//////////////////////////////////////////////////////////////////////////////////////
/******Block Type*************/
//#define DT_NONE 0
#define DT_BAD 0
#define DT_EXE 1
#define DT_NORMAL 2
#define DT_FAST 3
#define DT_AUDIO 4
#define DT_RGB 5
#define DT_BIT 6
#define SIG_EOF 7
#define DT_TXT 8
#define DT_PGM 9
#define DT_HLP 10
#define DT_PDF 11
#define DT_DELTA 12
#define DT_DLT 64
/***********************************/
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef int i32;
typedef long long i64;
typedef unsigned long      int32;
typedef unsigned short int int16;
typedef unsigned char        int8;
typedef signed long        sint32;
typedef signed short int   sint16;
typedef signed char         sint8;
typedef unsigned char          U8;   // 8 bit type
typedef unsigned short        U16;  // 16 bit type
typedef unsigned long         U32;  // 32 bit type
typedef __int64               U64;  // 64 bit type
typedef unsigned __int8      uint8,  byte, BYTE;

FILE *inp,*outp;
/// LZA MEMORY BANK ///////////////////////////////////
const U32 RBUF=1<<24;
U8 tmp[RBUF+(1<<17)];
U8 ServiceBuffer[RBUF+(1<<17)];
U8 BAD;
U8 *CWBuffer;
U32 probToBitNum[65536];
U32 REP[8];
U8 ArchiveCorrupt,NWVerify;
u8 num_threads=1;
/// ///////////////////////////////////////////////////////
const U32 DltIndex[DLT_CHANNEL_MAX]={1,2,3,4,5,6,7,8};
const U32 KB=1024;
const U32 MB=1048576;
U32 MinBlockSize=1<<12;
const U32 WorkBlockSize=1<<16;
const int NumStates = 128;
U16 char_prob[1<<16];
U16 mcha_prob[1<<16];
U16 scha_prob[1<<16];
U16 mediaprob[65536*3];
U16 distance_prob[32+64*64];
U16 rllz_prob[1<<16];
U16 rlc_prob[32+64*64];
U16 blkdist_prob[32+64*64];
U16 dt[1<<16];
U16 rt[8*NumStates];
U16 mtr[256*256];
U16 *ctx;
U32 totscan;
U32 LastRLEchn;
U8 firstType;
U8 MINREP;
U8 ARCTYPE;
U8 *bufx,Delta,deltarange=1;
U32 rstate,state,Entropia;
U32 MEM,HashNum6,HashNum5,HashNum4,HashNum3,HashNum2;
U32 N,scritto,ascritto,rscritto,testcons,CprFiles=0,DprFiles;  // buffer size: 16m
U8 *tufx,MultiComp=0,Nmax,Solid=0,Cpsctl=0;
U32 maxsize,Length,Width;
U8 chn,Verbose=0,NWFile=0,Creset=0;
U32 MXB,MXH,MXM,cxt;
U32 RTS;
U32 *RLC;
char EName[MAX_PATH];/// Get executable name //////
__int64 FileSize,totsize,piece,compsize,compresize=0,totalsize=0,origsize,Bufposition[2],addedsize=0;
__int64 Arcposition,Arcsize,totalfiles;
char *filename,*foldername,totalname[8192],Aname[8192],Oname[MAX_PATH],Dfolder[MAX_PATH],temp[ MAX_PATH ],temp2[MAX_PATH],from[MAX_PATH],part[MAX_PATH],extens[8192],wildcard[8192],Destination[MAX_PATH];
u8 FullComp;
using namespace std;
U32 FileListNum;
U32 DirListNum;
vector<string> listDir;
u32 bufxCurrPos,currBlockEndPos,TRYL,GLEN;


class U24
{  // 24-bit unsigned int
U8 b0, b1, b2;  // Low, mid, high byte
public:
explicit U24(int x=0): b0(x), b1(x>>8), b2(x>>16) {}
operator int() const {return (((b2<<8)|b1)<<8)|b0;}
};

template <class T> void alloc(T*&p, int n)
{
p=(T*)calloc(n, sizeof(T));
if (!p) printf("No memory \n"),exit(1);
}


unsigned long lenB(u32 number, u32 base)
{
  unsigned long nbit;
  nbit=0;
  while(number>0)
  {
   number>>=base;
   nbit++;
  }
  return (nbit);
}

enum PackType
{
	P_LIT,
	P_MAT,
	P_REP,
	P_RP1,
	P_RLC
};

typedef struct
{
	PackType type;
	U32  len;
	U32  dist;
	U8 dtp;
	U32 price;
	u8 lmin;
} parsing;

////////////////////
    struct MatchData
	{
		u32 pos;
		u32 len;
		u32 mps;
		u8 type;
		u32 DST;
		u8 dtp;
		u32 lmin;
	};

	struct LZSRparser
	{
		u8 finalChoice;
		u32 finalLen;
		u32 fstate;
		u32 price;
        u32 bestmatch;
        bool dtp;
		u32 REP[6];

		struct
		{
			u32 fromPos;
			u32 choice;
			u32 fromLen;
		} backChoice;

		u8 choiceNum;
		MatchData  matchList[16];
	};

	LZSRparser *parser;



U32 GetEntropy(U8 *src,U32 tsize)
{
  float somm;
  int avgFreq,freq[256]={0};
  somm = 0.0;
  for(int i=0;i<tsize;i++){freq[src[i]]++;}
  // Ciclo del calcolo dell'entropia
  for(int i=0;i<256;i++)	// Per ogni codice possibile
  {
    if (freq[i] != 0)
            {
			double r=2;
			 double lg2=log(r);
			 r = (double) freq[i] / (double) tsize;
			 double lg1=log(r);
 	            somm += -r * (lg1/lg2);
 	        }

  }
  // Stampa finale dei risltati
 return (U32)(somm*1000);
}

/// compute ICH (Slicing-by-4 my algorithm)
U32 Integrity_control(U32 previousICH ,unsigned char* data, size_t length)
{
  U32  ICH = ~previousICH ; // same as previousCrc32 ^ 0xFFFFFFFF
  U8* currentChar = data;
  while (length--)ICH= (ICH*0xAAAA)^*currentChar++;
  return (u32)(ICH); // same as crc ^ 0xFFFFFFFF
}

///   its a core function in this tool
int SetFileDateTime(char *szFilePath,FILETIME ftAccessed,bool isfile)
{
  DWORD dwAttr;
  dwAttr = GetFileAttributes(szFilePath);


  // open the file/folder
  HANDLE hFile = CreateFile(szFilePath,
        GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

  if(hFile == INVALID_HANDLE_VALUE)
      return 1;


    BOOL bVal ;

    bVal=SetFileTime(hFile,NULL,NULL,&ftAccessed);

    if(!bVal)
    {
      printf("no apply");
    }
      CloseHandle(hFile);

  if(!bVal)
      return 2;
  return 0;
}



int SetDirDateTime(char *szFilePath,FILETIME ftAccessed,bool isfile)
{
  DWORD dwAttr;
  dwAttr = GetFileAttributes(szFilePath);
  // open the file/folder
  HANDLE hFile = CreateFile(szFilePath,
        GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);
  if(hFile == INVALID_HANDLE_VALUE)
      return 1;
    BOOL bVal ;

    bVal=SetFileTime(hFile,&ftAccessed,&ftAccessed,&ftAccessed);
    if(!bVal)
    {
      printf("no apply");
    }


      CloseHandle(hFile);


  if(!bVal)
      return 2;
  return 0;
}
FILETIME GetFileDateTime(char *szFilePath)
{
  DWORD dwAttr;
  dwAttr = GetFileAttributes(szFilePath);
            FILETIME ftCreated;
            FILETIME ftModified;
            FILETIME ftAccessed;

  // open the file/folder
  HANDLE hFile = CreateFile(szFilePath,
        GENERIC_READ,FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);




    bool bVal =GetFileTime(hFile,&ftCreated,&ftAccessed,&ftModified);
    if(!bVal)
    {
      printf("no apply");
    }


      CloseHandle(hFile);


  return ftModified;
}
/// SIMPLE FAST BIT ENGINE ////////////////////////////////////////////////////////////////////////////////////////

void WriteSize(__int64  fsize)
{

      __int64 value=fsize;
      __int64 vsub=128,sub=128,rsub=0;
      int byte=0,i;

      for (;;)
      {
      if (value<(__int64) sub)
      {
      break;
      }
      rsub=sub;
      vsub*=128;
      sub+=vsub;
      byte++;
      }

      value-=rsub;

      for (i=0;;)
      {
      if (i==byte)
      {
      putc(value,outp);compsize++;
      break;
      }
      putc((value&127)+128,outp);compsize++;
      value/=128;
      i++;
      }
}

__int64  ReadSize()
{
__int64  value=0,vsub=1,sub=0;
U8 obj;

      for (;;)
      {
      obj=getc(inp);
      if (obj<128)
      {
      value+=obj*vsub;
      break;
      }
      value+=(obj-128)*vsub;
      vsub=vsub*128;
      sub+=vsub;
      }
return value+sub;
}

void lowercase(char * str)
{
for(int i=0;str[i]!='\0';i++)
if(str[i]>='A'&&str[i]<='Z')str[i]+=32;
}

void Clearpath(char *obj)
{
char s,dest[MAX_PATH];
int cnt=0;
  for (unsigned int i=0; i<strlen(obj); i++)
  {
  if (i==0 &&  (*(obj+i)==47 ||*(obj+i)==92) ) continue; /// REMOVE  "\" FROM START
  if (*(obj+i)==47) *(obj+i)=92;/// TRANSFORM  "/" TO "\"
  if (*(obj+i)==46 && (*(obj+i+1)==46 || *(obj+i+1)==47 ||*(obj+i+1)==92) ) continue; ///  IF OBJ[i]="." and  OBJ[i+1]="\" continue
  if (*(obj+i)==92 && (*(obj+i+1)==47 ||*(obj+i+1)==92) ) continue;

  *(dest+cnt++)=*(obj+i);
  }
  *(dest+cnt)=0;
strcpy(obj,dest);
}

char* GetFullPath(char *obj)
{
char s;
int lex=0,detini=0;
for (int i=strlen(obj);i>=0; i--)
{
s=*(obj+i);
if (s==92 || s==47) break;
detini=i;
}
for (int i=0; i<detini; i++)
{
s=*(obj+i);
*(part+lex)=s;
lex++;
}
*(part+lex)=0;
return part;
}


bool Wildcard(char *obj)
{
char s;
bool yes=0;
for (unsigned int i=0; i<=strlen(obj)+1; i++)
{
s=*(obj+i);
if (s==42 || s==46) yes=1;
}
return yes;

}


char* GetNameArchive(char *obj)
{
char s,block=0;
int lex=0,detini;
if (*(obj+strlen(obj)-1)==92) block=2;
for (int i=strlen(obj)-block;i>=0; i--)
{
s=*(obj+i);
if (s==92 || s==47) break;
detini=i;
}
for (int i=detini; i<strlen(obj)-block+1; i++)
{
s=*(obj+i);
*(Oname+lex)=s;
lex++;
}
*(Oname+lex)=0;
return Oname;
}


char* GetName(char *obj)
{
char s;
int lex=0,detini;
for (int i=strlen(obj);i>=0; i--)
{
s=*(obj+i);
if (s==92 || s==47) break;
detini=i;
}
for (int i=detini; i<strlen(obj); i++)
{
s=*(obj+i);
*(Oname+lex)=s;
lex++;
}
*(Oname+lex)=0;
return Oname;
}


char * Getpath(char *obj)
{
char s;
int lex=0,detini=0,init=0,i;
for ( i=strlen(obj)-1;i>=0; i--)
{
s=*(obj+i);
if (s==92 || s==47) break;
detini=i;
}
if (*(obj+1)==58) init=3;
for ( i=init; i<detini; i++)
{
s=*(obj+i);
*(part+lex)=s;
lex++;
}
*(part+lex)=0;
return part;
}


char* RemoveWild(char *obj)
{
char s;
int lex=0,detini;
for (int i=0;i<strlen(obj); i++)
{
if (*(obj+i)=='*') {break;}
detini=i;
}
detini++;
memcpy(Oname,obj,detini);
*(Oname+detini)=0;
return Oname;
}



void printStat(char *namefile,__int64 srcsize,__int64 destsize,int mode)
{
  char partname[64];
  char stat[256];
  char perc[32];
  if (strlen(namefile)>32)
  memcpy(partname,namefile+(strlen(namefile)-32),32),partname[32]=0;
  else
  memcpy(partname,namefile,32),partname[32]=0;
   __int64 percentage=(__int64)(((addedsize+1)*100)/(totalsize+1));

  sprintf(perc," [%d%%]",percentage);
  sprintf(stat," add.. %s %I64d to %I64d                                                   ",partname,srcsize,destsize);
  memcpy(stat+79-strlen(perc),perc,strlen(perc));stat[79]=0;

  if (mode)
  printf("%s\n",stat);
  else
  printf("%s\r",stat);
}
