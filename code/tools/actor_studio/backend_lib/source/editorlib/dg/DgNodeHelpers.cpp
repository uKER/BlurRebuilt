#include "stdafx.h"

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Core.h>

namespace EditorLib
{
namespace DgNodeHelpers
{

void BuildNodeList(
	NodeIdArray& ids,
	const Btl::String& selection, 
	bool useName,
	bool componentMode)
{
	Btl::Vector<Btl::String> tokenised;
	//Tokenise by space, not that spaces aren't allowed in names or ids.
	selection.Tokenise(tokenised, ',');

	if (tokenised.Size() == 0)
		return;

	//Reserve maximum storage.
	ids.Reserve(tokenised.Size());

	S32 startOffset;
	if (componentMode)
		//Use last item in list only.
		startOffset = tokenised.Size()-1;
	else
		startOffset = 0;

	for (S32 i = startOffset; i < tokenised.Size(); i++)
	{
		const Btl::String& str = tokenised[i];

		if (!useName)
		{
			ids.PushBack();
			ids.Back().FromString(str);
		}
		else
		{
			BIZ_ASSERT_MSG(false, "Finding a node by name is no longer supported!");
			/*
			const I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeByName(str);
			if (pNode)
				ids.PushBack(pNode->GetId());
			*/
		}
	}
}

void BuildPath(
	Btl::String& path,
	const I_DgObject* pDgObject)
{
	if (pDgObject->IsA(E_ObjType_SceneNode))
	{
		C_SceneNode* pSceneNode = (C_SceneNode*)pDgObject;
		if (pSceneNode->GetParent())
		{
			BuildPath(path, pSceneNode->GetParent());
			path += "|";
		}
		path += pSceneNode->GetName();
	}
}

}
}