#include "stdafx.h"
#include "FFMpegCommand.h"
#include "MovieMetadata.h"

FFMpegCommand::FFMpegCommand(const Json::JsonDictionary& recvData)
{
	sourceFile = recvData.Path(L"TargetPath");

	if (recvData.IsExistKey(L"VideoIndex"))
		videoIndex = recvData.Integer(L"VideoIndex");
	else
		videoIndex = 0;

	if (recvData.IsExistKey(L"AudioIndex"))
		audioIndex = recvData.Integer(L"AudioIndex");
	else
		audioIndex = -1;

	startTime = recvData.Integer(L"StartTime");

	MediaInformation::MovieMetadata movieInfo(sourceFile);

	if (movieInfo.isAvailable && movieInfo.videos.size() > 0)
	{
		option.Update(recvData.Integer(L"TargetBitrate"), movieInfo.videos[0].width, movieInfo.videos[0].height);
	}
}

string FFMpegCommand::Get(tstring outputFolder, int segmentListSize) const
{
	wstring audioMap = (0 <= audioIndex) ? String::Format(L"-map 0:%d", audioIndex) : L"";

	LPCTSTR format = LR"TEXT(%s\FFMpegnGinSoft.exe -y -ss %d.0 -i "%s" -map 0:%d %s -f segment -c:a aac -strict experimental -ar %d -b:a %dk -ac %d -s %dx%d -flags -global_header -c:v libx264 -pix_fmt yuv420p -profile:v high -level:v 4.1 -crf 18 -preset %s -tune zerolatency -qp 0 -r %f -x264opts "crf=%d:vbv-bufsize=%d:vbv-maxrate=%d:keyint=%d:bframes=0" -movflags +faststart -segment_list_size %d -segment_list "%s\index.m3u8" -deinterlace "%s\%%05d.ts")TEXT";

	tstring ffmpegCommand = String::Format(format
		, Path::ShortPathName(Path::ModuleFilePath()).c_str()
		, startTime
		, Path::ShortPathName(sourceFile).c_str()
		, videoIndex, audioMap.c_str()
		, option.audioSampleRate, option.audioBitrate, option.audioChannels
		, option.outputWidth, option.outputHeight, option.compress
		, option.frameRate, option.crf, option.videoBitrate, option.videoBitrate, option.keyframe
		, segmentListSize + 10 //이게 너무 작으면(아마도 10 이하) iOS에서 .ts 파일을 요청하지 않는다.
		, outputFolder.c_str(), outputFolder.c_str()
		);

	return String::WideCharToMultiByte(ffmpegCommand);
}

bool FFMpegCommand::IsAudioOnly() const
{
	return option.videoBitrate <= 128;
}
