// CEZEO software Ltd. https://www.cezeo.com
#include "compressor.h"
#include "system_utils.h"
#include "string_utils.h"
#include <io.h>
#include <errno.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// #include <fcntl.h>
#define _O_RDONLY 0x0000  // open for reading only
#define _O_WRONLY 0x0001  // open for writing only
#define _O_RDWR 0x0002    // open for reading and writing
#define _O_APPEND 0x0008  // writes done at eof
#define _O_CREAT 0x0100  // create and open file
#define _O_TRUNC 0x0200  // open and truncate
#define _O_EXCL 0x0400   // open only if file doesn't already exist
// O_TEXT files have <cr><lf> sequences translated to <lf> on read()'s and <lf>
// sequences translated to <cr><lf> on write()'s
#define _O_TEXT 0x4000      // file mode is text (translated)
#define _O_BINARY 0x8000    // file mode is binary (untranslated)
#define _O_WTEXT 0x10000    // file mode is UTF16 (translated)
#define _O_U16TEXT 0x20000  // file mode is UTF16 no BOM (translated)
#define _O_U8TEXT 0x40000   // file mode is UTF8  no BOM (translated)
// macro to translate the C 2.0 name used to force binary mode for files
#define _O_RAW _O_BINARY
#define _O_NOINHERIT 0x0080    // child process doesn't inherit file
#define _O_TEMPORARY 0x0040    // temporary file bit (file is deleted when last handle is closed)
#define _O_SHORT_LIVED 0x1000  // temporary storage file, try not to flush
#define _O_OBTAIN_DIR 0x2000   // get information about a directory
#define _O_SEQUENTIAL 0x0020   // file access is primarily sequential
#define _O_RANDOM 0x0010       // file access is primarily random
// #include <sys/stat.h>
#define _S_IFMT 0xF000    // File type mask
#define _S_IFDIR 0x4000   // Directory
#define _S_IFCHR 0x2000   // Character special
#define _S_IFIFO 0x1000   // Pipe
#define _S_IFREG 0x8000   // Regular
#define _S_IREAD 0x0100   // Read permission, owner
#define _S_IWRITE 0x0080  // Write permission, owner
#define _S_IEXEC 0x0040   // Execute/search permission, owner

namespace cab
{
  /*
  struct CCAB
  {
    // longs first
    ULONG cb;              // size available for cabinet on this media
    ULONG cbFolderThresh;  // Thresshold for forcing a new Folder

    // then ints
    UINT cbReserveCFHeader;  // Space to reserve in CFHEADER
    UINT cbReserveCFFolder;  // Space to reserve in CFFOLDER
    UINT cbReserveCFData;    // Space to reserve in CFDATA
    int iCab;                // sequential numbers for cabinets
    int iDisk;               // Disk number
#ifndef REMOVE_CHICAGO_M6_HACK
    int fFailOnIncompressible;  // TRUE => Fail if a block is incompressible
#endif

    //  then shorts
    USHORT setID;  // Cabinet set ID

    // then chars
    char szDisk[ CB_MAX_DISK_NAME ];    // current disk name
    char szCab[ CB_MAX_CABINET_NAME ];  // current cabinet name
    char szCabPath[ CB_MAX_CAB_PATH ];  // path for creating cabinet
  };
  */

  namespace
  {
    // FCI callbacks
    // will not override
    FNFCIALLOC(fnMemAlloc)
    {
      return HeapAlloc(GetProcessHeap(), NULL, cb);
    }

    // will not override
    FNFCIFREE(fnMemFree)
    {
      HeapFree(GetProcessHeap(), NULL, memory);
    }

    // file placed callback
    FNFCIFILEPLACED(fnFilePlaced)
    {
      // int (PCCAB pccab, _In_ LPSTR pszFile, long  cbFile, BOOL  fContinuation, void FAR *pv)
      return 0;
    }

    FNFCIGETNEXTCABINET(fnGetNextCab)
    {
      // BOOL fn(PCCAB pccab, ULONG cbPrevCab, void FAR *pv)
      return FALSE;
    }

    FNFCISTATUS(fnStatus)
    {
      // long fn(UINT typeStatus, ULONG cb1, ULONG cb2, void FAR *pv)
      return 0;
    }

