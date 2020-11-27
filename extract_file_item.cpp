// CEZEO software Ltd. https://www.cezeo.com
#include "extract_file_item.h"
#include <fcntl.h>
#include "string_utils.h"

namespace cab
{
  extract_file_item::extract_file_item(const std::wstring& name, const size_t size) : fileHandle(INVALID_HANDLE_VALUE), extract_item(name, size)
  {
  }

  extract_file_item::~extract_file_item()
  {
    // fileHandle is stored in the class
    fnFileClose(0);
  }

  FNOPEN(extract_file_item::fnFileOpen)
  {
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
      DWORD desiredAccess = 0;
      DWORD creationDisposition = 0;

      UNREFERENCED_PARAMETER(pmode);

      if (oflag & _O_RDWR)
      {
        desiredAccess = GENERIC_READ | GENERIC_WRITE;
      }
      else if (oflag & _O_WRONLY)
      {
        desiredAccess = GENERIC_WRITE;
      }
      else
      {
        desiredAccess = GENERIC_READ;
      }

      if (oflag & _O_CREAT)
      {
        creationDisposition = CREATE_ALWAYS;
      }
      else
      {
        creationDisposition = OPEN_EXISTING;
      }
      // convert from UTF-8 because we use it in worker
      fileName = StringUtils::MbToWide(pszFile, CP_UTF8);
      fileHandle = CreateFile(fileName.c_str(), desiredAccess, FILE_SHARE_READ, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    // always return this, we'll use it as pointer to this file.
    return (INT_PTR)this;
  }

  FNREAD(extract_file_item::fnFileRead)
  {
    // hf - this pointer
    DWORD bytesRead = (DWORD)-1;
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
      if (ReadFile(fileHandle, pv, cb, &bytesRead, NULL) == FALSE)
      {
        bytesRead = (DWORD)-1;
      }
    }
    return bytesRead;
  }

  FNWRITE(extract_file_item::fnFileWrite)
  {
    // hf - this pointer
    DWORD bytesWritten = (DWORD)-1;
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
      if (WriteFile(fileHandle, pv, cb, &bytesWritten, NULL) == FALSE)
      {
        bytesWritten = (DWORD)-1;
      }
    }
    return bytesWritten;
  }

  FNSEEK(extract_file_item::fnFileSeek)
  {
    // hf - this pointer, but we don't need it
    long result = -1;
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
      result = (long)SetFilePointer(fileHandle, dist, NULL, seektype);
    }
    return result;
  }

  FNCLOSE(extract_file_item::fnFileClose)
  {
    // hf - this pointer
    int result = -1;
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
      result = CloseHandle(fileHandle) ? 0 : -1;
      fileHandle = INVALID_HANDLE_VALUE;
    }
    return result;
  }
};  // namespace cab
