#pragma once

namespace CompressLib
{
	struct C_OutputTexture
	{
		C_OutputTexture()
		{
			dataSize = 0;
			pData = 0;
		}

		//Data size in bytes.
		U32	dataSize;
		//Pointer to our data.
		U8* pData;
	};
}
