// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <fdi.h>
#include <windows.h>
#include <string>

namespace cab
{
  class extract_item
  {
  public:
    extract_item(const std::wstring& name, const size_t size) : fileName(name), uncompressedFileSize(size){};
    virtual ~extract_item(){};

    virtual FNOPEN(fnFileOpen) = 0;
    // hf - this pointer
    virtual FNREAD(fnFileRead) = 0;
    // hf - this pointer
    virtual FNWRITE(fnFileWrite) = 0;
    // hf - this pointer
    virtual FNSEEK(fnFileSeek) = 0;
    // hf - this pointer
    virtual FNCLOSE(fnFileClose) = 0;

    std::wstring GetFileName() const
    {
      return fileName;
    }

    void SetFileName(const std::wstring& name)
    {
      fileName = name;
    }

  protected:
    size_t uncompressedFileSize;
    std::wstring fileName;
  };
  typedef extract_item* PEXTRACTITEM;
};  // namespace cab
