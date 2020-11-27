// CEZEO software Ltd. https://www.cezeo.com
#include "string_utils.h"

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

namespace StringUtils
{
  const size_t ptrInCharsSize = (sizeof(INT_PTR) * 2);
  const char hexDigits[] = "0123456789abcdef";

  char HexValue(char hexDigit)
  {
    switch (hexDigit)
    {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        return hexDigit - '0';

      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
        return hexDigit - 'A' + 10;

      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
        return hexDigit - 'a' + 10;
    }
    return 0;
  }

  std::string ToUtf8(const std::wstring& wstr)
  {
    return WideToMb(wstr, CP_UTF8);
  }

  // to codepage
  std::string WideToMb(const std::wstring& wstr, UINT codePage)
  {
    if (wstr.empty())
      return std::string();
    int size_needed = WideCharToMultiByte(codePage, 0, &wstr[ 0 ], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(codePage, 0, &wstr[ 0 ], (int)wstr.size(), &strTo[ 0 ], size_needed, NULL, NULL);
    return strTo;
  }

  std::wstring FromUtf8(const std::string& str)
  {
    return MbToWide(str, CP_UTF8);
  }

  std::wstring MbToWide(const std::string& str, UINT codePage)
  {
    if (str.empty())
      return std::wstring();
    int size_needed = MultiByteToWideChar(codePage, 0, &str[ 0 ], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(codePage, 0, &str[ 0 ], (int)str.size(), &wstrTo[ 0 ], size_needed);
    return wstrTo;
  }

  std::string StringToHex(const std::string& input)
  {
    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input)
    {
      output.push_back(hexDigits[ c >> 4 ]);
      output.push_back(hexDigits[ c & 15 ]);
    }
    return output;
  }

  std::string HexToString(const std::string& input)
  {
    std::string output;
    const size_t len = input.length();
    if ((len & 1) == 0)
    {
      output.reserve(len / 2);
      for (auto it = input.begin(); it != input.end();)
      {
        char hi = HexValue(*it++);
        char lo = HexValue(*it++);
        output.push_back(hi << 4 | lo);
      }
    }
    return output;
  }


  std::string PtrToHex(INT_PTR input)
  {
    std::string result(ptrInCharsSize, char(0));
    for (size_t index = 0; index < ptrInCharsSize; ++index)
    {
      result[index] = hexDigits[input & 0x0f];
      input >>= 4;
    }
    return result;
  }

  INT_PTR HexToPtr(const std::string& input)
  {
    INT_PTR result = 0;
    if (input.size() == ptrInCharsSize)
    {
      for (size_t index = ptrInCharsSize; index > 0; --index)
      {
        result <<= 4;
        result |= HexValue(input[index - 1]);
      }
    }
    return result;
  }
}  // namespace StringUtils