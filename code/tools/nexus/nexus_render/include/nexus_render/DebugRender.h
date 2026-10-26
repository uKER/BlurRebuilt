#pragma once

namespace NexusRender
{
	class DebugRender
	{
	public:
		DebugRender();
		~DebugRender();

		Tu::Status Create();
		void Destroy();

		void Clear();

		//Flush contents to screen.
		Tu::Status Draw();

		// 3D Rendering Calls
		void Draw3DLine(const TMathsNM::HVec3& A, const TMathsNM::HVec3& B, const TMathsNM::HVec4& colour);
		void Draw3DLine(const TMathsNM::HVec3& A, const TMathsNM::HVec3& B, const TMathsNM::HVec4& Acolour, const TMathsNM::HVec4& Bcolour);

		void Draw3DTriangle(const TMathsNM::HVec3& A, const TMathsNM::HVec3& B, const TMathsNM::HVec3& C, const TMathsNM::HVec4& colour);
		void Draw3DTriangle(const TMathsNM::HVec3& A, const TMathsNM::HVec3& B, const TMathsNM::HVec3& C, const TMathsNM::HVec4& Acolour, const TMathsNM::HVec4& Bcolour, const TMathsNM::HVec4& Ccolour);

		void Draw3DPoint(const TMathsNM::HVec3& pos, const F32 size, const TMathsNM::HVec4& colour);
		void Draw3DCross(const TMathsNM::HVec3& pos, const F32 size, const TMathsNM::HVec4& colour);
		void Draw3DStar(const TMathsNM::HVec3& pos, const F32 size, const TMathsNM::HVec4& colour);

		void Draw3DPlanePoint(const TMathsNM::HVec3& pos, const TMathsNM::HVec3& normal, const F32 size, const TMathsNM::HVec4& colour);
		void Draw3DPlaneCross(const TMathsNM::HVec3& pos, const TMathsNM::HVec3& normal, const F32 size, const TMathsNM::HVec4& colour);
		void Draw3DPlaneStar(const TMathsNM::HVec3& pos, const TMathsNM::HVec3& normal, const F32 size, const TMathsNM::HVec4& colour);

		void Draw3DArrow(const TMathsNM::HVec3& pos, const TMathsNM::HVec3& dir, const F32 length_scale, const F32 arrowhead_size, const TMathsNM::HVec4& colour);

		void Draw3DCircle(const TMathsNM::HVec3& centre, const TMathsNM::HVec3& normal, const F32 radius, const TMathsNM::HVec4& colour);

		void Draw3DSphere(const TMathsNM::HVec3& centre, const F32 radius, const TMathsNM::HVec4& colour);
		void Draw3DSphereOutline(const TMathsNM::HVec3& centre, const F32 radius, const TMathsNM::HVec4& colour);

		void Draw3DMatrix();
		void Draw3DMatrix(const TMathsNM::HMat44& m);

		void Draw3DBox(
			const TMathsNM::HVec3& min, 
			const TMathsNM::HVec3& max, 
			const TMathsNM::HMat44& m, 
			const TMathsNM::HVec4& colour);

		void Draw3DBox(
			const TMathsNM::HVec3& min, 
			const TMathsNM::HVec3& max, 
			const TMathsNM::HVec4& colour);

		Tu::Status OnCameraZoom();

		Tu::Status RebuildVertexBuffers();

	public:
		///Array of vertex buffers for this scene.
		VBWrapper<Primitive3DVertex> m_lineVertexBuffers3D;
		VBWrapper<Primitive3DVertex> m_matrixVertexBuffer;

		bool m_vertexBuffersBuilt;
	};
}
