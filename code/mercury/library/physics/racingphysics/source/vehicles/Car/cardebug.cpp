#include "stdafx.h"
#include "racingphysics/vehicles/car/car.h"

#include "racingphysics/debug/draw.h"
#include "display/render/basicrender.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

/////////////////////////////////////////////////////

namespace RacingPhysics
{

EXTERN_DEBUG_BOOL( g_displayDebugVehicle );
EXTERN_DEBUG_BOOL( g_showNetworkState );
EXTERN_DEBUG_BOOL( g_showVehicleDebugText );
EXTERN_DEBUG_INT( g_debugVehicleIndex );
EXTERN_DEBUG_BOOL( g_debugShowForces );
EXTERN_DEBUG_BOOL( g_debugShowSlipDiagram );
EXTERN_DEBUG_BOOL( g_debugShowSlipRecords );
//EXTERN_DEBUG_BOOL( g_debugVehicleCollisionMesh;
EXTERN_DEBUG_BOOL( g_showTyreSlipGraph );

const static int N_SEGMENTS = 16;

static float sinTable[N_SEGMENTS + 1];
static float cosTable[N_SEGMENTS + 1];

static void Precalc()
{
	static bool ready = false;
	if (ready) return;
	ready = true;

	for (int j = 0; j <= N_SEGMENTS; j++)
	{
		float b = j * 2 * Maths::Pi / N_SEGMENTS;
		sinTable[j] = sin(b);
		cosTable[j] = cos(b);
	}
}

class SevenSegmentDisplay
{
public:
	static void DisplayDigit(float x0, float y0, float size, int n)
	{
		RP_ASSERT(n >= 0 && n <= 9);

		for (int i = 0; i < 7; i++)
			if (s_digits[n][i])
				Display::BasicRender()->DrawColoured2DLine(	x0 + s_display[i].x0 * size, 
															y0 + s_display[i].y0 * size, 
															x0 + s_display[i].x1 * size, 
															y0 + s_display[i].y1 * size, 
															Maths::RGBA(ColourARGB::Green.Packed()));
	}

	static void DisplayMinus(float x0, float y0, float size)
	{
		for (int i = 0; i < 7; i++)
			Display::BasicRender()->DrawColoured2DLine(	x0 + s_display[5].x0 * size, 
														y0 + s_display[5].y0 * size, 
														x0 + s_display[5].x1 * size, 
														y0 + s_display[5].y1 * size, 
														Maths::RGBA(ColourARGB::Green.Packed()));
	}

