#pragma once

namespace CommonLib
{
	namespace Utility
	{
		namespace Path
		{
			//조회
			bool IsExist(const tstring& fileOrPath);
			LONGLONG FileSize(const tstring& fileName);
			vector<tstring> FolderList(const tstring& path);
			int FolderCount(const tstring& path);
			vector<tstring> FileList(const tstring& path, LPCWSTR* extList, int extListCount);
			vector<tstring> FileList(const tstring& path, const tstring& wildFormat, const function<bool(LPWSTR filename)>& task = nullptr);
			int FileCount(const tstring& path, LPCWSTR* extList, int extListCount);
			int FileCount(const tstring& wildFormatPath);

			tstring ModuleFilePathName();
			tstring ModuleFilePath();

			tstring ShortPathName(const tstring& fileName);

			//조작
			tstring LastComponent(const tstring& filePath);
			tstring RemoveLastComponent(const tstring& filePath);
			tstring Combine(const tstring& path, const tstring& foldername);
			tstring ReplaceExtension(const tstring& fileName, const tstring& ext);
			tstring RemoveExtension(const tstring& fileName);

			bool DeleteFile(const tstring& fileName, bool force = false);
			bool DeleteFolder(const tstring& path, bool force = false);
			bool CreateFolder(const tstring& path, bool force = false);

			void SplitFileName(const tstring& source, tstring* fileName, tstring* fileExtension);
			void Rename(const tstring& source, const tstring& target);

			bool CopyFile(tstring source, tstring target);
			tstring PathExtension(const tstring& path);
		}
	}
}