#ifndef EDITOR_LIB_XT_SEMANTIC_H
#define EDITOR_LIB_XT_SEMANTIC_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	static const char* kXtSemanticAttr					= "SceneView";
	static const char* kXtSemanticParam					= "semantic";
	//Semantic labels for use with the C_XtSemanticHelper class.
	static const char* kXtSemanticLabel_ActorArray		= "ActorArray";
	static const char* kXtSemanticLabel_NodeArray		= "NodeArray";
	static const char* kXtSemanticLabel_DataArray		= "DataArray";
	static const char* kXtSemanticLabel_Properties		= "Properties";
	static const char* kXtSemanticLabel_Layers			= "Layers";
	static const char* kXtSemanticLabel_Name			= "Name";
	static const char* kXtSemanticLabel_WorldMatrix		= "WorldMatrix";
	static const char* kXtSemanticLabel_ParentActor		= "ParentActor";
	static const char* kXtSemanticPrefix_Type			= "Type:";
}

#endif
