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
#include "Common.h""
#include "DirectCoder.h"
#include "BitEncoder.h"
#include "FileOp.h"
#include "Filters.h"
#include "Analize.h"
#include "Engine.h"
#include "Archive.h"

void progname()
{
printf("LZSR archiver v.0.3d.  Warning is demo version.                                  \n");
printf("Copyright (C) 2014-2026 NANIA Francesco A.                                       \n");
return;
}

void endx()
{
progname();
printf("Usage: LZSR <command> -<switch ..> <archive> <files...> <in path> <out path>    \n");
printf( "<Commands>                                                                     \n");
printf( " a                 Add files to archive                                        \n");
printf( " x                 Extract files in destination path                           \n");
printf( " v                 Verify all files in Archive (CRC32)                         \n");
printf( " l                 List archive                                                \n");
printf( "                                                                               \n");
printf( "<Switches>                                                                     \n");
printf( " r                Recurse subdirectories                                       \n");
printf( " s                Solid Mode ON [defaul OFF]                                   \n");
printf( " v                Verbose Mode                                                 \n");
printf( " b                [default -b1] set LZbuffer memory     -b0 [32MB]...-b5[1GB]  \n");
printf( "                                                                               \n");
printf( "Examples of compression :                                                      \n");
printf( "lzsr    a c:\\archive c:\\infile                                               \n");
printf( "lzsr    a -r  c:\\archive c:\\path                                             \n");
printf( "lzsr    a -r -b5 archive path                                             \n");
printf( "lzsr    a -r archive path\\*.bmp                                               \n");
printf( "Example of decompression :                                                     \n");
printf( "lzsr    x  c:\\archive c:\\outpath                                             \n");
printf( "lzsr    x  archive outpath                                                     \n");
printf( "lzsr    x archive outpath\\*.bmp                                               \n");
printf( "                                                                               \n");


    exit(1);
}
//////////////////////////// main ////////////////////////////

