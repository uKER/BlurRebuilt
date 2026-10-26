#ifndef TEXTUREATLAS_SKYLINE_PACK_H
#define TEXTUREATLAS_SKYLINE_PACK_H

#ifdef _MSC_VER
#pragma once
#endif

namespace TextureAtlas
{

	//sort images by height.
	//loop thru and add images causing minimal increase in the 'horizon'

	class C_HorizonPack
	{
	public:
		C_HorizonPack();

		bool Pack(const InputImageDescArray& input, OutputImageDescArray& output, const ImageSizeArray& atlasSizes, U32 border);

	private:
		struct Skyline
		{
			Skyline(U32 width);

			struct Span
			{
				Span();
				Span(U32 e, U32 h);

				bool operator <(const Span& span) const;
				bool operator >(const Span& span) const;

				bool operator ==(const Span& span) const;
				bool operator !=(const Span& span) const;

				U32 end, value;
			};

			ImagePosition Position(const C_InputImageDescription* desc, U32 border) const;
			void UpdateSpans(U32 xoffset, U32 width, U32 height);

			Btl::Vector<Span>	spans;
			mutable U32			horizon;
		};	
	};
}

#endif