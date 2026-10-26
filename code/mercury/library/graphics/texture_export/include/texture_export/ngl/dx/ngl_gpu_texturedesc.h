#ifndef __NGL_GPU_TEXTUREDESC_H__
#define __NGL_GPU_TEXTUREDESC_H__

struct gpuTextureDesc
{
	int  Width;
	int  Height;
	int  Depth;
	int  Levels;	// number of mipmap levels
	uint Format;
	uint Usage;
	uint Type;		// 2d, cubemap, volume
};

template < class T_Stream >
void Deserialise( T_Stream& stream, gpuTextureDesc &td )
{
	stream >> td.Width >> td.Height >> td.Depth >> td.Levels >> td.Format >> td.Usage >> td.Type;
}

template < class T_Stream >
void Serialise( T_Stream& stream, const gpuTextureDesc &td )
{
	stream << td.Width << td.Height << td.Depth << td.Levels << td.Format << td.Usage << td.Type;
}

#endif
