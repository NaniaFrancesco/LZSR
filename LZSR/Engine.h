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
#pragma once
DirectCoder DCoder;
BitEncoder Coder;

void LZTreset(int buflen,int mode)
{
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  free(RLC);alloc(RLC, 1<<21);
  ///////////////////////////////////////////////////////////////////////
  free(bufx);alloc(bufx, buflen);
  for(int i = 0; i < 65536*3; i++) mediaprob[i]=2048;
  for(int i = 0; i < 1<<16; i++) char_prob[i]=2048;
  for(int i = 0; i < 1<<16; i++) scha_prob[i]=2048;
  for(int i = 0; i < 1<<16; i++) mcha_prob[i]=2048;
  for(int i = 0; i < 1<<16; i++) rllz_prob[i]=2048;
  for(int i = 0; i < 32+64*64; i++) distance_prob[i]=2048,rlc_prob[i]=2048,blkdist_prob[i]=2048;
  for(int i = 0; i < 256*256; i++) mtr[i]=2048,dt[i]=2048;
  for(int i = 0; i < 8*NumStates; i++) rt[i]=2048;


  RTS=state=0;
  REP[7]=REP[6]=REP[5]=REP[4]=REP[3]=REP[2]=REP[1]=REP[0]=1;
  bufxCurrPos=0;
}

__int64 GetArchiveSize(char *ArchiveName)
{
    OpenInputFile(ArchiveName);
    Arcsize=GetFileSize(inp);
    Arcposition=0;
    unsigned short lname=0,rproc=0,ok=0;
    __int64 TotalSize=0;

    while(Arcposition<Arcsize)
    {
    FileSeek(inp, Arcposition);
	fread(&compsize, 1, sizeof(compsize), inp);
	fread(&ARCTYPE   , 1, sizeof(ARCTYPE), inp);Solid=ARCTYPE&1;Creset=(ARCTYPE>>1)&1;ok=(ARCTYPE>>2)&1;
    lname=ReadSize();
    fread(temp , 1, lname, inp);
    if(ok)FileSize=ReadSize(),TotalSize+=FileSize,totalfiles+=ok;
    Arcposition+=compsize;

    }
    /// END DATASIZE ////////////////////////////////////////////////////////////////

    fclose(inp);
return TotalSize;
}
void InitEncoding(){compsize=0;totsize=0;}
void InitDecoding(){totsize=0; }


void StartEncoders(void)
{
  DCoder.Initialize(RBUF+0x20000);
  Coder.Initizalize(RBUF+0x20000);
}

void StartDecoders()
{
DCoder.DecInit(inp);
Coder.DecInit(inp);
}


void StopEncoders()
{
  DCoder.Flush(outp);
  Coder.EncFlush(outp);

}


void CloseEncoding(char *namefile,bool isfile)
{
if (Verbose && isfile) printStat(namefile,FileSize,compsize,1);/// PRINT DATA SIZE ///
FileSeek(outp, Arcposition);
fwrite(&compsize, 1,sizeof(compsize),outp);
fclose(inp);
fclose(outp);

}


////////////////////// END ENCODER ///////////////////////////////////////////////////////////



void WriteDistance(int position,U16 *p)
{

	int slot,i,ctx=0,extra,base,bits;
     slot=lenB(position+1,1);
     Coder.EncodeSymbol(slot,p,5);
     base=(1<<(slot-1));
     extra=32+((slot)<<6);
     bits=MAX(slot-6,0);
     Coder.EncodeSymbol(((position+1-base)>>bits)&63,p+extra,MIN(6,slot-1));
     DCoder.putbits((position+1-base)&((1<<bits)-1),bits);

}



int GetDistance(U16 *p)
{
 int slot=Coder.DecodeSymbol(p,5),position=0,extra,base,bits; // slot     //
            extra=32+((slot)<<6);
            base=(1<<(slot-1));
            bits=MAX(slot-6,0);
            position+=Coder.DecodeSymbol(p+extra,MIN(6,slot-1))<<bits;
            position+=DCoder.getbits(bits);


return position+base-1;
}



