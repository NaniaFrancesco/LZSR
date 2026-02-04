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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Archiver
{
  __int64 num, sum;
  char name[MAX_PATH];
  UI &ui;
  Archiver( UI &u):  num(0), sum(0), ui(u){}
  template<class T> void operator() (const T &k)
  {
   ui.Processing(k);
   strcpy(name, k.Name().c_str());
   FileSize=Getfilesize(name);
   CMencode(name,GetName(name),Getpath(name),1); /// SINGLE TASK ACTIVE
   ++num;
   sum += FileSize;
 //  totalsize+=size;
  }
  Archiver& Done() { ui.Done(num, sum); return *this; }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void Extractor (  char *ver)
  {

    OpenInputFile(Aname);
    Arcsize=GetFileSize(inp);
    Arcposition=0;FileSeek(inp,Arcposition);
    unsigned short lname=0,rproc=0;
    __int64 cpsize;

    while(Arcposition<Arcsize)
    {
	fread(&cpsize, 1, sizeof(cpsize), inp);
    lname=ReadSize();fread(temp , 1, lname, inp);
    FileSeek(inp,Arcposition);
    GetName(temp);
    lowercase(Oname);
    NWFile=wildcmp(ver,Oname); /// WRITE OUPUT FILE ////
    CMdecode(Aname);
    if (ArchiveCorrupt){printf(" ERROR:  Archive %s is corrupt! ",Aname);exit(1);}
    Arcposition+=cpsize; FileSeek(inp, Arcposition);
  }


    fclose(inp);
    printf("\n Archive %s is valid !\n",Aname);


  }

 void Lister ( const char *ver)
  {
    OpenInputFile(Aname);
    Arcsize=GetFileSize(inp);
    Arcposition=0;
	FileSeek(inp,Arcposition);
	U8 IsFileType;
    unsigned short lname=0;
      while(Arcposition<Arcsize)
      {
        fread(&compsize, 1, sizeof(compsize), inp);
        fread(&ARCTYPE   , 1, sizeof(ARCTYPE), inp);Creset=(ARCTYPE&1);IsFileType=(ARCTYPE>>1)&1;
        lname=ReadSize();
        fread(temp , 1, lname, inp);
      FileSeek(inp,Arcposition);

      GetName(temp);
      bool ok = wildcmp(ver,Oname);
      if (ok)
      {
      DprFiles++;
      printf("%s",temp);
      if (Creset==0)printf(" - solid");
      printf("\n");
      }
  Arcposition+=compsize;
  FileSeek(inp,Arcposition);
  }



fclose(inp );
fclose(outp);


  }

 void VerifyArchive( )
  {

    OpenInputFile(Aname);
    Arcsize=GetFileSize(inp);
    Arcposition=0;
    FileSeek(inp,Arcposition);
    unsigned short lname=0,rproc=0;
    __int64 cpsize;
    ArchiveCorrupt=0;
    while(Arcposition<Arcsize)
    {
	fread(&cpsize, 1, sizeof(cpsize), inp);
    NWFile=0;NWVerify=1; /// WRITE OUPUT FILE ////
    FileSeek(inp,Arcposition);
    CMdecode(Aname);
    if (ArchiveCorrupt){printf("ERROR: Archive %s is corrupt! ",Aname);exit(1);}
    Arcposition+=cpsize; FileSeek(inp, Arcposition);
    }

    fclose(inp);
     printf("\n Archive %s is valid !\n",Aname);


  }
