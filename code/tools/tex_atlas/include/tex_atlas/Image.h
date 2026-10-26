#ifndef CS_GEN_IMAGE_H
#define CS_GEN_IMAGE_H

class BTCImageData;

namespace Tu
{
	class GenericDiskReader;
	class GenericDiskWriter;
}

namespace TexAtlas
{
	class Image
	{
	public:
		enum ImageFormat
		{
			UNKNOWN = 0,
			RGB = 1,
			RGBA = 2,
		};

		enum ImageFileType
		{
			BMP,
			TGA,
			JPG,

			BEST_GUESS
		};

		enum Component
		{
			R = 0,
			G = 1,
			B = 2,
			A = 3
		};

		Image();

		Image(const Image& rhs);

		~Image();

		const Image& operator=(const Image& rhs);

		Tu::Status New(U32 width, U32 height, ImageFormat format);

		Tu::Status Read(const std::string& fileName);

		Tu::Status Write(const std::string& fileName, ImageFileType fileType = BEST_GUESS) const;

		void Pixel(U32 x, U32 y, U8& r, U8& g, U8& b) const;

		void Pixel(U32 x, U32 y, U8& r, U8& g, U8& b, U8& a) const;

		const U8* Pixel(U32 x, U32 y) const;

		U8* Pixel(U32 x, U32 y);

		void FloodFill(U8 r, U8 g, U8 b);

		void FloodFill(U8 r, U8 g, U8 b, U8 a);

		void GrowBorder(U32 size, bool wrapTexture = true);

		void Resize(U32 newWidth, U32 newHeight);

		///Insert an image at the specified location.
		void Insert(U32 x, U32 y, const Image& rhs);

		void Rotate90CW();

		void Rotate90CCW();

		ImageFormat Format() const;

		U32 BitsPerPixel() const;

		U32 BytesPerPixel() const;

		U32 BytesPerPixel(ImageFormat format) const;

		U32 SizeBytes() const;

		U32 Width() const;

		U32 Height() const;

		void SetPixel(U32 insertX,  U32 insertY,  const U8* pPixel, ImageFormat format);

		void SetPixel(U32 x, U32 y, U8 r, U8 g, U8 b, U8 a);

		void SetPixel(U32 x, U32 y, U8 r, U8 g, U8 b);

		const U8* Data() const;

		void ConvertToNormalMap();

		void Clear();

	private:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		void OneTimeInit();
#endif
		inline const U8* PixelComponent(U32 x, U32 y, Image::Component component) const;

		inline U8* PixelComponent(U32 x, U32 y, Image::Component component);

		void CopyToBTCImageData(BTCImageData* pImageData) const;

		void CopyFromBTCImageData(BTCImageData* pImageData);

		void Serialise(Tu::GenericDiskWriter* pWriter) const;

		void Deserialise(Tu::GenericDiskReader* pReader);

		U32				m_width;
		U32				m_height;
		U8*				m_pPixelData;
		ImageFormat		m_format;
		U32				m_pixelSizeBytes;

		friend class ImageCache;
	};
}

#endif
