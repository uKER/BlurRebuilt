#ifndef GAMETEXTURE_198024BF_549D_4595_8406_49AD148F2ABC_H
#define GAMETEXTURE_198024BF_549D_4595_8406_49AD148F2ABC_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/TypeRegistry.h>
#include <resource_utils/ResourceUtilsDeclare.h>
#include <xt/XtGame.h>


namespace ResourceUtils
{

struct C_GameTexture
{
public:
	XtSz8	m_Filename;

	C_GameTexture();
	~C_GameTexture();

	Util::C_String GetFileName() const;
	static Resource::C_ResourceTypeId GetResourceTypeId();

private:
	C_GameTexture(const C_GameTexture& copy); // copy not allowed
	C_GameTexture& operator = (const C_GameTexture& copy); // copy not allowed

};

} // namespace ResourceUtils


#endif // GAMETEXTURE_198024BF_549D_4595_8406_49AD148F2ABC_H
