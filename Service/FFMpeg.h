#pragma once
	
extern "C"
{
#include "libavformat/avformat.h"

#include "libavcodec/avcodec.h"

#include "libavfilter/avfilter.h"

#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/fifo.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/parseutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"
}

struct AVDictionary
{
	int count;
	AVDictionaryEntry *elems;
};

void ReadMetaData(AVDictionary *m, std::wstring* target);
void ExecuteFFMpeg(const tstring& command, BYTE* buffer, int bufferSize, int* writtenSize);
vector<BYTE> CommandResult(tstring command);
bool ExecuteFFMpeg(const tstring& command);