u32 LZSRDecode(u8 *dst)
{
	u32 groupNum=0;
	u32 progress=0;
	int i=0,j,k,v;
	u32 a,b,c,cpyPos;
	u32 dstInWndPos,*HT;
	u8 *wndDst,*wndCpy,*wndSrc;
	u32 lastCopySize,compos,CLZ=0,CTS=0,CTX=0;
	U16 lvl,slen=1;
	bool notEnd;
	PackType x;
	notEnd=true;
	dstInWndPos=bufxCurrPos;
	lastCopySize=0;
	u32 blksize=GetDistance(blkdist_prob);
    BAD=DCoder.getbits(1);
    ctx=char_prob;

	while(i<blksize)
	{


         if(Coder.DecodeBit(dt[state<<1]))
         {
            dst[i]=bufx[bufxCurrPos]=bufx[CTS];
            CTS=(CTS+1)&(MEM-1);
            RLC[CTX]=bufxCurrPos;
            CTX=(CTX*(3<<5)+bufx[bufxCurrPos])&0x1FFFFF;
            CLZ+=CLZ<6;
			state=((bufx[bufxCurrPos]<<7)+bufx[CTS])&0x1FFF|(CLZ+1)*0x2000;;
            ctx=mcha_prob+(bufx[CTS]<<8);
            bufxCurrPos=(bufxCurrPos+1)&(MEM-1);
			i++;
         }
          else
         {
            if( CLZ>1 && Coder.DecodeBit(dt[state<<1|1]))
           {
            dst[i]=bufx[bufxCurrPos]=bufx[RLC[CTX]];
            CTS=(RLC[CTX]+1)&(MEM-1);
            RLC[CTX]=bufxCurrPos;
            CTX=(CTX*(3<<5)+bufx[bufxCurrPos])&0x1FFFFF;
            CLZ=1;
			state=((bufx[bufxCurrPos]<<7)+bufx[CTS])&0x1FFF|(CLZ+1)*0x2000;;
            ctx=mcha_prob+(bufx[CTS]<<8);
            bufxCurrPos=(bufxCurrPos+1)&(MEM-1);
			i++;

            }
            else
            {
           (BAD)?dst[i]=bufx[bufxCurrPos]=DCoder.getbits(8): dst[i]=bufx[bufxCurrPos]=Coder.cread(ctx);
            RLC[CTX]=bufxCurrPos;
            CTX=(CTX*(3<<5)+bufx[bufxCurrPos])&0x1FFFFF;
            state=CTX&0x1FFF;
            CTS=RLC[CTX];
            ctx=char_prob+(bufx[bufxCurrPos]<<8);
            bufxCurrPos=(bufxCurrPos+1)&(MEM-1);
			i++;
			CLZ=0;

            }


	  }

	}


	return blksize;
}





