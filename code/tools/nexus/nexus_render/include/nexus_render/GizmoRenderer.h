#ifndef NEXUS_RENDER_GIZMO_RENDERER_H
#define NEXUS_RENDER_GIZMO_RENDERER_H

namespace NexusRender
{
	class GizmoRenderer
	{
	public:
		void Clear();

		Tu::Status Update(const Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes);

		void Draw();

	protected:
		Tu::Status UpdateScene(U32 numRenderNodes, U32 nGizmoVerts);

		virtual void ResetGizmo(
			U32 vbOffset,
			U32 nGizmoVerts,
			RenderNode* pRenderNode) = 0;

		VBWrapper m_vbWrapper;

	private:
		U32 m_nGizmoVerts;
	};
}

#endif
