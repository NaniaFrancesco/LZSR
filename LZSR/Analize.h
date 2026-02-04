
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
/// ///////////////////////////////////////////////////////////////////////////////////////////////
U32 tx2;
void Testbit(bool y,U16 &p)
{
	U32 xmid=(tx2>>12)*p;
	if (y)
	tx2 = xmid;
	else
	tx2-=xmid;
}


unsigned short tct[65536*3];
unsigned long trc,tcxt;

void tencoden(bool y)
{
  int txmid=(tx2>>10)*tct[trc+tcxt];
 (y)?(tx2=txmid,tct[trc+tcxt]+=(1024-tct[trc+tcxt])>>4):(tx2-=txmid,tct[trc+tcxt]-=(tct[trc+tcxt]>>4));

(tcxt+=tcxt)+=y;
while(tx2<0x1000000)// Shift equal MSB's out
{
testcons++;
tx2<<=8;
}
}
void twrite(unsigned int c)
{
tcxt=1;
tencoden(c&128);
tencoden(c&64);
tencoden(c&32);
tencoden(c&16);
tencoden(c&8);
tencoden(c&4);
tencoden(c&2);
tencoden(c&1);
}

int IsMedia(U8 *src,int blksize)
{
    U32 TestBest=0,Delta=0;
    U32 DataType=DT_NORMAL;
    /// NO RLE DELTA ///////////////////////////////////////
	testcons=GetEntropy(src,blksize);
    TestBest=testcons;

    for(int ch=0; ch<4; ch++)
    {
    memcpy(ServiceBuffer,src,blksize);delta_encode(ServiceBuffer,blksize,1,DltIndex[ch]);
    testcons=GetEntropy(ServiceBuffer,blksize);
    if (testcons<TestBest*0.925) return 1;
    }

    ////////////////////////////////////////

    return 0;
}

int  IsEXE(U8 *src, int len)
{
  int e8=0, exe=0, obj=0;
  for (BYTE *p=src; p+4<src+len; p++)
  {
    if (*p == 0xE8 || *p == 0xE9)
    {
      e8++;
      if (p[4]==0xFF)
        exe++;
      if (p[4]==0)
        obj++;
    }
  }

  //const char *s = double(e8)/len >= 0.002   &&   double(exe+obj)/e8 >= 0.20?
  //                (exe/double(exe+obj) >= 0.20?  "exe" : "obj") : "";

  return (double(e8)/len >= 0.002   &&   double(exe+obj)/e8 >= 0.25  &&   double(exe)/e8 >= 0.01) ?  1 : 0;

}


int GetRLE(U8 *src,int blksize)
{
    U32 TestBest=0,Delta=0;
    U32 DataType=0;

    /// NO RLE DELTA ///////////////////////////////////////
	testcons=GetEntropy(src,blksize);
    TestBest=testcons;

    /// //////////////////////////////////////////////////////
    /// RLE DELTA TEST ///////////////////////////////////////////////

    for(int ch=0; ch<DLT_CHANNEL_MAX; ch++)
    {
    memcpy(ServiceBuffer,src,blksize);EncodeDelta(ServiceBuffer,blksize,DltIndex[ch]);
    testcons=GetEntropy(ServiceBuffer,blksize);
    if (testcons<TestBest*0.990 && LastRLEchn==DltIndex[ch]) {DataType=DltIndex[ch];TestBest=testcons;break;}
    if (testcons<TestBest*0.901) {DataType=DltIndex[ch];TestBest=testcons;LastRLEchn=DataType;break;}
    }
    ////////////////////////////////////////
    if(DataType)
    {
      return DT_DLT+(DataType-1);
    }

    return DT_NORMAL;
}

