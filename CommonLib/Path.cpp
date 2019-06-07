#include "StdAfx.h"
#include "Path.h"

namespace CommonLib
{
	namespace Utility
	{
		namespace Path
		{
			bool IsExist(const tstring& fileName)
			{
				return PathFileExists(fileName.c_str()) == TRUE;
			}

			LONGLONG FileSize(const tstring& fileName)
			{
				HANDLE hFile = CreateFile(
					fileName.c_str(),                // name of the write
					GENERIC_READ,          // open for writing
					FILE_SHARE_READ | FILE_SHARE_WRITE,// do not share
					NULL,                   // default security
					OPEN_EXISTING,          // overwrite existing
					FILE_ATTRIBUTE_NORMAL,  // normal file
					NULL);                  // no attr. template

				if (hFile == INVALID_HANDLE_VALUE)
				{
					return -1;
				}

				LARGE_INTEGER fileSize;
				GetFileSizeEx(hFile, &fileSize);

				CloseHandle(hFile);

				return fileSize.QuadPart;
			}

			vector<tstring> FolderList(const tstring& path)
			{
				vector<tstring> folderList;

				WIN32_FIND_DATA ffd;

				tstring wildPath = Path::Combine(path, L"*");

				HANDLE hFind = FindFirstFile(wildPath.c_str(), &ffd);

				if (INVALID_HANDLE_VALUE != hFind)
				{
					do
					{
						if (!(ffd.dwFileAttributes&(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)))
						{
							if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							{
								if (String::IsEqual(L".", ffd.cFileName) || String::IsEqual(L"..", ffd.cFileName))
									continue;

								folderList.push_back(ffd.cFileName);
							}
						}
					} while (FindNextFile(hFind, &ffd) != 0);

					FindClose(hFind);
				}

				return folderList;
			}

			int FolderCount(const tstring& path)
			{
				int result = 0;

				WIN32_FIND_DATA ffd;

				tstring wildPath = Path::Combine(path, L"*");

				HANDLE hFind = FindFirstFile(wildPath.c_str(), &ffd);

				if (INVALID_HANDLE_VALUE != hFind)
				{
					do
					{
						if (!(ffd.dwFileAttributes&(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)))
						{
							if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							{
								if (String::IsEqual(L".", ffd.cFileName) || String::IsEqual(L"..", ffd.cFileName))
									continue;

								result++;
							}
						}
					} while (FindNextFile(hFind, &ffd) != 0);

					FindClose(hFind);
				}

				return result;
			}

