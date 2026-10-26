#ifndef TGA_H
#define TGA_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <Common/System/I_Stream.h>

namespace FileFormat
{

//////////////////////////////////////////////////////////////////////////

/// C_TgaHeader
/// Class for reading and writing the a TGA file header

class C_TgaHeader //: public Sys::C_Streamable<C_TgaHeader>
{
	public:
		C_TgaHeader()
		{
		}
		
		C_TgaHeader(U32 width, U32 height)
		{
	        m_IdLength = 0;
	        m_ColourMapType = 0;
	        m_ImageType = 2;

	        m_ColourMapFirstEntryIndex = 0;
	        m_ColourMapLength = 0;
	        m_ColourMapEntrySize = 0;

	        m_XOrigin = 0;
	        m_YOrigin = 0;

	        m_ImageWidth = (U16)width;
	        m_ImageHeight = (U16)height;

	        m_PixelDepth = 32;
            m_ImageDescriptor = (0<<0) + (0);		
		}
		
        template< class T_Stream >
        void Serialise( T_Stream& stream )
        {
            stream << m_IdLength;
            stream << m_ColourMapType;
            stream << m_ImageType;
            stream << m_ColourMapFirstEntryIndex;
            stream << m_ColourMapLength;
            stream << m_ColourMapEntrySize;
            stream << m_XOrigin;
            stream << m_YOrigin;
            stream << m_ImageWidth;
            stream << m_ImageHeight;
            stream << m_PixelDepth;
            stream << m_ImageDescriptor;
        }

        template< class T_Stream >
        void Deserialise( T_Stream& stream )
        {
            stream >> m_IdLength;
            stream >> m_ColourMapType;
            stream >> m_ImageType;
            stream >> m_ColourMapFirstEntryIndex;
            stream >> m_ColourMapLength;
            stream >> m_ColourMapEntrySize;
            stream >> m_XOrigin;
            stream >> m_YOrigin;
            stream >> m_ImageWidth;
            stream >> m_ImageHeight;
            stream >> m_PixelDepth;
            stream >> m_ImageDescriptor;
        }

    public :
        
        // ID Length - Field 1 (1 byte):
        
        U8              m_IdLength;                 // This field identifies the number of bytes contained in Field 6, the Image ID Field.
                                                    // The maximum number of characters is 255. A value of zero indicates that no Image ID field is included with the image.
        
        // Color Map Type - Field 2 (1 byte):
        
        U8              m_ColourMapType;            // This field indicates the type of color map (if any) included with the image.
                                                    // There are currently 2 defined values for this field:
                                                    // 0 - indicates that no color-map data is included with this image.
                                                    // 1 - indicates that a color-map is included with this image.
        
        // Image Type - Field 3 (1 byte):
        
        U8              m_ImageType;                // The TGA File Format can be used to store Pseudo-Color, True-Color and Direct-Color images of various pixel depths.
                                                    // Truevision has currently defined seven image types:
        
                                                    // 0    No Image Data Included
                                                    // 1    Uncompressed, Color-mapped Image
                                                    // 2    Uncompressed, True-color Image
                                                    // 3    Uncompressed, Black-and-white Image
                                                    // 9    Run-length encoded, Color-mapped Image
                                                    // 10   Run-length encoded, True-color Image
                                                    // 11   Run-length encoded, Black-and-white Image
        
        // Color Map Specification - Field 4 (5 bytes):
        
        U16             m_ColourMapFirstEntryIndex; // Index of the first color map entry. Index refers to the starting entry in loading the color map.
        U16             m_ColourMapLength;          // Total number of color map entries included.
        U8              m_ColourMapEntrySize;       // Establishes the number of bits per entry. Typically 15, 16, 24 or 32-bit values are used.
        
        // Image Specification - Field 5 (10 bytes):
        
        U16             m_XOrigin;                  // These bytes specify the absolute horizontal coordinate for the lower left corner of the image as it is positioned
                                                    // on a display device having an origin at the lower left of the gDisplay.
        U16             m_YOrigin;                  // These bytes specify the absolute vertical coordinate for the lower left corner of the image as it is positioned
                                                    // on a display device having an origin at the lower left of the gDisplay.
        U16             m_ImageWidth;               // This field specifies the width of the image in pixels.
        U16             m_ImageHeight;              // This field specifies the height of the image in pixels.
        U8              m_PixelDepth;               // This field indicates the number of bits per pixel. This number includes the Attribute or Alpha channel bits.
                                                    // Common values are 8, 16, 24 and 32 but other pixel depths could be used.
        U8              m_ImageDescriptor;          // Bits 3-0: These bits specify the number of attribute bits per pixel.
                                                    // In the case of the TrueVista, these bits indicate the number of bits per pixel which are designated as Alpha Channel bits.
                                                    // For the ICB and TARGA products, these bits indicate the number of overlay bits available per pixel.
                                                    // See field 24 (Attributes Type) for more information.
                                                    // Bits 5 & 4: These bits are used to indicate the order in which pixel data is transferred from the file to the gDisplay.
                                                    // Bit 4 is for left-to-right ordering and bit 5 is for top-to-bottom ordering as shown below.
                                                    // Screen destination of first pixel    Image Origin
                                                    //                                      bit 5   bit 4
                                                    // bottom left                          0       0
                                                    // bottom right                         0       1
                                                    // top left                             1       0
                                                    // top right                            1       1
                                                    // Bits 7 & 6: Must be zero to insure future compatibility.
        
        // Image ID - Field 6 (variable):	

		//U8              *imageId;
        						                    // This optional field contains identifying information about the image.
                                                    // The maximum length for this field is 255 bytes.
                                                    // Refer to Field 1 for the length of this field.
                                                    // If field 1 is set to Zero indicating that no Image ID exists then these bytes are not written to the file.
        
        // Color Map Data - Field 7 (variable):
        
		//void*          colourMapData;
        							                // If the Color Map Type (field 2) field is set to zero indicating that no Color-Map exists then this field will not be present (i.e., no bytes written to the file).
                                                    // This variable-length field contains the actual color map information (LUT data).
                                                    // Field 4.3 specifies the width in bits of each color map entry while Field 4.2 specifies the number of color map entries in this field.
                                                    // These two fields together are used to determine the number of bytes contained in field 7.
                                                    // Each color map entry is stored using an integral number of bytes.
                                                    // The RGB specification for each color map entry is stored in successive bit-fields in the multi-byte enntries.
                                                    // Each color bit-field is assumed to be MIN(Field4.3/3, 8) bits in length.
                                                    // If Field 4.3 contains 24, then each color specification is 8 bits in length;
                                                    // if Field 4.3 contains 32, then each color specification is also 8 bits (32/3 gives 10, but 8 is smaller).
                                                    // Unused bit(s) in the multi-byte entries are assumed to specify attribute bits.
                                                    // The attribute bit field is often called the Alpha Channel, Overlay Bit(s) or Interrupt Bit(s).
                                                    // For the TARGA M-8, ATVista and NuVista, the number of bits in a color map specification is 24 (or 32).
                                                    // The red, green, and blue components are each represented by one byte.

		//void*          imageData;
	
};



}; // namespace FileFormat

#endif
