#include "stdafx.h"

#include <RaceBook/Objects/ObjectBase.h>

Debug::Logger &g_RaceBookObjectLogger( Debug::GetLogger("RaceBook.Objects") );

namespace RaceBook
{
	C_MessageIdxTracker C_MessageIdxTracker::s_instance;

	void C_MessageIdxTracker::AssignId( C_ObjectBase* obj )
	{
		objectList.PushBack(obj);
		obj->mObjectId = objectList.Size();

#ifndef CFG_GOLD
		for( int i = 0; i < objectList.Size() - 1; i++ )
		{
			if(objectList[i]->mObjectId == obj->mObjectId)
			{
				LOGWARN(g_RaceBookObjectLogger, "Attempted to assign id %u twice.", obj->mObjectId);
				LOGWARN(g_RaceBookObjectLogger, "Object list is size %i", objectList.Size());
			}
		}
#endif
	}

	void C_MessageIdxTracker::RemoveId( int i )
	{
		int idx = i - 1;
		if( idx < 0 )
			return;

		Btl::Vector<C_ObjectBase*>::Iterator it = objectList.Begin() + idx;
		objectList.Erase(it);
		for( int j = i; j < objectList.Size(); j++ )
		{
			objectList[j]->mObjectId--;
		}
	}

	void C_MessageIdxTracker::ReorderObjects()
	{
		for( int i = 0; i < objectList.Size(); i++ )
		{
			for( int j = i+1; j < objectList.Size(); j++ )
			{
				if( objectList[i]->mObjectId < objectList[j]->mObjectId )
				{
					C_ObjectBase* obj = objectList[i];
					objectList[i] = objectList[j];
					objectList[j] = obj;
				}
			}
		}
	}
} // namespace RaceBook

#ifdef RLC_DEBUG
#pragma optimize ("", on) //RLC_DEBUG
#endif