int GetDelta(U8 *src,int blksize, int tsm)
{

    int Deltac=0,pos,rc1,error;
    int TresN=0,rchn=chn,rdeltarange=deltarange,line=Length*tsm;

    /// NO DELTA - NO RGB////////////////////////////////////////
    testcons=GetEntropy(src,blksize);
    TresN=testcons;


    /// DELTA 3 ///////////////////////////////////////////////

    memcpy(ServiceBuffer,src,blksize);
    delta_encode(ServiceBuffer,blksize,1,3);
    testcons=GetEntropy(ServiceBuffer,blksize);
    if (testcons<TresN*1.05) Deltac=1,TresN=testcons;


    /// //////////////////////////////////////////////////////
    chn=tsm;deltarange=1;
    memcpy(ServiceBuffer,src,blksize);
    delta_encode(ServiceBuffer,blksize,1,3);
    delta_encode(ServiceBuffer,blksize,1,1);
    testcons=GetEntropy(ServiceBuffer,blksize);

    if (testcons<TresN*1.03) Deltac=2,TresN=testcons;


  return Deltac;
}

class Analyzer
{
public:
	Analyzer();
	~Analyzer();
	U32 lastScanType;
	U32 Entropia;
	U32 analyze(U8* src,U32 size);
	U32 analyzeHeader(U8 *src,U32 size);
    int lastsize;

private:

	int GetChnIdx(U8 *src,U32 size);

};



Analyzer::Analyzer() : lastScanType(0), Entropia(0), lastsize(0)
{

}


int Analyzer::GetChnIdx(U8 *src,U32 tsize)
{

    u32 sameDist[DLT_CHANNEL_MAX]={0},succValue[DLT_CHANNEL_MAX]={0};
	u32 progress,succChar,maxSame,maxSucc,minSame,minSucc,bestChnNum;

	progress=0;
	succChar=0;
	while(progress<tsize-16)
	{
			sameDist[0]+=(src[progress]==src[progress+1]);
			sameDist[1]+=(src[progress]==src[progress+2]);
			sameDist[2]+=(src[progress]==src[progress+3]);
			sameDist[3]+=(src[progress]==src[progress+4]);
			sameDist[4]+=(src[progress]==src[progress+5]);
			sameDist[5]+=(src[progress]==src[progress+6]);
			sameDist[6]+=(src[progress]==src[progress+7]);
			sameDist[7]+=(src[progress]==src[progress+8]);
			succValue[0]+=abs((signed)src[progress]-(signed)src[progress+1]);
			succValue[1]+=abs((signed)src[progress]-(signed)src[progress+2]);
			succValue[2]+=abs((signed)src[progress]-(signed)src[progress+3]);
			succValue[3]+=abs((signed)src[progress]-(signed)src[progress+4]);
			succValue[4]+=abs((signed)src[progress]-(signed)src[progress+5]);
			succValue[5]+=abs((signed)src[progress]-(signed)src[progress+6]);
			succValue[6]+=abs((signed)src[progress]-(signed)src[progress+7]);
			succValue[7]+=abs((signed)src[progress]-(signed)src[progress+8]);

			progress++;
	}


	maxSame=minSame=sameDist[0];
	maxSucc=minSucc=succValue[0];
	bestChnNum=0;

	for (int i=0;i<DLT_CHANNEL_MAX;i++)
	{
		if (sameDist[i]<minSame) minSame=sameDist[i];
		if (sameDist[i]>maxSame) maxSame=sameDist[i];
		if (succValue[i]>maxSucc) maxSucc=succValue[i];
		if (succValue[i]<minSucc)
		{
			minSucc=succValue[i];
			bestChnNum=i;
		}
	}



            if (
            maxSucc>succValue[bestChnNum]
            || (sameDist[bestChnNum]>(minSame>>2)*3)
            || (sameDist[0]<0.1*tsize))
            {
            return bestChnNum;
            }
            else
            return -1;

}


Analyzer::~Analyzer()
{

}