void LZSRnormalBlock(u8 *src,u32 blksize)
{
    parsing newchoice,choice;
	u32 j,*HT;
	u32 progress=0,tprogress,bestgain,CLZ=0,CTX=0,tmpLen,compos,newpos;
	u32 DistIndex;
	u32 piece;

    u32 currBlockStartPos=bufxCurrPos;
    int lmin,CTS=0,newlen;
    U32 distance;
    ctx=char_prob;


    currBlockEndPos=bufxCurrPos+blksize;


    progress=0;
	while(progress<blksize)
	{

        choice.type=P_LIT;
        if(CTS &&  (CTS!=bufxCurrPos ) && CTS<MEM &&  bufx[CTS]==src[progress] )
        {choice.type=P_MAT;}
        else  if( CLZ>1 && RLC[CTX]<MEM && RLC[CTX]!=bufxCurrPos && bufx[RLC[CTX]]==src[progress])
        {choice.type=P_REP;}

        switch (choice.type)
		{
            case P_LIT:
                Coder.EncodeBit(0,dt[state<<1]);
                if(CLZ>1) Coder.EncodeBit(0,dt[state<<1|1]);
                if (BAD)
                DCoder.putbits(src[progress],8);
                else
                {
                Coder.cwrite(src[progress],ctx);
                }
                RLC[CTX]=bufxCurrPos;
                bufx[bufxCurrPos]=src[progress];
                CTX=(CTX*(3<<5)+bufx[bufxCurrPos])&0x1FFFFF;
                CTS=RLC[CTX];
                ctx=char_prob+(bufx[bufxCurrPos]<<8);
                state=CTX&0x1FFF;
                bufxCurrPos=(bufxCurrPos+1)&(MEM-1);
                progress++;
                CLZ=0;

			break;
			case P_REP:
            Coder.EncodeBit(0,dt[state<<1  ]);
            Coder.EncodeBit(1,dt[state<<1|1]);
            bufx[bufxCurrPos]=src[progress];
            CTS=(RLC[CTX]+1)&(MEM-1);
            RLC[CTX]=bufxCurrPos;
            CTX=(CTX*(3<<5)+bufx[bufxCurrPos])&0x1FFFFF;
            CLZ=1;
            state=((bufx[bufxCurrPos]<<7)+bufx[CTS])&0x1FFF|(CLZ+1)*0x2000;
            bufxCurrPos=(bufxCurrPos+1)&(MEM-1);
            progress++;
            ctx=mcha_prob+(bufx[CTS]<<8);

			break;
            case P_MAT:
            Coder.EncodeBit(1,dt[state<<1  ]);
            bufx[bufxCurrPos]=src[progress];
            RLC[CTX]=bufxCurrPos;
            CTX=(CTX*(3<<5)+bufx[bufxCurrPos])&0x1FFFFF;
            CTS=(CTS+1)&(MEM-1);
            CLZ+=CLZ<6;
            state=((bufx[bufxCurrPos]<<7)+bufx[CTS])&0x1FFF|(CLZ+1)*0x2000;;
            bufxCurrPos=(bufxCurrPos+1)&(MEM-1);
            progress++;
            ctx=mcha_prob+(bufx[CTS]<<8);

			break;
        }

    }




}



void EncodeNormal(u8 *src,u32 blksize)
{
	u32 progress=0;
	u32 currBlockSize;
    WriteDistance(blksize,blkdist_prob);
    BAD=(GetEntropy(src,blksize)>7900);
    DCoder.putbits(BAD,1);
    ctx=char_prob;
    LZSRnormalBlock(src,blksize);
	return;
}




int SearchLZType(u8 *src,u32 blksize,u32 type)
{

	u32 A,B,C;
	u32 i,j,cmpPos1,cmpPos2,tmpLen,remainLen,remainLen2;
	u32 bestmatch,CTX;
    uint32_t hashes[5];

	remainLen=blksize;
    bestmatch=0;

	for(i=3;i<blksize;i+=4)
	{

    tmpLen=1;

    for(j=0;j<5;j++)
	 {
            if (cmpPos1 && (cmpPos1<MEM) )
            {
			cmpPos2=i;
			remainLen=blksize-i;
			remainLen2=MIN(remainLen,MEM-cmpPos1);

                if ( *(u32*)&bufx[cmpPos1]==*(u32*)&src[cmpPos2])
                {
				cmpPos1+=4;
				cmpPos2+=4;
				tmpLen=4;
					while((tmpLen<remainLen2)&&(bufx[cmpPos1++]==src[cmpPos2++])) tmpLen++;
                    bestmatch+=tmpLen;
                }
            }

        }
                    if (bestmatch>blksize>>4)
					{
						return DT_NORMAL;
					}

        i+=tmpLen;
	}


return type;
}