			vector<tstring> FileList(const tstring& path, const tstring& wildFormat, const function<bool(LPWSTR filename)>& task)
			{
				vector<tstring> fileList;

				WIN32_FIND_DATA ffd;
				HANDLE hFind = FindFirstFile(Path::Combine(path, wildFormat).c_str(), &ffd);

				if (INVALID_HANDLE_VALUE != hFind)
				{
					do
					{
						if (!(ffd.dwFileAttributes&(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)))
						{
							if (!(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
							{
								if (wcslen(ffd.cFileName) > 0)
								{
									if (task != nullptr)
									{
										if (task(ffd.cFileName))
											fileList.push_back(ffd.cFileName);
									}
									else
										fileList.push_back(ffd.cFileName);
								}
							}
						}
					} while (FindNextFile(hFind, &ffd) != 0);

					FindClose(hFind);
				}

				return fileList;
			}

			void SplitFileName(const tstring& source, tstring* fileName, tstring* fileExtension)
			{
				int pointPos = 0;

				for (int i = source.length() - 1; i > 0; --i)
				{
					if (source[i] == '.')
					{
						if (fileExtension)
							*fileExtension = &source[i + 1];

						if (fileName)
							copy(source.begin(), source.begin() + i, back_inserter(*fileName));

						break;
					}
				}
			}

			int FileCount(const tstring& wildFormatPath)
			{
				int count = 0;

				WIN32_FIND_DATA ffd;
				HANDLE hFind = FindFirstFile(wildFormatPath.c_str(), &ffd);

				if (INVALID_HANDLE_VALUE != hFind)
				{
					do
					{
						if (!(ffd.dwFileAttributes&(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY)))
						{
							if (wcslen(ffd.cFileName) > 0)
							{
								count++;
							}
						}
					} while (FindNextFile(hFind, &ffd) != 0);

					FindClose(hFind);
				}

				return count;
			}

			int FileCount(const tstring& path, LPCWSTR* extList, int extListCount)
			{
				int result = 0;

				FileList(path, L"*", [&](LPWSTR filename)
				{
					tstring fileExtension;
					SplitFileName(filename, nullptr, &fileExtension);

					for (int i = 0; i < extListCount; ++i)
					{
						if (0 == _wcsicmp(fileExtension.c_str(), extList[i]))
						{
							result++;
							break;
						}
					}

					return false;
				});

				return result;
			}

			vector<tstring> FileList(const tstring& path, LPCWSTR* extList, int extListCount)
			{
				return FileList(path, L"*", [&](LPWSTR filename)
				{
					tstring fileExtension;
					SplitFileName(filename, nullptr, &fileExtension);

					for (int i = 0; i < extListCount; ++i)
					{
						if (0 == _wcsicmp(fileExtension.c_str(), extList[i]))
							return true;
					}

					return false;
				});
			}

			tstring ModuleFilePathName()
			{
				TCHAR currentPath[MAX_PATH];
				ZeroMemory(currentPath, MAX_PATH);

				GetModuleFileName(NULL, &currentPath[0], MAX_PATH);

				return currentPath;
			}

			tstring ModuleFilePath()
			{
				return RemoveLastComponent(ModuleFilePathName());
			}

			tstring LastComponent(const tstring& filePath)
			{
				tstring fileName = filePath;

				int index = fileName.rfind('\\');

				if (index != tstring::npos)
				{
					if (String::Length(fileName) > index + 1)
						fileName = &fileName.c_str()[index + 1];
					else
						fileName = _T("");
				}

				return fileName;
			}

			tstring RemoveLastComponent(const tstring& filePath)
			{
				tstring fileName = filePath;

				int index = fileName.rfind('\\');

				return index == tstring::npos ? fileName : fileName.substr(0, index);
			}

			tstring RemoveExtension(const tstring& fileName)
			{
				WCHAR filePath[MAX_PATH];
				wcscpy_s(filePath, fileName.c_str());

				PathRemoveExtension(filePath);

				return filePath;
			}

			tstring Combine(const tstring& path, const tstring& foldername)
			{
				TCHAR combinedPath[MAX_PATH + 1];

				wcscpy_s(combinedPath, path.c_str());

				PathAppend(combinedPath, foldername.c_str());

				return combinedPath;
			}

			tstring ReplaceExtension(const tstring& fileName, const tstring& ext)
			{
				WCHAR filePath[MAX_PATH];
				wcscpy_s(filePath, fileName.c_str());

				PathRemoveExtension(filePath);

				tstring subtitlePath = filePath;
				subtitlePath.append(L".");
				subtitlePath.append(ext);

				return subtitlePath;
			}

			bool ForceDelete(const tstring& path)
			{
				if (String::Tokenize(path, '\\').size() <= 2)
				{
					abort();
				}

				vector<BYTE> target(path.length() * 2 + 4);
				ZeroMemory(&target[0], target.size());

				memcpy(&target[0], path.c_str(), path.length() * 2);

				SHFILEOPSTRUCT file_op = {
					NULL,
					FO_DELETE,
					(LPCWSTR)&target[0],
					L"",
					FOF_NOCONFIRMATION |
					FOF_NOERRORUI |
					FOF_NORECURSION |
					FOF_SILENT,
					false,
					0,
					L""
				};

				return 0 == SHFileOperation(&file_op);
			}

			bool DeleteFile(const tstring& fileName, bool force)
			{
				if (force)
					return ForceDelete(fileName);

				return ::DeleteFile(fileName.c_str()) == TRUE;
			}

			bool DeleteFolder(const tstring& path, bool force)
			{
				_ASSERT_EXPR(path.length() > 10, L"C:\\nGinTemp\\ 의 길이는 10이다. 즉, 최소 10은 넘어야 삭제한다.");

				if (String::Tokenize(path, '\\').size() <= 1)
				{
					abort();
				}

				if (path.length() <= 10)
					return false;

				if (force)
					return ForceDelete(path);

				return ::RemoveDirectory(path.c_str()) == TRUE;
			}

			/// Windows 2003 이후에서만 아래 함수를 지원함
			/// int SHCreateDirectory(HWND hwnd, LPCWSTR pszPath);
			bool CreateFolder(const tstring& path, bool force)
			{
				if (force)
				{
					TCHAR *SEP = _T("\\");
					tstring dir;
					tstring src = path;

					// tokenize
					TCHAR* pRlst = NULL;
					TCHAR buf[512] = { 0 };
					TCHAR* next_token = NULL;

					src._Copy_s(buf, 512, src.size());

					pRlst = _tcstok_s(buf, SEP, &next_token);

					while (pRlst != NULL)
					{
						dir += pRlst;
						dir += '\\';

						_tmkdir(dir.c_str());

						pRlst = _tcstok_s(NULL, SEP, &next_token);
					}

					return true;
				}
				else
					return ::CreateDirectory(path.c_str(), NULL) == TRUE;
			}

			tstring ShortPathName(const tstring& fileName)
			{
				WCHAR shortPath[512];
				GetShortPathName(fileName.c_str(), shortPath, 512);

				return shortPath;
			}

			void Rename(const tstring& source, const tstring& target)
			{
				if (IsExist(target))
				{
					THROW_LIBRARY(L"target is exist");
				}

				if (-1 == _wrename(source.c_str(), target.c_str()))
				{
					THROW_LIBRARY(L"rename fail");
				}
			}

			bool CopyFile(tstring source, tstring target)
			{
				return ::CopyFile(source.c_str(), target.c_str(), TRUE) != 0;
			}

			tstring PathExtension(const tstring& path)
			{
				return String::Tokenize(path, '.').back();
			}
		}
	}
}