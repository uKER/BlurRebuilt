#include "stdafx.h"
#include "racingphysics/vehicles/debugsliprecorder.h"

#if !PLATFORM_SPU
#include "racingphysics/debug/draw.h"
#include "display/render/basicrender.h"
#endif

//#pragma optimize("", off)

///////////////////////////////////

namespace RacingPhysics 
{

DECLARE_DEBUG_BOOL(debugSlipRecorderEnable,"RacingPhysics.Debug.SlipRecorder.Enable", false, Debug::Persist());
DECLARE_DEBUG_INT(debugSlipRecorderMaxRecords,"RacingPhysics.Debug.SlipRecorder.MaxSlipRecords", 5000, Debug::Range(0, 10000), Debug::Step(1), Debug::Persist());
DECLARE_DEBUG_BOOL(debugSlipRecorderUseZ,"RacingPhysics.Debug.SlipRecorder.UseZ", true, Debug::Persist());
DECLARE_DEBUG_FLOAT(debugSlipRecorderWidth,"RacingPhysics.Debug.SlipRecorder.LineWidth", 0.015f, Debug::Range(0.01f, 0.1f), Debug::Persist());

DebugSlipRecorder::DebugSlipRecorder() 
{
}

void DebugSlipRecorder::Initialise()
{
	RP_ASSERT(m_slipRecords.Empty());
}

void DebugSlipRecorder::Update(Point3R p, float slip, bool onGround)
{
	if (!debugSlipRecorderEnable)
	{
		m_slipRecords.Clear();
		return;
	}

	p += Maths::Constants<Normal3R>::Y() * 0.03f;

	if (!m_slipRecords.Empty())
	{
		Vector3R v = m_slipRecords.Back().Position() - p;
		float f = Magnitude2(v);

		if (f < 0.1f * 0.1f)
			return;
		else if (f > 10.f * 10.f)
			m_slipRecords.Clear();
	}

	while (m_slipRecords.Size() > debugSlipRecorderMaxRecords)
		m_slipRecords.PopFront();

	m_slipRecords.PushBack(SlipRecord(p, slip, onGround));
}

float g_slipRecorderDepthBias = 0.01f;

#if !PLATFORM_SPU

void DebugSlipRecorder::Display() const
{
	if (m_slipRecords.Size() < 2)
		return;

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	
	Display::Device()->SetMaxBeginVerticesUsage(100000);
	Display::BasicRender()->BindColoured3D();

	if (debugSlipRecorderUseZ)
	{
		Display::Device()->SetDepthBias(g_slipRecorderDepthBias);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	}
	else
	{
		Display::Device()->SetDepthEnable(Display::E_Bool_False);
	}

	// Draw with triangles, for width, and lines, for distance.
	// Unfortunately depth bias doesn't work for lines, so still need
	// a switch to turn off z.
	{
		DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);

		Point3R previous;
		int vertexCount = 0;

		for (Btl::List<SlipRecord>::ConstIterator it = m_slipRecords.Begin(); it != m_slipRecords.End(); ++it)
		{
			int c = int(255.f * it->Slip());

			if (it->Slip() < 0.f)
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB(c, 0, 255));
			else if (c < 255)
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB(c, 255, 0));
			else
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB(255, Maths::Max(0, (255 * 2) - c), 0));

			Point3R p = it->Position();

			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
		}

		DrawPrimitive::Instance().End();
	}

	// triangles
	{
		DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_TriangleStrip);

		Point3R previous;
		int vertexCount = 0;

		Vector3R v;
		Point3R p;

		for (Btl::List<SlipRecord>::ConstIterator it = m_slipRecords.Begin(); it != m_slipRecords.End(); ++it)
		{
			int c = int(255.f * it->Slip());

			if (it->Slip() < 0.f)
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB(c, 0, 255));
			else if (c < 255)
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB(c, 255, 0));
			else
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB(255, Maths::Max(0, (255 * 2) - c), 0));

			p = it->Position();
	
			Vector3R s = Vector3R(0.f, 0.f, 0.f);

			if (it != m_slipRecords.Begin())
				s += Normalised((it - 1)->Position() - p);

			if (it + 1 != m_slipRecords.End())
				s += Normalised(p - (it + 1)->Position());

			previous = p;

			v = (Normalised(s) ^ Maths::Constants<Normal3R>::Y()) * (float) debugSlipRecorderWidth;

			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - v.X(), p.Y() - v.Y(), p.Z() - v.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());

			vertexCount++;
			if (vertexCount == 100 && (it + 1) != m_slipRecords.End())
			{
				DrawPrimitive::Instance().End();
				DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_TriangleStrip);
				vertexCount = 0;

				DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - v.X(), p.Y() - v.Y(), p.Z() - v.Z());
				DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());
			}
		}

		DrawPrimitive::Instance().End();
	}

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthBias(g_slipRecorderDepthBias);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
}

#endif

void DebugSlipRecorder::Shutdown()
{
	Btl::List<SlipRecord> empty;
	Btl::Swap(m_slipRecords, empty);
}

}
