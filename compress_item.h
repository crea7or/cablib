#pragma once

#include <fci.h>
#include <windows.h>
#include <string>

namespace cab
{
  class compress_item
  {
  public:
    compress_item(const std::wstring& name, const size_t size) : fileName(name), uncompressedFileSize(size){};
    virtual ~compress_item(){};

    virtual INT_PTR fnFileOpen(char* namePtr, int oflag, int mode, int* errorPtr) = 0;
    virtual UINT fnFileRead(void* memoryPtr, UINT bytes, int* errorPtr) = 0;
    virtual UINT fnFileWrite(void* memoryPtr, UINT bytes, int* errorPtr) = 0;
    virtual long fnFileSeek(long distance, int seekType, int* errorPtr) = 0;
    virtual int fnFileClose(int* errorPtr) = 0;

    std::wstring GetFileName() const
    {
      return fileName;
    }

    void SetFileName(const std::wstring& name)
    {
      fileName = name;
    }

  protected:
    size_t uncompressedFileSize{0};
    std::wstring fileName;
  };
  typedef compress_item* PCOMPRESSITEM;
};  // namespace cab
