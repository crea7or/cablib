// CEZEO software Ltd. https://www.cezeo.com
#include "extract_memory_item.h"
#include "string_utils.h"

namespace cab
{
  extract_memory_item::extract_memory_item(const uint8_t* sourceDataPtr, const size_t sourceSize) : extract_item(L"<memory>", sourceSize)
  {
    data.set(sourceDataPtr, sourceSize, buffer::TYPE::KEEPER);
  }

  extract_memory_item::extract_memory_item(const std::wstring& name, const size_t size) : extract_item(name, size)
  {
    data.resize(size);
  }

  extract_memory_item::~extract_memory_item()
  {
  }

  FNOPEN(extract_memory_item::fnFileOpen)
  {
    UNREFERENCED_PARAMETER(pmode);
    // convert from UTF-8 because we use it in worker and should transfer name via char*
    fileName = StringUtils::MbToWide(pszFile, CP_UTF8);
    data.clear();
    position = 0;
    // nothing to open more
    // always return this, we'll use it as pointer to this file.
    return (INT_PTR)this;
  }

  FNREAD(extract_memory_item::fnFileRead)
  {
    // hf - this pointer, but we don't need it
    UINT bytesRead = 0;
    size_t newPosition = position + cb;
    if (data.size() >= newPosition)
    {
      memcpy(pv, &data[ position ], cb);
      bytesRead = cb;
      position = newPosition;
    }
    return bytesRead;
  }

  FNWRITE(extract_memory_item::fnFileWrite)
  {
    // hf - this pointer, but we don't need it
    UINT bytesWritten = 0;
    size_t newPosition = position + cb;
    if (data.size() < newPosition)
    {
      // expand vector for more storage
      data.resize(newPosition, 0);
    }
    memcpy(&data[ position ], pv, cb);

    bytesWritten = cb;
    position = newPosition;

    return bytesWritten;
  }

  FNSEEK(extract_memory_item::fnFileSeek)
  {
    // hf - this pointer, but we don't need it
    long result = -1;
    size_t newPosition = 0;
    switch (seektype)
    {
      // case FILE_BEGIN: // 0 - don't need it, startPosition already initizlized
      case FILE_CURRENT:  // 1
      {
        newPosition = position;
      }
      break;
      case FILE_END:  // 2
      {
        newPosition = data.size();
      }
      break;
    }

    if (dist < 0)
    {
      // moving back
      if (newPosition >= (size_t)std::abs(dist))
      {
        newPosition -= (size_t)std::abs(dist);
      }
      // bad dist
    }
    else
    {
      // moving forward
      newPosition += (size_t)dist;
      if (newPosition > data.size())
      {
        data.resize(newPosition, 0);
      }
    }
    position = newPosition;
    return (long)position;
  }

  FNCLOSE(extract_memory_item::fnFileClose)
  {
    // hf - this pointer
    // nothing to close
    return 0;
  }

  buffer& extract_memory_item::GetBuffer()
  {
    return data;
  }
};  // namespace cab
