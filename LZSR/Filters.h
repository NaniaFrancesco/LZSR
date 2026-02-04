
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
/// FILTERS /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  unsigned int Ex0,Ex1;
  unsigned int Ei,Ek;
  U8 Ecs; // cache size, F8 - 5 bytes


  void init( void ) {
    Ecs = 0xFF;
    Ex0 = Ex1 = 0;
    Ei  = 0;
    Ek  = 5;
  }

  int cache_byte( int c ) {
    int d = Ecs&0x80 ? -1 : (U8)(Ex1);
	Ex1>>=8;
	Ex1|=(Ex0<<24);
	Ex0>>=8;
    Ex0|=(c <<24);
    Ecs<<=1; Ei++;
    return d;
  }

  unsigned int xswap( unsigned int Ex )
  {
    Ex<<=7;
    return (Ex>>24)|((U8)(Ex>>16)<<8)|((U8)(Ex>>8)<<16)|((U8)(Ex)<<(24-7));
  }

  unsigned int yswap(unsigned int Ex ) {
    Ex = ((U8)(Ex>>24)<<7)|((U8)(Ex>>16)<<8)|((U8)(Ex>>8)<<16)|(Ex<<24);
    return Ex>>7;
  }


  int Eforward( int c )
  {
	  unsigned int Ex;
    if( Ei>=Ek ) {
      if( (Ex1&0xFE000000)==0xE8000000 ) {
        Ek = Ei+4;
         Ex= Ex0 - 0xFF000000;
        if( Ex<0x02000000 ) {
          Ex = (Ex+Ei) & 0x01FFFFFF;
          Ex = xswap(Ex);
          Ex0 = Ex + 0xFF000000;
        }
      }
    }
    return cache_byte(c);
  }

  int inverse( int c ) {
	  unsigned int Ex;
    if( Ei>=Ek ) {
      if( (Ex1&0xFE000000)==0xE8000000 ) {
        Ek = Ei+4;
        Ex = Ex0 - 0xFF000000;
        if( Ex<0x02000000 ) {
          Ex = yswap(Ex);
          Ex = (Ex-Ei) & 0x01FFFFFF;
          Ex0 = Ex + 0xFF000000;
        }
      }
    }
    return cache_byte(c);
  }

  int Eflush() {
	  int d;
    if( Ecs!=0xFF ) {
      while( Ecs&0x80 ) cache_byte(0),++Ecs;
      d = cache_byte(0); ++Ecs;
      return d;
    } else {
      init();
      return -1;
    }
  }


void Forward_E89( U8 * inpbuf, unsigned int inplen ) {
  int i,j,c;
  init();
  for( i=0,j=0; i<inplen; i++ ) {
    c = Eforward( inpbuf[i] );
    if( c>=0 ) inpbuf[j++]=c;
  }
  while( (c=Eflush())>=0 ) inpbuf[j++]=c;
}

void Inverse_E89( U8* inpbuf, unsigned int inplen ) {

  int i,j,c;

  init();
  for( i=0,j=0; i<inplen; i++ ) {
    c = inverse( inpbuf[i] );
    if( c>=0 ) inpbuf[j++]=c;
  }
  while( (c=Eflush())>=0 ) inpbuf[j++]=c;
}
/////  BMP and WAVE DELTA ENCODE ///////////////////////
void delta_encode(unsigned char *ebuf,int length,int range,int nchl)
{

    U32 t[32]={0};
    U32 original[32]={0};
    int chl=0;
    for (int i=0; i+(range-1) <length; i+=range)
    {
        switch(range)
        {
        case 1:
        original[chl] = *(U8*)(ebuf+i);
        *(U8*)(ebuf+i)=(*(U8*)(ebuf+i)- t[chl]);
        break;
        case 2:
        original[chl] = *(U16*)(ebuf+i);
        *(U16*)(ebuf+i)=(*(U16*)(ebuf+i)- t[chl]);
        break;
		case 3:
        original[chl] = *(U24*)(ebuf+i);
        *(U24*)(ebuf+i)=(U24)(*(U24*)(ebuf+i)- t[chl]);
        break;
        case 4:
        original[chl] = *(U32*)(ebuf+i);
        *(U32*)(ebuf+i)=(*(U32*)(ebuf+i)- t[chl]);
        break;
        }
        t[chl] = original[chl];
        chl++;
        if (chl==nchl) chl=0;


    }
}

