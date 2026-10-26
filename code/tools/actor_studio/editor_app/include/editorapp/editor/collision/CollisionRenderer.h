#ifndef EDITOR_COLLISION_RENDERER_H
#define EDITOR_COLLISION_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_CollisionRenderer : public Display::I_Renderer
	{
	public:
		C_CollisionRenderer();
		~C_CollisionRenderer();

		void Render();

	private:
		C_CollisionRenderer(const C_CollisionRenderer& rhs);
		const C_CollisionRenderer& operator=(const C_CollisionRenderer& rhs);

		void RenderCollisionScene(const EditorLib::C_CollisionScene* pCollisionScene);
	};
}

#endif
