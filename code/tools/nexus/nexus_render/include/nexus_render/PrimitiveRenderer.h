#ifndef NEXUS_RENDER_PRIMITIVE_RENDERER_H
#define NEXUS_RENDER_PRIMITIVE_RENDERER_H

namespace NexusRender
{
	class PrimitiveRenderer
	{
	public:
		PrimitiveRenderer();

		~PrimitiveRenderer();

		void Clear();

		Tu::Status Draw();

		Tu::Status Rebuild();

		void DrawLine(
			const Maths::HMat44& m,
			const Maths::HVec3& a, 
			const Maths::HVec3& b,
			const Maths::HVec4& colour);

		void DrawStar(
			const Maths::HMat44& m,
			const F32 radius,
			const Maths::HVec4& colour);

		void DrawCircle(
			const Maths::HMat44& m,
			const F32 radius,
			const Maths::HVec4& colour);

		void DrawCone(
			const Maths::HMat44& m,
			const F32 radius,
			const F32 height,
			const Maths::HVec4& colour);

		void DrawBox(	
			const Maths::HMat44& m,
			const Maths::HVec3& a, 
			const Maths::HVec3& b,
			const Maths::HVec4& colour);

	private:
		//Used to accumulate vertices prior to creating a VB.
		Ut::UtVector<Ut::DynamicVector<D3DVertexPosCol> > m_vbCopy;

		VBWrapper	m_vbWrapper;
		bool		m_dirty;
	};
}

#endif
