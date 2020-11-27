// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <stdint.h>
#include <stdexcept>

class buffer final
{
public:
  enum TYPE
  {
    KEEPER,  // does not manage memory that holding
    HOLDER,  // manage memory by this class, owning transfered here
  };

  buffer() = default;
  // new empty buffer
  buffer(const size_t size);
  // KEEPER - memory viewer (does not own nor manage memeory)
  // HOLDER - allocate new memory for data and copy it to this memory
  buffer(const uint8_t* ptr, const size_t size, TYPE type = HOLDER);
  buffer(const buffer& src);
  buffer(buffer&& src) noexcept;
  ~buffer() noexcept;

  // KEEPER - memory viewer (does not own nor manage memeory)
  // HOLDER - allocate new memory for data and copy it to this memory
  void set(const uint8_t* ptr, const size_t size, TYPE type = HOLDER);

  // subsript access
  uint8_t& operator[](std::size_t idx);
  const uint8_t& operator[](std::size_t idx) const;

  // get data pointer
  uint8_t* data() const noexcept;
  // get size of data
  size_t size() const noexcept;
  // set new size for data (reallocate with copy or without)
  void resize(const size_t new_size, bool copy = true);
  // clear data
  void clear() noexcept;
  // current buffer type
  buffer::TYPE type() const noexcept;

private:
  void allocate(const size_t size, const uint8_t* ptr = nullptr);
  void destroy() noexcept;

  uint8_t* buffer_ptr{nullptr};
  size_t buffer_size{0};
  TYPE buffer_type{HOLDER};
};
