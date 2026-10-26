#include "stdafx.h"
#include "racingphysics/vehicles/motorbike/motorbike.h"
//#include "racingphysics/vehicles/vehiclemanager.h"
#include "racingphysics/physics/physicstimer.h"
#include "racingphysics/debug/draw.h"
#include "display/render/basicrender.h"

#ifdef DEBUG_RACING_PHYSICS
#endif

////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

EXTERN_DEBUG_BOOL( g_displayDebugVehicle );
EXTERN_DEBUG_BOOL( g_showNetworkState );
EXTERN_DEBUG_BOOL( g_showVehicleDebugText );
EXTERN_DEBUG_INT( g_debugVehicleIndex );
EXTERN_DEBUG_BOOL( g_debugShowForces );
EXTERN_DEBUG_BOOL( g_debugShowSlipRecords );
//EXTERN_DEBUG_BOOL( g_debugVehicleCollisionMesh );
EXTERN_DEBUG_BOOL( g_debugShowSlipDiagram );

void Motorbike::DisplayDebug() const
{
	if (g_debugVehicleIndex != -1 && g_debugVehicleIndex != VehicleIndex())
		return;

	//PixEvent pix(0xffffffff, "Motorbike::DisplayDebug");

	Vehicle::DisplayDebug();

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetMaxBeginVerticesUsage(100000);
	Display::BasicRender()->BindColoured3D();

	if (g_displayDebugVehicle)
	{
		// wheels
		for (int w = 0; w < m_wheels.Size(); w++)
		{
			//Viewport3D::SetWorldTransformation_Full(GmToXG(m_wheels[w].Body().GetAxis()));
			Display::BasicRender()->BindColoured3D(&m_wheels[w].Body().GetAxis().GetMatrix());

			float tubeRadius = Dynamics().geometry.TubeRadius(w);
			float rcInner = Dynamics().geometry.WheelRadius(w) - Dynamics().geometry.TubeRadius(w) * 2.f;
			float rcOuter = Dynamics().geometry.WheelRadius(w);
			float rcCentre = (rcInner + rcOuter) * 0.5f;

			if (m_wheels[w].OnGround())
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Yellow);
			else
				DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);

			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);

			int nSegments = 32;

			for (int j = 0; j <= nSegments; j++)
			{
				float b = j * 2 * Maths::Pi / nSegments;
				DrawPrimitive::Instance().SetVertexDataXYZ(0.f, rcOuter * sin(b), rcOuter * cos(b));
			}

			DrawPrimitive::Instance().End();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);

			for (int j = 0; j <= nSegments; j++)
			{
				float b = j * 2 * Maths::Pi / nSegments;
				DrawPrimitive::Instance().SetVertexDataXYZ(0.f, rcInner * sin(b), rcInner * cos(b));
			}

			DrawPrimitive::Instance().End();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);

			for (int j = 0; j <= nSegments; j++)
			{
				float b = j * 2 * Maths::Pi / nSegments;
				DrawPrimitive::Instance().SetVertexDataXYZ(tubeRadius, rcCentre * sin(b), rcCentre * cos(b));
			}

			DrawPrimitive::Instance().End();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);

			for (int j = 0; j <= nSegments; j++)
			{
				float b = j * 2 * Maths::Pi / nSegments;
				DrawPrimitive::Instance().SetVertexDataXYZ(-tubeRadius, rcCentre * sin(b), rcCentre * cos(b));
			}

			DrawPrimitive::Instance().End();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);

			for (int j = 0; j <= 4; j++)
			{
				float b = j * 2 * Maths::Pi / 4;
				DrawPrimitive::Instance().SetVertexDataXYZ(0.f, rcInner * sin(b + Maths::Pi), rcInner * cos(b + Maths::Pi));
				DrawPrimitive::Instance().SetVertexDataXYZ(0.f, rcInner * sin(b), rcInner * cos(b));
			}

			DrawPrimitive::Instance().End();
		}

		// body
		{
			//Viewport3D::SetWorldTransformation_Full(GmToXG(Body().GetAxis()));
			Display::BasicRender()->BindColoured3D(&Body().GetAxis().GetMatrix());

			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Blue);

			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.f, 0.f);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.f, -Dynamics().geometry.wheelBase * 0.5f);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.f, Dynamics().geometry.wheelBase * 0.5f);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.f, Dynamics().geometry.cofGOffsetZ);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, Dynamics().geometry.cofGOffsetY, Dynamics().geometry.cofGOffsetZ);
			
			DrawPrimitive::Instance().End();
		}

		// rider
		{
			//Viewport3D::SetWorldTransformation_Full(GmToXG(Rider().GetAxis()));
			Display::BasicRender()->BindColoured3D(&Rider().GetAxis().GetMatrix());

			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);

			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.1f, 0.f);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, -0.1f, 0.f);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.0f, 0.1f);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.0f, -0.1f);
			
			DrawPrimitive::Instance().End();
		}

		// centre of gravity
		{
			Point3R cofg_Local = Body().GetAxis().Local(CofG());

			//Viewport3D::SetWorldTransformation_Full(GmToXG(Body().GetAxis()));
			Display::BasicRender()->BindColoured3D(&Body().GetAxis().GetMatrix());

			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::White);

			DrawPrimitive::Instance().SetVertexDataXYZ(cofg_Local.X(), cofg_Local.Y() + 0.1f, cofg_Local.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(cofg_Local.X(), cofg_Local.Y() - 0.1f, cofg_Local.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(cofg_Local.X(), cofg_Local.Y(), cofg_Local.Z() + 0.1f);
			DrawPrimitive::Instance().SetVertexDataXYZ(cofg_Local.X(), cofg_Local.Y(), cofg_Local.Z() - 0.1f);
			
			DrawPrimitive::Instance().End();

		}

		// fork
		{
			//Viewport3D::SetWorldTransformation_Full(GmToXG(Fork().GetAxis()));
			Display::BasicRender()->BindColoured3D(&Fork().GetAxis().GetMatrix());

			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);

			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.f, -ForkOffset());
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, 0.f, 0.f);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.f, Dynamics().geometry.wheelBase * 0.5f, 0.f);
			DrawPrimitive::Instance().SetVertexDataXYZ(0.25f, Dynamics().geometry.wheelBase * 0.5f, 0.f);
			DrawPrimitive::Instance().SetVertexDataXYZ(-0.25f, Dynamics().geometry.wheelBase * 0.5f, 0.f);

			DrawPrimitive::Instance().End();
		}

		// springs
		{
			//Viewport3D::SetWorldTransformation_Full();
			Display::BasicRender()->BindColoured3D();

			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);

			for (int i = 0; i < m_wheels.Size(); i++)
			{
				Point3R p0 = m_wheels[i].Body().Position();
				Point3R p1 = RestingWheelPosition_Global(i);
				DrawPrimitive::Instance().SetVertexDataXYZ(p0.X(), p0.Y(), p0.Z());
				DrawPrimitive::Instance().SetVertexDataXYZ(p1.X(), p1.Y(), p1.Z());
			}

			DrawPrimitive::Instance().End();
		}
	}
	
	if (g_debugShowForces)
	{
		float forceScaleFactor = (Mass() * 5.f);

		// ground force
		{
			//Viewport3D::SetWorldTransformation_Full();
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
						Normal3R n = Normalised(m_wheels[wi].Body().GetAxis().AxisX() ^ m_wheels[wi].GroundForce());
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

		// feet
		{
			//Viewport3D::SetWorldTransformation_Full();
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		
			const Point3R& p = m_footPosition;
			Vector3R v = m_footForce.GetValue() / forceScaleFactor;
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());	

			v = m_footFriction.GetValue() / forceScaleFactor;
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Magenta);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());	

			DrawPrimitive::Instance().End();
		}

		// steering
		{
			//Viewport3D::SetWorldTransformation_Full(GmToXG(Fork().GetAxis()));
			Display::BasicRender()->BindColoured3D(&Fork().GetAxis().GetMatrix());
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);

			float d = 0.25f;
			float s = (m_steeringTorque.GetValue() | Fork().GetAxis().AxisY()) / (d * forceScaleFactor);
			Point3R p = Point3R(-d * Maths::Sign(s), Dynamics().geometry.wheelBase * 0.5f, 0.f);
			Vector3R f = Constants<Vector3R>::Z() * Maths::Abs(s); 

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - f.X(), p.Y() - f.Y(), p.Z() - f.Z());	

			DrawPrimitive::Instance().End();
		}

		// air friction
		{
			//Viewport3D::SetWorldTransformation_Full();
			Display::BasicRender()->BindColoured3D();
			DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		
			const Point3R& p = Body().CofG();
			Vector3R v = m_airFrictionForce.GetValue() / forceScaleFactor;
			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Magenta);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + v.X(), p.Y() + v.Y(), p.Z() + v.Z());	

			DrawPrimitive::Instance().End();
		}

		// angular momentum
		//{
		//	Vector3R a = Engine().Body().AngularVelocity();
		//	//a += m_wheels[0].Body().AngularMomentum() + ((m_wheels[0].Body().Position() - Body().CofG()) ^ m_wheels[0].Body().Momentum());
		//	//a += m_wheels[1].Body().AngularMomentum() + ((m_wheels[1].Body().Position() - Body().CofG()) ^ m_wheels[1].Body().Momentum());
		//	a *= 0.005f;
		//	const Point3R& p = Body().CofG();

		//	DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		//	DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);
		//	DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z(), 1);
		//	DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + a.X(), p.Y() + a.Y(), p.Z() + a.Z(), 1);	
		//	DrawPrimitive::Instance().End();
		//}
	}

	// network state
	if (g_showNetworkState && m_networkState.networkStateValid)
	{
		//Viewport3D::SetWorldTransformation_Full();
		Display::BasicRender()->BindColoured3D();
		DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
		
		{
			Point3R p = m_predictedNetworkState.axis.Position();
			Vector3R az = m_predictedNetworkState.axis.AxisZ() * 0.1f;
			Vector3R ax = m_predictedNetworkState.axis.AxisX() * 0.1f;
			Vector3R ay = m_predictedNetworkState.axis.AxisY() * 0.1f;

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + az.X(), p.Y() + az.Y(), p.Z() + az.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - az.X(), p.Y() - az.Y(), p.Z() - az.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ax.X(), p.Y() + ax.Y(), p.Z() + ax.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ax.X(), p.Y() - ax.Y(), p.Z() - ax.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ay.X(), p.Y() + ay.Y(), p.Z() + ay.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ay.X(), p.Y() - ay.Y(), p.Z() - ay.Z());
		}

		{
			Point3R p = m_networkState.axis.Position();
			Vector3R az = m_networkState.axis.AxisZ() * 0.1f;
			Vector3R ax = m_networkState.axis.AxisX() * 0.1f;
			Vector3R ay = m_networkState.axis.AxisY() * 0.1f;

			DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Yellow);
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + az.X(), p.Y() + az.Y(), p.Z() + az.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - az.X(), p.Y() - az.Y(), p.Z() - az.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ax.X(), p.Y() + ax.Y(), p.Z() + ax.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ax.X(), p.Y() - ax.Y(), p.Z() - ax.Z());
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + ay.X(), p.Y() + ay.Y(), p.Z() + ay.Z());	
			DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - ay.X(), p.Y() - ay.Y(), p.Z() - ay.Z());
		}

		DrawPrimitive::Instance().End();
	}

	if (g_debugShowSlipRecords)
		for (int i = 0; i < m_wheels.Size(); i++)
			m_wheels[i].m_debugSlipRecorder.Display();

	//if (g_debugVehicleCollisionMesh)
	//	m_collisionObject.Display(Body().GetAxis());

	if (g_debugShowSlipDiagram)
	{
		Display::BasicRender()->BindColoured3D();
		
		float yStart = 125.f;
		float screenRight = (float) Display::Device()->GetViewport().GetW();

		for (int i = 0; i < 2; i++)
		{
			const MotorbikeWheel& wheel(Wheel(i));

			float x0 = screenRight - 80.f;
			float y0 = (yStart + ((i == 1) ? 80.f : 0.f));
			float r = 30.f;
			float separation = 4.f;
			Display::BasicRender()->DrawColoured2DCircle(x0, y0, r, Maths::RGBA(ColourARGB::Red.Packed()));

			if (wheel.OnGround())
			{
				Display::BasicRender()->DrawColoured2DLine(x0, y0, x0 + wheel.SlipAngleWithCamber() * r / wheel.TransverseTyreSlip(), y0 + wheel.WheelSpin() * r / wheel.LongitudinalTyreSlip(), Maths::RGBA(ColourARGB::Green.Packed()));
				Display::BasicRender()->DrawColoured2DLine(x0 - r - separation, y0 + r, x0 - r - separation, y0 + r - (float) (Magnitude(wheel.GroundForce()) * r / (Mass() * PhysicsConstants::GRAVITY)), Maths::RGBA(ColourARGB::Blue.Packed()));
			}
		}

		// controls
		{
			float x0 = screenRight - 45;
			float y0 = yStart + 105.f;
			float separation = 10.f;
			float scale = 125.f;

			Display::BasicRender()->DrawColoured2DLine(x0, y0, x0, y0 - (1.f - Engine().ClutchEngaged()) * scale, Maths::RGBA(ColourARGB::Blue.Packed()));
			Display::BasicRender()->DrawColoured2DLine(x0 + separation, y0, x0 + separation, y0 - FullBrakes() * scale, Maths::RGBA(ColourARGB::Green.Packed()));
			Display::BasicRender()->DrawColoured2DLine(x0 + separation * 2.f, y0, x0 + separation * 2.f, y0 - Throttle() * scale, Maths::RGBA(ColourARGB::Red.Packed()));
			Display::BasicRender()->DrawColoured2DLine(x0 + separation * 3.f, y0, x0 + separation * 3.f, y0 - Engine().AdjustedThrottle() * scale, Maths::RGBA(ColourARGB::Yellow.Packed()));
		}

		// steering
		{
			float x0 = screenRight - 175.f;
			float y0 = yStart + 80.f;
			float l = 80.f;

			{
				float s = MaxSinRoll();
				float c = Maths::Sqrt(1.f - s * s);

				Display::BasicRender()->DrawColoured2DLine(x0, y0, x0 + l * s, y0 - l * c, Maths::RGBA(ColourARGB::Red.Packed()));
				Display::BasicRender()->DrawColoured2DLine(x0, y0, x0 - l * s, y0 - l * c, Maths::RGBA(ColourARGB::Red.Packed()));
			}

			{
				float s = MaxSinRoll() * Dynamics().steering.rollFraction * m_steerSmooth;
				float c = Maths::Sqrt(1.f - s * s);

				Display::BasicRender()->DrawColoured2DLine(x0, y0, x0 + l * s, y0 - l * c, Maths::RGBA(ColourARGB::Yellow.Packed()));
			}

			{
				float s = SinRoll();
				float c = Maths::Sqrt(1.f - s * s);

				Display::BasicRender()->DrawColoured2DLine(x0, y0, x0 + l * s, y0 - l * c, Maths::RGBA(ColourARGB::Green.Packed()));
			}

			{
				float a = (Fork().GetAxis().AxisZ() | Body().GetAxis().AxisX());

				Display::BasicRender()->DrawColoured2DLine(x0, y0 + 20.f, x0 + l * a, y0 + 20.f, Maths::RGBA(ColourARGB::Green.Packed()));
			}

			{
				float s = 1.f - 1.f / Wheel(1).m_frame.slipScaleFactor.GetValue();

				Display::BasicRender()->DrawColoured2DLine(x0, y0 + 30.f, x0 + l * s, y0 + 30.f, Maths::RGBA(ColourARGB::Red.Packed()));
			}
		}

		/*if (m_dynamicsChangedCounter > 0)
		{
			float x0 = UI::gSimple2D.ScreenRight() - 50.f;
			float y0 = yStart + 225.f;
			float s = 10.f;

			Draw::SetColour(ColourARGB::Red);
			Draw::Quad2D(x0, y0, s, s);
		}*/
	}

	/*if (g_showVehicleDebugText)
	{
		D3DXVECTOR4	pos;
		bool onScreen = Viewport3D::CurrentView()->TransformToScreen(&pos, GmToXG(GetAxis().Position() + Vector3R(0, 1.f, 0)));

		if (pos.Z() > 0.0f && onScreen)
		{
			float offset = XBoxApp::WideScreen() ? (Viewport3D::CurrentView()->Width() * 1.f / 8.f) : 0.f;
			float UIwidth = XBoxApp::WideScreen() ? (640.f * 4.f / 3.f) : 640.f;
			float UIheight = 480.f;
			float x = (pos.X() - offset) * UIwidth / Viewport3D::CurrentView()->Width();
			float y = pos.Y() * UIheight / Viewport3D::CurrentView()->Height();
			float gameTimer = 0.f;

			float fontScale = 1.0f;   
			const float fontHeight = 16.0f * fontScale;
			const WCHAR *pText = NULL;
			WCHAR temp_string[64];

			int framesSinceLastUpdate = PhysicsTimer::Instance().FramesSinceRaceStart() - m_predictedNetworkState.framesSinceRaceStart;

			pText = wMkStrLocal(temp_string, 64, L"%i", framesSinceLastUpdate);
			UI::gFont.DrawDebugText(x, y, UI::E_SpriteLayer_1, 0xFFFFFF00, pText, UI::E_Justify_TopCentre, 0, fontHeight);
		}
	}*/
}

