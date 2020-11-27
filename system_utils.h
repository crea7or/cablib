// CEZEO software Ltd. https://www.cezeo.com
#pragma once

#include <windows.h>
#include <string>
#include <stdint.h>

namespace SystemUtils
{
  extern const wchar_t localAppData[];
  extern const wchar_t pathSeparator;

  // used to add filename to the path with checking the trailing backward and forward slashes
  std::wstring PathAppendFileName(const std::wstring& path, const std::wstring& fileName);

  // get folder from path to file
  std::wstring GetFolderFromPath(const std::wstring& fileName);

  // used to extract filename from the path
  std::wstring GetFileNameFromPath(const std::wstring& fileFullPath);

  // return lowercase file extension
  std::wstring GetExtensionFromPath(const std::wstring& fileName);

  // current module folder without trailing slash
  std::wstring GetModuleFolder();

  // current module full path
  std::wstring GetModulePath();

  // current module name
  std::wstring GetModuleFile();

  // get last write time for file
  uint64_t GetFileLastWriteTime(const std::wstring& file);

  // get file size
  bool GetFileSize(const std::wstring& path, uint64_t& fileSize);

  // load file to buffer
  bool FileToString(const std::wstring& filePath, std::string& data, size_t maxSize = 1024 * 1024);

  // generate temp file name (full path) and return in tempFileName
  bool GenerateTempFileName(std::wstring& tempFileName);

  // get temp folder
  bool GetTempFolder(std::wstring& tempFolder);

};  // namespace SystemUtils
