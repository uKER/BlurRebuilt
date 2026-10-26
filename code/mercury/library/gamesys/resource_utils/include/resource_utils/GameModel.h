#ifndef GAMEMODEL_D009C220_1E7C_4F9E_A8E4_9FBD7F8473CF_H
#define GAMEMODEL_D009C220_1E7C_4F9E_A8E4_9FBD7F8473CF_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/TypeRegistry.h>
#include <resource_utils/ResourceUtilsDeclare.h>
#include <xt/XtGame.h>


namespace ResourceUtils
{

struct C_GameModel
{
public:
	XtSz8							m_Filename;
	XtFlags<U32, E_GameFileFlags>	m_Flags;

	C_GameModel();
	~C_GameModel();

	Util::C_String GetFileName() const;
	static Resource::C_ResourceTypeId GetResourceTypeId();

private:
	C_GameModel(const C_GameModel& copy); // copy not allowed
	C_GameModel& operator = (const C_GameModel& copy); // copy not allowed

};

} // namespace ResourceUtils


#endif // GAMEMODEL_D009C220_1E7C_4F9E_A8E4_9FBD7F8473CF_H
