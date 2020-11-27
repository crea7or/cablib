// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <windows.h>
#include <stdint.h>
#include <string>

#include "buffer.h"

class resource_buffer final
{
public:
  resource_buffer(const UINT resource_id, const std::wstring& resource_type = L"BINARY", const HINSTANCE resource_instance = NULL);
  ~resource_buffer(){};

  // get data ptr, valid until resoure buffer is valid
  uint8_t* data() const noexcept;
  // get size of data
  size_t size() const noexcept;

private:
  buffer buffer_data;
};