void BMPencode(U8 *src, int blksize)  /// ENCODE BMP - //////////////////////////////
{
    U32 pos=0,line=Length*3,Height=(blksize/line)+1,delta,CTX,*HT;
    memcpy(bufx+bufxCurrPos,src,blksize);

    bufxCurrPos+=blksize;
    (bufxCurrPos>=MEM)?bufxCurrPos=0:0;
    U16 pred=0;
    int rc1,rc2,k;
    char error1=0;
    WriteDistance(blksize,blkdist_prob);
    WriteDistance(Length,blkdist_prob);
    WriteDistance(Width,blkdist_prob);
    delta=GetDelta(src,blksize,3);
    DCoder.putbits(delta,2);
    if (delta==1) delta_encode (src,blksize,1,3);
    if (delta==2) delta_encode (src,blksize,1,3),delta_encode (src,blksize,1,1);
                for (int k = 0; k<3; k++)
               {
                pos=k;
                while(pos<blksize)
                {
                rc1=(pos>2)?(src[pos-3]):(0);
                pred=((rc1+error1)&255)+(k<<8);
                Coder.cwrite(src[pos],mediaprob+(pred<<8));
                error1=(src[pos]-rc1)&127;
                pos+=3;
                }

                }
}

int  BMPdecode(U8 *src)  /// DECODE BMP - //////////////////////////////
{
    U32 blksize,
        rc1,
        rc2,
        len,
        delta,
        Height,
        CTX,
        *HT,
        pos;
    blksize=GetDistance(blkdist_prob);
    Length=GetDistance(blkdist_prob);
    Width=GetDistance(blkdist_prob);
    delta=DCoder.getbits(2);


     int line=Length*3,k;
     Height=(blksize/line)+1;
     char error1=0;
     U16 pred=0;


                for (int k = 0; k<3; k++)
               {
                pos=k;
                while(pos<blksize)
                {
                    rc1=(pos>2)?(tmp[pos-3]):(0);
                    pred=((rc1+error1)&255)+(k<<8);
                    tmp[pos]=Coder.cread(mediaprob+(pred<<8));
                    error1=(tmp[pos]-rc1)&127;
                    pos+=3;
                }

              }


   if (delta==1) delta_decode(tmp,blksize,1,3);
   if (delta==2) delta_decode(tmp,blksize,1,1),delta_decode(tmp,blksize,1,3);



    memcpy(src+bufxCurrPos,tmp,blksize);

    bufxCurrPos+=blksize;
    (bufxCurrPos>=MEM)?bufxCurrPos=0:0;


return blksize;

}


void WAVencode(U8 *src,U32 position, int blksize)  /// ENCODE JPG - //////////////////////////////
{
    U32 Hash,pos=0,ip,len,rip,REPI,rc1=0,rc2=0,BestGain,newGain,nnl,RMINREP,line=Length*3,delta,CTX,*HT;

  memcpy(bufx+bufxCurrPos,src,blksize);

    bufxCurrPos+=blksize;
   (bufxCurrPos>=MEM)?bufxCurrPos=0:0;


    WriteDistance(blksize,blkdist_prob);
    WriteDistance(deltarange,blkdist_prob);
    WriteDistance(chn,blkdist_prob);

    delta_encode (src,blksize,deltarange,chn);
    split (src,blksize, chn,deltarange);
    Wave_Transform(src,blksize,deltarange);


    U16 pred=0;
    int error=0;

              int PASS1=chn*deltarange;
              for (int k2 = deltarange-1; k2 >=0; k2--)
              for (int k1 = chn-1; k1 >=0; k1--)
              {
              pos=k1*deltarange+k2;
              while(pos<blksize)
              {
                (pos>PASS1)?rc1=(src[pos-PASS1]<<8):(rc1=0,pred=0);
                (k2<deltarange-1 && pos+1<blksize)?rc1=src[pos+1]<<8:0;
                Coder.cwrite(src[pos],mediaprob+rc1);
                pos+=PASS1;
              }

              }

  /// ////////////////////////////////////////////////////////////////////////////////////////////////////
}