	static void DisplayValue(float x0, float y0, float size, int n)
	{
		int remaining = Maths::Abs(n);

		if (remaining < 0)
			return; // can actually happen

		bool minus = n < 0;
		float offset = 0.f;

		do
		{
			int d = remaining % 10;
			remaining -= d;
			remaining /= 10;
			DisplayDigit(x0 + offset, y0, size, d);
			offset -= size + 3.f;
		} while (remaining > 0);

		if (minus)
			DisplayMinus(x0 + offset, y0, size);
	}

private:
	struct Segment {U8 x0, y0, x1, y1;};
	static Segment s_display[7];
	static U8 s_digits[10][7];
};

//  4
// 0 2
//  5
// 1 3
//  6

SevenSegmentDisplay::Segment SevenSegmentDisplay::s_display[7] = {{0, 0, 0, 1}, {0, 1, 0, 2}, {1, 0, 1, 1}, {1, 1, 1, 2}, {0, 0, 1, 0}, {0, 1, 1, 1}, {0, 2, 1, 2}};

U8 SevenSegmentDisplay::s_digits[10][7] =	{ // 0  1  2  3  4  5  6
												{1, 1, 1, 1, 1, 0, 1}, // 0
												{0, 0, 1, 1, 0, 0, 0}, // 1
												{0, 1, 1, 0, 1, 1, 1}, // 2
												{0, 0, 1, 1, 1, 1, 1}, // 3
												{1, 0, 1, 1, 0, 1, 0}, // 4
												{1, 0, 0, 1, 1, 1, 1}, // 5
												{1, 1, 0, 1, 1, 1, 1}, // 6
												{0, 0, 1, 1, 1, 0, 0}, // 7
												{1, 1, 1, 1, 1, 1, 1}, // 8
												{1, 0, 1, 1, 1, 1, 1}, // 9
											};

void Car::DisplayDebug() const
{
	if (g_debugVehicleIndex != -1 && g_debugVehicleIndex != VehicleIndex())
		return;

	PERF_EVENT("Car::DisplayDebug");

	Vehicle::DisplayDebug();

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetViewShaderConstants();
	//Display::Device()->SetMaxBeginVerticesUsage(1000);
	Display::BasicRender()->BindColoured3D();

	if (g_displayDebugVehicle)
	{
		Precalc();

		for (int w = 0; w < m_wheels.Size(); w++)
		{
			Axis axis;
			axis.SetRotation(m_wheels[w].GetRotationMatrix());
			axis.SetPosition(m_wheels[w].GetWheelPosition());

			Normal3R suspensionAxis = m_wheels[w].CalculateSuspensionAxis(Body().GetAxis());

			float toe = ((w % 2) ? -1.f : 1.f) * m_wheels[w].SuspensionDynamics().toeAngle + m_wheels[w].GetAckermanSteeringCorrection(m_wheels[w].CalculateSteeringAngle());
			axis.Rotate(suspensionAxis, toe);

			float camberAngle = ((w % 2) ? 1.f : -1.f) * m_wheels[w].SuspensionDynamics().camberAngle;
			axis.Rotate(Normalised(suspensionAxis ^ axis.AxisX()), camberAngle);

			const MathsStorage::Matrix44& tempMatrix = axis.GetMatrix();
			Display::BasicRender()->BindColoured3D(&tempMatrix);

			if (m_wheels[w].OnGround())
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Yellow);
			else
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);

			int nSegments = 16;
			float radius = m_wheels[w].CollisionRadius();
			float offset = ((w % 2) ? -0.1f : 0.1f);

			for (int i = 0; i < 2; i++)
			{
				DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);
			
				for (int j = 0; j <= nSegments; j++)
				{
					float b = j * 2 * Maths::Pi / nSegments;
					DrawPrimitive::Instance().SetVertexDataXYZ(offset * (i == 0 ? 1.f : -1.f), radius * sinTable[j], radius * cosTable[j]);
				}

				DrawPrimitive::Instance().End();
			}

			{
				DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);

				for (int j = 0; j <= 2; j++)
				{
					float b = j * 2 * Maths::Pi / 4;
					DrawPrimitive::Instance().SetVertexDataXYZ(offset * 1.f, radius * sin(b + Maths::Pi), radius * cos(b + Maths::Pi));
					DrawPrimitive::Instance().SetVertexDataXYZ(offset * 1.f, radius * sin(b), radius * cos(b));
				}

