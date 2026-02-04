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
///  BIT CODER ////////////////////////////////////////////////////////////////////////////////////////////////

struct DirectCoder
{

private:

public:
   unsigned char *DCbuffer;
   U32 Bposition;

void Initialize(U32 memsize)
{
 free(DCbuffer);alloc(DCbuffer,memsize);
 Bposition=0;
}

void DecInit(FILE *src)
{
 U32 memsize=ReadSize();
 free(DCbuffer);alloc(DCbuffer,memsize);
 fread(DCbuffer,1,memsize,src);
 Bposition=0;
}

void putbits(unsigned data,unsigned bits)
{
  *(U64*) (DCbuffer+(Bposition>>3))|= ((U64)data *(U64)(1<< (Bposition&7)));
  Bposition += bits;
}

U64 getbits(unsigned bits)
{
  unsigned data =(U64)((*(U64*) (DCbuffer+(Bposition>>3) )>>(Bposition&7))&((1<<bits)-1));
  Bposition += bits;
  return data;
}

void Flush(FILE *dst)
{
WriteSize((Bposition>>3)+1);
fwrite(DCbuffer,1, (Bposition>>3)+1 ,dst);    compsize+=(Bposition>>3)+1;
}

};


