#include "stdafx.h"
#include "SubtitleMetadata.h"
#include "FFMpeg.h"

namespace MediaInformation
{
	SubtitleMetadata::SubtitleMetadata(int index, AVStream *st)
	{
		this->index = index;
		codecName = String::MultiByteToWideChar(st->codec->codec_descriptor->long_name);

		uint8_t* buffer = nullptr;
		int bufferSize = 0;

		if(st->codec->codec_descriptor->id == AV_CODEC_ID_DVD_SUBTITLE)
		{
			buffer = st->codec->extradata;//dvdsub
			bufferSize = st->codec->extradata_size;
		}
		else
		{
			buffer = st->codec->subtitle_header; //ass,ssa
			bufferSize = st->codec->subtitle_header_size;
		}

		header.resize(bufferSize+1);

		memset(&header[0], 0, header.size());
		memcpy(&header[0], buffer, bufferSize);

		type = st->codec->codec_descriptor->id;
	}
}

/*
AV_CODEC_ID_DVD_SUBTITLE
AV_CODEC_ID_DVB_SUBTITLE,
AV_CODEC_ID_TEXT,  ///< raw UTF-8 text
AV_CODEC_ID_XSUB,
AV_CODEC_ID_SSA,
AV_CODEC_ID_MOV_TEXT,
AV_CODEC_ID_HDMV_PGS_SUBTITLE,
AV_CODEC_ID_DVB_TELETEXT,
AV_CODEC_ID_SRT,
AV_CODEC_ID_MICRODVD   = MKBETAG('m','D','V','D'),
AV_CODEC_ID_EIA_608    = MKBETAG('c','6','0','8'),
AV_CODEC_ID_JACOSUB    = MKBETAG('J','S','U','B'),
AV_CODEC_ID_SAMI       = MKBETAG('S','A','M','I'),
AV_CODEC_ID_REALTEXT   = MKBETAG('R','T','X','T'),
AV_CODEC_ID_SUBVIEWER1 = MKBETAG('S','b','V','1'),
AV_CODEC_ID_SUBVIEWER  = MKBETAG('S','u','b','V'),
AV_CODEC_ID_SUBRIP     = MKBETAG('S','R','i','p'),
AV_CODEC_ID_WEBVTT     = MKBETAG('W','V','T','T'),
AV_CODEC_ID_MPL2       = MKBETAG('M','P','L','2'),
AV_CODEC_ID_VPLAYER    = MKBETAG('V','P','l','r'),
AV_CODEC_ID_PJS        = MKBETAG('P','h','J','S'),
AV_CODEC_ID_ASS        = MKBETAG('A','S','S',' '),  ///< ASS as defined in Matroska
*/
