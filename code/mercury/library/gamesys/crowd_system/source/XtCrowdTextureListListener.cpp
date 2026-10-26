#include "CrowdSystemPch.h"

#include <crowd_system/XtCrowdTextureListListener.h>
#include <crowd_system/XtCrowd.h>

namespace AnimatedCrowd
{

C_XtCrowdTextureListListener::C_XtCrowdTextureListListener()
{
	C_XtCrowdTextureList::AddListener(this);
}

C_XtCrowdTextureListListener::~C_XtCrowdTextureListListener()
{
	C_XtCrowdTextureList::RemoveListener(this);
}

void C_XtCrowdTextureListListener::OnXtCrowdTextureListCreated(XtRecord* pRecord, const C_XtCrowdTextureList* pTextureList)
{
}

void C_XtCrowdTextureListListener::OnXtCrowdTextureListDestroyed(XtRecord* pRecord, const C_XtCrowdTextureList* pTextureList)
{
}

}; // namespace AnimatedCrowd
