#ifndef EDITOR_LIB_XT_DEFINITIONS_H
#define EDITOR_LIB_XT_DEFINITIONS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	typedef Ut::UtList<Xt::DomRecord*> XtRecordList;

	enum E_XtSchemaType
	{
		E_XtSchemaType_Record,
		E_XtSchemaType_Other
	};

#define DEF(e, str) e,
	enum E_XtType
	{
		#include "Types/XtTypeMapping.h"
		E_XtType_Count
	};
#undef DEF

	struct C_XtDomUpdate
	{
		enum E_UpdateType
		{
			E_UpdateType_Element,
			E_UpdateType_Event,

			E_UpdateType_Unknown
		};

		C_XtDomUpdate() : type(E_UpdateType_Unknown), pElement(0) {}
		C_XtDomUpdate(Xt::DomElement* _pElement) :
			type(E_UpdateType_Element), pElement(_pElement) {}
		C_XtDomUpdate(Xt::DomEvent* _pEvent) :
			type(E_UpdateType_Event), pEvent(_pEvent) {}

		Xt::DomElement* GetXtData() 
		{ 
			if (type == E_UpdateType_Event)
				return pEvent->from;
			else if (type == E_UpdateType_Element)
				return pElement;
			assert(!"Unknown update type!");
			return 0;
		}

		E_UpdateType type;
		union
		{
			Xt::DomElement* pElement;
			Xt::DomEvent* pEvent;
		};
	};

	static const char* kXtDefaultPack		= "TestLevels";
	static const char* kXtLevelRecordType	= "LevelActorList";
}

#endif