				DrawPrimitive::Instance().End();
			}
		}

		{
			// axles
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);

			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);

			Point3R p0 = m_wheels[0].CalculateRestPosition(Body().GetAxis());
			Point3R p1 = m_wheels[1].CalculateRestPosition(Body().GetAxis());
			Point3R p2 = m_wheels[2].CalculateRestPosition(Body().GetAxis());
			Point3R p3 = m_wheels[3].CalculateRestPosition(Body().GetAxis());

			DrawPrimitive::Instance().SetVertexDataXYZ(p0.X(), p0.Y(), p0.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p1.X(), p1.Y(), p1.Z());

			DrawPrimitive::Instance().SetVertexDataXYZ(p2.X(), p2.Y(), p2.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p3.X(), p3.Y(), p3.Z());

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);

			for (int i = 0; i < m_wheels.Size(); i++)
			{
				Point3R p0 = m_wheels[i].GetWheelPosition();
				Point3R p1 = m_wheels[i].CalculateRestPosition(Body().GetAxis());

				DrawPrimitive::Instance().SetVertexDataXYZ(p0.X(), p0.Y(), p0.Z());
				DrawPrimitive::Instance().SetVertexDataXYZ(p1.X(), p1.Y(), p1.Z());
			}

			// centre of gravity
			{
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);

				Point3R pf = Lerp(p0, p1, 0.5f);
				Point3R pr = Lerp(p2, p3, 0.5f);

				DrawPrimitive::Instance().SetVertexDataXYZ(pf.X(), pf.Y(), pf.Z());
				DrawPrimitive::Instance().SetVertexDataXYZ(pr.X(), pr.Y(), pr.Z());

				Point3R cofG = Body().CofG();
				float f = ((pf - cofG) | (pf - pr)) / Magnitude2(pf - pr);
				Point3R p = Lerp(pf, pr, f);

				DrawPrimitive::Instance().SetVertexDataXYZ(cofG.X(), cofG.Y(), cofG.Z());
				DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			}

			DrawPrimitive::Instance().End();
		}

		// engine
		{
			Axis axis;
			axis.SetRotation(Engine().GetRotor().GetRotationMatrix());

			axis.SetPosition(GetAxis().Position());

			Display::BasicRender()->BindColoured3D(&axis.GetMatrix());

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);

			int nSegments = 16;
			float radius = 0.3f;

			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);
			
			for (int j = 0; j <= nSegments; j++)
			{
				float b = j * 2 * Maths::Pi / nSegments;
				DrawPrimitive::Instance().SetVertexDataXYZ(0.f, radius * sinTable[j], radius * cosTable[j]);
			}

			DrawPrimitive::Instance().End();
			
			float rpm = m_engine.RPM();
			float maxRPM = Dynamics().engine.engineTorque_Rpm.x1;
			int nBlur = int((N_SEGMENTS * rpm) / maxRPM);

			for (int i = 0; i < 4; i++)
			{
				DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);

				for (int j = 0; j <= nBlur; j++)
				{
					float b = j * 2 * Maths::Pi / nSegments;
					DrawPrimitive::Instance().SetVertexDataXYZ(0.f, radius * ((i + 1) / 5.f) * sin(b), radius * ((i + 1) / 5.f) * cos(b));
				}

				DrawPrimitive::Instance().End();
			}

		}
	}

	if (g_debugShowForces)
	{
		float forceScaleFactor = Mass() * 2.f * (1.f + CalculateDownforceFactor(50.f));

		// gravity
		{
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);

			Point3R p = CofG();
			float f = Mass() * PhysicsConstants::GRAVITY / forceScaleFactor;

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y() - f, p.Z());

			DrawPrimitive::Instance().End();
		}

		// downforce
		if (!Air())
		{
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);

			Point3R p = CofG();
			Normal3R n = -Body().GetAxis().AxisY();
			Vector3R f = m_dd.frame.downforce.GetValue() / forceScaleFactor;

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Yellow);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + f.X(), p.Y() + f.Y(), p.Z() + f.Z());

			DrawPrimitive::Instance().End();
		}

		// ground force
		{
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		
			for (int wi = 0; wi < m_wheels.Size(); wi++)
			{
				if (m_wheels[wi].OnGround()) 
				{
					const Point3R& p = m_wheels[wi].GroundPosition();

					{
						Vector3R v = m_wheels[wi].GroundForce() / forceScaleFactor;
						DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Orange);
						DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
						DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());	
					}

					{
						Vector3R v = m_wheels[wi].GroundFriction() / forceScaleFactor;
						DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Magenta);
						DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
						DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());
					}

					{
						Normal3R n = Normalised(m_wheels[wi].GetRotationMatrix().AxisX() ^ m_wheels[wi].GroundForce());
						Vector3R t = m_wheels[wi].GroundTorque() / forceScaleFactor;
						float s = Maths::Sqrt(Magnitude(t));
						Vector3R vx = Normalised(t ^ n) * s * 0.5f;
						Vector3R vz = Normalised(t ^ vx) * s;

						DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Cyan);
						DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - vx.X(), p.Y() - vx.Y(), p.Z() - vx.Z());
						DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + vx.X(), p.Y() + vx.Y(), p.Z() + vx.Z());

						DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - vx.X(), p.Y() - vx.Y(), p.Z() - vx.Z());
						DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - vx.X() - vz.X(), p.Y() - vx.Y() - vz.Y(), p.Z() - vx.Z() - vz.Z());

						DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + vx.X(), p.Y() + vx.Y(), p.Z() + vx.Z());
						DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + vx.X() + vz.X(), p.Y() + vx.Y() + vz.Y(), p.Z() + vx.Z() + vz.Z());
					}
				}
			}

			DrawPrimitive::Instance().End();
		}

		// steering torque
		{
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		
			Normal3R n = GetAxis().AxisY();
			Vector3R t = m_dd.frame.steeringTorque.GetValue() / forceScaleFactor;
			float s = Maths::Sqrt(Magnitude(t));
			Vector3R vx = Normalised(t ^ n) * s * 0.5f;
			Vector3R vz = Normalised(t ^ vx) * s;

			Point3R p0 = m_wheels[0].CalculateRestPosition(Body().GetAxis());
			Point3R p1 = m_wheels[1].CalculateRestPosition(Body().GetAxis());
			Point3R p = Lerp(p0, p1, 0.5f);

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::White);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - vx.X(), p.Y() - vx.Y(), p.Z() - vx.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + vx.X(), p.Y() + vx.Y(), p.Z() + vx.Z());

			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - vx.X(), p.Y() - vx.Y(), p.Z() - vx.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - vx.X() - vz.X(), p.Y() - vx.Y() - vz.Y(), p.Z() - vx.Z() - vz.Z());

			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + vx.X(), p.Y() + vx.Y(), p.Z() + vx.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + vx.X() + vz.X(), p.Y() + vx.Y() + vz.Y(), p.Z() + vx.Z() + vz.Z());

			DrawPrimitive::Instance().End();
		}
		
		// air friction
		{
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		
			const Point3R& p = Body().CofG();
			Vector3R v = m_dd.frame.airFrictionForce.GetValue() / forceScaleFactor;
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Magenta);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());	

			DrawPrimitive::Instance().End();
		}

		// network force
		{
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		
			const Point3R& p = Body().CofG();
			Vector3R v = m_dd.frame.networkForce.GetValue() / forceScaleFactor;
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::White);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());	

			DrawPrimitive::Instance().End();
		}


		// velocity
		if (Magnitude2(Body().Velocity()) > 1.f)
		{
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		
			const Point3R& p = Body().CofG();
			Normal3R v = Normalised(Body().Velocity());
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());	

			DrawPrimitive::Instance().End();
		}
	}

	if (g_showNetworkState && GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK)
	{
		Display::BasicRender()->BindColoured3D();
		DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);

		if (m_dd.predictedNetworkState.networkStateValid)
		{
			Point3R p = m_dd.predictedNetworkState.axis.Position();
			Vector3R az = m_dd.predictedNetworkState.axis.AxisZ() * 0.2f;
			Vector3R ax = m_dd.predictedNetworkState.axis.AxisX() * 0.1f;
			Vector3R ay = m_dd.predictedNetworkState.axis.AxisY() * 0.1f;

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Blue);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + az.X(), p.Y() + az.Y(), p.Z() + az.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - az.X(), p.Y() - az.Y(), p.Z() - az.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ax.X(), p.Y() + ax.Y(), p.Z() + ax.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ax.X(), p.Y() - ax.Y(), p.Z() - ax.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ay.X(), p.Y() + ay.Y(), p.Z() + ay.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ay.X(), p.Y() - ay.Y(), p.Z() - ay.Z());

			const Point3R& p1 = Body().GetAxis().Position();
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p1.X(), p1.Y(), p1.Z());	
		}

		if (m_networkState.networkStateValid)
		{
			Point3R p = m_networkState.axis.Position();
			Vector3R az = m_networkState.axis.AxisZ() * 0.2f;
			Vector3R ax = m_networkState.axis.AxisX() * 0.0f;
			Vector3R ay = m_networkState.axis.AxisY() * 0.1f;

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Yellow);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + az.X(), p.Y() + az.Y(), p.Z() + az.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - az.X(), p.Y() - az.Y(), p.Z() - az.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ax.X(), p.Y() + ax.Y(), p.Z() + ax.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ax.X(), p.Y() - ax.Y(), p.Z() - ax.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ay.X(), p.Y() + ay.Y(), p.Z() + ay.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ay.X(), p.Y() - ay.Y(), p.Z() - ay.Z());
		}

		if (m_dd.predictedNetworkStateJump.networkStateValid)
		{
			Point3R p = m_dd.predictedNetworkStateJump.axis.Position();
			Vector3R az = m_dd.predictedNetworkStateJump.axis.AxisZ() * 0.2f;
			Vector3R ax = m_dd.predictedNetworkStateJump.axis.AxisX() * 0.1f;
			Vector3R ay = m_dd.predictedNetworkStateJump.axis.AxisY() * 0.1f;

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + az.X(), p.Y() + az.Y(), p.Z() + az.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - az.X(), p.Y() - az.Y(), p.Z() - az.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ax.X(), p.Y() + ax.Y(), p.Z() + ax.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ax.X(), p.Y() - ax.Y(), p.Z() - ax.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ay.X(), p.Y() + ay.Y(), p.Z() + ay.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ay.X(), p.Y() - ay.Y(), p.Z() - ay.Z());

			const Point3R& p1 = Body().GetAxis().Position();
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p1.X(), p1.Y(), p1.Z());	
		}

		DrawPrimitive::Instance().End();
	}


	if (g_debugShowSlipRecords)
		for (int i = 0; i < m_wheels.Size(); i++)
			m_wheels[i].m_debugSlipRecorder.Display();

	if (g_debugShowSlipDiagram)
	{
		Display::BasicRender()->BindColoured3D();
		
		float yStart = 125.f;
		//float screenRight = (float) Display::Device()->GetViewport().GetW() - 20.f;
		float screenRight = 1024.f - 20.f;
		float forceScale = Mass() * PhysicsConstants::GRAVITY * 0.25f * (1.f + CalculateDownforceFactor(50.f));

		for (int i = 0; i < m_wheels.Size(); i++)
		{
			const CarWheel& wheel(Wheel(i));

			float x0 = screenRight - ((i % 2) ? 80.f : 160.f);
			float y0 = (yStart + ((i > 1) ? 80.f : 0.f));
			float r = 30.f;
			float separation = 4.f;
			Display::BasicRender()->DrawColoured2DCircle(x0, y0, r, Maths::RGBA(ColourARGB::Red.Packed()));

			if (wheel.OnGround())
			{
				Display::BasicRender()->DrawColoured2DLine(x0, y0, x0 + wheel.SlipAngle() * r / wheel.TransverseTyreSlip(), y0 + wheel.WheelSpin() * r / wheel.LongitudinalTyreSlip(), Maths::RGBA(ColourARGB::Green.Packed()));
				Display::BasicRender()->DrawColoured2DLine(x0 - r - separation, y0 + r, x0 - r - separation, y0 + r - (float) (Magnitude(wheel.GroundForce()) * r / forceScale), Maths::RGBA(ColourARGB::Blue.Packed()));
			}
		}

		// controls
		{
			float x0 = screenRight - 45;
			float y0 = yStart + 105.f;
			float separation = 10.f;
			float scale = 125.f;

			Display::BasicRender()->DrawColoured2DLine(x0, y0, x0, y0 - (1.f - Transmission().ClutchEngaged()) * scale, Maths::RGBA(ColourARGB::Blue.Packed()));
			Display::BasicRender()->DrawColoured2DLine(x0 + separation, y0, x0 + separation, y0 - Brake() * scale, Maths::RGBA(ColourARGB::Green.Packed()));
			Display::BasicRender()->DrawColoured2DLine(x0 + separation * 2.f, y0, x0 + separation * 2.f, y0 - Throttle() * scale, Maths::RGBA(ColourARGB::Red.Packed()));
			Display::BasicRender()->DrawColoured2DLine(x0 + separation * 3.f, y0, x0 + separation * 3.f, y0 - Engine().AdjustedThrottle() * scale, Maths::RGBA(ColourARGB::Yellow.Packed()));
			
			float x1 = screenRight - 120.f;
			float y1 = yStart + 120.f;
			float scale1 = 80;

			Display::BasicRender()->DrawColoured2DLine(x1 - scale1, y1 - 5, x1 - scale1, y1 + 5, Maths::RGBA(ColourARGB::Red.Packed()));	
			Display::BasicRender()->DrawColoured2DLine(x1 + scale1, y1 - 5, x1 + scale1, y1 + 5, Maths::RGBA(ColourARGB::Red.Packed()));
			Display::BasicRender()->DrawColoured2DLine(x1, y1, x1 + Steer() * scale1, y1, Maths::RGBA(ColourARGB::Green.Packed()));	
		}

		// speed
		{
			float x0 = screenRight - 105;
			float y0 = yStart + 125.f;
			float size = 10.f;

			SevenSegmentDisplay::DisplayValue(x0 + 52.f, y0, size, int(Magnitude(Body().Velocity()) / PhysicsConstants::MPH));
		}

		// av
		{
			float x0 = screenRight - 105;
			float y0 = yStart + 147.f;
			float size = 10.f;

			SevenSegmentDisplay::DisplayValue(x0 + 52.f, y0, size, int((Body().AngularVelocity() | Body().GetAxis().AxisY()) * 100.f));
		}

		if (m_networkState.networkStateValid)
		{
			float x0 = screenRight - 105;
			float y0 = yStart + 169.f;
			float size = 10.f;

			SevenSegmentDisplay::DisplayValue(x0 + 52.f, y0, size, m_dd.predictedNetworkState.framesPredicted);
		}	
	}

	if (g_showTyreSlipGraph)
	{
		Display::BasicRender()->BindColoured2D();

		float x0 = 100.f;
		float y0 = 300.f;
		float xSize = 300.f;
		float ySize = -200.f;

		// axis
		{
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
			DrawPrimitive::Instance().SetVertexDataXY(x0, y0);
			DrawPrimitive::Instance().SetVertexDataXY(x0 + xSize, y0);
			DrawPrimitive::Instance().SetVertexDataXY(x0, y0);
			DrawPrimitive::Instance().SetVertexDataXY(x0, y0 + ySize);

			for (int i = 0; i < 11; i++)
			{
				DrawPrimitive::Instance().SetVertexDataXY(x0 + xSize * (i / 10.f), y0);
				DrawPrimitive::Instance().SetVertexDataXY(x0 + xSize * (i / 10.f), y0 - 4.f);

				DrawPrimitive::Instance().SetVertexDataXY(x0, y0 + ySize * (i / 10.f));
				DrawPrimitive::Instance().SetVertexDataXY(x0 - 4.f, y0 + ySize * (i / 10.f));
			}

			DrawPrimitive::Instance().End();
		}

		// curves
		for (int j = 0; j < 10; j++)
		{
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);

			float spin = (float) j;

			for (int i = 0; i < 100; i++)
			{
				float slip = i / 10.f;

				float s = Maths::Sqrt(Maths::Squared(slip) + Maths::Squared(spin));
				float f = Wheel(0).GetTyreSaturation(s);
				float fx = f * (slip / s);

				DrawPrimitive::Instance().SetVertexDataXY(x0 + xSize * (i / 100.f), y0 + ySize * fx);
			}

			DrawPrimitive::Instance().End();
		}

		// wheels
		{
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);

			for (int i = 0; i < 4; i++)
			{
				float slip = Abs(Wheel(i).SlipAngle() / Wheel(i).LongitudinalTyreSlip());
				float spin = Abs(Wheel(i).WheelSpin() / Wheel(i).TransverseTyreSlip());

				float s = Maths::Sqrt(Maths::Squared(slip) + Maths::Squared(spin));
				float f = Wheel(i).GetTyreSaturation(s);
				float fx = f * (slip / s);
				float fz = f * (spin / s);

				float xx = x0 + slip * xSize / 10.f; 
				float yx = y0 + fx * ySize;

				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);
				DrawPrimitive::Instance().SetVertexDataXY(xx - 2, yx);
				DrawPrimitive::Instance().SetVertexDataXY(xx + 2, yx);
				DrawPrimitive::Instance().SetVertexDataXY(xx, yx - 2);
				DrawPrimitive::Instance().SetVertexDataXY(xx, yx + 2);

				float xz = x0 + spin * xSize / 10.f; 
				float yz = y0 + fz * ySize; 

				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Blue);
				DrawPrimitive::Instance().SetVertexDataXY(xz - 2, yz);
				DrawPrimitive::Instance().SetVertexDataXY(xz + 2, yz);
				DrawPrimitive::Instance().SetVertexDataXY(xz, yz - 2);
				DrawPrimitive::Instance().SetVertexDataXY(xz, yz + 2);
			}

			DrawPrimitive::Instance().End();
		}

	}

	if (m_dynamicsChangedCounter > 0)
	{
		float x0 = 1024.f - 72.f;
		float y0 = 295.f;
		float s = 10.f;

		DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);
		DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_TriangleFan);
		DrawPrimitive::Instance().SetVertexDataXY(x0, y0);
		DrawPrimitive::Instance().SetVertexDataXY(x0 + s, y0);
		DrawPrimitive::Instance().SetVertexDataXY(x0 + s, y0 + s);
		DrawPrimitive::Instance().SetVertexDataXY(x0, y0 + s);
		DrawPrimitive::Instance().End();
	}

	//if (g_debugVehicleCollisionMesh)
	//	m_collisionObject.Display(Body().GetAxis());
}

}
