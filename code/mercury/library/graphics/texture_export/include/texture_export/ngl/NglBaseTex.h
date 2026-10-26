#ifndef NGLBASETEX_H
#define NGLBASETEX_H

#include "ngl/NglBaseDriver.h"

namespace Sys
{
class I_BinaryStream;
}

// virtual base for classes that can write textures to a binary stream
class NglBaseTex : public NglBaseDriver
{
public:

	enum OutputType
	{
		HEADER_AND_DATA,
		JUST_HEADER,
		JUST_DATA,
	};

	virtual ACTRESULT SerialOutTexture( Sys::I_BinaryStream* stream, const ActTools::Texture* aTexture, OutputType outputType = HEADER_AND_DATA ) = 0;
};

#endif // NGLBASETEX_H

