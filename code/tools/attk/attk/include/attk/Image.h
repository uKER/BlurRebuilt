#ifndef ATTK_IMAGE2D_H
#define ATTK_IMAGE2D_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	class C_Resize;
	class C_DevILUtility;
	///\brief Image class.
	///
	class C_Image : public C_BaseImage
	{
	public:
		///\brief Default constructor.
		///
		C_Image();

		///\brief Copy constructor.
		///
		///@param rhs The image to copy.
		C_Image(const C_Image& rhs);

		///\brief Destructor.
		///
		~C_Image();

		///\brief Asignment operator.
		///
		///@param rhs The image to copy.
		///@return Reference to this object.
		const C_Image& operator=(const C_Image& rhs);

		///\brief Returns a pointer to the image's palette data.
		///
		///If no palette data is present this function returns null.
		///
		///@return Palette data pointer.
		U8* GetPaletteDataPtr();

		///\brief Returns a pointer to the image's palette data.
		///
		///If no palette data is present this function returns null.
		///
		///@return Palette data pointer.
		const U8* GetPaletteDataPtr() const;

		///\brief Uploads the given palette data.
		///
		///Note that the Attk performs a memcpy of any palette data passed in, it does not
		///take ownership of the data pointed to.
		///
		///The image's palette format will be set to match the palette format of the incoming data.
		///
		///@param pPalette The palette data.
		///@param format The format of the incoming palette data.
		///@return Status value indicating the success or failure of the operation.
		C_Status SetPaletteData(const U8* pPalette, E_PaletteFormat format);

		///\brief A quick method for initialising a simple image.
		///
		///This function begins by first clearing any existing pixel data.
		///
		///The width, height and depth values specified are used to intitialise an image
		///with those dimensions. These dimensions must be equal to that of any image
		///data being passed in.
		///
		///The pixel data for the mip-level is initialised using the format specified 
		///if valid. This the format that any data specified by pData should be in.
		///
		///Note that the Attk performs a memcpy of any pixel data passed in, it does not
		///take ownership of the data pointed to.
		///
		///@param width The image width.
		///@param height The image height.
		///@param depth The image depth.
		///@param format The incoming data format.
		///@param pData Pixel data.
		///@return Status value indicating the success or failure of the operation.
		C_Status NewPixelData(
			U32 width, 
			U32 height,
			U32 depth,
			E_PixelFormat format = E_PixelFormat_Unknown,
			const U8* pData = 0);

		C_Status MovePixelData(
			U32 width, 
			U32 height,
			U32 depth,
			E_PixelFormat format,
			U8* pData);

		///\brief Deletes pixel data.
		///
		void DeletePixelData();

		///\brief Deletes any palette data and resets the image palette format to PALETTE_FORMAT_NONE.
		///
		void DeletePaletteData();

		///\brief Returns a pixel.
		///
		///@param x x-coordinate of pixel.
		///@param y y-coordinate of pixel.
		///@param z z-coordinate of pixel.
		///@return A pointer to the pixel.
		C_PixelR32G32B32A32F GetPixel(	
			U32 x, 
			U32 y,
			U32 z = 0) const;

		C_PixelR32G32B32A32F GetPalettisedPixel(	
			U32 x, 
			U32 y,
			U32 z = 0) const;

		///\brief Returns a pointer to a pixel.
		///
		///@param x x-coordinate of pixel.
		///@param y y-coordinate of pixel.
		///@param z z-coordinate of pixel.
		///@return A pointer to the pixel.
		U8* GetPixelDataPtr(
			U32 x, 
			U32 y,
			U32 z = 0);

		///\brief Returns a pointer to a pixel.
		///
		///@param x x-coordinate of pixel.
		///@param y y-coordinate of pixel.
		///@param z z-coordinate of pixel.
		///@return A pointer to the pixel.
		const U8* GetPixelDataPtr(
			U32 x, 
			U32 y,
			U32 z = 0) const;

		///\brief Returns a pointer to a pixel.
		///
		///@return A pointer to the pixel data.
		U8* GetPixelDataPtr();

		///\brief Returns a pointer to a pixel.
		///
		///@return A pointer to the pixel data.
		const U8* GetPixelDataPtr() const;

		///\brief Returns the number of bytes required to store our pixel data.
		///
		///@return The storage required in bytes.
		U32 GetPixelDataSize() const;

		///\brief Set pixel.
		///
		///@param x x-coordinate of pixel.
		///@param y y-coordinate of pixel.
		///@param z z-coordinate of pixel.
		void SetPixel(
			const C_PixelR32G32B32A32F& pixel,
			U32 x, 
			U32 y,
			U32 z = 0);

		///\brief Set pixel data.
		///
		///@param pSource Pointer to the data to copy.
		///@param format The format of the incoming pixel data.
		///@return Status value indicating the success or failure of the operation.
		C_Status SetPixelData(
			const U8* pSource, 
			E_PixelFormat format);

		C_Status Fill(U8* pPixel);

		C_Status Fill(Attk::E_Channel channel, F32 value);

		C_Status Fill(const C_PixelR32G32B32A32F& pixel);

		C_Status Fill(U32 channels, const C_PixelR32G32B32A32F& pixel);

		///\brief Convert the pixel format to that specified.
		///
		///@param pixelFormat The new pixel format.
		///@return Status value indicating the success or failure of the operation.
		C_Status ChangePixelFormat(E_PixelFormat pixelFormat);

		///\brief Convert the current palette format to that specified.
		///
		///@param paletteFormat The new palette format.
		///@return Status value indicating the success or failure of the operation.
		C_Status ChangePaletteFormat(E_PaletteFormat paletteFormat);

		void DisownPixelData();

		///\brief Reads the specified file from disk.
		///
		///@param pFileName The file to read.
		///@param imageFormat The image format.
		///@param pOptions Options describing how the read operation is to be performed.
		///@return Status value indicating the success or failure of the operation.
		C_Status Read(
			const char* pFileName, 
			E_ImageFormat imageFormat = E_ImageFormat_BestGuess,
			const C_Options* pOptions = 0);

		///\brief Write the specified file from disk.
		///
		///@param pFileName The file to write.
		///@param imageFormat The image format.
		///@param pOptions Options describing how the write operation is to be performed.
		///@return Status value indicating the success or failure of the operation.
		C_Status Write(
			const char* pFileName, 
			E_ImageFormat imageFormat = E_ImageFormat_BestGuess,
			const C_Options* pOptions = 0) const;

		///\brief Clears the contents of this image.
		///
		///All data contained is deleted such that the class is reset to its initial state.
		void Clear();

		///Insert an image at the specified location
		void Insert(U32 insertX, U32 insertY, const Attk::C_Image& rhs);

	private:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		///\brief Perform one-off initialisation. Called by constructor and copy constructor.
		///
		void _OneTimeInit();

		///\brief Initialise new pixel data
		///
		///@param width The image width.
		///@param height The image height.
		///@param depth The image depth.
		///@param format The incoming data format.
		///@param pData Pixel Data.
		///@return Status value indicating the success or failure of the operation.
		C_Status _NewPixelData(
			U32 width, 
			U32 height,
			U32 depth,
			E_PixelFormat format,
			const U8* pData);

		C_Status _MovePixelData(
			U32 width, 
			U32 height,
			U32 depth,
			E_PixelFormat format,
			U8* pData);

		C_Status _ResizeDevIL(
			U32 width, 
			U32 height,
			U32 depth);

		///Palette data.
		U8* m_pPaletteData;

		U8* m_pPixelData;
#endif
		FRIEND_CLASS C_DevILUtility;
	};


}

#endif