void delta_decode(unsigned char *ebuf,int length,int range,int nchl)
{

    unsigned int i;
    U32 t[32]={0};
    U32 original[32]={0};
    int channe=0;
    switch(range)
    {
        case 1:
        for (i=0; i <length; i++)
        {
        *(U8*)(ebuf+i)= (*(U8*)(ebuf+i)+ t[channe]);
        t[channe] = *(U8*)(ebuf+i);
        channe++;if (channe==nchl)channe=0;
        }
        break;

        case 2:
        for (i=0; i +1<length; i+=2)
        {
        *(U16*)(ebuf+i)= (*(U16*)(ebuf+i)+ t[channe]);
        t[channe] = *(U16*)(ebuf+i);
        channe++;if  (channe==nchl) channe=0;
        }
        break;

        case 3:
        for (i=0; i+2 <length; i+=3)
        {
        *(U24*)(ebuf+i)=(U24)(*(U24*)(ebuf+i)+ t[channe]);
        t[channe] = *(U24*)(ebuf+i);
        channe++;if  (channe==nchl) channe=0;
        }
        break;

        case 4:
        for (i=0; i+3 <length; i+=4)
        {
        *(U32*)(ebuf+i)= (*(U32*)(ebuf+i)+ t[channe]);
        t[channe] = *(U32*)(ebuf+i);
        channe++;if (channe==nchl) channe=0;
        }
        break;
    }

}




void EncodeDelta(U8 *src,U32 tsize,U32 chnNum)
{
	if (tsize<128) return;
    U32 dstPos,j,prevByte;
	U32 lastDelta;
    U8 *temp;
    alloc(temp,tsize);
	memcpy(temp,src,tsize);

	dstPos=0;
	prevByte=0;
	lastDelta=0;
	    for (int i=0;i<chnNum;i++)
		for(j=i;j<tsize;j+=chnNum)
		{
			src[dstPos++]=temp[j]-prevByte;
			prevByte=temp[j];
		}
  free(temp);
}



void DecodeDelta(U8 *src,U32 tsize,U32 chnNum)
{
	U32 dstPos,j,prevByte;
	U32 lastDelta;

	if (tsize<128)
		return;



	memcpy(ServiceBuffer,src,tsize);

	dstPos=0;
	prevByte=0;
	lastDelta=0;
	    for (int i=0;i<chnNum;i++)
		for(j=i;j<tsize;j+=chnNum)
		{
			src[j]=ServiceBuffer[dstPos++]+prevByte;
			prevByte=src[j];

		}
}



typedef unsigned __int64 uint64;

#define MAX_ORDER		18

typedef struct {
	int shift;
	int round;
	int error;
	int sum;
	short qm[MAX_ORDER+1];
	short dx[MAX_ORDER+1];
	short dl[MAX_ORDER+1];
} fltst;

__inline void memshl (short  *pA, short *pB,short  *pC, short *pD)
{
    for (int i = 0; i < MAX_ORDER-1; i++)
    {
	*pA++ = *pB++;*pC++ = *pD++;
	}
	*pA   = *pB;*pC   = *pD;
}

int get_predictor (fltst *fs,int range)
{
    switch( range)
    {
    case 1:
    return (char)(fs->sum >> fs->shift);
    case 2:
    return (short)(fs->sum >> fs->shift);
    default:
    return 0;
	}
}
void update_predictor (fltst *fs, short value,short pred)
{
	short *pA = fs->dl+MAX_ORDER;
	short *pB = fs->qm+MAX_ORDER;
	short *pM = fs->dx+MAX_ORDER;
	*(pM-0) = ((*(pA-1) >> 12) | 1)<<2;
	*(pM-1) = ((*(pA-2) >> 12) | 1)<<2;
	*(pM-2) = ((*(pA-3) >> 12) | 1)<<2;
	*(pM-3) = ((*(pA-4) >> 12) | 1)<<1;
	*(pM-4) = ((*(pA-5) >> 12) | 1)<<1;
	*(pM-5) = ((*(pA-6) >> 12) | 1)<<1;
	*(pM-6) = ((*(pA-7) >> 12) | 1);
	*(pM-7) = ((*(pA-8) >> 12) | 1);
    *pA = (short) value;
    fs->error = (short) pred;
	*(pA-1) = *(pA  ) - *(pA-1);
	memshl (fs->dl, fs->dl + 1,fs->dx, fs->dx + 1);
	pA = fs->dl;
	pB = fs->qm;
	pM = fs->dx;
    fs->sum = fs->round;
	int mtp=(abs(fs->error)<3)?0:(fs->error<0)?-1:1;
	for (int i = 0; i < MAX_ORDER; i++){fs->sum += *pA++ * (*pB += mtp* *pM++), pB++;}
}

void filter_init (fltst *fs, int shift)
{
	memset (fs, 0, sizeof(fltst));
	fs->shift = shift;
	fs->round = 1 << (shift - 1);
	fs->sum=0;
}

fltst fst[256];


void encoder_init( int nch, int range)
{
	int eset [3] = { 10, 10 };
	int i;

	for (i = 0; i < 256; i++)
    {
		filter_init(fst+i, eset[range-1]);

	}
}


