#ifndef ATTK_BASE_IMAGE_H
#define ATTK_BASE_IMAGE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	///\brief Image header class.
	///
	class C_BaseImage
	{
	public:
		///\brief Default constructor.
		///
		C_BaseImage();

		///\brief Copy constructor.
		///
		///@param rhs Image base to copy.
		C_BaseImage(const C_BaseImage& rhs);

		///\brief Destructor.
		///
		virtual ~C_BaseImage();

		///\brief Asignment operator.
		///
		///@param rhs Image base to copy.
		///@return Reference to this object.
		const C_BaseImage& operator=(const C_BaseImage& rhs);

		///\brief Returns the number of channels in the current pixel format.
		///
		///For example, if an RGBA format was used then this function would return 4.
		///
		///@return Number of channels.
		U32 GetChannelsPerPixel() const;

		///\brief Returns the number of bits per pixel required by the current pixel format.
		///
		///@return Number of bits per pixel.
		U32 GetBitsPerPixel() const;

		///\brief Returns the number of bytes per pixel required by the current pixel format.
		///
		///@return Number of bytes per pixel.
		U32 GetBytesPerPixel() const;

		///\brief Returns the number of channels in the current palette format.
		///
		///For example, if an RGBA format was used then this function would return 4.
		///
		///@return Number of channels.
		U32 GetChannelsPerPaletteEntry() const;

		///\brief Returns the number of bits per palette entry in the current palette format.
		///
		///@return Number of bits.
		U32 GetBitsPerPaletteEntry() const;

		///\brief Returns the number of bytes per palette entry in the current palette format.
		///
		///@return Number of bytes.
		U32 GetBytesPerPaletteEntry() const;

		///\brief Returns the number of channels required by the specified pixel format.
		///
		///@param format The format to query.
		///@return Number of channels.
		static U32 GetChannelsPerPixel(E_PixelFormat format);

		///\brief Returns the number of channels required by the specified pixel format.
		///
		///@param format The format to query.
		///@return Number of channels.
		static U32 GetChannelsPaletteEntry(E_PaletteFormat format);

		///\brief Returns number of bits per pixel required by the specified pixel format.
		///
		///@param format The format to query.
		///@return Number of bits.
		static U32 GetBitsPerPixel(E_PixelFormat format);

		///\brief Returns the number of bits per palette entry in the specified palette format.
		///
		///@param format The format to query.
		///@return Number of bits.
		static U32 GetBitsPerPaletteEntry(E_PaletteFormat format);

		///\brief Returns the number of bytes per pixel in the specified pixel format.
		///
		///@param format The format to query.
		///@return Number of bytes.
		static U32 GetBytesPerPixel(E_PixelFormat format);

		///\brief Returns the number of bytes per palette entry required by the specified palette format.
		///
		///@param format The format to query.
		///@return Number of bytes.
		static U32 GetBytesPerPaletteEntry(E_PaletteFormat format);

		///\brief Returns the number of palette entries for the given pixel format.
		///
		///@param format Pixel format.
		///@return The number of palette entries.
		static U32 GetNumPaletteEntries(E_PixelFormat format);

		///\brief Returns the size of the palette data array.
		///
		///@return palette data size in bytes.
		static U32 GetPaletteDataSize(E_PixelFormat pixelFormat, E_PaletteFormat paletteFormat);

		///\brief Returns the number of palette entries for the current pixel format.
		///
		///@return The number of palette entries.
		U32 GetNumPaletteEntries() const;

		///\brief Returns the number of bytes required to store palette data in the current palette format.
		///
		///@return The storage required in bytes.
		U32 GetPaletteDataSize() const;

		///\brief Returns the current pixel format.
		///
		///@return Pixel format.
		E_PixelFormat GetPixelFormat() const;

		///\brief Returns the current palette format.
		///
		///@return Palette format.
		E_PaletteFormat GetPaletteFormat() const;

		U32 GetWidth() const;

		U32 GetHeight() const;

		U32 GetDepth() const;

		///\brief Reads the image header from the specified file from disk.
		///
		///@param pFileName The file to read.
		///@param imageFormat The image format.
		///@param pOptions Options describing how the read operation is to be performed.
		///@return Status value indicating the success or failure of the operation.
		virtual C_Status Read(
			const char* pFileName,
			E_ImageFormat imageFormat = E_ImageFormat_BestGuess,
			const C_Options* pOptions = 0);

		///\brief Read image header from memory in the specified format.
		///
		///@param pSource The location to read from.
		///@param size The size of the data to read.
		///@param imageFormat The image format.
		///@param pOptions Options describing how the read operation is to be performed.
		///@return Status value indicating the success or failure of the operation.
		virtual C_Status Read(
			const void* pSource,
			U32 size,
			E_ImageFormat imageFormat = E_ImageFormat_BestGuess,
			const C_Options* pOptions = 0);

		///\brief Clears the contents of this image header.
		///
		///All data contained is deleted such that the class is reset to its initial state.
		virtual void Clear();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void _OneTimeInit();

		E_PixelFormat	m_pixelFormat;
		E_PaletteFormat	m_paletteFormat;

		U32				m_width;
		U32				m_height;
		U32				m_depth;

		friend class C_Resize;
		friend class C_Image;
#endif
	};
}

#endif
