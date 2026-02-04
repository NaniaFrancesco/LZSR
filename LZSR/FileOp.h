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
/// FILE Operations /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int wildcmp(const char *wild, const char *string)
{
  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*'))
  {
    if ((*wild != *string) && (*wild != '?'))
    {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string)
  {
    if (*wild == '*')
    {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*')
  {
    wild++;
  }
  return !*wild;
}


double Getfilesize(const char* fname)
{
  if (!fname && !*fname)
     return 0.0;
  HANDLE h;
  WIN32_FIND_DATA info;

  if ((h=FindFirstFile(fname,&info))
      != INVALID_HANDLE_VALUE)
  {
     FindClose(h);
     if ((info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
         == 0) // Is it a file?
     {
        union
        {
          struct { DWORD low, high; } lh;
          __int64 size; // MS Widthe int extension
        } file;
        file.lh.low = info.nFileSizeLow;
        file.lh.high= info.nFileSizeHigh;
        return file.size; // will be casted to double
     }
     // It's a directory, not a file
  }
  return 0.0; // No such name.
}


__int64 GetFileSize(FILE *input)
{
  __int64 tsize;
  fseeko64(input , 0 , SEEK_END);
  tsize = ftello64 (input);
  rewind (input);
  return tsize;
}

void OpenInputFile(const char *filename)
{
//  printf("Archive %s ",filename);
  inp=fopen64(filename, "rb");
  if (inp==NULL)
  {
  printf("Source file %s error!",filename);
  exit(1);
  }

}
void OpenOutputFile(const char *filename)
{

  outp=fopen64(filename, "rb+");
  if (!outp)
  {
  outp=fopen64(filename, "wb");
  if (!outp) {printf("Source file %s error!",filename); exit(1);}
  fclose(outp);
  outp=fopen64(filename, "rb+");
  }
  else
  fseeko64 (outp , 0 , SEEK_END);

}


void FileSeek(FILE *input,__int64 position)
{
fseeko64 (input , position , SEEK_SET);
}

BOOL DirectoryExists(const char* dirName) {
  DWORD attribs = ::GetFileAttributesA(dirName);
  if (attribs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

int  PPMread(U8 *bufx, int lscan)
{
int t=2;

while
(
t+32<lscan
&&
(
(bufx[t  ]<48 || bufx[t  ]>57)
||
(bufx[t+1]<48 || bufx[t+1]>57)
 )
) t++;
sscanf((char*)bufx+t, "%d %d \n", &Length,&Width);
//if (Length>16 && Width>16) printf("PPM Image %d x %d \n",Length,Width);
return Length*Width;
}

void CreateDir(char* Path)///////////////////////////////
{
 char DirName[MAX_PATH];
 char* p = Path;
 char* q = DirName;
 while(*p)
 {
   if (('\\' == *p) || ('/' == *p))
   {
     if (':' != *(p-1))
     {
        CreateDirectory(DirName, NULL);
     }
   }
   *q++ = *p++;
   *q = '\0';
 }
CreateDirectory(DirName, NULL);
}///////////////////////////////////////////////////


void AnalizeDIR(std::vector<string> &dout, char * path )
{
//Declare all needed handles
WIN32_FIND_DATA FindFileData;
HANDLE hFind;
char fname[ MAX_PATH + 256 ];
char pathbak[ MAX_PATH ];     //Make a backup of the directory the user chose
strcpy( pathbak, path );//Find the first file in the directory the user chose
hFind = FindFirstFile ( "*.*", &FindFileData );//Use a do/while so we process whatever FindFirstFile returned

    while ( FindNextFile ( hFind, &FindFileData ) && hFind != INVALID_HANDLE_VALUE )
    {

        if ( hFind != INVALID_HANDLE_VALUE )
        {
            if (!(strcmp( FindFileData.cFileName, "." ) ) || !( strcmp( FindFileData.cFileName, ".." ))) continue;
            strcpy( path, pathbak );//Append the file found on to the path of the directory the user chose
            sprintf( path, "%s\\%s", path, FindFileData.cFileName );

            if ( ( SetCurrentDirectory( path ) ) )
            {
             sprintf(path,"%s\\",path);
             Clearpath(path);
             dout.push_back(path);DirListNum++;
             AnalizeDIR(dout,path);
            }
            else
            {
            totalsize+=Getfilesize(path);
            } //FILE


        }
    } /// END WHILE

    FindClose ( hFind );
}


bool filexist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

//BROWSE FOLDER - Opens a browse folder dialog.
void SearchDIR(char * path)
{
path=RemoveWild(path);
SetCurrentDirectory (path);
if(!filexist(path)) listDir.push_back(path);DirListNum++;
AnalizeDIR( listDir,path);
return;
}// END BROWSE FOLDER


//public domain archiver template by Sami Runsas, 2009, www.compressionratings.com
//compile: g++ -Wall -D_FILE_OFFSET_BITS=64 archiver.cpp -o archiver
#ifdef _MSC_VER

#else
#include <dirent.h>
#endif
#ifndef MASK_H
#define MASK_H
#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) || defined(_POSIX2_VERSION)
#include <fnmatch.h>
#else
inline static int fnmatch(const char *m, const char *s, int dummy = 0) {
  if (*m == '*') for (++m; *s; ++s) if (!fnmatch(m, s)) return 0;
  return (!*s || !(*s == *m || *m == '?')) ? *m | *s : fnmatch(++m, ++s);
}
#endif

template<class T> struct Mask: T {
  Mask(const T &s = "*"): T(s) {}
  bool operator== (const T &s) const { return !fnmatch(T::c_str(), s.c_str(), 0); }
  bool operator!= (const T &s) const { return !(*this == s); }
};

#endif

template<class T> inline static bool FileExists(const T &s) { return !access(s.c_str(), F_OK); }
#ifndef POSIX_FILE_ITERATOR_H
#define POSIX_FILE_ITERATOR_H

#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)


template<class T> inline static bool RelativeDir(const T &s) { return s == "." || s == ".."; }

template<class T> class FileIterator: public T, private stat,dirent
{
  DIR *dir;
  const typename T::NameT path;
  void Set() { T::Name(path + FileName()), ::stat(T::Name().c_str(), this), T::Size(st_size); }
  bool DE(dirent *de) { return de && ((static_cast<dirent &>(*this) = *de), Set(), true); }
 public:
  typedef T TypeT;
  FileIterator(const typename T::NameT &s): dir(opendir(s.empty() ? "." : s.c_str())), path(s.empty() || s == "/" ? s : s + '/') {}
  ~FileIterator() { dir && closedir(dir); }
  bool operator++ () { return dir && DE(readdir(dir)); }
  bool IsFile() const { return S_ISREG(st_mode); }
  bool IsDirectory() const { return S_ISDIR(st_mode) && !RelativeDir(FileName()); }
  typename T::NameT FileName() const { return typename T::NameT(d_name); }
};

#endif

template<class S> struct PathAndMask {
  S p;
  Mask<S> m;
  PathAndMask(const S &s): p(s) {
    std::replace(p.begin(), p.end(), '\\', '/');
    RelativeDir(p) && (p += "/*", true);
    typename S::size_type k = p.find_last_of('/');
    Mask<S>("?:*") == p && (k == 2 && (p.insert(2, "/."), k += 2) || k == p.npos && (k = (RelativeDir(p.substr(2)) ? p.length() : 2), p.insert(2, "."), true));
    m = (k != p.npos ? p.substr(k + 1) : p);
    Mask<S>("?:") == p.erase(k != p.npos ? k + !k : 0) && (p += '.', true);
    m.empty() && (m = Mask<S>(), true);
  }
  const S& P() const { return p; }
  const Mask<S>& M() const { return m; }
};

template<class Iter> class Walker: public std::vector<typename Iter::TypeT> {
  typedef typename Iter::TypeT::NameT S;
  bool recurse;
  void Walk(Iter f, const Mask<S> &m = Mask<S>()) {
    while (++f) m != f.FileName() || (f.IsFile() && (this->push_back(f), true)) || (f.IsDirectory() && recurse && (Walk(f.Name()), true));
  }
 public:
  Walker(): recurse(false) {}
  Walker& Recurse(bool r = true) { recurse = r; return *this; }
  Walker& operator() (const PathAndMask<S> &pm) { Walk(pm.P(), pm.M()); return *this; }
  Walker& operator() (const char *s) { return (*this)(S(s)); }
};

template<class T> inline static bool OverwriteFileYN(const T &s)
 {
  std::cout << "file " << s << " already exist, overwrite? (y/n)";
  std::string inp;
  std::cin >> inp;
  if (inp != "y") throw std::string("user abort"); //if (std::cin.get() != 'y') nope, since there is no way to flush the eol in std::istream
  return true;
}
template<class T> inline static bool RemoveIfExists(const T &s) { return FileExists(s) && OverwriteFileYN(s) && !remove(s.c_str()); }

struct File: std::fstream
{
};
struct FileIn: File {
};
struct FileOut: File
{
};

template<class T> struct FileProperties;
template<class T> std::istream& operator>> (std::istream &s, FileProperties<T> &k) { return (s >> k.name >> k.size).get(), s; }
template<class T> std::ostream& operator<< (std::ostream &s, const FileProperties<T> &k) {
  if (&s == &std::cout) return s << k.Name() << ", " << k.Size() << " bytes";
  return s << k.Name() << '\n' << k.Size() << '\n';
}
template<class T> struct FileProperties {
  typedef T NameT;
  T name;
  __int64 size; //add date, time, permissions, etc...
  FileProperties& Size(__int64 n) { size = n; return *this; }
  FileProperties& Name(const T &s) { name = s; return *this; }
  __int64 Size() const { return size; }
  const T& Name() const { return name; }
  std::string Extension() const {
    typename T::size_type k = name.find_last_of('.');
    return k == name.npos ? name : name.substr(k + 1);
  }
  bool operator< (const FileProperties &b) const { return Extension() < b.Extension(); }
  friend std::istream& operator>> <> (std::istream&, FileProperties<T>&);
  friend std::ostream& operator<< <> (std::ostream&, const FileProperties<T>&);
};



struct UI {
  bool verbose;
  UI(bool v): verbose(v) {}
  operator bool () const { return true; }
  template<class T> UI& Processing(const T &k) { verbose && std::cout << k.Name() << std::endl; return *this; }
  template<class T> UI& Skip(const T &k) { verbose && std::cout << "skipping file " << k.Name() << std::endl; return *this; }
  UI& Done(__int64 files, __int64 bytes) { return *this; }
};

struct Arguments: std::vector<std::string>
{
  Arguments(int argc, char **argv)
  { std::copy(argv + 1, argv + argc, std::inserter(*this, begin())); }

  value_type Pop()
  {
    if (empty()) return value_type();
    value_type s(front());
    return erase(begin()), s;
  }

  bool Bool(const value_type &s)
  {
    std::vector<std::string>::iterator i = std::find(begin(), end(), s);
    return i != end() && (erase(i), true);
  }
  struct SubstrMatch: std::string {
    SubstrMatch (const std::string &s): std::string(s) {}
    bool operator() (const std::string &b) const { return b.substr(0, size()) == *this; }
  };
  template<class T> bool Num(T &n, const value_type &s) {
    std::vector<value_type>::iterator i = std::find_if(begin(), end(), SubstrMatch(s));
    return i != end() && (std::istringstream(i->substr(s.size())) >> n, erase(i), true);
  }
  unsigned Num(const value_type &s, unsigned default_value = 0) {
    unsigned k = default_value;
    return Num(k, s), k;
  }
};


