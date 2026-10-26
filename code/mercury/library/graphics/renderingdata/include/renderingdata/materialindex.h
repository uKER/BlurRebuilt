#ifndef RD_MATERIAL_INDEX_H
#define RD_MATERIAL_INDEX_H
#ifdef _MSC_VER
#pragma once
#endif

namespace RenderingData
{
	struct MaterialIndex
	{
		union
		{
			S32 data;

			struct
			{
				S32 index : 28;
				S32 listIndex : 4; // 0 = constant, > 0 = dynamic
			};
		};
	};
}

#endif