int  WAVdecode(U8 *src,U32 position)  /// ENCODE JPG - //////////////////////////////
{
    U32 blksize,
        pos=0,
        c,
        rc1,
        rc2,
        len,
        delta,
        CTX,
        *HT,
        ip,
        cd;
    U16 pred=0;
    int error=0;

    blksize=GetDistance(blkdist_prob);
    deltarange=GetDistance(blkdist_prob);
    chn=GetDistance(blkdist_prob);



    int line=Length;

              int PASS1=chn*deltarange;
              for (int k2 = deltarange-1; k2 >=0; k2--)
              for (int k1 = chn-1; k1 >=0; k1--)
              {
              pos=k1*deltarange+k2;
              while(pos<blksize)
              {
                (pos>PASS1)?(rc1=tmp[pos-PASS1]<<8):(rc1=0);
                (k2<deltarange-1 && pos+1<blksize)?rc1=tmp[pos+1]<<8:0;
                tmp[pos]=Coder.cread(mediaprob+rc1);
                pos+=PASS1;
              }

              }


    Wave_Untransform(tmp,blksize,deltarange);
    invsplit (tmp,blksize, chn,deltarange);
    delta_decode(tmp,blksize,deltarange,chn);


    memcpy(src+bufxCurrPos,tmp,blksize);

    bufxCurrPos+=blksize;
    (bufxCurrPos>=MEM)?bufxCurrPos=0:0;



return blksize;

}



void CompressRLE(U8 *src, int blksize,int chnNum )
{
	 U32 pos,rpos,slot,c,len,m,little;
     WriteDistance(chnNum,blkdist_prob);
     EncodeDelta(src,blksize,chnNum);
     EncodeNormal(src,blksize);
	return;
}

int DecompressRLE(U8 *src)
{
	U32 c,pos;
    int Type, little,rpos;
    int blksize;
    int chnNum=GetDistance(blkdist_prob);
    blksize=LZSRDecode(tmp);
	DecodeDelta(tmp,blksize,chnNum);
	return blksize;
}
void CompressDLT(U8 *src, int blksize,int chnNum )
{
	 U32 pos,rpos,slot,c,len,m,little;
     WriteDistance(chnNum,blkdist_prob);
     delta_encode(src,blksize,1,chnNum);
     EncodeNormal(src,blksize);
	return;
}
int DecompressDLT(U8 *src)
{
	U32 c,pos;
    int Type, little,rpos;
    int blksize;
    int chnNum=GetDistance(blkdist_prob);
    blksize=LZSRDecode(tmp);
	delta_decode(tmp,blksize,1,chnNum);
	return blksize;
}
void CompressEXE(U8 *src, U32 position,int blksize)
{
	 U32 pos,rpos,slot,c,len,m,little;
     Forward_E89(src,blksize);
     EncodeNormal(src,blksize);// LZTencode(src,bufxCurrPos, blksize);
	 return;
}

int DecompressEXE(U8 *src,U32 position)
{
	U32 c,pos;
    int Type, little,rpos;
    int blksize;
    blksize=LZSRDecode(tmp);
	Inverse_E89(tmp,blksize);
	return blksize;
}

void CompressMemBlock(U8 *src,U32 tsize,U32 type)
{
  if (tsize==0) return;

     if (type==DT_AUDIO )
	{
		if (chn<1 || chn>8 || deltarange<1 || deltarange>4) type=DT_NORMAL;
	}
            if ((type==DT_RGB || type==DT_PGM || type==DT_AUDIO ) && SearchLZType(src,tsize,type)==DT_NORMAL)
			{
				type=DT_NORMAL;
			}

       if (type>=DT_DLT && type<DT_DLT+DLT_CHANNEL_MAX)
    {
    DCoder.putbits(1,3);
    U32 chnNum=DltIndex[type-DT_DLT];
    CompressRLE(src,tsize,chnNum);
    }
    else if (type>=DT_DELTA && type<DT_DELTA+DLT_CHANNEL_MAX)
    {
    DCoder.putbits(4,3);
    U32 chnNum=DltIndex[type-DT_DELTA];
    CompressDLT(src,tsize,chnNum);
    }
    else if (type==DT_AUDIO)
	{
    DCoder.putbits(3,3);
    WAVencode(src,bufxCurrPos,tsize);
	}
    else if (type==DT_RGB && chn && deltarange && (Length>0 && Length<65536 ) && (Width>0 && Width<65536 ) )
	{
    DCoder.putbits(2,3);
	BMPencode(src,tsize);
	}
    else if(type==DT_EXE)
	{
    DCoder.putbits(5,3);
    CompressEXE(src,bufxCurrPos, tsize);
	}
     else
	{
    DCoder.putbits(0,3);
    EncodeNormal(src, tsize);
	}

}