    FNFCIOPEN(fnOpen)
    {
      // INT_PTR fn(_In_ LPSTR pszFile, int oflag, int pmode, int FAR* err, void FAR* pv)
      // convert from UTF-8 because we use it in worker
      std::wstring fileName(StringUtils::FromUtf8(pszFile));
      int result = _wopen(fileName.c_str(), oflag, pmode);
      if (result == -1)
      {
        *err = errno;
      }
      return (INT_PTR)result;
    }

    FNFCIGETOPENINFO(fnGetOpenInfo)
    {
      // INT_PTR fn(_In_ LPSTR pszName, USHORT *pdate, USHORT *ptime, USHORT *pattribs, int FAR *err, void FAR *pv)
      std::wstring fileName(StringUtils::FromUtf8(pszName));
      WIN32_FILE_ATTRIBUTE_DATA fileAttrs;
      if (GetFileAttributesEx(fileName.c_str(), GetFileExInfoStandard, &fileAttrs))
      {
        // UTC time
        FileTimeToDosDateTime(&fileAttrs.ftLastWriteTime, pdate, ptime);
        *pattribs = (USHORT)(fileAttrs.dwFileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE));
        // file names in utf
        *pattribs |= _A_NAME_IS_UTF;
        return fnOpen(pszName, _O_RDONLY | _O_BINARY, _S_IREAD, err, nullptr);
      }
      else
      {
        *err = GetLastError();
        return -1;
      }
    }

    FNFCIREAD(fnRead)
    {
      // UINT fn(INT_PTR hf, void FAR* memory, UINT cb, int FAR* err, void FAR* pv)
      UINT result = (UINT)_read((int)hf, memory, cb);
      if (result != cb)
      {
        *err = errno;
      }
      return result;
    }

    FNFCIWRITE(fnWrite)
    {
      // UINT fn(INT_PTR hf, void FAR* memory, UINT cb, int FAR* err, void FAR* pv)
      UINT result = (UINT)_write((int)hf, memory, cb);
      if (result != cb)
      {
        *err = errno;
      }
      return result;
    }

    FNFCICLOSE(fnClose)
    {
      // int fn(INT_PTR hf, int FAR* err, void FAR* pv)
      int result = _close((int)hf);
      if (result != 0)
      {
        *err = errno;
      }
      return result;
    }

    FNFCISEEK(fnSeek)
    {
      // long fn(INT_PTR hf, long dist, int seektype, int FAR* err, void FAR* pv)
      long result = _lseek((int)hf, dist, seektype);
      if (result == -1)
      {
        *err = errno;
      }
      return result;
    }

    FNFCIDELETE(fnDelete)
    {
      // int fn(_In_ LPSTR pszFile, int FAR* err, void FAR* pv)
      std::wstring fileName(StringUtils::FromUtf8(pszFile));
      int result = _wremove(fileName.c_str());
      if (result != 0)
      {
        *err = errno;
      }
      return result;
    }

