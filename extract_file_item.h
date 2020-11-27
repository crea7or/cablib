// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include "extract_item.h"
#include <fdi.h>
#include <string>

namespace cab
{
  class extract_file_item : public extract_item
  {
  public:
    extract_file_item(const std::wstring& name, const size_t size);
    virtual ~extract_file_item();

    virtual FNOPEN(fnFileOpen) override;
    virtual FNREAD(fnFileRead) override;
    virtual FNWRITE(fnFileWrite) override;
    virtual FNSEEK(fnFileSeek) override;
    virtual FNCLOSE(fnFileClose) override;

  protected:
    HANDLE fileHandle;
  };
  typedef extract_file_item* PEXTRACTFILEITEM;
}  // namespace cab