U32 Analyzer::analyzeHeader(U8 *src,U32 tsize)
{
    if (tsize<34) return lastScanType;

    lastScanType=DT_NORMAL;lastsize=0;firstType=DT_NORMAL;FullComp=0;

	if (*(U32*)(src)== 0x00035F3F ){lastsize=*(U32*)(src+12);FullComp=(FileSize-*(U32*)(src+12))<64;lastScanType=firstType=DT_HLP;return DT_HLP;} /// HLP//

    if (*(src)== 0x25 &&  *(src+1)== 0x50 && *(src+2)== 0x44 && *(src+3)== 0x46 ){lastsize=FileSize;FullComp=1;lastScanType=firstType=DT_PDF;return DT_PDF;} /// PDF//

    if (*(src)==66  && *(src+1)==77 && *(U16*)(src+28)==24 && *(U32*)(src+18)>7 && *(U32*)(src+18)<65536 && *(U32*)(src+22)>7 && *(U32*)(src+22)<65536  )///*BMP 24b
    {deltarange=1;chn=3;Length=*(U32*)(src+18);Width=*(U32*)(src+22);lastsize=*(U32*)(src+2);firstType=DT_RGB;lastScanType =DT_RGB;FullComp=abs(FileSize-*(U32*)(src+2))<64 ;return DT_RGB;}

    if (*(src)==80  && *(src+1)==54  && *(src+2)==10 && abs(PPMread(src,tsize)*3-FileSize)<64 && Length>7 && Length<65536 &&  Width>7 && Width<65536 )
    {deltarange=1;chn=3;lastsize=FileSize;lastScanType =DT_RGB;FullComp=1;firstType=DT_RGB;return DT_RGB;} ///PPM

    if (*(src)==77  && *(src+1)==90  && *(src+2)==144&& *(src+3)==0 && *(src+4)==3 ){lastScanType=firstType=DT_EXE;return DT_EXE;} /// EXE//

    if (*(src)==82  && *(src+1)==73 && *(src+2)==70 && *(src+3)==70  && *(src+8)==87  && *(src+9)==65 && *(src+10)==86 && *(src+11)==69)
    {
    deltarange=(*(U16*)(src+34)>>3);
    chn=*(U16*)(src+22);
    (deltarange==3)?(deltarange=1,chn*=3):(0);
    (deltarange==4)?(deltarange=2,chn*=2):(0);
    if (  deltarange<1 || deltarange>4 || chn<1 || chn>8  ) return DT_NORMAL;
    FullComp=(FileSize-*(U32*)(src+4))<64;
    lastsize=FileSize;
    firstType=DT_AUDIO;
    lastScanType =DT_AUDIO;
    return DT_AUDIO;
    } /// wave //


    if (*(src)==80 && *(src+1)==53  && abs(PPMread(src,tsize)-FileSize)<64 && Length>7 && Length<65536 &&  Width>7 && Width<65536 )
        {chn=1;deltarange=1;lastsize=FileSize;PPMread(src,tsize);FullComp=1;firstType=DT_PGM;lastScanType =DT_PGM;return DT_PGM;} ///PGM

    if (*(src)==66  && *(src+1)==77 && *(U16*)(src+28)==8 && *(U32*)(src+18)>7 && *(U32*)(src+18)<65536 && *(U32*)(src+22)>7 && *(U32*)(src+22)<65536 )
     {chn=1;deltarange=1;Length=*(U32*)(src+18);Width=*(U32*)(src+22);lastsize=*(U32*)(src+2);FullComp=(FileSize-*(U32*)(src+2))<64;firstType=DT_PGM;lastScanType =DT_PGM;return DT_PGM;}        ///*BMP  8b

	if (src[0]==0 && src[1]==0 &&src[3]==0)
	{
		if ( (src[2]==2 || src[2]==10) && (src[16]==24 || src[16]==32) && *(U16*)(src+12)>7 && *(U16*)(src+12)<65536 && *(U16*)(src+14)>7 && *(U16*)(src+14)<65536)
		{
			Length=*(U16*)(src+12);
			Width=*(U16*)(src+14);
			FullComp=abs(FileSize-(*(U16*)(src+12) *  *(U16*)(src+14) * (src[16]>>3) ))<64 ;
			lastsize=FileSize;
			lastScanType =DT_RGB;
			chn=3;
			deltarange=1;
			firstType=DT_RGB;
			return DT_RGB;
		}
	}


  return DT_NORMAL;
}