    FNFCIGETTEMPFILE(fnGetTempFile)
    {
      // BOOL fn(_Out_writes_bytes_(cbTempName) char* pszTempName, _In_range_(>=, MAX_PATH) int cbTempName, void FAR* pv)
      memset(pszTempName, 0, cbTempName);
      std::wstring tempPath;
      if (SystemUtils::GenerateTempFileName(tempPath))
      {
        if (PathFileExists(tempPath.c_str()))
        {
          DeleteFile(tempPath.c_str());
        }
        std::string tempFilePath(StringUtils::ToUtf8(tempPath));
        if (tempFilePath.size() <= (size_t)cbTempName)
        {
          memcpy(pszTempName, &tempFilePath[ 0 ], tempFilePath.size());
          return TRUE;
        }
      }
      // error
      return FALSE;
    }
  }  // namespace

  compressor::compressor()
  {
    memset(&compressErrors, 0, sizeof(compressErrors));
    memset(&cabParams, 0, sizeof(cabParams));

#ifdef CAB_STATIC_LINK
    pfnFCICreate = FCICreate;
    pfnFCIAddFile = FCIAddFile;
    pfnFCIFlushFolder = FCIFlushFolder;
    pfnFCIFlushCabinet = FCIFlushCabinet;
    pfnFCIDestroy = FCIDestroy;
#else
    cabinetDll.load("cabinet.dll");
    if (nullptr != cabinetDll)
    {
      // use fci.lib
      // dynamic link to cabinet.dll
      pfnFCICreate = (fnFCICreate)GetProcAddress(cabinetDll, "FCICreate");
      pfnFCIAddFile = (fnFCIAddFile)GetProcAddress(cabinetDll, "FCIAddFile");
      pfnFCIFlushFolder = (fnFCIFlushFolder)GetProcAddress(cabinetDll, "FCIFlushFolder");
      pfnFCIFlushCabinet = (fnFCIFlushCabinet)GetProcAddress(cabinetDll, "FCIFlushCabinet");
      pfnFCIDestroy = (fnFCIDestroy)GetProcAddress(cabinetDll, "FCIDestroy");
    }
    else
    {
      throw(std::runtime_error("cabinet.dll can not be loaded"));
    }
#endif

    if (nullptr == pfnFCICreate || nullptr == pfnFCIDestroy || nullptr == pfnFCIAddFile || nullptr == pfnFCIFlushFolder || nullptr == pfnFCIFlushCabinet || nullptr == pfnFCIDestroy)
    {
      throw(std::runtime_error("fci funciton are not found"));
    }
  }

  compressor::~compressor()
  {
    destroy_context();
  }

  void compressor::create_context(const std::wstring& target_path)
  {
    destroy_context();

    // fill cab params
    memset(&cabParams, 0, sizeof(CCAB));
    cabParams.cb = 0x7FFFFFFF;
    cabParams.cbFolderThresh = 0x7FFFFFFF;
    cabParams.iCab = 1;
    cabParams.iDisk = 1;
    cabParams.setID = 0;
    // char szCab[ CB_MAX_CABINET_NAME ];  // current cabinet name
    // char szCabPath[ CB_MAX_CAB_PATH ];  // path for creating cabinet
    std::string cabPath(StringUtils::ToUtf8(SystemUtils::GetFolderFromPath(target_path)));
    std::string cabName(StringUtils::ToUtf8(SystemUtils::GetFileNameFromPath(target_path)));
    if (cabPath.size() < CB_MAX_CAB_PATH && cabName.size() < CB_MAX_CABINET_NAME)
    {
      // copy path into structure
      memcpy(cabParams.szCabPath, cabPath.c_str(), cabPath.size());
      memcpy(cabParams.szCab, cabName.c_str(), cabName.size());
      // create FCI context
      compressHandler = pfnFCICreate(&compressErrors, fnFilePlaced, fnMemAlloc, fnMemFree, fnOpen, fnRead, fnWrite, fnClose, fnSeek, fnDelete, fnGetTempFile, &cabParams, this);
    }
    else
    {
      throw std::runtime_error("cab file path too long");
    }
  }

  void compressor::destroy_context()
  {
    if (nullptr != compressHandler)
    {
      if (nullptr != pfnFCIDestroy)
      {
        pfnFCIDestroy(compressHandler);
      }
      compressHandler = nullptr;
    }
  }

  void compressor::add_file(const std::wstring& filePath)
  {
    uint64_t fileSize = 0;
    // check if file exist to check the size
    if (SystemUtils::GetFileSize(filePath.c_str(), fileSize))
    {
      // ñabinet.dll supports max 2GB
      if (fileSize < 0x7FFF0000)
      {
        std::string fileName(StringUtils::ToUtf8(SystemUtils::GetFileNameFromPath(filePath)));
        std::string fullPath(StringUtils::ToUtf8(filePath));
        if (!pfnFCIAddFile(compressHandler, &fullPath[ 0 ], &fileName[ 0 ], FALSE, fnGetNextCab, fnStatus, fnGetOpenInfo, tcompTYPE_LZX | tcompLZX_WINDOW_HI))
        {
          throw std::runtime_error("can't add file to archive");
        }
      }
      else
      {
        throw std::runtime_error("file size too big for cab file");
      }
    }
    else
    {
      throw std::runtime_error("can't get file size");
    }
  }

  void compressor::file_to_cab(const std::wstring& target_path, const std::wstring& file_path)
  {
    // create compression context
    create_context(target_path);

    // add file
    add_file(file_path);

    // flush cabinet data
    pfnFCIFlushCabinet(compressHandler, FALSE, fnGetNextCab, fnStatus);

    // destroy context
    destroy_context();
  }

  void compressor::files_to_cab(const std::wstring& target_path, const std::vector<std::wstring>& files)
  {
    // create compression context
    create_context(target_path);

    // add files
    for (const std::wstring& file : files)
    {
      add_file(file);
    }

    // flush cabinet data
    pfnFCIFlushCabinet(compressHandler, FALSE, fnGetNextCab, fnStatus);

    // destroy context
    destroy_context();
  }
}  // namespace cab
