#pragma once
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
///  BIT RANGE CODER ////////////////////////////////////////////////////////////////////////////////////////////////

struct BitEncoder
{

private:



public:
   unsigned char *bufstart,*bufend,*ransptr;
   __int64 x1;
   __int64 x2,base;
   unsigned char cache;
   unsigned short cacheSize;


  void ShiftLow()
  {

    if ((x1^0xFF0000)>0xFFFF )
    {
      while(cacheSize)
      {
      *ransptr++=(cache+(x1>>24));
      cache = 0xFF;
      cacheSize--;
      }
      cache =(x1>>16);
    }
    cacheSize++;
    x1= (x1<<8)&0xFFFFFF;
  }

    int  ReturnCompressSize()
  {
  return ransptr-bufstart;
  }

 void EncFlush(FILE *dst)
{
   for(int i = 0; i <4; i++)ShiftLow();
   WriteSize(ReturnCompressSize());
   fwrite(bufstart,1, ReturnCompressSize() ,dst);    compsize+=ReturnCompressSize();
}

// Initializes a rANS decoder.
// Unlike the encoder, the decoder works forwards as you'd expect.
void DecInit(FILE *src)
{
    U32 memsize=ReadSize();
    free(bufstart);alloc(bufstart,memsize);
    fread(bufstart,1,memsize,src);
    ransptr=bufstart;
    x2=0xFFFFFF;base=0;for(int i = 0; i < 4; i++)base=(base<<8)+*ransptr++;
}


inline void EncodeBit(bool y,unsigned short &p)
{
    while (x2<0x10000){x2<<=8;ShiftLow();}

	U32 xmid= (x2*p)>>12;
	if (y)
{
	x2 = xmid;
	p+=(0xFFF-p)>>5;
    cxt+=cxt+1;
}
	else
{
	x1+=xmid;
	x2-=xmid;
	p-=p>>5;
    cxt+=cxt;
}


}


inline int DecodeBit(unsigned short &p)
{
	while(x2<0x10000){ x2<<=8;base=(base<<8)|*ransptr++;}
	U32 xmid= (x2*p)>>12;
	if (base<xmid)
{
	x2 = xmid;
	p+=(0xFFF-p)>>5;
	cxt+=cxt+1;
	return 1;
}
	else
{
	x2-=xmid;
	base-=xmid;
	p-=p>>5;
	cxt+=cxt;
	return 0;
};

}

void cwrite(unsigned int c,U16 *p)
{
cxt=1;
EncodeBit(bool(c&128),p[cxt]);
EncodeBit(bool(c&64 ),p[cxt]);
EncodeBit(bool(c&32 ),p[cxt]);
EncodeBit(bool(c&16 ),p[cxt]);
EncodeBit(bool(c&8  ),p[cxt]);
EncodeBit(bool(c&4  ),p[cxt]);
EncodeBit(bool(c&2  ),p[cxt]);
EncodeBit(bool(c&1  ),p[cxt]);
}

unsigned int cread(U16 *p)
{
cxt=1;
DecodeBit(p[cxt]);
DecodeBit(p[cxt]);
DecodeBit(p[cxt]);
DecodeBit(p[cxt]);
DecodeBit(p[cxt]);
DecodeBit(p[cxt]);
DecodeBit(p[cxt]);
DecodeBit(p[cxt]);
return cxt&255;
}

void EncodeSymbol(int symbol,U16 *prob,int numBit)
{
  cxt = 1;
  bool bit;
  for (int i = numBit-1; i >= 0;i--)
  {
    bit = (symbol >> i) & 1;
    EncodeBit(bit, prob[cxt]);
  }
}

int DecodeSymbol(U16 *prob,int numBit)
{
  cxt = 1;
  int i=numBit;
  while (i) DecodeBit(prob[cxt]),i--;
  return cxt&((1<<numBit)-1);
}

void Initizalize(U32 memsize)
{
free(bufstart);alloc(bufstart,memsize);ransptr=bufstart;x1=0;x2=0xFFFFFF;cacheSize=1;cache=0;
}

};

