// CEZEO software Ltd. https://www.cezeo.com
#include <iostream>
#include "extractor.h"
#include "compressor.h"
#include "system_utils.h"

// from resources.h
#define IDR_BINARY1 1

int wmain(int argc, wchar_t* argv[])
{
  std::wstring pathToCab; // path?


  // cab file in memeory to memory
  {
    std::string cabData;
    if (SystemUtils::FileToString(pathToCab, cabData))
    {
      std::map<std::wstring, buffer> files;
      cab::extractor cab;
      cab.memory_to_memory((uint8_t*)&cabData[0], cabData.size(), files);
      std::wcout << L"extracted: " << files.size() << std::endl;
    }
  }

  // extract resource to memory
  {
    std::map<std::wstring, buffer> files;
    cab::extractor cab;
    cab.resources_to_memory(IDR_BINARY1, files);
    std::wcout << L"extracted: " << files.size() << std::endl;
    for (const std::pair<std::wstring, buffer>& item : files)
    {
      std::wcout << L"file: " << item.first << " size: " << item.second.size() << std::endl;
    }
  }

  // extract cab to folder
  {
    cab::extractor cab;
    cab.cab_to_files(pathToCab, L"m:\\ttt1");
  }

  // extract
  {
    cab::compressor caber;
    std::vector<std::wstring> filesVector;
    filesVector.push_back(L"C:\\file\\vc141.pdb");
    filesVector.push_back(L"C:\\file\\vc141.obj");
    caber.files_to_cab(L"C:\\temp.cab", filesVector);
  }
}
