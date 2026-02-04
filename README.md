High-performance data archiver - Dual Licensed (GPLv3/Commercial)

Zenodo DOI: https://zenodo.org/records/18485963

DOWNLOAD EXE: http://heartofcomp.altervista.org/

Licensing

LZSR is a dual-licensed project. We believe in the power of open-source while providing a path for commercial integration.

LZSR Archiver Copyright (C) 2014-2026, Nania Francesco Antonio

Open Source License

For most users, the GNU GPLv3 is the appropriate license. You can use, modify, and redistribute LZSR for free, provided that any derivative work is also released under the GPLv3. See the LICENSE file for the full text.

Commercial License

If the GPLv3's copyleft requirements (specifically the obligation to share your source code) are not compatible with your project's goals, we offer a Commercial License.

Benefits of the Commercial License:

•	Proprietary Integration: Use LZSR in closed-source products.

•	No Copyleft: You are not required to share your application's source code.

•	Dedicated Support: Direct access to the lead developer for integration assistance.

•	Legal Indemnification: Full ownership warranty for corporate compliance.

To purchase a commercial license or request a quote, please contact us at: miadebora@hotmail.it

To compile with Code::blocks (GCC 15.2)

Compiler flags: -O4 -march=x86-64-v2 -mtune=generic -ffast-math -fpermissive -std=c++23 -finline-functions 

Linker flags : -flto -static-libgcc -static-libstdc++ -static

Usage:

 LZSR <command> -<switch ..> <archive> <files...> <in path> <out path>
 
<Commands>
 
 a                 Add files to archive
 
 x                 Extract files in destination path
 
 v                 Verify all files in Archive (CRC32)
 
 l                 List archive


<Switches>
 
 r                Recurse subdirectories
 
 s                Solid Mode ON [defaul OFF]
 
 v                Verbose Mode
 
 b                [default -b1] set LZbuffer memory     -b0 [32MB]...-b5[1GB]
 

Examples of compression :

lzsr    a c:\archive c:\infile

lzsr    a -m0 c:\archive c:\infile

lzsr    a -r  c:\archive c:\path

lzsr    a -r  -h7 -b5 archive path

lzsr    a -r archive path\*.bmp

lzsr   a -r -m2  archive *

Example of decompression :

lzsr    x  c:\archive c:\outpath

lzsr    x  archive outpath

lzsr    x archive outpath\*.bmp