void Wave_Transform(U8 *src,int blksize,int range)  /// ENCODE Wave //////////////////////////////
{

              int  pos=0;
    switch( range)
    {
            case 1:
            {
                char *p,tmp,prev; encoder_init(chn,range);
                for (int k1 = 0; k1 <chn; k1++)
                {
                    pos=k1;p=(char*)src+k1;
                    while(pos<blksize)
                    {
                    prev=*p;
                    tmp=get_predictor(fst,deltarange);
                    *p=(char)(*p-tmp);
                    update_predictor(fst,prev,*p);
                    pos+=chn;p+=chn;
                    }

                }
             }
                 break;

              case 2:
              {
                short *p,tmp,prev; encoder_init(chn,range);
                for (int k1 = 0; k1 <chn; k1++)
                {
                    pos=k1;p=(short*)src+k1;
                    while(pos<blksize>>1)
                    {
                    prev=*p;
                    tmp=get_predictor(fst,deltarange);
                    *p=(short)(*p-tmp);
                    update_predictor(fst,prev,*p);
                    pos+=chn;p+=chn;
                    }

                }
             }
                 break;


              case 4:
              {
                int *p,tmp,prev; encoder_init(chn,range);
                for (int k1 = 0; k1 <chn; k1++)
                {
                    pos=k1;p=(int*)src+k1;
                    while(pos<blksize>>2)
                    {
                    prev=*p;
                    tmp=(int)(get_predictor(fst,deltarange));
                    *p=(int)(*p-tmp);
                    update_predictor(fst,prev,*p);
                    pos+=chn;p+=chn;
                    }

                }

              }
              break;



    }





}

void Wave_Untransform(U8 *src,int blksize,int range)  /// ENCODE Wave //////////////////////////////
{

              int  pos=0;
    switch( range)
    {
            case 1:
            {
                char *p,tmp,prev; encoder_init(chn,range);
                for (int k1 = 0; k1 <chn; k1++)
                {
                    pos=k1;p=( char*)src+k1;
                    while(pos<blksize)
                    {
                    tmp=*p;
                    *p=(char)(tmp+get_predictor(fst,deltarange));
                    update_predictor(fst,*p,tmp);
                    pos+=chn;p+=chn;
                    }
                }
            }
                 break;
              case 2:
              {
                short *p,tmp,prev; encoder_init(chn,range);
                for (int k1 = 0; k1 <chn; k1++)
                {
                    pos=k1;p=( short*)src+k1;
                    while(pos<blksize>>1)
                    {
                    tmp=*p;
                    *p=(short)(tmp+get_predictor(fst,deltarange));
                    update_predictor(fst,*p,tmp);
                    pos+=chn;p+=chn;
                    }
                }
             }
                 break;


              case 4:
              {
                int *p,tmp,prev; encoder_init(chn,range);
                for (int k1 = 0; k1 <chn; k1++)
                {
                    pos=k1;p=( int*)src+k1;
                    while(pos<blksize>>2)
                    {
                    tmp=*p;
                    *p=(int)(tmp+get_predictor(fst,deltarange) );
                    update_predictor(fst,*p,tmp);
                    pos+=chn;p+=chn;
                    }
                }

              }
              break;



    }





}


void split (U8 *source, long blksize, long Channel,int range)
{
    switch( range)
    {
        case 1:
        {
        delta_encode (source,blksize,range,Channel);
        }
        break;
        case 2:
        {
        short *src=( short*)source;
        long	i, j, n;
            if (Channel > 1)
            for (i = 0, n = (Channel - 1); i < (blksize>>1)/Channel; i++)
            {
            for (j = 0; j < n; j++) src[i * Channel + j] = src[i * Channel + (j+1)] - src[i * Channel + j];
            src[i*Channel+n] = src[i*Channel+n] - (src[i*Channel+(n-1)]>>1);
            }
        }
        break;

         case 4:
        {
        int *src=( int*)source;
        long	i, j, n;
            if (Channel > 1)
            for (i = 0, n = (Channel - 1); i < (blksize>>2)/Channel; i++)
            {
            for (j = 0; j < n; j++) src[i * Channel + j] = src[i * Channel + (j+1)] - src[i * Channel + j];
            src[i*Channel+n] = src[i*Channel+n] - (src[i*Channel+(n-1)]>>1);
            }
        }
        break;
	}

}

void invsplit (U8 *source, long blksize, long Channel,int range)
{
    switch( range)
    {
        case 1:
        {
          delta_decode(source,blksize,range,Channel);
        }
        break;
        case 2:
        {
        short *src=( short*)source;
        long	i, j, n;
        if (Channel > 1)
        for (i = 0, n = (Channel - 1); i < (blksize>>1)/Channel; i++)
        {
        src[i * Channel + n] = src[i * Channel + n] + (src[i * Channel + (n-1)]>>1);
        for (j = n; j > 0; j--) src[i * Channel + j-1] = src[i * Channel + j] - src[i * Channel + j-1];
        }
        }
        break;
        case 4:
        {
        int *src=( int*)source;
        long	i, j, n;
        if (Channel > 1)
        for (i = 0, n = (Channel - 1); i < (blksize>>2)/Channel; i++)
        {
        src[i * Channel + n] = src[i * Channel + n] + (src[i * Channel + (n-1)]>>1);
        for (j = n; j > 0; j--) src[i * Channel + j-1] = src[i * Channel + j] - src[i * Channel + j-1];
        }
        }
        break;

    }

}


