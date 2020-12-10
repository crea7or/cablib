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
  allocate_copy(size);
  // zero sized buffer is not an error
}

buffer::buffer(const uint8_t* ptr, const size_t size, const TYPE type)
{
  set(ptr, size, type);
}

buffer::buffer(const buffer& src)
{
  if (KEEPER == src.buffer_type)
  {
    // just copy ptr, size and type
    // doesn't hold memory
    buffer_ptr = src.buffer_ptr;
    buffer_size = src.buffer_size;
  }
  else
  {
    // so we should allocate and copy the memory if the source hold it
    // allocate new memory if source is holder also copy from buffer
    allocate_copy(src.buffer_size, src.buffer_ptr);
  }
  buffer_type = src.buffer_type;
}

buffer::buffer(buffer&& src) noexcept : buffer()
{
  swap(*this, src);
}

buffer::~buffer()
{
  destroy();
}

void buffer::allocate_copy(const size_t size, const uint8_t* copy_from_ptr)
{
  if (size > 0)
  {
    uint8_t* new_buffer_ptr = (uint8_t*)malloc(size);
    if (nullptr != new_buffer_ptr)
    {
      // now we can destroy old buffer
      destroy();
      // assign new buffer to current
      buffer_ptr = new_buffer_ptr;
      buffer_size = size;
      // now we're holder
      buffer_type = HOLDER;
      if (copy_from_ptr != nullptr)
      {
        memcpy(buffer_ptr, copy_from_ptr, size);
      }
    }
    else
    {
      throw std::runtime_error(noMemory);
    }
  }
  else
  {
    // zero size allocate effectively just destroying the old data if any
    destroy();
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

// KEEPER - memory viewer (does not own nor manage memory)
// HOLDER - allocate new memory for data and copy it to this memory
void buffer::set(const uint8_t* ptr, const size_t size, TYPE type)
{
  if (HOLDER == type)
  {
    // allocate new memory if source is holder also copy from buffer
    // allocate will destroy old data
    allocate_copy(size, ptr);
  }
  else
  {
    // KEEPER
    destroy();
    // keeper just use supplied ptr and size
    buffer_ptr = const_cast<uint8_t*>(ptr);  // workaround for keeper unmodified buffer
    buffer_size = size;
  }
  buffer_type = type;
}

void buffer::resize(const size_t new_size, bool copy)
{
  // allow resize of empty KEEPER
  if (KEEPER == buffer_type && !empty())
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
        // important to set the holder, because we can be here from empty KEEPER(default constructed buffer)
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

buffer& buffer::operator=(const buffer& src)
{
  if (&src != this)
  {
    buffer temporary(src);
    swap(*this, temporary);
  }
  return *this;
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

bool buffer::empty() const noexcept
{
  return nullptr == buffer_ptr || 0 == buffer_size;
}

buffer::TYPE buffer::type() const noexcept
{
  return buffer_type;
}

ptr_iterator<uint8_t> buffer::begin() const
{
  return ptr_iterator<uint8_t>(data());
}

ptr_iterator<uint8_t> buffer::end() const
{
  return ptr_iterator<uint8_t>(data() + size());
}
