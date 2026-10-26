#ifndef RP_SLIP_RECORDER_H
#define RP_SLIP_RECORDER_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/list.h"
#include "racingphysics/physics/geometry.h"

namespace RacingPhysics
{

class DebugSlipRecorder
{
public:
	DebugSlipRecorder();

	void Initialise();
	void Update(Point3R p, float slip, bool onGround);
	void Display() const;
	void Shutdown();

private:

	struct SlipRecord
	{
		SlipRecord() {}
		SlipRecord(const Point3R& position, float slip, bool onGround) 
		{
			assert(slip >= 0.f);
			v.X(position.X()), v.Y(position.Y()), v.Z(position.Z());
			if (!onGround) 
				slip = -1.f;

			v.W(slip);
		}

		const Point3R& Position() const {return (const Point3R&) v;}
		float Slip() const {return v.W();}

	private:
		Vector4R v;
	};

	Btl::List<SlipRecord> m_slipRecords;
};

}

#endif
