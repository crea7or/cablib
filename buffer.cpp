// CEZEO software Ltd. https://www.cezeo.com
#include "buffer.h"

namespace
{
  char noMemory[] = "can't allocate memory";
  char zeroBuffer[] = "zero size buffer";
  char resizeForKeeper[] = "can't resize non owning buffer";
  char invalidRange[] = "access invalid range";
}  // namespace

buffer::buffer(const size_t size)
{
  allocate(size);
}

buffer::buffer(const uint8_t* ptr, const size_t size, TYPE type)
{
  set(ptr, size, type);
}

buffer::buffer(const buffer& src)
{
  if (KEEPER == buffer_type)
  {
    // just copy ptr, size and type
    // doesn't hold memory
    buffer_ptr = src.buffer_ptr;
    buffer_size = src.buffer_size;
  }
  else
  {
    // allocate new memory if source is holder also copy from buffer
    allocate(src.buffer_size, src.buffer_ptr);
  }
  buffer_type = src.buffer_type;
}

buffer::buffer(buffer&& src) noexcept
{
  buffer_ptr = src.buffer_ptr;
  buffer_size = src.buffer_size;
  buffer_type = src.buffer_type;
  // leave src in correct, empty state
  src.buffer_ptr = nullptr;
  src.buffer_size = 0;
}

buffer::~buffer()
{
  destroy();
}

void buffer::allocate(const size_t size, const uint8_t* ptr)
{
  destroy();
  if (size > 0)
  {
    buffer_ptr = (uint8_t*)malloc(size);
    if (nullptr != buffer_ptr)
    {
      buffer_size = size;
      // now we're holder
      buffer_type = HOLDER;
      if (ptr != nullptr)
      {
        memcpy(buffer_ptr, ptr, size);
      }
    }
    else
    {
      throw std::runtime_error(noMemory);
    }
  }
  else
  {
    throw std::logic_error(zeroBuffer);
  }
}

void buffer::destroy() noexcept
{
  if (KEEPER != buffer_type && nullptr != buffer_ptr)
  {
    free(buffer_ptr);
  }
  buffer_ptr = nullptr;
  buffer_size = 0;
}

// KEEPER - memory viewer (does not own nor manage memeory)
// HOLDER - allocate new memory for data and copy it to this memory
void buffer::set(const uint8_t* ptr, const size_t size, TYPE type)
{
  destroy();
  if (nullptr != ptr && size > 0 && HOLDER == type)
  {
    // allocate new memory if source is holder also copy from buffer
    allocate(size, ptr);
  }
  else
  {
    // keeper just use supplied ptr and size
    buffer_ptr = const_cast<uint8_t*>(ptr);  // workaround for keeper unmodified buffer
    buffer_size = size;
  }
  buffer_type = type;
}

void buffer::resize(const size_t new_size, bool copy)
{
  if (KEEPER == buffer_type)
  {
    throw std::logic_error(resizeForKeeper);
  }
  else
  {
    if (new_size > 0)
    {
      uint8_t* new_buffer_ptr = (uint8_t*)malloc(new_size);
      if (nullptr != new_buffer_ptr)
      {
        // copy old data to new buffer
        if (copy && size() > 0)
        {
          size_t to_copy = size();
          if (size() > new_size)
          {
            to_copy = new_size;
            // copy old data
            memcpy(new_buffer_ptr, data(), to_copy);
          }
        }
        // destroy old buffer
        destroy();
        // assign new buffer to current
        buffer_ptr = new_buffer_ptr;
        buffer_size = new_size;
        buffer_type = HOLDER;
      }
      else
      {
        throw std::runtime_error(noMemory);
      }
    }
    else
    {
      // just destroy buffer
      destroy();
    }
  }
}

uint8_t& buffer::operator[](std::size_t idx)
{
  if (idx < size())
  {
    return buffer_ptr[ idx ];
  }
  else
  {
    throw std::out_of_range(invalidRange);
  }
}

const uint8_t& buffer::operator[](std::size_t idx) const
{
  if (idx < size())
  {
    return buffer_ptr[ idx ];
  }
  else
  {
    throw std::out_of_range(invalidRange);
  }
}

uint8_t* buffer::data() const noexcept
{
  return buffer_ptr;
}

size_t buffer::size() const noexcept
{
  return buffer_size;
}

void buffer::clear() noexcept
{
  destroy();
}

buffer::TYPE buffer::type() const noexcept
{
  return buffer_type;
}