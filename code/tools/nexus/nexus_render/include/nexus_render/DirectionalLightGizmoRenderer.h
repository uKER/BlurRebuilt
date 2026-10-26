#ifndef NEXUS_RENDER_DIRECTIONAL_LIGHT_GIZMO_RENDERER_H
#define NEXUS_RENDER_DIRECTIONAL_LIGHT_GIZMO_RENDERER_H

namespace NexusRender
{
	class DirectionalLightGizmoRenderer : public GizmoRenderer
	{
	public:
		Tu::Status UpdateScene(U32 numRenderNodes);

	private:
		void ResetGizmo(
			U32 vbOffset,
			U32 nGizmoVerts,
			RenderNode* pRenderNode);
	};
};

#endif