/////////////////////////////////////////////////////////////////

bool g_holdBikeControls = false;

void CircleToSquare(float inX, float inY, float& outX, float& outY)
{
	//using namespace Maths;

	float scale = 1.f;

	if (Abs(inX) > Abs(inY))
	{
		float f = inY / inX;
		scale = Sqrt(f * f + 1.f);
	}
	else if (Abs(inY) > Abs(inX))
	{
		float f = inX / inY;
		scale = Sqrt(f * f + 1.f);
	}

	outX = inX * scale;
	outY = inY * scale;
}

void Motorbike::ClearAllFeedback()
{
	m_feedback.Update(false);

	//if (PadIndex() > -1)
	//	Controller::Instance().SetRumble(PadIndex(), 0, 0);
}

/*void Motorbike::ReadControls(bool readFromPads)
{
	MotorbikeControls& controls = Controls();

	controls.Validate();

	if (PadIndex() > -1)
	{
		const ControllerGamepad& gamePad = Controller::Instance().Pad(PadIndex());

		if (GetControlType() == Vehicle::CONTROL_TYPE_PLAYER 
			&& readFromPads)
		{
			controls.isWheel = gamePad.IsControllerForceWheel();

			float steerL = -gamePad.Analogue(E_PAD_STEER_LEFT)/255.f;
			float steerR = (gamePad.Analogue(E_PAD_STEER_RIGHT)/255.f);
			float steer = steerL + steerR;
			m_previousSteering = steer;

			//extern Debug::Float g_predictionFrames;

			//if (!controls.isWheel)
			//	controls.steer = Maths::Clamped((steer - m_previousSteering) * g_predictionFrames + steer, -1.f, 1.f);
			//else
				controls.steer = steer;

			controls.throttle = gamePad.Analogue(E_PAD_ACCEL) / 255.f;
			controls.fullBrakes = gamePad.Analogue(E_PAD_BRAKE) / 255.f;
			controls.rearBrake = gamePad.Digital(E_PAD_HANDBRAKE);
			//controls.frontBrake = 0;//gamePad.Digital(E_PAD_LOOK_BACK);

			float leanF = gamePad.Analogue(E_PAD_LEAN_FORWARD)/255.f;
			float leanB = -(gamePad.Analogue(E_PAD_LEAN_BACK)/255.f);
			controls.lean = leanF + leanB;

			// if lean and steer are on the same thumbstick, apply correction
			{
				bool steerLThumb = gamePad.KeyMap(E_PAD_STEER_LEFT, 0) >= BUTTON_DIR_ANALOGUE_THUMB_L
									&& gamePad.KeyMap(E_PAD_STEER_LEFT, 0) < BUTTON_DIR_ANALOGUE_THUMB_L + 4;

				bool steerRThumb = gamePad.KeyMap(E_PAD_STEER_LEFT, 0) >= BUTTON_DIR_ANALOGUE_THUMB_R
									&& gamePad.KeyMap(E_PAD_STEER_LEFT, 0) < BUTTON_DIR_ANALOGUE_THUMB_R + 4;

				bool leanLThumb = gamePad.KeyMap(E_PAD_LEAN_BACK, 0) >= BUTTON_DIR_ANALOGUE_THUMB_L
									&& gamePad.KeyMap(E_PAD_STEER_LEFT, 0) < BUTTON_DIR_ANALOGUE_THUMB_L + 4;

				bool leanRThumb = gamePad.KeyMap(E_PAD_LEAN_BACK, 0) >= BUTTON_DIR_ANALOGUE_THUMB_R
									&& gamePad.KeyMap(E_PAD_STEER_LEFT, 0) < BUTTON_DIR_ANALOGUE_THUMB_R + 4;

				if ((steerLThumb && leanLThumb) || (steerRThumb && leanRThumb))
				{
					if (controls.lean < 0.f)
						CircleToSquare(controls.steer, controls.lean, controls.steer, controls.lean);
				}
			}

			controls.gearUp = gamePad.Pressed(E_PAD_GEAR_UP);
			controls.gearDown = gamePad.Pressed(E_PAD_GEAR_DOWN);

			controls.reverse = 0;//gamePad.Digital(E_PAD_REVERSE);

#ifndef CFG_GOLD
			if (gamePad.Digital(E_PAD_GEAR_UP) && gamePad.RawDigital(BUTTON_BACK))	// BUTTON_Y is E_PAD_GEAR_UP
				Reset();
#endif

			if (!controls.active)
				controls.Clear();

			if (g_holdBikeControls)
			{
				controls.steer = -1.f;
				controls.throttle = 1.f;
			}

			controls.horn = gamePad.Digital(E_PAD_HORN);

			RP_ASSERT(controls.Validate());
		}

#ifndef CFG_GOLD
		if (gamePad.RawDigital(BUTTON_BACK) && gamePad.RawPressed(BUTTON_A))
			ToggleAIControl();
#endif

		if (controls.active && GetControlType() == Vehicle::CONTROL_TYPE_PLAYER)
			m_feedback.Update(m_RumbleEnabled);
		else
			m_feedback.Update(false);
		
		Controller::Instance().SetRumble(PadIndex(), (U16) (m_feedback.RumbleL() * 65535), (U16) (m_feedback.RumbleR() * 65535));

		if (GetControlType() == Vehicle::CONTROL_TYPE_PLAYER &&
			gamePad.IsControllerForceWheel())
		{
			float s_steeringTorqueFeedbackScale = 0.01f;

			CarFFParameters ffParameters;

			ffParameters.friction = 0.f;
			ffParameters.gripScale = Maths::Clamped((float)Magnitude(m_steeringTorque.GetValue()) * s_steeringTorqueFeedbackScale, 0.f, 1.f);
			ffParameters.tyreSlip = 1.f;
			ffParameters.slideAngle = 0.f;

			float rough = 0.f;
			float onGround = 0.f;
			float rumble = 0.f;
			float chatter = 0.f;

			extern Debug::Float g_wheelFeedbackSpeed;
			extern Debug::Float g_wheelTyreChatter;
			extern Debug::Float g_wheelTyreChatterPeriod;
			extern Debug::Float g_rumbleStripOscillationScale;
			extern Debug::Float g_roughGroundOscillationScale;
			extern Debug::Float g_rumbleStripOscillationPeriod;

			for (int i= 0; i < 2; i++)
			{
				if (Wheel(i).OnGround())
				{
					CollisionMeshSurfaceFlags::SurfaceType surfaceType = Wheel(i).SurfaceType();

					if (surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRASS
						|| surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRAVEL
						|| surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_STICKY)
					{
						rough += 0.25f;
						rumble += 0.25f * g_roughGroundOscillationScale;
					}

					if (surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_RUMBLE)
						rumble += 0.25f * g_rumbleStripOscillationScale;

					onGround += 0.5f;

					//rumble += abs(Wheel(i).SpringSpeed()) * g_springOscillationScale * 0.25f;
					//rough += abs(Wheel(i).SpringSpeed()) * g_springOscillationScale * 0.25f;

					if (i == 1)
					{
						float f = fabs(Wheel(i).SlipAngle() / Wheel(i).TransverseTyreSlip());

						if (f < 1.f)
							chatter += Maths::Max(f - 0.75f, 0.f) * 2.f * g_wheelTyreChatter;
						else
							chatter += Maths::Max(2.f - f, 0.f) * 0.5f * g_wheelTyreChatter;
					}
				}
			}

			rough = Maths::Clamp(rough, 0.f, 1.f);

			float speedScale = Maths::Abs(Speed_LocalZ()) / g_wheelFeedbackSpeed;
			speedScale = Maths::Min(speedScale, 1.f);

			ffParameters.oscillation = speedScale * (rumble + chatter) * onGround + m_feedback.Collision() * Dynamics().rumble.collisionScale;
			ffParameters.period = (Random(1.f) + 0.25f) * rough + 
									(g_rumbleStripOscillationPeriod * rumble + g_wheelTyreChatterPeriod * chatter + m_feedback.Collision()) / (rumble + chatter + 0.01f) * (1.f - rough) 
									/ (Maths::Abs(Speed_LocalZ() * 0.1f + 1.f));

			Controller::Instance().SetForceFeedbackParameters(ControllerPort(), ffParameters);
		}
		else
			Controller::Instance().SetForceFeedbackParameters(ControllerPort(), CarFFParameters());
	}

	m_checkControlsPressed = true;
}*/

