// CEZEO software Ltd. https://www.cezeo.com
#include "extract_memory_worker.h"
#include <fcntl.h>
#include "string_utils.h"

namespace cab
{
  namespace
  {
    const char notUniqueKey[] = "not unique key in map";
  }

  extract_memory_worker::extract_memory_worker(const uint8_t* sourceData, size_t sourceSize) : cabData(sourceData), cabSize(sourceSize)
  {
  }

  extract_memory_worker::~extract_memory_worker()
  {
    for (PEXTRACTMEMORYITEM item : sources)
    {
      delete item;
    }
    for (PEXTRACTMEMORYITEM item : files)
    {
      delete item;
    }
  }

  FNOPEN(extract_memory_worker::fnFileOpen)
  {
    // here we open source cab file (mostry twice)
    // pszFile is not used in this call because we know the source file name
    // this is source cab file
    PEXTRACTMEMORYITEM memoryItemPtr = new extract_memory_item(cabData, cabSize);
    sources.push_back(memoryItemPtr);
    // return pointer to file
    return (INT_PTR)memoryItemPtr;
  }

  FNFDINOTIFY(extract_memory_worker::fnNotify)
  {
    INT_PTR result = 0;
    PEXTRACTMEMORYITEM memoryItemPtr(nullptr);
    switch (fdint)
    {
      case fdintCOPY_FILE:
      {
        // Append the destination directory to the file name.
        // name in CP_UTF8 code page
        std::wstring fileName(StringUtils::MbToWide((LPCSTR)pfdin->psz1, CP_UTF8));
        memoryItemPtr = new extract_memory_item(fileName, pfdin->cb);
        files.emplace_back(memoryItemPtr);
        result = (INT_PTR)memoryItemPtr;
      }
      break;

      case fdintCLOSE_FILE_INFO:
      {
        memoryItemPtr = (extract_memory_item*)pfdin->hf;
        if (memoryItemPtr != nullptr)
        {
          result = !memoryItemPtr->fnFileClose(pfdin->hf);
        }
      }
      break;

      case fdintNEXT_CABINET:
      {
        if (pfdin->fdie != FDIERROR_NONE)
        {
          result = -1;
        }
      }
      break;

      case fdintPARTIAL_FILE:
      {
        result = 0;
      }
      break;

      case fdintCABINET_INFO:
      {
        result = 0;
      }
      break;

      case fdintENUMERATE:
      {
        result = 0;
      }
      break;

      default:
      {
        result = -1;
      }
      break;
    }

    return result;
  }

  void extract_memory_worker::MoveFilesTo(std::vector<PEXTRACTMEMORYITEM>& moveToFiles)
  {
    moveToFiles = std::move(files);
  }

  void extract_memory_worker::MoveFilesTo(std::map<std::wstring, buffer>& moveToFiles)
  {
    std::pair<std::map<std::wstring, buffer>::iterator, bool> result;
    for (PEXTRACTMEMORYITEM memItem : files)
    {
      result = moveToFiles.insert({memItem->GetFileName(), std::move(memItem->GetBuffer())});
      if (!result.second)
      {
        throw std::logic_error(notUniqueKey);
      }
    }
  }
};  // namespace cab
