#ifndef ATTKUT_GENERATE_NORMAL_MAP_H
#define ATTKUT_GENERATE_NORMAL_MAP_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_GenerateNormalMap
	{
	public:
		static Attk::C_Status GenerateNormalMap(Attk::C_Image& dest, const Attk::C_Image& src);
	};
}

#endif
