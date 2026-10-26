#ifndef _SAMPLETYPES_H_
#define _SAMPLETYPES_H_

namespace Audio
{
	namespace Core
	{
		////////////////////////////////////////////////////////////////////////////////
		//	Sample types
		////////////////////////////////////////////////////////////////////////////////
		const static U8 uPCM32BitFloat = 0x00;
		const static U8 uPCM8BitSignedInt = 0x01;
		const static U8 uPCM8BitUnsignedInt = 0x02;
		const static U8 uPCM16BitSignedInt = 0x03;
		const static U8 uPCM16BitUnsignedInt = 0x04;
		const static U8 uPCM24BitSignedInt = 0x05;
		const static U8 uPCM24BitUnsignedInt = 0x06;
		const static U8 uADPCM4Bit = 0x7;
		const static U8 uXMA16Bit = 0x8;
		const static U8 uXMA216Bit = 0x9;
		const static U8 uMP3 = 0xa;

		const static U8 uInvalidFormat = 0xff;
	}
};

#endif // _SAMPLETYPES_H_