void LZSRCompress(U8 *src,U32 tsize,int fixedDataType)
{
	U32 lastType,thisType;
	U32 lastBegin,lastSize;
	U32 currBlockSize;
	U32 progress;

	lastBegin=lastSize=0;
	lastType=fixedDataType;

	progress=0;


    currBlockSize=MIN(MinBlockSize,tsize-progress);
   // thisType=m_analyzer.analyze(src,size);
   // if (thisType==DT_RGB ) currBlockSize=size,exit(1);
    StartEncoders();
	while (progress<tsize)
	{

        thisType=ANL.analyze(src+progress,currBlockSize);


	    if (thisType==DT_AUDIO)
        {
		if (chn<1 || chn>8 || deltarange<1 || deltarange>4) thisType=DT_NORMAL;
        }

        if (thisType>=DT_DLT && thisType<DT_DLT+DLT_CHANNEL_MAX)
		{
			if (lastType!=thisType)
			{
				CompressMemBlock(src+lastBegin,lastSize,lastType);
			}


				if (lastType==thisType)
				{
					lastSize+=currBlockSize;
				}
				else
				{
					lastBegin=progress;
					lastSize=currBlockSize;
					lastType=thisType;
				}


		}
		else if (thisType==DT_RGB  && (Length>1 && Length<65536 ) && (Width>1 && Width<65536) )
		{

            if (lastType!=thisType)
			{
				CompressMemBlock(src+lastBegin,lastSize,lastType);
			}


				if (lastType==thisType)
				{
					lastSize+=currBlockSize;
				}
				else
				{
					lastBegin=progress;
					lastSize=currBlockSize;
					lastType=thisType;
				}
				LastRLEchn=0;


		}
		else if (thisType==DT_PGM  && (Length>0 && Length<65536 ) && (Width>1 && Width<65536) )
		{

            if (lastType!=thisType)
			{
				CompressMemBlock(src+lastBegin,lastSize,lastType);
			}

				if (lastType==thisType)
				{
					lastSize+=currBlockSize;
				}
				else
				{
					lastBegin=progress;
					lastSize=currBlockSize;
					lastType=thisType;
				}
				LastRLEchn=0;


		}
            else if (thisType==DT_AUDIO )
		{

           	  if (lastType!=thisType)
			{
				CompressMemBlock(src+lastBegin,lastSize,lastType);
			}


				if (lastType==thisType)
				{
					lastSize+=currBlockSize;
				}
				else
				{
					lastBegin=progress;
					lastSize=currBlockSize;
					lastType=thisType;
				}
				LastRLEchn=0;

		}
		 else if (thisType>=DT_DELTA && thisType<DT_DELTA+DLT_CHANNEL_MAX)
		{

           	 if (lastType!=thisType)
			{
				CompressMemBlock(src+lastBegin,lastSize,lastType);
			}


				if (lastType==thisType)
				{
					lastSize+=currBlockSize;
				}
				else
				{
					lastBegin=progress;
					lastSize=currBlockSize;
					lastType=thisType;
				}
		}
		else
		{

            if (lastType!=thisType)
			{
				CompressMemBlock(src+lastBegin,lastSize,lastType);
			}

				if (lastType==thisType)
				{
					lastSize+=currBlockSize;
				}
				else
				{
					lastBegin=progress;
					lastSize=currBlockSize;
					lastType=thisType;
				}

				LastRLEchn=0;

		}
		progress+=currBlockSize;
		totscan+=currBlockSize;
		currBlockSize=MIN(MinBlockSize,tsize-progress);
	}
	if (lastBegin<tsize) CompressMemBlock(src+lastBegin,lastSize,lastType);
	StopEncoders();
}



