#ifndef __DISPLAY_TEXTUREDESC_H__
#define __DISPLAY_TEXTUREDESC_H__

namespace Display
{

// on-disk texture description header

struct TextureDesc
{
	S32 Width;
	S32 Height;
	S32 Depth;
	S32 Levels;	// number of mipmap levels
	U32 Format;
	U32 Usage;
	U32 Type;	// 2d, cubemap, volume
};

template < class T_Stream >
void Deserialise( T_Stream& stream, TextureDesc &td )
{
	stream >> td.Width >> td.Height >> td.Depth >> td.Levels >> td.Format >> td.Usage >> td.Type;
}

template < class T_Stream >
void Serialise( T_Stream& stream, const TextureDesc &td )
{
	stream << td.Width << td.Height << td.Depth << td.Levels << td.Format << td.Usage << td.Type;
}

};

#endif
