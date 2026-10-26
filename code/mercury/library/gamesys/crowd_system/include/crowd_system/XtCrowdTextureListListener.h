#ifndef XT_CROWD_TEXTURE_LIST_LISTENER_H
#define XT_CROWD_TEXTURE_LIST_LISTENER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/list.h>

struct XtRecord;

namespace AnimatedCrowd
{

class C_XtCrowdTextureList;

class C_XtCrowdTextureListListener: public Btl::ListElementDouble<C_XtCrowdTextureListListener>
{
public:
	C_XtCrowdTextureListListener();
	virtual ~C_XtCrowdTextureListListener();

	virtual void OnXtCrowdTextureListCreated(XtRecord* pRecord, const C_XtCrowdTextureList* pTextureList);
	virtual void OnXtCrowdTextureListDestroyed(XtRecord* pRecord, const C_XtCrowdTextureList* pTextureList);
};

}; // namespace AnimatedCrowd

#endif // XT_CROWD_TEXTURE_LIST_LISTENER_H
