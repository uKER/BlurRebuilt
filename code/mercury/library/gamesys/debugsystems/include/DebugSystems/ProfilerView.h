
#ifndef CAMERA2_PROFILERVIEW_H
#define CAMERA2_PROFILERVIEW_H

#pragma warning (push, 3)
#include <btl/containers/vector.h>
#pragma warning (pop)

namespace Display
{
	struct C_BasicRenderColouredVertex2D;
}

namespace Perf
{
	class ProfilerCommon;
}

namespace DebugSystems
{

class C_ProMark
{
public:
	const char* m_Name;
	F32 m_Time; // Time in milliseconds
	U32 m_Colour;
};

class C_ProTotal
{
public:
	C_ProTotal(const char* name, U32 colour, F32 total, S8 depth);

	const char* m_Name;
	U32 m_Colour;
	F32 m_Total; // Total time in milliseconds
	S8 m_Depth;
};

class C_ProfilerView
{
public:
	C_ProfilerView();
	void Init();
	void Render();
	static void ProfilerCallback(Perf::ProfilerCommon* profiler, void* that);

private:
	Btl::Vector<C_ProMark> m_ProMark;
	Btl::Vector<C_ProTotal> m_ProTotal;

	U64 m_LastFrameEnd;
	Display::C_BasicRenderColouredVertex2D* m_VertexBuffer;
	S32 m_VertexPos;
	S32 m_VertexCount;
	F32 m_LastTime;
	bool m_bCallbackRegistered;

	void Callback(Perf::ProfilerCommon* profiler);
	void AddTimeToCategory(const char* name, U32 colour, F32 ms, S8 depth);
	void AddRect(F32 x1, F32 y1, F32 x2, F32 y2, U32 colour);
	void AddMark(const char* name, S32 timeVal, S8 depth);
};

}

#endif