int main(int argc, char* argv[])
{

GetModuleFileName( NULL,EName, MAX_PATH );
SYSTEM_INFO sysinfo;
GetSystemInfo( &sysinfo );
int numCPU = sysinfo.dwNumberOfProcessors;
strcpy(EName,GetNameArchive(EName));

unsigned int decompress = 0;


GetCurrentDirectory(MAX_PATH,Dfolder);



if ( argv[1] && argv[1][0] == 'a' )  // ADDING /////
{
///Default Settings ///////////////////
MEM=1<<26;MXB=1;MinBlockSize=1<<12;Solid=0;
/// /////////////////////////////////
    argv++,argc--;
    noc:
    while( argv[1] && argv[1][0] == '-' )
    {
        char *p = (char*)argv[1]+1;
        while( *p )
        {
            switch( tolower(*p) )
            {

                /// SCELTA DELLE OPZIONI DI COMPRESSIONE ////////////////////////
                case 'b':
                if (*(p+1)=='0'){MEM=1<<25;MXB=0;argv++,argc--;goto noc;}
                if (*(p+1)=='1'){MEM=1<<26;MXB=1;argv++,argc--;goto noc;}
				if (*(p+1)=='2'){MEM=1<<27;MXB=2;argv++,argc--;goto noc;}
                if (*(p+1)=='3'){MEM=1<<28;MXB=3;argv++,argc--;goto noc;}
				if (*(p+1)=='4'){MEM=1<<29;MXB=4;argv++,argc--;goto noc;}
                if (*(p+1)=='5'){MEM=1<<30;MXB=5;argv++,argc--;goto noc;}
                break;
				case 's': /// RECURSIVE ARCHIVE COMPRESSION FOR FOLDER //////
                {

                 Solid=1;argv++,argc--;goto noc;
                }
                break;
                ////////////////////////////////////////////////
                case 'r': /// RECURSIVE ARCHIVE COMPRESSION FOR FOLDER //////
                {

                 MultiComp=1;argv++,argc--;goto noc;
                }
                break;
				case 'v': /// RECURSIVE ARCHIVE COMPRESSION FOR FOLDER //////
                {

                Verbose=1;argv++,argc--;goto noc;
                }
                break;
                default :   printf( "\n Unknown option! \n" );exit(1);
            }
            p++;
        }
    }

	if (argc<2) endx();

    progname();





    if (argc>2)
	{
	  if (argv[1][1]==':' ) strcpy(Aname,argv[1]); else sprintf( Aname, "%s\\%s", Dfolder,argv[1] );
	  argv++,argc--;
	}
	else
	{
      if (argv[1][1]==':' ) strcpy(Aname,argv[1]); else sprintf( Aname, "%s\\%s", Dfolder,argv[1] );
	  printf("Archiver not defined: Activate File-Folder control..  \n");

	  strcpy(temp2,Aname);
      if (Wildcard(Aname)) GetFullPath(Aname),strcpy(Aname,part);


      if (DirectoryExists(Aname))
	  {

	    if (argv[1][1]==':' )
		{
		if (!Wildcard(argv[1]) && *(argv[1]+strlen(argv[1]))!=92) sprintf( argv[1], "%s%s", argv[1],"\\*" );
	    sprintf( Aname, "%s\\%s%s", Dfolder,GetNameArchive(Aname),".lza" );
		}
		else
        {
		if (!Wildcard(argv[1]) && *(argv[1]+strlen(argv[1]))!=92) sprintf( argv[1], "%s%s", argv[1],"\\*" );
		sprintf(Aname, "%s\\%s.lza", Dfolder,GetNameArchive(Aname) );
		}

	  }
	  else
      {
	  if (argv[1][1]==':' ) strcpy(Aname,argv[1]); else sprintf( Aname, "%s\\%s", Dfolder,argv[1] );
	  sprintf(argv[1], "%s\\%s", Dfolder,"*" );
       }



    }

	Clearpath(Aname);
    printf("Archive is %s. %u threads active \n",Aname,num_threads);
    origsize=Getfilesize(Aname);
    if (argv[1][0]=='$'   ) argv[1][0]='*',argv[1][1]=0,printf("argc=%s \n",argv[1]);


   parser=(LZSRparser*)malloc(sizeof(LZSRparser)*(WorkBlockSize+65536));for(U32 i=0;i<WorkBlockSize+65536;i++){parser[i].price=0x0FFFFFFF;}





    //FileListSize=0;
  char origin[MAX_PATH];DirListNum=0;addedsize=0;

  for(int i=1;i<argc;i++)
  {
    strcpy(origin,argv[i]);

    if (origin[1]!=':')
    {
        sprintf(origin, "%s\\%s", Dfolder,argv[i] );Clearpath(origin);
    }
    else
    {
        if(origin[2]!=92 && origin[2]!=47 ){memcpy(part,origin,2);part[2]=0;memcpy(Oname,origin+2,strlen(origin)-2);Oname[strlen(origin)-2]=0;sprintf(origin, "%s\\%s",part,Oname );Clearpath(origin); }
    }

    if(filexist(origin))  {totalsize+=Getfilesize(origin); listDir.push_back(GetFullPath(origin));DirListNum++;}
    else
    SearchDIR(origin);

    strcpy(argv[i],origin);
  }



    try
  {
    Arguments a(argc, argv);
    typedef std::string FileNameType;
    UI ui(0);
    bool recurse(MultiComp);
    typedef Walker<FileIterator<FileProperties<FileNameType> > > W;
    W w (std::for_each(a.begin(), a.end(), W().Recurse(recurse)));
    if (w.empty()) printf ("WARNING: No files");
    std::sort(w.begin(), w.end());
    std::for_each(w.begin(), w.end(), Archiver(ui)).Done();
  }
  catch (const std::exception &e)
  {
    std::cout << "error: " << e.what() << std::endl;
  }
   catch (const std::string &s)

   {
    std::cout << s << std::endl;
  } catch (...)
   {
    std::cout << "error" << std::endl;
  }



  /// LAST PHASE
    for (int i=0;i<DirListNum;i++)
    {
    strcpy(part,listDir[i].c_str());
    memcpy(origin,part+3,MAX_PATH);
    sprintf(origin,"%s\\",origin);
    sprintf(part,"%s\\",part);
    Clearpath(part);
    Clearpath(origin);
   // printf(" %s %s  \n",part,fname);
    FileSize=0;Oname[0]=0;CMencode(part,Oname,origin,0);
    }

        compresize=Getfilesize(Aname);
  free(parser);



  free(bufx);

 printf(" Compressed %I64d bytes to %I64d bytes \n",totalsize,compresize-origsize);

}
else if ( argv[1] && argv[1][0] == 'x' )  /// EXTRACTING ////////////////////////////////
{
    argv++,argc--;decompress=1;
    if (argc<2) endx();
    nac:
    while( argv[1] && argv[1][0] == '-' )
    {
        char *p = (char*)argv[1]+1;
        while( *p )
        {
            switch( tolower(*p) )
            {
				case 'v': /// RECURSIVE ARCHIVE COMPRESSION FOR FOLDER //////
                {
                Verbose=1;argv++,argc--;goto nac;
                }
                break;
                default :   printf( "\n Unknown option! \n" );exit(1);
            }
            p++;
        }
    }



    Arcposition=0;


    if (argv[1][1]==':')
    strcpy(Aname,argv[1]);
    else
    sprintf( Aname, "%s\\%s", Dfolder,argv[1] );
	Clearpath(Aname);

    int ltx=2;
	if (argc==2) strcpy(argv[1], "*"),ltx=1;


    progname();


    while (ltx<argc)
    {
    printf("ARGV=%s\n",argv[ltx]);
	if (argv[ltx][1]==':')
    strcpy(Destination,argv[ltx]);
    else
    sprintf( Destination, "%s\\%s", Dfolder,argv[ltx] );

	if (!Wildcard(Destination) && *(Destination+strlen(Destination))!=92) sprintf( Destination, "%s%s", Destination,"\\*" );

	GetFullPath(Destination);
	GetName((char *)Destination);
	strcpy(Destination,part);

	if (strlen(Oname)==0)strcpy(Oname,"*");
	lowercase(Oname);
    strcpy(wildcard,Oname);


    printf("Archive=%s Wild=%s DEST=%s\n",Aname,Oname,Destination);


    Extractor(wildcard);

	ltx++;

    }


	 free(bufx);
	exit(1);
}/////////////////////////////////////////////////////////////////////////////////////////
else if ( argv[1] && argv[1][0] == 'l' )  /// LIST ////
{
int flisted=0;

argv++,argc--;

if (argc<2) endx();

Arcposition=0;
if (argv[1][1]==':')
    strcpy(Aname,argv[1]);
    else
    sprintf( Aname, "%s\\%s", Dfolder,argv[1] );
    int ltx=2;
    if (argc==2) strcpy( argv[1], "*"),ltx=1;

    printf("Archive List ... %s \n", Aname);
   // printf("argc=%d  \n", argc);
    while (ltx<argc) Lister(argv[ltx]),ltx++;

printf(" n.%d files in archive %s \n",DprFiles,Aname);
    fclose(inp);
    fclose(outp);
exit(1);
}
else if ( argv[1] && argv[1][0] == 'v'    )  /// LIST ////
{
    argv++,argc--;
    nic:
    while( argv[1] && argv[1][0] == '-' )
    {
        char *p = (char*)argv[1]+1;
        while( *p )
        {
            switch( tolower(*p) )
            {
				case 'v': /// RECURSIVE ARCHIVE COMPRESSION FOR FOLDER //////
                {
                Verbose=1;argv++,argc--;goto nic;
                }
                break;
                default :   printf( "\n Unknown option! \n" );exit(1);
            }
            p++;
        }
    }
    decompress=1;
    Arcposition=0;
    strcpy(Aname,argv[1]);
	Clearpath(Aname);
    VerifyArchive();
exit(1);
}
else endx();
//



    fclose(inp);
    fclose(outp);
return (0);
}
