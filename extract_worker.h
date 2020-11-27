// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <fdi.h>

namespace cab
{
  class extract_worker
  {
  public:
    extract_worker(){};
    virtual ~extract_worker(){};

    virtual FNOPEN(fnFileOpen) = 0;
    virtual FNFDINOTIFY(fnNotify) = 0;
  };

  typedef extract_worker* PEXTRACTWORKER;
};  // namespace cab
