#ifndef EDITOR_RENDERER
#define EDITOR_RENDERER

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_DgNodeSelection;

	class I_EditorRenderer : public AftkRender::C_Renderer
	{
	public:
		virtual void Clear() = 0;
		virtual void Update(AftkRender::E_RendererPass pass = AftkRender::E_RendererPass_Normal) = 0;

		virtual void GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection) = 0;
	
		virtual void Render() = 0;
	};
}

#endif