// CEZEO software Ltd. https://www.cezeo.com
#include "extractor.h"
#include "extract_worker.h"
#include "extract_file_worker.h"
#include "extract_memory_worker.h"
#include "extract_item.h"
#include <fcntl.h>
#include "string_utils.h"
#include "resource_buffer.h"

namespace cab
{
  namespace
  {
    // FDI callbacks
    // will not override
    FNALLOC(fnMemAlloc)
    {
      return HeapAlloc(GetProcessHeap(), NULL, cb);
    }

    // will not override
    FNFREE(fnMemFree)
    {
      HeapFree(GetProcessHeap(), NULL, pv);
    }

    // worker handled
    FNOPEN(fnFileOpen)
    {
      // here we have CabWorker object pointer in the pszFile
      if (pszFile != nullptr)
      {
        std::string hexPointer(pszFile);
        if (hexPointer.size() == StringUtils::ptrInCharsSize)
        {
          hexPointer = hexPointer.substr(0, StringUtils::ptrInCharsSize);
          PEXTRACTWORKER cabWorkerPtr = reinterpret_cast<PEXTRACTWORKER>(StringUtils::HexToPtr(hexPointer));
          if (cabWorkerPtr != nullptr)
          {
            return cabWorkerPtr->fnFileOpen(pszFile, oflag, pmode);
          }
        }
      }
      return 0;
    }

    // item handled
    FNREAD(fnFileRead)
    {
      PEXTRACTITEM cabItemPtr((PEXTRACTITEM)hf);
      return cabItemPtr->fnFileRead(hf, pv, cb);
    }

    // item handled
    FNWRITE(fnFileWrite)
    {
      PEXTRACTITEM cabItemPtr((PEXTRACTITEM)hf);
      return cabItemPtr->fnFileWrite(hf, pv, cb);
    }

    // item handled
    FNSEEK(fnFileSeek)
    {
      PEXTRACTITEM cabItemPtr((PEXTRACTITEM)hf);
      return cabItemPtr->fnFileSeek(hf, dist, seektype);
    }

    // item handled
    FNCLOSE(fnFileClose)
    {
      PEXTRACTITEM cabItemPtr((PEXTRACTITEM)hf);
      return cabItemPtr->fnFileClose(hf);
    }

    // worker handled
    FNFDINOTIFY(fnNotify)
    {
      PEXTRACTWORKER cabWorkerPtr((PEXTRACTWORKER)pfdin->pv);
      return cabWorkerPtr->fnNotify(fdint, pfdin);
    }
  }  // namespace

  extractor::extractor()
  {
    memset(&extractErrors, 0, sizeof(extractErrors));

#ifdef CAB_STATIC_LINK
    // use fci.lib
    pfnFDICreate = FDICreate;
    pfnFDICopy = FDICopy;
    pfnFDIDestroy = FDIDestroy;
#else
    // dynamic link to cabinet.dll
    cabinetDll.load("cabinet.dll");
    if (nullptr != cabinetDll)
    {
      pfnFDICreate = (fnFDICreate)GetProcAddress(cabinetDll, "FDICreate");
      pfnFDICopy = (fnFDICopy)GetProcAddress(cabinetDll, "FDICopy");
      pfnFDIDestroy = (fnFDIDestroy)GetProcAddress(cabinetDll, "FDIDestroy");
    }
    else
    {
      throw(std::runtime_error("cabinet.dll can not be loaded"));
    }
#endif

    if (nullptr == pfnFDICreate || nullptr == pfnFDICopy || nullptr == pfnFDIDestroy)
    {
      throw(std::runtime_error("fdi funciton are not found"));
    }

    // create FDI context
    create_context();
  }

  extractor::~extractor()
  {
    destroy_context();
  }

  void extractor::create_context()
  {
    // create FDI context
    extractHandler = pfnFDICreate(fnMemAlloc, fnMemFree, fnFileOpen, fnFileRead, fnFileWrite, fnFileClose, fnFileSeek, cpu80386, &extractErrors);
    if (nullptr == extractHandler)
    {
      throw(std::runtime_error("can't create fdi context"));
    }
  }

  void extractor::destroy_context()
  {
    if (nullptr != extractHandler)
    {
      pfnFDIDestroy(extractHandler);
      extractHandler = nullptr;
    }
  }

  bool extractor::cab_to_files(const std::wstring& cabPath, const std::wstring& outputDir)
  {
    bool result = false;
    try
    {
      if (CreateDirectory(outputDir.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
      {
        extract_file_worker fileWorker(cabPath, outputDir);
        std::string ptrs(StringUtils::PtrToHex((INT_PTR)(&fileWorker)));
        result = pfnFDICopy(extractHandler, &ptrs[ 0 ], &ptrs[ 0 ], 0, fnNotify, NULL, (void*)(&fileWorker)) == TRUE;
      }
    }
    catch (...)
    {
      // do nothing
    }
    return result;
  }

  bool extractor::memory_to_memory(uint8_t* sourceData, size_t sourceSize, std::map<std::wstring, buffer>& output)
  {
    bool result = false;
    try
    {
      extract_memory_worker memoryWorker(sourceData, sourceSize);
      std::string ptrs(StringUtils::PtrToHex((INT_PTR)(&memoryWorker)));
      result = pfnFDICopy(extractHandler, &ptrs[ 0 ], &ptrs[ 0 ], 0, fnNotify, NULL, (void*)(&memoryWorker)) == TRUE;
      memoryWorker.MoveFilesTo(output);
    }
    catch (...)
    {
      // do nothing
    }
    return result;
  }

  bool extractor::resources_to_memory(const UINT cabResourceId, std::map<std::wstring, buffer>& output)
  {
    bool result = false;
    try
    {
      // assume binary resoruce type and current module instance
      resource_buffer resource(cabResourceId);
      extract_memory_worker memoryWorker(resource.data(), resource.size());
      std::string ptrs(StringUtils::PtrToHex((INT_PTR)(&memoryWorker)));
      result = pfnFDICopy(extractHandler, &ptrs[ 0 ], &ptrs[ 0 ], 0, fnNotify, NULL, (void*)(&memoryWorker)) == TRUE;
      memoryWorker.MoveFilesTo(output);
    }
    catch (...)
    {
      // do nothing
    }
    return result;
  }
};  // namespace cab
