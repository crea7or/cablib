// CEZEO software Ltd. https://www.cezeo.com
#include "extract_file_worker.h"
#include <fcntl.h>
#include "string_utils.h"

namespace cab
{
  extract_file_worker::extract_file_worker(const std::wstring& cabFullPath, const std::wstring& outputFolder) : cabFullPath(cabFullPath), outputFolder(outputFolder)
  {
  }

  extract_file_worker::~extract_file_worker()
  {
    for (PEXTRACTFILEITEM item : sources)
    {
      delete item;
    }
    for (PEXTRACTFILEITEM item : files)
    {
      delete item;
    }
  }

  FNOPEN(extract_file_worker::fnFileOpen)
  {
    // here we open source cab file (mostry twice)
    // pszFile is not used in this call because we know the source file name
    PEXTRACTFILEITEM fileItemPtr = new extract_file_item(cabFullPath, 0);
    sources.push_back(fileItemPtr);
    // convert to utf-8
    std::string cabFullPathUtf8(StringUtils::WideToMb(cabFullPath, CP_UTF8));
    fileItemPtr->fnFileOpen(&cabFullPathUtf8[ 0 ], oflag, pmode);
    // return pointer to file
    return (INT_PTR)fileItemPtr;
  }

  FNFDINOTIFY(extract_file_worker::fnNotify)
  {
    INT_PTR result = 0;
    PEXTRACTFILEITEM fileItemPtr(nullptr);
    switch (fdint)
    {
      case fdintCOPY_FILE:
      {
        // Append the destination directory to the file name.
        // name in ACP code page
        std::wstring fileName(StringUtils::MbToWide((LPCSTR)pfdin->psz1, CP_UTF8));
        std::wstring filePath(outputFolder + L"\\" + fileName);
        std::string filePathUtf8(StringUtils::WideToMb(filePath, CP_UTF8));
        fileItemPtr = new extract_file_item(filePath, pfdin->cb);
        files.emplace_back(fileItemPtr);
        // Copy file
        result = fileItemPtr->fnFileOpen(&filePathUtf8[ 0 ], _O_WRONLY | _O_CREAT, 0);
      }
      break;

      case fdintCLOSE_FILE_INFO:
      {
        fileItemPtr = (extract_file_item*)pfdin->hf;
        result = !fileItemPtr->fnFileClose(pfdin->hf);
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
};  // namespace cab
