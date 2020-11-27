// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <fdi.h>
#include <map>
#include "buffer.h"
#include "dll.h"

namespace cab
{
  class extractor final
  {
  public:
    extractor();
    ~extractor();

    bool cab_to_files(const std::wstring& cabPath, const std::wstring& outputDir);
    bool memory_to_memory(uint8_t* sourceData, size_t sourceSize, std::map<std::wstring, buffer>& output);
    bool resources_to_memory(const UINT cabResourceId, std::map<std::wstring, buffer>& output);

  private:
    void create_context();
    void destroy_context();

    // cabinet.dll FDI functions pointers
    typedef HFDI (*fnFDICreate)(PFNALLOC, PFNFREE, PFNOPEN, PFNREAD, PFNWRITE, PFNCLOSE, PFNSEEK, int, PERF);
    typedef BOOL (*fnFDICopy)(HFDI, LPSTR, LPSTR, INT, PFNFDINOTIFY, PFNFDIDECRYPT, void*);
    typedef BOOL (*fnFDIDestroy)(HFDI);

    fnFDICreate pfnFDICreate{nullptr};
    fnFDICopy pfnFDICopy{nullptr};
    fnFDIDestroy pfnFDIDestroy{nullptr};

    HFDI extractHandler{nullptr};
    ERF extractErrors;
    dll cabinetDll;
  };
};  // namespace cab