void CMencode(char* namefile,char *single,char *directory,U8 IsFileType)
{
    ///VARIABLES//////////////////////////////////////////////////////////////////////////////////////////////////////
    int type;
    U32 CRCKEY;

    /// FILE CREATION /////////////////////////////////////////////////////////////////////////////////////////////
    if (IsFileType) OpenInputFile(namefile);
    OpenOutputFile(Aname);

    /// END FILE CREATION /////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////// MEMORY CONTROL /////////////////////////////////////////////////////////////////////////////////
	Arcposition=GetFileSize(outp);        // GET SIZE OF ARCHIVE
    FileSeek(outp, Arcposition); // SEEK TO END OF ARCHIVE
    FileSize = Getfilesize(namefile);            // GET FILE SIZE
    ///   /////////////////////////////////////////////////////////////////////////

    if (!Solid)
    {
         LZTreset(MEM+0x100,1);
         Creset=1;
    }
	else
    {
          if (!Cpsctl)
		 {
         LZTreset(MEM+0x100,1);
         Cpsctl=1;
         Creset=1;
         }
         else
         Creset=0;
    }

   CprFiles++;
    compsize=0;totsize=0;
    unsigned short lname;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    InitEncoding();
	/// /////////////////////////////////// WRITE HEADER /////////////////////////////////////////////////////////////////////////////////
    fwrite(&compsize, 1,sizeof(compsize),outp);                              compsize+=sizeof(compsize);
	ARCTYPE=Creset|IsFileType<<1|MXB<<2; fwrite(&ARCTYPE, 1,sizeof(ARCTYPE), outp);                                   compsize+=sizeof(ARCTYPE);
    memset(temp,0,MAX_PATH);
    memcpy(temp,directory,strlen(directory));
    memcpy(temp+strlen(directory),single,strlen(single));//printf("NAME= %s \n",temp);
    lname=strlen(temp)+1;WriteSize(lname);
    fwrite(temp, 1,lname, outp);                                        compsize+=lname;
    /// /////////////////////////////////// WRITE HEADER /////////////////////////////////////////////////////////////////////////////////
   char newdir[MAX_PATH];
   FILETIME time;

if (IsFileType)
{
WriteSize(FileSize);
type=-1;
CRCKEY=0;

while (totsize<FileSize) //////////////// While /////////////////////////////////////////////////////////////
{
  piece=MIN(RBUF,MEM-bufxCurrPos);
  piece=MIN(piece,FileSize-totsize);
  fread(tmp,1, piece,inp);
  CRCKEY=Integrity_control(CRCKEY,tmp,piece);
  totscan=0;

  if (type==-1) type=ANL.analyzeHeader(tmp,piece);

   LZSRCompress(tmp,piece,type);



  totsize+=piece;
  addedsize+=piece;
  (bufxCurrPos>=MEM)?bufxCurrPos=0:0;
  if (Verbose) printStat(namefile,totsize,compsize,0);
 // printf(" %s %I64d  -> %I64d  ratio %f\r",namefile,totsize,compsize,(float)compsize/(totsize));

}/// End While totsize /////////////////////////////////////////////////////////////////////
  fwrite(&CRCKEY,1,sizeof(CRCKEY),outp);compsize+=sizeof(CRCKEY);
    time=GetFileDateTime(namefile); fwrite(&time,1,  sizeof(time),outp);compsize+=sizeof(time);
}
else
   {    // DIR TIME
         GetFullPath(namefile);
        for (int i=3; i<strlen(part)+1;i++)
        {
            if (part[i]==47 || part[i]==92)
            {
            memset(newdir,0,MAX_PATH);
            memcpy(newdir,part,i+1);
            time=GetFileDateTime(newdir);
            fwrite(&time, 1, sizeof(time),outp);compsize+=sizeof(time);
            }
        }
    }
    /// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CloseEncoding(namefile,IsFileType);/// FLUSH ////////
fclose(inp);
fclose(outp);
///  ////////////////


}

