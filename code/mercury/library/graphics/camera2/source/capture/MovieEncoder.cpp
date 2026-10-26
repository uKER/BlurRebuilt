
#include "stdafx.h"
#include <camera2/capture/MovieEncoder.h>
#include <camera2/capture/LockScreen.h>
#include <System/Serialise/I_FileSystem.h>

#ifdef COMPILE_OGG
extern "C"
{
#include <theora/theora.h>
#include <ogg/ogg.h>
}
#include "xgraphics.h"
#endif

namespace Cam2
{

// Constructor for 360, PS3 and PC
C_MovieEncoder::C_MovieEncoder()
{
	m_Encoder = 0;
	m_File = 0;
	m_VideoData = 0;
	m_TotalBytes = 0;
}

#ifdef COMPILE_OGG

static Debug::String MovieInfo("Camera2.MovieEncoder.Info", "");

class C_OggEncoder
{
public:
// Things we can't forward declare are here:
	ogg_stream_state oggs;
	yuv_buffer yuv;
	theora_state state;

	C_OggEncoder()
	{
		memset(&oggs, 0, sizeof(oggs));
		memset(&yuv, 0, sizeof(yuv));
		memset(&state, 0, sizeof(state));
	}
};

class C_OggPacket
{
public:
	ogg_packet packet;

