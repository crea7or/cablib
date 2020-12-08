// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <stdint.h>
#include <stdexcept>
#include <iterator>

template < typename T >
class ptr_iterator : public std::iterator< std::forward_iterator_tag, T >
{
  typedef ptr_iterator< T > iterator;
  pointer pos_;

public:
  ptr_iterator() : pos_(nullptr)
  {}
  ptr_iterator(T* v) : pos_(v)
  {}
  ~ptr_iterator() = default;

  iterator operator++(int)  // postfix
  {
    return pos_++;
  }
  iterator& operator++()  // prefix
  {
    ++pos_;
    return *this;
  }
  reference operator*() const
  {
    return *pos_;
  }
  pointer operator->() const
  {
    return pos_;
  }
  iterator operator+(difference_type v) const
  {
    return pos_ + v;
  }
  bool operator==(const iterator& rhs) const
  {
    return pos_ == rhs.pos_;
  }
  bool operator!=(const iterator& rhs) const
  {
    return pos_ != rhs.pos_;
  }
};

template < typename T >
ptr_iterator< T > begin(T* val)
{
  return ptr_iterator< T >(val);
}

template < typename T, typename Tsize >
ptr_iterator< T > end(T* val, Tsize size)
{
  return ptr_iterator< T >(val) + size;
}

class buffer final
{
public:
  enum TYPE
  {
    KEEPER,  // does not manage memory that holding
    HOLDER,  // manage memory by this class, owning transfered here
  };

  buffer() noexcept = default;
  // new empty buffer
  buffer(const size_t size);
  // KEEPER - memory viewer (does not own nor manage memeory)
  // HOLDER - allocate new memory for data and copy it to this memory
  buffer(const uint8_t* ptr, const size_t size, const TYPE type = HOLDER);
  buffer(const buffer& src);
  buffer(buffer&& src) noexcept;
  ~buffer() noexcept;
  // assignment
  buffer& operator=(const buffer& src);

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
  // if buffer is empty
  bool empty() const noexcept;

  // swap objects
  friend void swap(buffer& a, buffer& b) noexcept
  {
    // enable ADL
    using std::swap;
    swap(a.buffer_ptr, b.buffer_ptr);
    swap(a.buffer_size, b.buffer_size);
    swap(a.buffer_type, b.buffer_type);
  }

  // current buffer type
  buffer::TYPE type() const noexcept;

  ptr_iterator< uint8_t > begin() const;
  ptr_iterator< uint8_t > end() const;

private:
  // allocate memory and optionally copy from copy_from_ptr to allocated buffer
  void allocate_copy(const size_t size, const uint8_t* copy_from_ptr = nullptr);
  void destroy() noexcept;

  uint8_t* buffer_ptr{nullptr};
  size_t buffer_size{0};
  TYPE buffer_type{KEEPER};
};