static void CMdecode(char *namefile)
{

int c,pos,newpiece,dblock,sblock;
unsigned short lname=0;
U8 IsFileType;
U32 CRCKEY,CRCVER;
/// /////////////////////////////////// READ HEADER /////////////////////////////////////////////////////////////////////////////////
fread(&compsize, 1, sizeof(compsize), inp);
fread(&ARCTYPE   , 1, sizeof(ARCTYPE), inp);Creset=(ARCTYPE&1);IsFileType=(ARCTYPE>>1)&1;MEM=1<<(((ARCTYPE>>2)&7)+25);
lname=ReadSize();
fread(temp , 1, lname, inp);
Getpath(temp);
sprintf( from, "%s\\%s", Destination,part);
sprintf( part, "%s\\%s", Destination,temp);
if (!IsFileType) strcpy(from,part);

/// /////////////////////////////////// READ HEADER /////////////////////////////////////////////////////////////////////////////////
if (NWFile) CreateDir(from);
 FILETIME time,ftime;
    char newdir[MAX_PATH];
    char finaldir[MAX_PATH];


//
if (!IsFileType) goto finalize;
FileSize    =ReadSize();//ReadFile(input,&size,  sizeof(size), &dwRead, NULL);
//MEM     =ReadSize();//fread(&MEM, 1, sizeof(MEM), inp);//ReadFile(input,&MEM,  sizeof(MEM), &dwRead, NULL);
  /// MEMORY /////////////////////////////////////////////////////////////////////////////////////////////////////
     if (Creset) // ONLY FOR CRESET COMMAND
    {
         LZTreset(MEM+0x100,0);
    }

    if (NWFile)
    {
    OpenOutputFile(part);
	}


if (FileSize < 0) fprintf(stderr, "size error\n"), exit(1);


/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InitDecoding();



U8 TENC;
CRCKEY=0;
while (totsize<FileSize)
{
dblock=MIN(RBUF,MEM-bufxCurrPos);
dblock=MIN(dblock,FileSize-totsize);
StartDecoders();sblock=0;
while(sblock<dblock)
{
TENC=DCoder.getbits(3);
if (TENC==0 ){piece=LZSRDecode(tmp);}
if (TENC==1 ){piece=DecompressRLE(bufx);}
if (TENC==2 ){piece=BMPdecode(bufx);}
if (TENC==3 ){piece=WAVdecode(bufx,bufxCurrPos);}
if (TENC==4 ){piece=DecompressDLT(bufx);}
if (TENC==5 ){piece=DecompressEXE(bufx,0);}
if (NWFile) FileSeek(outp, totsize),fwrite(&tmp,1, piece,outp);
CRCKEY=Integrity_control(CRCKEY,tmp,piece);
sblock+=piece;
totsize+=piece;
(bufxCurrPos>=MEM)?bufxCurrPos=0:0;
}

}

finalize:

if (NWFile)
{
  fclose(outp); /// End Decoding file
    if (IsFileType)
    {
    // FILETIME

    fread(&CRCVER, 1, sizeof(CRCVER), inp);
    if (CRCKEY!=CRCVER ) {ArchiveCorrupt=1;printf( "error %s \n",part);}
    if (Verbose) printf( " %s Integrity OK!\n",part);
    fread(&time, 1, sizeof(time), inp),SetFileDateTime(part, time,1);
    }
    else
    {
        // DIR TIME
        for (int i=0;i<strlen(temp)+1;i++)
        {
            if (temp[i]==47 || temp[i]==92)
            {
            memset(newdir,0,MAX_PATH);
            memcpy(newdir,temp,i+1);
            sprintf(finaldir, "%s\\%s",Destination,newdir);
            Clearpath(finaldir);
         //   printf( " %s \n",finaldir);
            //  CreateDir(finaldir);
            fread(&ftime, 1, sizeof(ftime), inp);
            SetDirDateTime(finaldir,ftime,0);
        }
        }

    }

}
else
{
    // only verify
    if (NWVerify && IsFileType)
    {
    fread(&CRCVER, 1, sizeof(CRCVER), inp);
    if (CRCKEY!=CRCVER ) {ArchiveCorrupt=1;}
    if (Verbose) printf( " %s Integrity OK!\n",part);
    }

}



}
