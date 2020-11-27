// CEZEO software Ltd. https://www.cezeo.com
#include "system_utils.h"
#include "string_utils.h"

// windows
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <windows.h>

#define TEMP_FILE_NAME_PREFIX L"cabf"

namespace SystemUtils
{
  extern const wchar_t localAppData[] = L"LOCALAPPDATA";
  extern const wchar_t pathSeparator = L'\\';

  namespace
  {
    const wchar_t forwardSlash = L'/';
    const char hexDigits[] = "0123456789abcdef";
    const wchar_t defaultTempFolder[] = L"C:\\Temp";
  }  // namespace

  // used to add filename to the path with checking the trailing backward and forward slashes
  std::wstring PathAppendFileName(const std::wstring& path, const std::wstring& fileName)
  {
    std::wstring result(path);
    if (result.size() > 0)
    {
      if (result[ result.size() - 1 ] != pathSeparator && result[ result.size() - 1 ] != forwardSlash)
      {
        result += pathSeparator;
      }
      result += fileName;
    }
    return result;
  }

  // used to extract filename from the path
  std::wstring GetFileNameFromPath(const std::wstring& fileFullPath)
  {
    wchar_t fileFullName[ _MAX_PATH ];
    wchar_t fileName[ _MAX_FNAME ];
    wchar_t fileExtension[ _MAX_EXT ];
    _wsplitpath_s(fileFullPath.c_str(), NULL, 0, NULL, 0, fileName, _MAX_FNAME, fileExtension, _MAX_EXT);
    _wmakepath_s(fileFullName, _MAX_PATH, NULL, NULL, fileName, fileExtension);
    return std::wstring(fileFullName);
  }

  std::wstring GetModuleFolder()
  {
    wchar_t filePath[ _MAX_PATH ];
    GetModuleFileName(NULL, filePath, _MAX_PATH);
    return std::wstring(GetFolderFromPath(filePath));
  }

  std::wstring GetModuleFile()
  {
    wchar_t filePath[ _MAX_PATH ];
    GetModuleFileName(NULL, filePath, _MAX_PATH);
    return std::wstring(GetFileNameFromPath(filePath));
  }

  std::wstring GetModulePath()
  {
    wchar_t filePath[ _MAX_PATH ];
    GetModuleFileName(NULL, filePath, _MAX_PATH);
    return std::wstring(filePath);
  }

  std::wstring GetFolderFromPath(const std::wstring& fileName)
  {
    wchar_t filePath[ _MAX_PATH ];
    wchar_t driveName[ _MAX_DRIVE ], path[ _MAX_PATH ];
    _wsplitpath_s(fileName.c_str(), driveName, _MAX_DRIVE, path, _MAX_PATH, NULL, 0, NULL, 0);
    _wmakepath_s(filePath, _MAX_PATH, driveName, path, NULL, NULL);
    return std::wstring(filePath);
  }

  // return lowercase file extension
  std::wstring GetExtensionFromPath(const std::wstring& fileName)
  {
    wchar_t fileExtension[ _MAX_EXT ];
    ZeroMemory(fileExtension, _MAX_EXT);
    _wsplitpath_s(fileName.c_str(), NULL, 0, NULL, 0, NULL, 0, fileExtension, _MAX_EXT);
    CharLower(fileExtension);
    return std::wstring(fileExtension);
  }

  // get file size
  bool GetFileSize(const std::wstring& path, uint64_t& fileSize)
  {
    bool result = false;
    HANDLE fileHandle = CreateFile(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
      LARGE_INTEGER size;
      result = GetFileSizeEx(fileHandle, &size) != FALSE;
      fileSize = size.QuadPart;
      CloseHandle(fileHandle);
    }
    return result;
  }

  // get last write time for file
  uint64_t GetFileLastWriteTime(const std::wstring& file)
  {
    uint64_t result = 0;
    FILETIME lastWriteTime{0};
    HANDLE fileHandle = CreateFile(file.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
      GetFileTime(fileHandle, NULL, NULL, &lastWriteTime);
      result = lastWriteTime.dwLowDateTime | ((uint64_t)lastWriteTime.dwHighDateTime << 32);
      CloseHandle(fileHandle);
    }
    return result;
  }

  // load file to buffer
  bool FileToString(const std::wstring& filePath, std::string& data, size_t maxSize)
  {
    bool result = false;
    uint64_t size = 0;
    if (GetFileSize(filePath, size) && size <= maxSize )
    {
      data.resize((size_t)size); // safe uint64_t to size_t
      HANDLE fileHandle = CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (fileHandle != INVALID_HANDLE_VALUE)
      {
        DWORD read = 0;
        result = ReadFile(fileHandle, &data[0], (DWORD)size, &read, NULL) != FALSE && read == size; // safe uint64_t to DWORD
        CloseHandle(fileHandle);
      }
    }
    return result;
  }

  // generate temp file name
  bool GenerateTempFileName(std::wstring& tempFileName)
  {
    bool result = false;
    wchar_t tempFileNameBuffer[ MAX_PATH ];
    std::wstring tempFolder;
    if (GetTempFolder(tempFolder))
    {
      //  Generates a temporary file name.
      UINT returnedId = GetTempFileName(tempFolder.c_str(),     // directory for tmp files
                                        TEMP_FILE_NAME_PREFIX,  // temp file name prefix
                                        0,                      // create unique name
                                        tempFileNameBuffer);    // buffer for name
      if (returnedId > 0)
      {
        tempFileName.assign(tempFileNameBuffer);
        result = true;
      }
    }
    return result;
  }

  // get temp path to temp folder, the returned string ends with a backslash (if success)
  bool GetTempFolder(std::wstring& tempFolder)
  {
    bool result = false;
    wchar_t tempPathBuffer[ MAX_PATH ];
    //  Gets the temp path env string (no guarantee it's a valid path).
    DWORD resultInChars = GetTempPath(MAX_PATH, tempPathBuffer);
    if (resultInChars > (MAX_PATH + 1) || resultInChars == 0)
    {
      // error
    }
    else
    {
      tempFolder.assign(tempPathBuffer, size_t(resultInChars));
      result = true;
    }
    return result;
  }
}  // namespace SystemUtils