// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <windows.h>
#include <string>

class dll final
{
public:
  dll() noexcept {};
  dll(const dll& src) = delete;
  dll(dll&& src) noexcept
  {
    dllHandle = src.dllHandle;
    src.dllHandle = nullptr;
  }

  dll(const std::string& libraryName) noexcept
  {
    load(libraryName);
  }

  dll(const std::wstring& libraryName) noexcept
  {
    load(libraryName);
  }

  ~dll() noexcept
  {
    if (nullptr != dllHandle)
    {
      FreeLibrary(dllHandle);
    }
  }

  HMODULE load(const std::string& libraryName) noexcept
  {
    dllHandle = LoadLibraryA(libraryName.c_str());
    return dllHandle;
  }

  HMODULE load(const std::wstring& libraryName) noexcept
  {
    dllHandle = LoadLibraryW(libraryName.c_str());
    return dllHandle;
  }

  operator HMODULE() const noexcept
  {
    return dllHandle;
  }

private:
  HMODULE dllHandle{nullptr};
};
