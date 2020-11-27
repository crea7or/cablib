// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <windows.h>
#include <algorithm>
#include <string>

namespace StringUtils
{
  // for outside use
  extern const size_t ptrInCharsSize;
  extern const char hexDigits[];
  char HexValue(char hexDigit);

  // convert a wide string to a utf8 string
  std::string ToUtf8(const std::wstring& wstr);

  // to codepage
  std::string WideToMb(const std::wstring& wstr, UINT codePage = CP_UTF8);

  // convert an utf8 string to a wide string
  std::wstring FromUtf8(const std::string& str);

  std::wstring MbToWide(const std::string& str, UINT codePage = CP_UTF8);

  // convert a wchar to a string by truncation (value should be ascii!)
  inline std::string wchar2string(const wchar_t* value)
  {
    std::wstring wstr(value);
    return std::string(wstr.begin(), wstr.end());
  }

  // convert a char to a wide string
  inline std::wstring char2wstring(const char* value)
  {
    std::string str(value);
    return std::wstring(str.begin(), str.end());
  }

  // convert a std::wstring to a string by truncation (value should be ascii!)
  inline std::string wstring2string(const std::wstring& value)
  {
    return std::string(value.begin(), value.end());
  }

  // convert a string to wide string
  inline std::wstring string2wstring(const std::string& value)
  {
    return std::wstring(value.begin(), value.end());
  }

  template <typename T>
  inline void RemoveNonDigits(std::basic_string<T>& string)
  {
    string.erase(std::remove_if(string.begin(), string.end(), [](T x) { return (x >= (T)'0' && (T)'9' <= x); }), string.end());
  }

  // slice string with one delimiter char(wchar...)
  // zero length pieces will be skipped
  template <typename _inputIterator, typename _containerType>
  void each_slice(_inputIterator _begin, _inputIterator _end, typename _inputIterator::value_type _delimiter, _containerType& _conttype)
  {
    _inputIterator _first = _begin, _cur = _begin;
    for (; _begin != _end; ++_begin)
    {
      if (*_begin == _delimiter)
      {
        if (_first != _cur)
        {
          _conttype.push_back(std::basic_string<typename _inputIterator::value_type>(_first, _cur));
        }
        _first = ++_cur;
      }
      else
      {
        ++_cur;
      }
    }
    if (_first != _cur)
    {
      _conttype.push_back(std::basic_string<typename _inputIterator::value_type>(_first, _cur));
    }
  }

  // Slice string with any of delimiters in char*(wchar*...)
  template <typename _inputIterator, typename _containerType>
  void any_slice(_inputIterator _begin, _inputIterator _end, const typename _inputIterator::value_type* _delimiter, _containerType& _conttype)
  {
    size_t indexCurrent = 0, stringSize = 0;
    while (_delimiter[ stringSize ] != 0)
    {
      ++stringSize;
    };

    if (stringSize > 0)
    {
      _inputIterator _first = _begin, _cur = _begin;
      for (; _begin != _end; ++_begin)
      {
        for (indexCurrent = 0; indexCurrent < stringSize; ++indexCurrent)
        {
          if (*_begin == _delimiter[ indexCurrent ])
          {
            break;
          }
        }
        if (indexCurrent != stringSize)
        {
          if (_first != _cur)
          {
            _conttype.push_back(std::basic_string<typename _inputIterator::value_type>(_first, _cur));
          }
          _first = ++_cur;
        }
        else
        {
          ++_cur;
        }
      }
      if (_first != _cur)
      {
        _conttype.push_back(std::basic_string<typename _inputIterator::value_type>(_first, _cur));
      }
    }
  }

  // trim spaces from start (in place)
  template <typename _charType>
  static inline void trimLeft(std::basic_string<_charType>& s, _charType trim)
  {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [ trim ](_charType ch) { return ch != trim; }));
  }

  // trim spaces from end (in place)
  template <typename _charType>
  static inline void trimRight(std::basic_string<_charType>& s, _charType trim)
  {
    s.erase(std::find_if(s.rbegin(), s.rend(), [ trim ](_charType ch) { return ch != trim; }).base(), s.end());
  }

  // trim spaces from both ends (in place)
  template <typename _charType>
  static inline void trim(std::basic_string<_charType>& s, _charType trim)
  {
    trimLeft(s, trim);
    trimRight(s, trim);
  }

  // if input container begins with match
  template <typename _containerType>
  bool begins_with(const _containerType& input, const _containerType& match)
  {
    return input.size() >= match.size() && std::equal(match.begin(), match.end(), input.begin());
  }

  std::string StringToHex(const std::string& input);
  std::string HexToString(const std::string& input);

  std::string PtrToHex(INT_PTR input);
  INT_PTR HexToPtr(const std::string& input);

};  // namespace StringUtils
