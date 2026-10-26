#ifndef EDITOR_MISC_VIEWPORT_HELPERS_H
#define EDITOR_MISC_VIEWPORT_HELPERS_H

namespace EditorLib
{
	class C_NodeId;
}

namespace Editor
{

class C_RenderViewport;

class C_ViewportHelpers
{
public:
	enum E_ViewportView
	{
		E_ViewportView_Perspective,
		E_ViewportView_Front,
		E_ViewportView_Top,
		E_ViewportView_Side
	};

	static void SetViewportCamera(C_RenderViewport* pViewport, E_ViewportView view);
	static void SetViewportObject(C_RenderViewport* pViewport, EditorLib::C_NodeId nodeId);
};

}

#endif