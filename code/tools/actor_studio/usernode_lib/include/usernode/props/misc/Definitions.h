#ifndef EDITOR_PROPS_DEFINITIONS_H
#define EDITOR_PROPS_DEFINITIONS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Core.h>
#include <EditorLib/CoreConfig.h>

#include <EditorApp/Editor/Manipulators/ManipulatorContextFactory.h>

namespace Editor
{
	class C_RenderCore;
	class C_RenderScene;
	class C_RenderPipeline;
}

template<typename TYPE>
INLINE void RegisterManipulatorContext() { if (EditorLib::C_Core::Instance().GetConfig().appType == EditorLib::E_AppType_Interactive) Editor::C_ManipulatorContextFactory::Instance().Register<TYPE>(); }
template<typename TYPE>
INLINE void RegisterRenderNode() { if (EditorLib::C_Core::Instance().GetConfig().appType == EditorLib::E_AppType_Interactive) { Editor::C_RenderCore::Instance().GetRenderScene().GetDgNodeFactory().Register<TYPE>(); Editor::C_PropertyVisibilityManager::Instance().Register<TYPE>(TYPE::GetStaticDisplayName()); } }

#define REGISTER_MANIPULATOR_CONTEXT(type)	RegisterManipulatorContext<type>();
#define REGISTER_RENDER_NODE(type)			RegisterRenderNode<type>();

#define REGISTER_CONTEXTS(name) { void Register##name##Contexts(); Register##name##Contexts(); }
#define REGISTER_RENDER_NODES(name) { void Register##name##RenderNodes(); Register##name##RenderNodes(); }
#endif