	C_OggPacket()
	{
		memset(&packet, 0, sizeof(packet));
	}
};

static inline int CalcAddr(int x, int y, int width)
{
	return XGAddress2DTiledOffset(x, y, width, 4) << 2;
}

bool C_MovieEncoder::Open(const C_MovieSettings& settings)
{
	m_Encoder = new C_OggEncoder;

	if (!m_Encoder)
		return false;

	m_Settings = settings;
	m_Settings.width  = (m_Settings.width  + 15) & ~15;
	m_Settings.height = (m_Settings.height + 15) & ~15;

	m_File = Sys::GetDefaultFileSystem()->Open(m_Settings.filename, Sys::E_FileAccess_Write);
	if (!m_File)
		return false;

	m_TotalBytes = 0;

	ogg_stream_init(&m_Encoder->oggs, rand());

	// Init video compression:
	theora_info info;
	memset(&info, 0, sizeof(info));

	theora_info_init(&info);
	
	info.width  = info.frame_width  = m_Settings.width;
	info.height = info.frame_height = m_Settings.height;
	info.fps_numerator = m_Settings.fps;
	info.fps_denominator = 1;
	info.colorspace = OC_CS_UNSPECIFIED;
	info.pixelformat = OC_PF_420;
	info.target_bitrate = m_Settings.bitrate;
	info.quality = settings.quality;
	info.quick_p = 1;
	info.keyframe_auto_p = 1;
	info.keyframe_frequency = 64;
	info.keyframe_frequency_force = 64;
	info.keyframe_data_target_bitrate = info.target_bitrate * 3 / 2;
	info.keyframe_auto_threshold = 80;
	info.keyframe_mindistance = 8;
	info.noise_sensitivity = 1;
	
	theora_encode_init(&m_Encoder->state, &info);
	theora_info_clear(&info);
	
	// Encode first packets:
	C_OggPacket op;
	memset(&op.packet, 0, sizeof(op.packet));
	theora_encode_header(&m_Encoder->state, &op.packet);
	WritePacket(op);
	
	theora_comment encoderComment;
	theora_comment_init(&encoderComment);
	theora_encode_comment(&encoderComment, &op.packet);
	WritePacket(op);
	
	theora_comment_clear(&encoderComment);
	theora_encode_tables(&m_Encoder->state, &op.packet);
	WritePacket(op);
	FlushOgg();

	// Allocate space for video data:
	int videoLen = m_Settings.width * m_Settings.height;
	m_VideoData = new U8[videoLen + (videoLen >> 1)]; // Y and quarter size U and V = 1.5
	if (!m_VideoData)
	{
		SAFE_DELETE_PTR(m_File)
		return false;
	}
	
	memset(m_VideoData, 0, videoLen + (videoLen >> 1));

	// Point YUV buffer to video data:
	m_Encoder->yuv.y_width = m_Encoder->yuv.y_stride = m_Settings.width;
	m_Encoder->yuv.y_height = m_Settings.height;
	m_Encoder->yuv.y = m_VideoData; // Point to Y frame
	m_Encoder->yuv.uv_width = m_Encoder->yuv.uv_stride = m_Settings.width >> 1;
	m_Encoder->yuv.uv_height = m_Settings.height >> 1;
	m_Encoder->yuv.u = m_Encoder->yuv.y + videoLen;        // U frame is after Y frame
	m_Encoder->yuv.v = m_Encoder->yuv.u + (videoLen >> 2); // V frame is after U frame

	return true;
}

void C_MovieEncoder::WritePacket(C_OggPacket& op)
{
	if (!m_File)
		return;

	// Put stream into OGG pages:
	ogg_stream_packetin(&m_Encoder->oggs, &op.packet);
	ogg_page page;
	memset(&page, 0, sizeof(page));
	int ret = ogg_stream_pageout(&m_Encoder->oggs, &page);
	if (ret == 0)
		return;

	m_File->Write(page.header, page.header_len);
	m_File->Write(page.body, page.body_len);
	
	AddBytes(page.header_len + page.body_len);
}

void C_MovieEncoder::FlushOgg()
{
	if (!m_Encoder || !m_File)
		return;

	ogg_page page;
	memset(&page, 0, sizeof(page));

	for(;;)
	{
		int ret = ogg_stream_flush(&m_Encoder->oggs, &page);
		if (ret <= 0) return;

		m_File->Write(page.header, page.header_len);
		m_File->Write(page.body, page.body_len);

		AddBytes(page.header_len + page.body_len);
	}
}

void C_MovieEncoder::AddBytes(S32 amount)
{
	m_TotalBytes += amount;

	char text[128] = {0};
	sprintf(text, "%dk written", m_TotalBytes >> 10);
	MovieInfo.SetValue(text);
}

void C_MovieEncoder::Close()
{
	FlushOgg();
	
	if (m_Encoder)
	{
		theora_clear(&m_Encoder->state);
		ogg_stream_clear(&m_Encoder->oggs);
	}
	
	// Clean up:
	SAFE_DELETE_ARY(m_VideoData)
	SAFE_DELETE_PTR(m_File)
	SAFE_DELETE_PTR(m_Encoder)
}

static inline void GetRgb(U8 *ps, unsigned int format, int &red, int &green, int &blue)
{
	switch (format)
	{
		default:
			red   = ps[1];
			green = ps[2];
			blue  = ps[3];
		break;
		case D3DFMT_LE_A8R8G8B8:
		case D3DFMT_LE_X8R8G8B8:
			red   = ps[2];
			green = ps[1];
			blue  = ps[0];
		break;

		case D3DFMT_LE_X2R10G10B10:
		{
			unsigned int pix = ps[0] | (ps[1]<<8) | (ps[2]<<16) | (ps[3]<<24);
			red   = (U8)(pix>>22); // Red
			green = (U8)(pix>>12); // Green
			blue  = (U8)(pix>> 2); // Blue
		}	
		break;
	}
}

void C_MovieEncoder::AddFrame(const C_LockScreenInfo& si)
{
	if (!m_Encoder || !m_File)
		return;

	GetYuvFrame(si);

	C_OggPacket op;
	memset(&op.packet, 0, sizeof(op.packet));

	// Encode a YUV frame:
	theora_encode_YUVin(&m_Encoder->state, &m_Encoder->yuv);
	
	theora_encode_packetout(&m_Encoder->state, 0, &op.packet);
	WritePacket(op);
	FlushOgg();
}

void C_MovieEncoder::GetYuvFrame(const C_LockScreenInfo& si)
{
	if (!m_Encoder)
		return;

	int x = 0, y = 0;
	
	for (y = 0; y < m_Encoder->yuv.y_height; y++)
	{
		int sy = y * si.height / m_Encoder->yuv.y_height;

		for (x = 0; x < m_Encoder->yuv.y_width; x++)
		{
			// Get the tiled offset to this pixel:
			int sx = x * si.width / m_Encoder->yuv.y_width;
			U8 *ps = (U8 *)si.bits + CalcAddr(sx, sy, si.pitch >> 2);

			int red = 0, green = 0, blue = 0;
			GetRgb(ps, si.format, red, green, blue);
			
			// Calculate luminance (Y) of yuv:
			int lum = ((66 * red + 129 * green + 25 * blue + 128) >> 8) + 16;
			m_Encoder->yuv.y[x + y * m_Encoder->yuv.y_stride] = (U8)lum;
		}
	}

	// Cb and Cr
	for (y = 0; y < m_Encoder->yuv.uv_height; y++)
	{
		int sy = y * si.height / m_Encoder->yuv.uv_height;
		for (x = 0; x < m_Encoder->yuv.uv_width; x++)
		{
			int sx = x * si.width / m_Encoder->yuv.uv_width;
			U8 *ps = (U8 *)si.bits + CalcAddr(sx, sy, si.pitch >> 2);

			int red = 0, green = 0, blue = 0;
			GetRgb(ps, si.format, red, green, blue);

			// Calculate u and v of YUV:
			int u = (( -38 * red - 74 * green + 112 * blue + 128) >> 8) + 128;
			int v = (( 112 * red - 94 * green -  18 * blue + 128) >> 8) + 128;

			m_Encoder->yuv.u[x + y * m_Encoder->yuv.uv_stride] = (U8)u;
			m_Encoder->yuv.v[x + y * m_Encoder->yuv.uv_stride] = (U8)v;
		}
	}
}

#else

// If not compiling OGG/Theora:
bool C_MovieEncoder::Open(const C_MovieSettings&) { return false; }
void C_MovieEncoder::Close() { }
void C_MovieEncoder::AddFrame(const C_LockScreenInfo&) { }
void C_MovieEncoder::WritePacket(C_OggPacket&) { }
void C_MovieEncoder::FlushOgg() { }
void C_MovieEncoder::AddBytes(S32) { }
void C_MovieEncoder::GetYuvFrame(const C_LockScreenInfo&) { }
#endif


C_MovieSettings::C_MovieSettings()
{
#ifdef COMPILE_OGG
	filename = "movie.ogg";
	width = 768;
	height = 432;
	bitrate = 2000000;
	quality = 16;
	fps = 30;
#endif
}

} // End of namespace