///////////////////////////////////////////////////////////////////

void MotorbikeFeedback::AddRumble(float value, float balance)
{
	value = Maths::Clamped(value, 0.f, 1.f);
	m_rumbleL = Maths::AddSmooth(m_rumbleL, value * (1.f - balance));
	m_rumbleR = Maths::AddSmooth(m_rumbleR, value * balance);
}

void MotorbikeFeedback::AddRumble(float value, float cutOff, float scale, float balance)
{
	float v = Maths::Clamped(Maths::Max(value - cutOff, 0.f) * scale, 0.f, 1.f);
	m_rumbleL = Maths::AddSmooth(m_rumbleL, v * (1.f - balance));
	m_rumbleR = Maths::AddSmooth(m_rumbleR, v * balance);
}

void MotorbikeFeedback::Update(bool rumbleEnabled)
{
	const MotorbikeDynamicsXt::MotorbikeRumbleXt& settings = Dynamics().rumble;

	if (rumbleEnabled)
	{
		m_rumbleL = 0.f;
		m_rumbleR = 0.f;

		// suspension
		if (settings.singleEffect == 0 || settings.singleEffect == -1)
		{
			for (int i = 0; i < 2; i++)
			{
				float s = Maths::Abs(m_pMotorbike->SuspensionSpeed(i));
				AddRumble(s, settings.suspensionCutOff, settings.suspensionScale, settings.suspensionLRBalance);
			}
		}

		// tyre slip
		if (settings.singleEffect == 1 || settings.singleEffect == -1)
		{
			if (m_pMotorbike->Wheel(1).OnGround())
			{
				//Surfaces::SurfaceModifierType surfaceModifier = m_pMotorbike->Wheel(1).SurfaceModifier();

				//if (surfaceModifier != Surfaces::SURFACE_MODIFIER_ICE)
				{
					float s = Maths::Abs(m_pMotorbike->Wheel(1).WheelSpin()) / m_pMotorbike->Wheel(1).LongitudinalTyreSlip();
					AddRumble(s, settings.wheelSpinCutOff, settings.wheelSpinScale, settings.wheelSpinLRBalance);
				}
			}
		}

		// engine
		if (settings.singleEffect == 2 || settings.singleEffect == -1)
		{
			float f = m_pMotorbike->Engine().RPM() / Dynamics().engine.limiterRPM;
			f *= 1.f - m_pMotorbike->Engine().ClutchEngaged() * settings.engineClutchScale;
			AddRumble(f, settings.engineCutOff, settings.engineScale, settings.engineLRBalance);
		}

		// surfaces
		if (settings.singleEffect == 3 || settings.singleEffect == -1)
		{
			float s = Magnitude(m_pMotorbike->Body().Velocity());

			for (int i = 0; i < 2; i++)
			{
				if (m_pMotorbike->Wheel(i).OnGround())
				{
					/*CollisionMeshSurfaceFlags::SurfaceType surfaceType = m_pMotorbike->Wheel(i).SurfaceType();

					if (surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRASS ||
						surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRAVEL ||
						surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_STICKY)
					{
						AddRumble(s, settings.surfaceSpeedCutOff, settings.surfaceSpeedScale, settings.surfaceSpeedLRBalance);
					}
					else if (	surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_RUMBLE ||
								surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_RUMBLE_LINES)
					{
						float f = (float) sin(PhysicsTimer::Instance().PhysicsGetTimer() * s / settings.rumbleStripPeriod);
						f *= s * settings.surfaceSpeedScale;
						AddRumble(f, settings.rumbleStripCutOff, settings.rumbleStripScale, settings.rumbleStripLRBalance);
					}*/

					/*Surfaces::SurfaceModifierType surfaceModifier = m_pMotorbike->Wheel(i).SurfaceModifier();

					if (surfaceModifier == Surfaces::SURFACE_MODIFIER_SHALLOW_PUDDLE)
						AddRumble(s * settings.surfaceSpeedScale, 0.f, settings.shallowPuddleScale, settings.shallowPuddleBalance);

					if (surfaceModifier == Surfaces::SURFACE_MODIFIER_DEEP_PUDDLE)
						AddRumble(s * settings.surfaceSpeedScale, 0.f, settings.deepPuddleScale, settings.deepPuddleBalance);*/
				}
			}
		}

		// collisions
		if (settings.singleEffect == 4 || settings.singleEffect == -1)
		{
			m_collisionRumble *= settings.collisionDecay;
			AddRumble(m_collisionRumble * settings.collisionScale, settings.collisionLRBalance);
		}

		// gear change
		if (settings.singleEffect == 5 || settings.singleEffect == -1)
		{
			m_gearRumble *= settings.gearDecay;
			AddRumble(m_gearRumble * settings.gearScale, settings.gearLRBalance);
		}

		// test
		if (settings.singleEffect == 6 || settings.singleEffect == -1)
		{
			m_rumbleL += settings.testLeft;
			m_rumbleR += settings.testRight;
		}
	}
	else
	{
		Clear();
	}
}

void MotorbikeFeedback::HandleCollision(float impulse)
{
	m_collisionRumble = Maths::Max(m_collisionRumble, impulse);
}

void MotorbikeFeedback::HandleGearChange(float impulse)
{
	m_gearRumble = impulse;
}

}


