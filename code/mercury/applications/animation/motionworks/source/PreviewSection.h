#ifndef PREVIEWSECTION_H
#define PREVIEWSECTION_H

#include "SkinnedMesh.h"
#include <btl/Containers/vector.h>
#include <animation/common/ObjectTransform.h>
#include <sigslot/sigslot.h>
#include "Gui.h"

namespace Animation
{
	class C_JointTransform;
}

namespace Display
{
	class C_DebugRender;
}

namespace Render
{
	namespace Font
	{
		class C_FontRenderer;
	}
}

class C_PreviewSection: /*public Display::I_Renderer, */public sigslot::has_slots<>
{
public:
	C_PreviewSection();
	~C_PreviewSection();

	void OnRender();
	
	void RenderHud( S32 selectedJointIndex );

	void RenderConstraints( S32 selectedJointIndex );

	
	//
	// UI interaction
	//
	void OnMouseMove(S32 x, S32 y, U32 buttons);
	void OnMouseDown(S32 x, S32 y, U32 buttons);
	void OnMouseUp(S32 x, S32 y, U32 buttons);
	void OnKeyDown(U32 key);
	void OnKeyUp(U32 key);
	void OnResize(U32 w, U32 h);

private:
	void GuiShowJointTransform(const Animation::C_JointTransform& jointTransform);

	typedef Btl::Vector<Animation::C_JointTransform> JointTransformVector;
	typedef Btl::Vector<Animation::C_ObjectTransform> ObjectTransformVector;

	ObjectTransformVector	m_transformBuffer;
	C_SkinnedMeshRenderer	m_skinnedMeshRenderer;
	
	C_GuiRender				m_guiRender;
	C_Gui					m_gui;
	
	U32						m_viewportWidth;
	U32						m_viewportHeight;
	
	// mouse x, y, left down, right down, middle down
	
	Render::Font::C_FontRenderer* m_fontRenderer;
};


#endif
