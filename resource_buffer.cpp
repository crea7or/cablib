// CEZEO software Ltd. https://www.cezeo.com
#include "resource_buffer.h"

resource_buffer::resource_buffer(const UINT resource_id, const std::wstring& resource_type, const HINSTANCE resource_instance)
{
  HINSTANCE instance_handle = resource_instance;
  if (NULL == instance_handle)
  {
    // get current exe instance handle
    instance_handle = GetModuleHandle(NULL);
  }

  HRSRC resource = FindResource(instance_handle, MAKEINTRESOURCE(resource_id), resource_type.c_str());
  if (NULL != resource)
  {
    size_t resource_size = SizeofResource(instance_handle, resource);
    if (resource_size > 0)
    {
      HGLOBAL global_memory = LoadResource(instance_handle, resource);
      if (NULL != global_memory)
      {
        uint8_t* resource_ptr = (uint8_t*)LockResource(global_memory);
        if (NULL != resource_ptr)
        {
          if (NULL != resource_instance)
          {
            // allocate memory and copy data to it
            buffer_data.set(resource_ptr, resource_size, buffer::TYPE::HOLDER);
          }
          else
          {
            // create keeper buffer without copying/allocation memory
            buffer_data.set(resource_ptr, resource_size, buffer::TYPE::KEEPER);
          }
        }
        else
        {
          throw std::runtime_error("can't lock resource handle");
        }
      }
      else
      {
        throw std::runtime_error("can't load resource handle");
      }
    }
    else
    {
      throw std::runtime_error("resource empty");
    }
  }
  else
  {
    throw std::runtime_error("resource not found");
  }
}

uint8_t* resource_buffer::data() const noexcept
{
  return buffer_data.data();
}

size_t resource_buffer::size() const noexcept
{
  return buffer_data.size();
}