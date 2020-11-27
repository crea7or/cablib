// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <fci.h>
#include <map>
#include "buffer.h"
#include "buffer.h"
#include "dll.h"

namespace cab
{
  class compressor final
  {
  public:
    compressor();
    ~compressor();

    // complete routines
    void file_to_cab(const std::wstring& target_path, const std::wstring& file_path);
    void files_to_cab(const std::wstring& target_path, const std::vector<std::wstring>& files);

  private:
    void create_context(const std::wstring& target_path);
    void destroy_context();

    void add_file(const std::wstring& filePath);

    // cabinet.dll FCI functions pointers
    typedef HFCI(DIAMONDAPI* fnFCICreate)(PERF, PFNFCIFILEPLACED, PFNFCIALLOC, PFNFCIFREE, PFNFCIOPEN, PFNFCIREAD, PFNFCIWRITE, PFNFCICLOSE, PFNFCISEEK, PFNFCIDELETE, PFNFCIGETTEMPFILE, PCCAB, void*);
    typedef BOOL(DIAMONDAPI* fnFCIAddFile)(HFCI, char*, char*, BOOL, PFNFCIGETNEXTCABINET, PFNFCISTATUS, PFNFCIGETOPENINFO, TCOMP);
    typedef BOOL(DIAMONDAPI* fnFCIFlushFolder)(HFCI, PFNFCIGETNEXTCABINET, PFNFCISTATUS);
    typedef BOOL(DIAMONDAPI* fnFCIFlushCabinet)(HFCI, BOOL, PFNFCIGETNEXTCABINET, PFNFCISTATUS);
    typedef BOOL(DIAMONDAPI* fnFCIDestroy)(HFCI);

    fnFCICreate pfnFCICreate{nullptr};
    fnFCIAddFile pfnFCIAddFile{nullptr};
    fnFCIFlushFolder pfnFCIFlushFolder{nullptr};
    fnFCIFlushCabinet pfnFCIFlushCabinet{nullptr};
    fnFCIDestroy pfnFCIDestroy{nullptr};

    HFCI compressHandler{nullptr};
    ERF compressErrors;
    dll cabinetDll;
    CCAB cabParams;
  };
};  // namespace cab
