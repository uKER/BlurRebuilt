#ifndef EDITOR_LIB_XT_HELPERS_H
#define EDITOR_LIB_XT_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Xt/XtDefinitions.h>

namespace EditorLib
{
	namespace XtHelpers
	{
		struct C_PackRecords
		{
			XtId packId;
			XtRecordList levelRecords;
		};
		typedef Ut::Vector<C_PackRecords> XtPackRecordsArray;

		C_Status BuildLevelList(XtPackRecordsArray& packRecordsArray, Xt::DomDb* pDB);

		bool IsA(LarvaType* pLarvaType, const char* typeName);

		bool IsChildOfOrEqualTo(const Xt::DomElement* pParent, const Xt::DomElement* pChild);

		bool AreRelated(const Xt::DomElement* l, const Xt::DomElement* r);

		Xt::DomBlock* GetNearestBlock(Xt::DomElement* pData);

		bool GetMatrix(Matrix44& matrix, Xt::DomComponent* pActor);
	};

	#define XT_HELPERS_AT_BLOCK_CHECKED(block, from, name) \
		block = Xt::AsBlock(Xt::At(from, name)); \
		if (!block) \
		{ \
			LOGERROR(C_Core::Instance().GetLogger(), "Required block \"%s\" is missing from actor! Please inform the tools team.\n", name); \
		} 

	#define XT_HELPERS_AT_ARRAY_CHECKED(arr, from, name) \
		arr = Xt::AsArray(Xt::At(from, name)); \
		if (!arr) \
		{ \
			LOGERROR(C_Core::Instance().GetLogger(), "Required array \"%s\" is missing from actor! Please inform the tools team.\n", name); \
		} 

	#define XT_HELPERS_AT_FIELD_CHECKED(field, from, name) \
			field = Xt::AsField(Xt::At(from, name)); \
			if (!field) \
			{ \
				LOGERROR(C_Core::Instance().GetLogger(), "Required field \"%s\" is missing from actor! Please inform the tools team.\n", name); \
			}

	#define XT_HELPERS_AT_REF_CHECKED(ref, from, name) \
		ref = Xt::AsRef(Xt::At(from, name)); \
		if (!ref) \
		{ \
			LOGERROR(C_Core::Instance().GetLogger(), "Required ref \"%s\" is missing from actor! Please inform the tools team.\n", name); \
		} 
}

#endif