U32 tmpcount=0;
U32 lastType=0;


U32 Analyzer::analyze(U8 *src,U32 tsize)
{
  if (tsize<34 ) return lastScanType;

  if (lastsize>0) {lastsize-=tsize;return lastScanType;}

  int IsFileType=GetRLE(src,tsize);
  bool MediaData=(IsFileType>=DT_DLT && IsFileType<DT_DLT+DLT_CHANNEL_MAX) || (IsFileType>=DT_DELTA && IsFileType<DT_DELTA+DLT_CHANNEL_MAX);

  for (int i=0; MediaData  && i<tsize-34;i++)
  {
    if (
        *(src+i)==66  && *(src+1+i)==77 && *(U16*)(src+28+i)==24
        && (*(U32*)(src+18+i)>7 && *(U32*)(src+18+i)<65536
        &&  *(U32*)(src+22+i)>7 && *(U32*)(src+22+i)<65536 )
        && (FileSize-(totsize+totscan)>=*(U32*)(src+2+i))
        )
    {deltarange=1;chn=3;Length=*(U32*)(src+18+i);Width=*(U32*)(src+22+i);lastsize=*(U32*)(src+2+i);FullComp=1;lastScanType =DT_RGB;return lastScanType;}        ///*BMP 24b

    if (  *(src+i)==82  && *(src+1+i)==73 && *(src+2+i)==70 && *(src+3+i)==70  && *(src+8+i)==87  && *(src+9+i)==65 && *(src+10+i)==86 && *(src+11+i)==69 && (FileSize-(totsize+totscan))>=*(U32*)(src+4+i) )
    {
    deltarange=(*(U16*)(src+34+i)>>3);
    chn=*(U16*)(src+22+i);
    (deltarange==3)?(deltarange=1,chn*=3):(0);
    (deltarange==4)?(deltarange=2,chn*=2):(0);
    if ( ( deltarange<1 || deltarange>4 || chn<1 || chn>8)  ) continue;
    FullComp=1;
    lastsize=*(U32*)(src+4+i);
    lastScanType =DT_AUDIO;
    return lastScanType;
    } /// wave //

    if (  *(src+i)==80  && *(src+1+i)==53  && *(src+2+i)==10 && FileSize>=PPMread(src+i,tsize) && Length>7 && Length<65536 && Width>7 && Width<65536)
    {chn=1;deltarange=1;lastsize=PPMread(src+i,tsize);FullComp=1;lastScanType =DT_PGM;return lastScanType;}

    if ( *(src+i)==66  && *(src+1+i)==77  && *(U16*)(src+28+i)==8 && (*(U32*)(src+18+i)>7 && *(U32*)(src+18+i)<65536) && (*(U32*)(src+22+i)>7 && *(U32*)(src+22+i)<65536)  && (FileSize-(totsize+totscan))>=*(U32*)(src+2+i) )
    {chn=1;deltarange=1;Length=*(U32*)(src+18+i);Width=*(U32*)(src+22+i);lastsize=*(U32*)(src+2+i);FullComp=1 ;lastScanType =DT_PGM;return lastScanType;}        ///*BMP  8b

    if ( *(src+i)==80  && *(src+1+i)==54  && *(src+2+i)==10  && (FileSize-(totsize+totscan))>=PPMread(src+i,tsize)*3 && Length>7 && Length<65536 && Width>7 && Width<65536 )
    {deltarange=1;chn=3;lastsize=PPMread(src+i,tsize)*3;FullComp=1;lastScanType =DT_RGB;return lastScanType;} ///PPM
    if(GetChnIdx(src,tsize)>-1) return IsFileType;
  }


  lastsize=0;
  if (IsEXE(src, tsize)) {lastScanType=DT_EXE;return DT_EXE;}


return lastScanType;//DT_NORMAL;
}

Analyzer ANL;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
