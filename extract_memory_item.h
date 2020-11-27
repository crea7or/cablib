// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include "extract_item.h"
#include "buffer.h"

namespace cab
{
  class extract_memory_item : public extract_item
  {
  public:
    extract_memory_item(const uint8_t* sourceDataPtr, const size_t sourceSize);
    extract_memory_item(const std::wstring& name, const size_t size);
    virtual ~extract_memory_item();

    virtual FNOPEN(fnFileOpen) override;
    virtual FNREAD(fnFileRead) override;
    virtual FNWRITE(fnFileWrite) override;
    virtual FNSEEK(fnFileSeek) override;
    virtual FNCLOSE(fnFileClose) override;

    buffer& GetBuffer();

  protected:
    size_t position{0};
    buffer data;
  };
  typedef extract_memory_item* PEXTRACTMEMORYITEM;
};  // namespace cab
