
#pragma once


class Debug2dVertex;
class Debug3dVertex;

class DebugRender
{
public:
                                        DebugRender();
                                        ~DebugRender();

	void								Init(SRD::D3dWindow* pDisplayWindow);
    void                                Close();

    // 2D Rendering Calls
    void                                Draw2dLine(const SRD::Position2& start, const SRD::Position2& end, const SRD::Colour4& colour);
    void                                Draw2dCircle(const SRD::Position2& centre, const F32 radius, const SRD::Colour4& colour);
    void                                Draw2dArrow(const SRD::Position2& start, const SRD::Position2& dir, const F32 arrowheadSize, const SRD::Colour4& colour);

    // 3D Rendering Calls

    void                                Draw3dLine(const SRD::Position3& A, const SRD::Position3& B, const SRD::Colour4& colour);
	void                                Draw3dLine(const SRD::Position3& A, const SRD::Position3& B, const SRD::Colour4& Acolour, const SRD::Colour4& Bcolour);

	void                                Draw3dTriangle(const SRD::Position3& A, const SRD::Position3& B, const SRD::Position3& C, const SRD::Colour4& colour);
    void                                Draw3dTriangle(const SRD::Position3& A, const SRD::Position3& B, const SRD::Position3& C, const SRD::Colour4& Acolour, const SRD::Colour4& Bcolour, const SRD::Colour4& Ccolour);

    void                                Draw3dPoint(const SRD::Position3& pos, const F32 size, const SRD::Colour4& colour);
    void                                Draw3dCross(const SRD::Position3& pos, const F32 size, const SRD::Colour4& colour);
    void                                Draw3dStar(const SRD::Position3& pos, const F32 size, const SRD::Colour4& colour);

    void                                Draw3dPlanePoint(const SRD::Position3& pos, const SRD::Normal3& normal, const F32 size, const SRD::Colour4& colour);
    void                                Draw3dPlaneCross(const SRD::Position3& pos, const SRD::Normal3& normal, const F32 size, const SRD::Colour4& colour);
    void                                Draw3dPlaneStar(const SRD::Position3& pos, const SRD::Normal3& normal, const F32 size, const SRD::Colour4& colour);

    void                                Draw3dArrow(const SRD::Position3& pos, const SRD::Position3& dir, const F32 length_scale, const F32 arrowhead_size, const SRD::Colour4& colour);

    void                                Draw3dCircle(const SRD::Position3& centre, const SRD::Normal3& normal, const F32 radius, const SRD::Colour4& colour);

    void                                Draw3dSphere(const SRD::Position3& centre, const F32 radius, const SRD::Colour4& colour);
    void                                Draw3dSphereOutline(const SRD::Position3& centre, const F32 radius, const SRD::Colour4& colour);

                                        // TODO : change to HMat43?
    //void                                Draw3dMatrix(const Matrix44& m);

	void                                Draw3dBox(const SRD::Position3& min, const SRD::Position3& max, const SRD::Matrix44& m, const SRD::Colour4& colour);
                                        // TODO : change to HMat43?
	void                                Draw3dGrid(const SRD::Matrix44& m);

    // TODO : implement plane rendering functions
    //void                                Draw3dPlane(const CPlane& plane, const Matrix44& m, F32 plane_size, F32 normal_length, BizMaths::CVector4 colour);
    //void                                Draw3dPlane(const CPlane& plane, const BizMaths::CVector4 closest_pos, const Matrix44& m, F32 plane_size, F32 normal_length, BizMaths::CVector4 colour);

public:
	SRD::D3dWindow*	                    m_pD3dWindow;

    DebugVertexBuffer<Debug3dVertex>    m_DynamicVertexBuffer3D;
    DebugVertexBuffer<Debug3dVertex>    m_StaticVertexBuffer3D;

    DebugVertexBuffer<Debug2dVertex>    m_DynamicVertexBuffer2D;

	///
	///Array of vertex buffers for this scene.
//	Ut::UtVector<IDirect3DVertexBuffer9*> m_lineVertexBuffers;

    U32                                 m_MatrixBaseVertex;
};
