#ifndef NEXUS_RENDER_PRIMITIVES_H
#define NEXUS_RENDER_PRIMITIVES_H

#define NEXUS_RENDER_PRIMITIVES_POINTS_PER_CIRCLE 16
#define NEXUS_RENDER_PRIMITIVES_POINTS_PER_CONE_BODY 4

#define NEXUS_RENDER_PRIMITIVES_STAR_VERTS		8
#define NEXUS_RENDER_PRIMITIVES_LINE_VERTS		2
#define NEXUS_RENDER_PRIMITIVES_CIRCLE_VERTS	NEXUS_RENDER_PRIMITIVES_POINTS_PER_CIRCLE*2
#define NEXUS_RENDER_PRIMITIVES_CONE_VERTS		NEXUS_RENDER_PRIMITIVES_CIRCLE_VERTS + NEXUS_RENDER_PRIMITIVES_POINTS_PER_CONE_BODY * NEXUS_RENDER_PRIMITIVES_LINE_VERTS
#define NEXUS_RENDER_PRIMITIVES_BOX_VERTS		NEXUS_RENDER_PRIMITIVES_LINE_VERTS*12

namespace NexusRender
{
	class Primitives
	{
	public:
		static void DrawLine(
			D3DVertexPosCol* pVB,
			const Maths::HMat44& m,
			const Maths::HVec3& A, 
			const Maths::HVec3& B,
			const Maths::HVec4& colour);

		static void DrawStar(
			D3DVertexPosCol* pVB,
			const Maths::HMat44& m,
			const F32 radius,
			const Maths::HVec4& colour);

		static void DrawCircle(
			D3DVertexPosCol* pVB,
			const Maths::HMat44& m,
			const F32 radius,
			const Maths::HVec4& colour);

		static void DrawCone(
			D3DVertexPosCol* pVB,
			const Maths::HMat44& m,
			const F32 radius,
			const F32 height,
			const Maths::HVec4& colour);

		static void DrawBox(
			D3DVertexPosCol* pVB,
			const Maths::HMat44& m,
			const Maths::HVec3& min, 
			const Maths::HVec3& max, 
			const Maths::HVec4& colour);
	};
}

#endif
