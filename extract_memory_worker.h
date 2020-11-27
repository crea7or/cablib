// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include "extract_worker.h"
#include "extract_memory_item.h"
#include <string>
#include <vector>
#include <map>

namespace cab
{
  class extract_memory_worker : public extract_worker
  {
  public:
    extract_memory_worker(const uint8_t* sourceData, size_t sourceSize);
    virtual ~extract_memory_worker();

    // called only for source files
    virtual FNOPEN(fnFileOpen) override;
    virtual FNFDINOTIFY(fnNotify) override;

    void MoveFilesTo(std::vector<PEXTRACTMEMORYITEM>& moveToFiles);
    void MoveFilesTo(std::map<std::wstring, buffer>& moveToFiles);

  protected:
    // extracted files
    std::vector<PEXTRACTMEMORYITEM> files;

    // source cab file data
    const uint8_t* cabData{nullptr};
    const size_t cabSize{0};
    // source cab files opened for extraction
    // need a vector because library opens it twice at least for unknown reason
    std::vector<PEXTRACTMEMORYITEM> sources;
  };
  typedef extract_memory_worker* PCABMEMORYWORKER;
};  // namespace cab
