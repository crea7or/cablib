// CEZEO software Ltd. https://www.cezeo.com
#include "compressor.h"
#include "system_utils.h"
#include "string_utils.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#pragma region cab crap

#define CYAN (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define GREY (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)

void print(WORD color, const std::string& name, const char* valuePtr = nullptr)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
  BOOL restore = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleScreenBufferInfo);
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
  printf(name.c_str());
  if (nullptr != valuePtr)
  {
    printf(": ");
    printf(valuePtr);
  }
  printf("\n");
  if (restore)
  {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), consoleScreenBufferInfo.wAttributes);
  }
}

int wmain(int cmdArguments, wchar_t* argsListPtr[])
{
  int result = -1;
  if (cmdArguments < 3 || cmdArguments > 4)
  {
    print(RED, "wrong arguments, use: makecabx 'cab_file_path' 'files_list_to_compress_path' (optional)'path_to_filenames_in_list'");
  }
  else
  {
    std::wstring cabFilePath(argsListPtr[ 1 ]);
    std::wstring fileToCompressPath(argsListPtr[ 2 ]);
    std::wstring fileToCompressFolder;
    if (4 == cmdArguments)
    {
      // path to files from command line 'path_to_filenames_in_list'
      fileToCompressFolder = argsListPtr[ 3 ];
    }
    else
    {
      // path to files same as 'files_list_to_compress_path'
      fileToCompressFolder = SystemUtils::GetFolderFromPath(fileToCompressPath);
    }

    if (fileToCompressFolder.empty())
    {
      print(RED, "empty path to files supplied");
      return 97;
    }

    // read files list
    std::vector<std::string> configFiles;
    std::vector<std::wstring> compressFiles;

    uint64_t size = 0;
    if (SystemUtils::GetFileSize(fileToCompressPath, size))
    {
      if (size > 0)
      {
        std::string data;
        if (SystemUtils::FileToString(fileToCompressPath, data))
        {
          StringUtils::any_slice(data.begin(), data.end(), "\n\r", configFiles);
          configFiles.erase(std::remove_if(configFiles.begin(), configFiles.end(),
                                           [ &fileToCompressFolder, &compressFiles ](std::string& fn) {
                                             if (fn.size() > 0 && fn[ 0 ] != '.' && fn[ 0 ] != ';')
                                             {
                                               // remove quotes if any
                                               StringUtils::trim(fn, '"');
                                               // build full path
                                               std::wstring fwpath(SystemUtils::PathAppendFileName(fileToCompressFolder, StringUtils::string2wstring(fn)));
                                               if (PathFileExists(fwpath.c_str()))
                                               {
                                                 compressFiles.emplace_back(fwpath);
                                                 return false;
                                               }
                                               else
                                               {
                                                 print(RED, "source file is not available", StringUtils::ToUtf8(fwpath).c_str());
                                                 return true;
                                               }
                                             }
                                             else
                                             {
                                               return true;
                                             }
                                           }),
                            configFiles.end());
        }
        else
        {
          print(RED, "can't open files list to compress", StringUtils::ToUtf8(fileToCompressPath).c_str());
        }
      }
      else
      {
        print(RED, "empty files list to compress", StringUtils::ToUtf8(fileToCompressPath).c_str());
      }
    }
    else
    {
      print(RED, "can't open file with files list to compress", StringUtils::ToUtf8(fileToCompressPath).c_str());
    }

    if (compressFiles.size() == 0)
    {
      print(RED, "no vaild files found in list to compress", StringUtils::ToUtf8(fileToCompressPath).c_str());
    }
    else
    {
      bool createNewCab = true;
      // check if target exists - compare modification time with sources to detect wether we need to update cab or not
      if (PathFileExists(cabFilePath.c_str()))
      {
        uint64_t cabFileTime = SystemUtils::GetFileLastWriteTime(cabFilePath.c_str());
        // check modification time of files list
        createNewCab = cabFileTime <= SystemUtils::GetFileLastWriteTime(fileToCompressPath.c_str());
        if (!createNewCab)
        {
          for (const std::wstring& file : compressFiles)
          {
            if (SystemUtils::GetFileLastWriteTime(file) >= cabFileTime)
            {
              createNewCab = true;
              break;
            }
          }
        }
      }

      if (createNewCab)
      {
        try
        {
          cab::compressor compr;
          compr.files_to_cab(cabFilePath, compressFiles);
          print(GREEN, "cab file created", StringUtils::ToUtf8(cabFilePath).c_str());
          result = 0;
        }
        catch (const std::exception& ex)
        {
          print(RED, ex.what());
        }
      }
      else
      {
        // file shouldn't be updated because source files are not updated
        print(GREEN, "cab file exists and sources are older", StringUtils::ToUtf8(cabFilePath).c_str());
        result = 0;
      }
    }
  }

  return result;
}
