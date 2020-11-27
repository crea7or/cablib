// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include "extract_worker.h"
#include "extract_file_item.h"
#include <string>
#include <vector>

namespace cab
{
  class extract_file_worker : public extract_worker
  {
  public:
    extract_file_worker(const std::wstring& cabFullPath, const std::wstring& outputFolder);
    virtual ~extract_file_worker();

    // called only for source files
    virtual FNOPEN(fnFileOpen) override;
    virtual FNFDINOTIFY(fnNotify) override;

  protected:
    // extracted files
    std::vector<PEXTRACTFILEITEM> files;
    // source cab files opened for extraction
    // need a vector because library opens it twice at least for unknown reason
    std::vector<PEXTRACTFILEITEM> sources;
    // full path to the cab file path
    const std::wstring cabFullPath;
    const std::wstring outputFolder;
  };

  typedef extract_file_worker* PEXTRACTFILEWORKER;
};