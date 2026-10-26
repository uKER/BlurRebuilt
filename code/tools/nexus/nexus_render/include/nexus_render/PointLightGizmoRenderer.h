#ifndef NEXUS_RENDER_POINT_LIGHT_GIZMO_RENDERER_H
#define NEXUS_RENDER_POINT_LIGHT_GIZMO_RENDERER_H

namespace NexusRender
{
	class PointLightGizmoRenderer : public GizmoRenderer
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
