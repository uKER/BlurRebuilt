#include "stdafx.h"
#include "PreviewSection.h"
#include "PreviewState.h"
#include "GlobalSignals.h"
#include <Render/Debug/DebugRender.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/common/ObjectTransform.h>
#include <animation/common/ObjectTransform.inl>
#include <animation/pose/PoseLayout.h>
#include <animation/skeleton/Skeleton.h>
#include <Render/Render/GridRender.h>
#include <Display/Render/DebugRender.h>
#include <Render/Font/Font.h>

#include <animation/common/AnimationMaths.h>

Debug::Bool gDrawJointSpheres("Skeleton.DrawJointSpheres", false);
Debug::Bool gDrawBones("Skeleton.DrawBones", true);
Debug::Bool gDrawSimpleBones("Skeleton.DrawSimpleBones", true);
Debug::Float gSelectedJointPulseRate("Skeleton.SelectedJointPulseRate", 0.8f, Debug::Range(0.f, 10.f));

F32 GetPulse(F32 time)
{
	return (sinf(gSelectedJointPulseRate * time * Maths::Pi2) * 0.5f + 0.5f);
}


F32 GetPulseSize(F32 time)
{
	return 0.9f + 0.1f * GetPulse(time);
}

Maths::RGBA GetPulseColour(const Maths::RGBA& from, const Maths::RGBA& to, F32 time)
{
	F32 p = GetPulse(time);
	S32 r = static_cast<S32>(Maths::Clamped((to.R() - from.R()) * p, 0.f, 1.f));
	S32 g = static_cast<S32>(Maths::Clamped((to.G() - from.G()) * p, 0.f, 1.f));
	S32 b = static_cast<S32>(Maths::Clamped((to.B() - from.B()) * p, 0.f, 1.f));
	S32 a = static_cast<S32>(Maths::Clamped((to.A() - from.A()) * p, 0.f, 1.f));
	return Maths::RGBA(r, g, b, a);
}


void RenderJoint(DbUtil::I_DebugRender& debugRenderer,
				 const Maths::C_Quaternion& rotation,
				 const Maths::HVec3& translation,
				 const Maths::HVec3& scale,
				 U32 alpha = 255,
				 F32 size = 0.02f)
{
	Maths::HMat43 worldMatrix(Maths::HMat33(Maths::C_MatrixScale(scale)) * Maths::HMat33(rotation), translation);

	// Draw axes.
	const Maths::RGBA kRed(255, 0, 0, alpha);
	const Maths::RGBA kGreen(0, 255, 0, alpha);
	const Maths::RGBA kBlue(0, 0, 255, alpha);

	F32 axisSize = size * 1.2f;
	F32 arrowSize = axisSize * 0.2f;
	debugRenderer.Draw3dArrow(worldMatrix.Translation(), worldMatrix.AX(), axisSize, arrowSize, kRed);
	debugRenderer.Draw3dArrow(worldMatrix.Translation(), worldMatrix.AY(), axisSize, arrowSize, kGreen);
	debugRenderer.Draw3dArrow(worldMatrix.Translation(), worldMatrix.AZ(), axisSize, arrowSize, kBlue);

	if (!gDrawJointSpheres) return;

	// Draw sphere.
	Maths::HF32 sphereRadius = size;
	const Maths::RGBA kGrey(190, 190, 190, alpha);

	static const U32 numSegments = 16;

	for (U32 i = 0; i < numSegments; ++i)
	{
		F32 alpha = Maths::Pi2 * static_cast<F32>(i) / numSegments;
		F32 nextAlpha = Maths::Pi2 * static_cast<F32>((i + 1)) / numSegments;
		F32 cosAlpha = Maths::Cos(alpha);
		F32 sinAlpha = Maths::Sin(alpha);
		F32 cosNextAlpha = Maths::Cos(nextAlpha);
		F32 sinNextAlpha = Maths::Sin(nextAlpha);

		Maths::HVec3 startA = sinAlpha * worldMatrix.AX() + cosAlpha * worldMatrix.AY();
		Maths::HVec3 endA   = sinNextAlpha * worldMatrix.AX() + cosNextAlpha * worldMatrix.AY();
		Maths::HVec3 startB = sinAlpha * worldMatrix.AZ() + cosAlpha * worldMatrix.AY();
		Maths::HVec3 endB   = sinNextAlpha * worldMatrix.AZ() + cosNextAlpha * worldMatrix.AY();

		debugRenderer.Draw3dLine(worldMatrix.Translation() + sphereRadius * startA,
								 worldMatrix.Translation() + sphereRadius * endA,
								 kGrey);
		debugRenderer.Draw3dLine(worldMatrix.Translation() + sphereRadius * startB,
								 worldMatrix.Translation() + sphereRadius * endB,
								 kGrey);
	}

	//debugRenderer.Draw3dSphere(worldMatrix.Translation(), sphereRadius, kGrey);
}

void RenderBone(DbUtil::I_DebugRender& debugRenderer,
				const Maths::C_Quaternion& rootRotation,
				const Maths::HVec3& rootTranslation,
				const Maths::HVec3& tipTranslation,
				const Maths::RGBA& colour,
				F32 size = 0.005f)
{
	if (!gDrawBones) return;

	Maths::HMat43 rootWorldMatrix(Maths::HMat33(rootRotation), rootTranslation);

	// Draw pyramid.
	static const F32 kEps = 0.000001f;

	Maths::HVec3 rootToTip = tipTranslation - rootTranslation;
	
	if ( rootToTip.LengthSquared() < kEps )
	{
		return;
	}
	
	if (gDrawSimpleBones)
	{
		debugRenderer.Draw3dLine( rootWorldMatrix.Translation(), tipTranslation, colour );
	}
	else
	{
		Maths::HVec3 basePos = rootWorldMatrix.Translation() + Maths::SafeNormalise(rootToTip) * size;
		Maths::C_Normal baseU = Maths::SafeNormalise(rootToTip ^ Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), Maths::C_Normal::AxisPositiveX());
		Maths::C_Normal baseV = Maths::SafeNormalise(rootToTip ^ baseU, Maths::C_Normal::AxisPositiveY());
		
		debugRenderer.Draw3dTriangleOutline(basePos + baseU * size - baseV * size,
											basePos + baseU * size + baseV * size,
											tipTranslation,
											colour);
		debugRenderer.Draw3dTriangleOutline(basePos - baseU * size - baseV * size,
											basePos + baseU * size - baseV * size,
											tipTranslation,
											colour);
		debugRenderer.Draw3dTriangleOutline(basePos - baseU * size + baseV * size,
											basePos - baseU * size - baseV * size,
											tipTranslation,
											colour);
		debugRenderer.Draw3dTriangleOutline(basePos + baseU * size + baseV * size,
											basePos - baseU * size + baseV * size,
											tipTranslation,
											colour);
	}
}

void DrawHighlight(
	DbUtil::I_DebugRender& debugRenderer,
	const Maths::HVec3& pos,
	const Maths::RGBA& colour
)
{
	debugRenderer.SetDepthEnable(false);
	debugRenderer.Draw3dSphereOutline( pos, 0.05f, colour );
	debugRenderer.SetDepthEnable(true);
}

Maths::Point3 GetJointNamePos( const Animation::C_ObjectTransform& worldJointTransform )
{
	return worldJointTransform.TransformPoint( Maths::MakeVF32( 0.02f ) * Maths::Const< Maths::Point3 >::UnitY() );
}


void RenderSkeletonJoint(
	DbUtil::I_DebugRender& renderer,
	U32 jointIndex,
	const Animation::C_Skeleton& skeleton,
	const Animation::C_ObjectTransform* modelTransforms,
	const Maths::Mat43& modelWorldMatrix,
	const Animation::C_ObjectTransform& modelWorldTransform,
	const Maths::RGBA& colour,
	bool showAxis,
	bool showBoneNames,
	bool showBone,
	F32 size = 0.02f
)
{
	renderer.SetDepthEnable( false );

	Maths::HVec3 jointPos = modelWorldTransform.TransformPoint( modelTransforms[jointIndex].GetPosition() );
	
	if ( showAxis )
	{
		RenderJoint(renderer,
					Maths::C_Quaternion( modelTransforms[jointIndex].GetRotation() * modelWorldTransform.GetRotation() ),
					jointPos,
					Maths::HVec3(modelTransforms[jointIndex].GetScale() * modelWorldTransform.GetScale()),
					colour.A(),
					size);
	}

	if ( showBoneNames )
	{
		renderer.Draw3dText( Maths::HVec3(GetJointNamePos( modelTransforms[jointIndex] * modelWorldTransform )),
							 skeleton.GetJointName(jointIndex).GetString() );
		renderer.SetDepthEnable( false );
	}
	
	if ( showBone )
	{
		S32 parentIndex = skeleton.GetJointParentIndex(jointIndex);
		if (parentIndex >= 0)
		{
			Maths::C_Quaternion parentRotation = Maths::C_Quaternion( modelTransforms[parentIndex].GetRotation() * modelWorldTransform.GetRotation() );
			Maths::HVec3 parentPos = modelWorldTransform.TransformPoint(modelTransforms[parentIndex].GetPosition());

			RenderBone(renderer,
					   parentRotation,
					   parentPos,
					   jointPos,
					   colour,
					   size);
		}
	}
}

void RenderSkeleton(DbUtil::I_DebugRender& renderer,
					S32 selectedJointIndex,
					const Btl::BitVector& filteredJoints,
					const Animation::C_Skeleton& skeleton,
					const Animation::C_ObjectTransform* modelTransforms,
					const Maths::Mat43& modelWorldMatrix,
					const Animation::C_ObjectTransform& modelWorldTransform,
					bool showBoneNames,
					F32 selectedSize)
{
	const F32 size = 0.02f;
	//const F32 selectedSize = 0.04f;
	
	Maths::RGBA colour( 0x77, 0x77, 0x77, 0xff );
	Maths::RGBA filteredColour( 0xcc, 0xcc, 0xcc, 0xff );
	
	for (U32 i = 0; i < skeleton.GetNumJoints(); ++i)
	{
		bool isFiltered = filteredJoints.GetBitValue(i);
		const Maths::RGBA& col = isFiltered ? filteredColour : colour;		
		
		bool showAxis = ( i != selectedJointIndex ) && isFiltered;
			
		RenderSkeletonJoint( renderer, i, skeleton, modelTransforms, modelWorldMatrix, modelWorldTransform, col, showAxis, showBoneNames && showAxis, true );
	}
}




Debug::Int	gGridSize("Preview.Grid.Size", 25, Debug::Range(0, 50));

void RenderGrid(const Maths::HVec3& origin,
				const Maths::RGBA& lineColour = Maths::RGBA(128,128,128),
				const Maths::RGBA& floorColour = Maths::RGBA(55,56,77))
{
	PERF_EVENT("RenderGrid");

	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	F32 fGridSize = static_cast<F32>(gGridSize);

	F32 ox = floorf(origin.X());
	F32 oy = 0.f; // origin.Y();
	F32 oz = floorf(origin.Z());

	Display::BasicRender()->DrawColoured3DBox(ox - fGridSize, 
											  oy,
											  oz - fGridSize,
										      ox + fGridSize,
											  oy,
											  oz + fGridSize,
											  floorColour
											   );


	S32 divisionsPerUnit = 1;
	
	F32 x = -gGridSize;
	F32 dx = 1.f / divisionsPerUnit;
	
	Maths::RGBA lightLineColour(
		lineColour.R() + ( 32 - lineColour.R() ) / 2,
		lineColour.G() + ( 32 - lineColour.G() ) / 2,		
		lineColour.B() + ( 32 - lineColour.B() ) / 2
	);
	
	// TODO: ideally draw stronger lines after lighter ones.
					
	
	
	for ( S32 i = 0, count = divisionsPerUnit * ( 1 + gGridSize * 2 ); i < count; ++i )
	{
	
		const Maths::RGBA* pColour = &lightLineColour;
			
		if ( ( i % divisionsPerUnit ) == 0 ) pColour = &lineColour;
	
		Display::BasicRender()->DrawColoured3DLine(x + ox,
												   oy,
												   oz - (F32)gGridSize,
												   x + ox,
												   oy,
												   oz + (F32)gGridSize,
												   *pColour);
												   
		x += dx;										   
	}
	
	F32 z = -gGridSize;
	F32 dz = 1.f / divisionsPerUnit;

	for ( S32 i = 0, count = divisionsPerUnit * ( 1 + gGridSize * 2 ); i < count; ++i )
	{
		const Maths::RGBA* pColour = &lightLineColour;
			
		if ( ( i % divisionsPerUnit ) == 0 ) pColour = &lineColour;

		Display::BasicRender()->DrawColoured3DLine(ox - (F32)gGridSize,
												   oy,
												   oz + z,
												   ox + (F32)gGridSize,
												   oy,
												   oz + z,
												   *pColour);
		z += dz;
	}

	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
}

const Maths::Quaternion ApplyRotationLimits(
		const Maths::Quaternion& rotation,
		const Maths::Vector3& minLimits,
		const Maths::Vector3& maxLimits
	)
{
	const Maths::VF32 zero  = Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();

#if 1
	Maths::Normal3 zAxis = Maths::Const<Maths::Normal3>::AxisZ();
	Maths::Normal3 rotatedZAxis = zAxis * rotation;

	Maths::Quaternion shortestRot = Animation::CalculateShortestRotationQuaternion(zAxis, rotatedZAxis);
	Maths::Quaternion q = Maths::Select(Maths::Real( shortestRot) < zero, -shortestRot, shortestRot);

	// r = twist * swing
	// r * !swing = twist

	Maths::Quaternion twist = rotation * !shortestRot;

#else
	const Maths::Quaternion q = Maths::Select(Maths::Real( rotation ) < zero, -rotation, rotation);
#endif
	const Maths::Vec4F32 q4 = Maths::GetVec4F32(q);

	const Maths::Vec4F32 clampedQAsVec4 = Maths::Clamp( q4, Maths::GetVec4F32( minLimits ), Maths::GetVec4F32( maxLimits ) );
	const Maths::VF32 w = Maths::Sqrt( one - Maths::Dot3( clampedQAsVec4, clampedQAsVec4 ) );
	const Maths::Quaternion result = Maths::MakeQuaternion( Maths::MakeVec4F32( Maths::MakeVector3( clampedQAsVec4 ), w ) );


	return twist * result;

}

Debug::Int gTestLimitsDivisions("Limits.Divisions", 16, Debug::Range(1, 100));
Debug::Float gTestLimitsMinX("Limits.MinX", -45.f, Debug::Range(-180.f, +180.f));
Debug::Float gTestLimitsMaxX("Limits.MaxX", 45.f, Debug::Range(-180.f, +180.f));
Debug::Float gTestLimitsMinY("Limits.MinY", -45.f, Debug::Range(-180.f, +180.f));
Debug::Float gTestLimitsMaxY("Limits.MaxY", 45.f, Debug::Range(-180.f, +180.f));
Debug::Bool gTestLimitsUseQuaternions("Limits.UseQuaternions", true);
Debug::Bool gTestLimitsRender("Limits.Render", false);


void RenderLimits(const Maths::HMat43& worldMatrix)
{

	F32 minX = gTestLimitsMinX;
	F32 maxX = gTestLimitsMaxX;
	F32 minY = gTestLimitsMinY;
	F32 maxY = gTestLimitsMaxY;

	F32 sinHalfMinX = sinf(0.5f * Maths::DegToRad(minX));
	F32 sinHalfMaxX = sinf(0.5f * Maths::DegToRad(maxX));
	F32 sinHalfMinY = sinf(0.5f * Maths::DegToRad(minY));
	F32 sinHalfMaxY = sinf(0.5f * Maths::DegToRad(maxY));

	F32 range = 150.f;

	F32 dx = (2.f * range) / (F32) gTestLimitsDivisions;
	F32 dy = (2.f * range) / (F32) gTestLimitsDivisions;

	Maths::HVec3 zAxis(0.f, 0.f, 1.f);

	//zAxis = zAxis * worldMatrix;
	
	Maths::HVec3 pos = worldMatrix.Translation();

	for (F32 x = -range; x <= range; x += dx)
	{
		F32 xrad = Maths::DegToRad(x);

		Maths::HMat33 rotXMatrix = Maths::HMat33(Maths::RotX(xrad));

		for (F32 y = -range; y <= range; y += dx)
		{
			F32 yrad = Maths::DegToRad(y);

			// Z axis, relative to parent.
			Maths::HMat33 rotYMatrix = Maths::HMat33(Maths::RotY(yrad));

			Maths::HMat33 rotMatrix = rotXMatrix * rotYMatrix;

			Maths::HVec3 rotatedAxisBefore = zAxis * rotMatrix;

			// Convert to Euler angles, apply limits and display.

			Maths::Quaternion rotQ = Maths::MakeUnitQuaternion(Maths::Mat33(rotMatrix));

			if (gTestLimitsUseQuaternions)
			{
				Maths::Quaternion newQ = ApplyRotationLimits(rotQ, Maths::MakeVector3(sinHalfMinX, sinHalfMinY, -1000.f), Maths::MakeVector3(sinHalfMaxX, sinHalfMaxY, 1000.f));
				rotMatrix = Maths::HMat33(Maths::C_Quaternion(newQ));
			}
			else
			{
#if 0
				if ((x - dx <= minX && x + dx >= minX) ||
					(x - dx <= maxX && x + dx >= maxX) ||
					(y - dx <= minY && y + dx >= minY) ||
					(y - dx <= maxY && y + dx >= maxY))
				{
				}
#else


				// Convert to Euler angles.
				Maths::EulerXYZ euler = Maths::MakeEulerXYZ(Maths::Mat33(rotMatrix));

				// Apply limits.
				F32 ex = Maths::Clamped(Maths::GetF32(Maths::X(euler)), Maths::DegToRad(minX), Maths::DegToRad(maxX));
				F32 ey = Maths::Clamped(Maths::GetF32(Maths::Y(euler)), Maths::DegToRad(minY), Maths::DegToRad(maxY));
				F32 ez = Maths::GetF32(Maths::Z(euler));

				Maths::EulerXYZ newEuler = Maths::MakeEulerXYZ(Maths::MakeVector3(ex, ey, ez));

				const Maths::Mat33 rotX( Maths::MakeMat33_RotX( X( newEuler ) ) );
				const Maths::Mat33 rotY( Maths::MakeMat33_RotY( Y( newEuler ) ) );
				const Maths::Mat33 rotZ( Maths::MakeMat33_RotZ( Z( newEuler ) ) );
			    
				rotMatrix = Maths::HMat33(rotX * rotY * rotZ);
#endif
			}

			Maths::HVec3 rotatedAxis = zAxis * rotMatrix;

			F32 dot = rotatedAxis | rotatedAxisBefore;

			// Draw only if limits were active.
			if (dot < 0.99f)
			{
				U32 r = (U32)(255.f * (x - range) / (2.f * range));
				U32 g = (U32)(255.f * (y - range) / (2.f * range));
				U32 b = (U32)(255.f);

				Maths::RGBA colour(r, g, b, 0xff);
						
				Display::BasicRender()->DrawColoured3DLine(pos, pos + ((rotatedAxis * 0.1f) * worldMatrix.Rotation()), colour);
			}
		}

	}
}


//
// C_PreviewSection
//

C_PreviewSection::C_PreviewSection():
	//Display::I_Renderer("MWViewRenderer"),
	m_gui(m_guiRender),
	m_fontRenderer(NULL)
{
	if ( Render::Font::FontSystem() )
	{
		//m_fontSystem->SetAssetPath( "./fonts" );
		m_fontRenderer = Render::Font::FontSystem()->GetFontRenderer( "dontship/fonts/ArialDebug.font" );
		const F32 kSize = 18.f;
		m_fontRenderer->SetScaleX(kSize);
		m_fontRenderer->SetScaleY(kSize);
	}
	
	C_GlobalSignals::Instance().onViewportMouseMove.connect(this, &C_PreviewSection::OnMouseMove);	
	C_GlobalSignals::Instance().onViewportMouseDown.connect(this, &C_PreviewSection::OnMouseDown);	
	C_GlobalSignals::Instance().onViewportMouseUp.connect(this, &C_PreviewSection::OnMouseUp);	
	C_GlobalSignals::Instance().onViewportResize.connect(this, &C_PreviewSection::OnResize);
}

C_PreviewSection::~C_PreviewSection()
{
}
void C_PreviewSection::OnMouseMove(S32 x, S32 y, U32 buttons)
{
	m_gui.mouse.x = F32(x);
	m_gui.mouse.y = F32(y);
	m_gui.mouse.buttons.left = (buttons & 1) != 0;
	m_gui.mouse.buttons.middle = (buttons & 2) != 0;
	m_gui.mouse.buttons.right = (buttons & 4) != 0;
}

void C_PreviewSection::OnMouseDown(S32 x, S32 y, U32 buttons)
{
	m_gui.mouse.x = F32(x);
	m_gui.mouse.y = F32(y);
	m_gui.mouse.buttons.left = (buttons & 1) != 0;
	m_gui.mouse.buttons.middle = (buttons & 2) != 0;
	m_gui.mouse.buttons.right = (buttons & 4) != 0;
	//m_gui.mouse.buttons.up = false;
	m_gui.mouse.buttons.down = true;
}

void C_PreviewSection::OnMouseUp(S32 x, S32 y, U32 buttons)
{
	m_gui.mouse.x = F32(x);
	m_gui.mouse.y = F32(y);
	m_gui.mouse.buttons.left = (buttons & 1) != 0;
	m_gui.mouse.buttons.middle = (buttons & 2) != 0;
	m_gui.mouse.buttons.right = (buttons & 4) != 0;
	m_gui.mouse.buttons.up = true;
	//m_gui.mouse.buttons.down = false;
}

void C_PreviewSection::OnKeyDown(U32 key)
{
}

void C_PreviewSection::OnKeyUp(U32 key)
{
}

void C_PreviewSection::OnResize(U32 w, U32 h)
{
	m_viewportWidth = w;
	m_viewportHeight = h;
}

Maths::Quaternion MakeRotation( const Maths::Normal3& axis, F32 sinHalfAngle )
{
	Maths::Vector3 imag = axis * Maths::MakeVF32( sinHalfAngle );
	Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	Maths::VF32 w = Maths::Sqrt( one - Maths::LengthSquared( imag ) );
	return Maths::MakeQuaternion( Maths::MakeVec4F32( imag, w ) );
}

void C_PreviewSection::OnRender()
{

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth,
							 Maths::RGBA(57, 67, 79, 225));
							 
	m_gui.SetWindowSize(F32(m_viewportWidth), F32(m_viewportHeight));
	m_gui.Begin();
	
	// create a big area that becomes active when clicked
	// and returns true if being dragged.
	// TODO: do need to capture mouse
	
	if (m_gui.Camera('CAMC', 0.f, 0.f, m_viewportWidth, m_viewportHeight, gClipPreviewState->camera))
	{
		// ?
	}

	if ( gClipPreviewState->GetNumJoints() <= 0 )
	{
		RenderGrid(Maths::HVec3());
		RenderHud( -1 );
		if ( Display::DebugRenderExists() )
		{
			Display::DebugRender()->Draw();
		}
		m_gui.End();
		return;
	}
	
	
	const U32 numJoints = gClipPreviewState->GetNumJoints();
	
	if ( numJoints <= 0 )
	{
		RenderGrid(Maths::HVec3());
		if ( Display::DebugRenderExists() )
		{
			Display::DebugRender()->Draw();
		}
		m_gui.End();
		return;
	}	
	
	m_transformBuffer.Resize(numJoints);

	// Copy current world-space joint transforms.
	for (U32 i = 0; i < numJoints; ++i)
	{
		m_transformBuffer[i] = gClipPreviewState->GetModelObjectTransform(i);
	}

	// Render grid at character origin.	
	RenderGrid( Maths::HVec3( gClipPreviewState->rootTransform.TransformPoint( Maths::MakePoint3(m_transformBuffer[ 0 ].GetTranslation()) ) ) );

	// Render world coordinate system at world origin.
	if (gClipPreviewState->showWorldAxes)
	{
		RenderJoint(*DbUtil::DefaultDebugRenderSys()->Immediate(),
					Maths::C_Quaternion(),
					//Maths::HVec3(gClipPreviewState->rootTransform.TransformPoint(m_transformBuffer[0].GetPosition())),
					Maths::HVec3(0.f, 0.f, 0.f),
					Maths::HVec3(Maths::HF32(1.f)),
					255,
					0.5f
				);
	}


	// Render mesh.
	if ( gClipPreviewState->showSkinnedMesh || gClipPreviewState->showWireframe )
	{
		m_skinnedMeshRenderer.RenderSkinnedMesh(
			gClipPreviewState->GetSkinnedMesh(),
			gClipPreviewState->rootTransform.AsMat43(),
			gClipPreviewState->camera.GetWorldMatrix(),
			gClipPreviewState->showSkinnedMesh,
			gClipPreviewState->showWireframe );
	}
		
	S32 selectedJointIndex = gClipPreviewState->GetSelectedJointIndex();
	if ( selectedJointIndex < 0 || selectedJointIndex >= S32( numJoints ) )
	{
		selectedJointIndex = -1;
	}

	//m_gui.SetFixedWidth(150.f);
	//m_gui.Label(Btl::String128::Format("%f, %f, %s", m_gui.mouse.x, m_gui.mouse.y, m_gui.mouse.buttons.left ? "L" : ".").Chars());

	// Render skeleton.
	if ( gClipPreviewState->showSkeleton )
	{
	
		if ( gClipPreviewState->hideUnselectedJoints && selectedJointIndex >= 0 )
		{
			RenderSkeletonJoint(
				*DbUtil::DefaultDebugRenderSys()->Immediate(),
				selectedJointIndex,
				gClipPreviewState->GetGlobalSkeleton(),
				&m_transformBuffer[0],
				gClipPreviewState->rootTransform.AsMat43(),
				gClipPreviewState->rootTransform,
				Maths::RGBA( 0xdd, 0xdd, 0xdd, 0xff ),
				true,
				gClipPreviewState->showJointNames,
				true,
				0.04f * GetPulseSize(gClipPreviewState->GetGlobalTime())
			);
		}
		else
		{
			RenderSkeleton(
				*DbUtil::DefaultDebugRenderSys()->Immediate(),
				selectedJointIndex,
				gClipPreviewState->filteredJoints,
				gClipPreviewState->GetGlobalSkeleton(),
				&m_transformBuffer[0],
				gClipPreviewState->rootTransform.AsMat43(),
				gClipPreviewState->rootTransform,
				gClipPreviewState->showJointNames && !gClipPreviewState->showOnlySelectedJointName,
				0.04f * GetPulseSize(gClipPreviewState->GetGlobalTime())
			);
		}
	}
	
	// Render highlight around selected joint.
	if ( selectedJointIndex >= 0 )
	{
		Animation::C_ObjectTransform selectedJointTransform = m_transformBuffer[selectedJointIndex] * gClipPreviewState->rootTransform;
		
		DbUtil::I_DebugRender& dr = *DbUtil::DefaultDebugRenderSys()->Immediate();
		
				
		// Render highlight around selected joint.
		DrawHighlight(
		   dr,
		   Maths::HVec3( selectedJointTransform.GetPosition() ),
		   Maths::RGBA( 255, 255, 0 )
		   );

		// Render selected joint axes.
		dr.SetDepthEnable(false);
		RenderSkeletonJoint(
			*DbUtil::DefaultDebugRenderSys()->Immediate(),
			selectedJointIndex,
			gClipPreviewState->GetGlobalSkeleton(),
			&m_transformBuffer[0],
			gClipPreviewState->rootTransform.AsMat43(),
			gClipPreviewState->rootTransform,
			Maths::RGBA( 0xdd, 0xdd, 0xdd, 0xff ),
			true,
			gClipPreviewState->showJointNames,
			gClipPreviewState->showSkeleton,
			0.07f * GetPulseSize(gClipPreviewState->GetGlobalTime())
		);


		if (gTestLimitsRender)
		{
			S32 parentIndex = gClipPreviewState->GetGlobalSkeleton().GetJointParentIndex(selectedJointIndex);
			Animation::C_JointTransform parentTransform;
			parentTransform.SetIdentity();
			if (parentIndex >= 0)
			{
				parentTransform = m_transformBuffer[parentIndex];
			}

			Animation::C_JointTransform transform;
			transform.SetIdentity();
			transform.SetRotation(parentTransform.GetRotation());
			transform.SetTranslation(m_transformBuffer[selectedJointIndex].GetTranslation());
			RenderLimits((transform * gClipPreviewState->rootTransform).AsMat43());
		}

		// Render selected joint axes.
		dr.SetDepthEnable(true);
	}
	
	RenderConstraints( selectedJointIndex );

	RenderHud( selectedJointIndex );
		
	if ( Display::DebugRenderExists() )
	{
		Display::DebugRender()->Draw();
	}
	
	DbUtil::DefaultDebugRenderSys()->RenderDeferred();	
	
	m_gui.End();
	
}

DECLARE_DEBUG_FLOAT(gDebugTextOffset, "Preview.TextOffset", 10.f, Debug::Range(-500.f, 10.f));

void C_PreviewSection::RenderHud( S32 selectedJointIndex )
{
	AnimationGraph::C_Graph* graph = AnimationGraph::C_Graph::s_activeGraph;
	
	m_gui.SetAutoWidth();
	
	m_gui.BeginGroup(GroupFlags::AlignLeft | GroupFlags::AlignTop | GroupFlags::LayoutVertical, 0.f, 0.f);
	
	if (m_gui.TreeNode('HUD ', "", false))
	{
		Btl::String128 str;
		const Animation::C_Pose& pose = gClipPreviewState->m_pose;		
	
		m_gui.BeginChildren();

		if ( graph )
		{
			//
			// Command List
			//
			
			str.Printf("Command List%s", gClipPreviewState->commandListOk ? "" : " (OVERFLOW)");
			
			U32 id = 'CMDL';
		
			if (m_gui.TreeNode(++id, str.Chars(), false))
			{
				m_gui.BeginChildren();
				
				gClipPreviewState->enableCommandListPushPopElimination = m_gui.CheckBox(++id, "Optimise Push/Pops", gClipPreviewState->enableCommandListPushPopElimination);
		
				// Draw command list.
				
				static const char* blendModes[] = 
				{
					"ACC",
					"LRP",
					"ADD",
					"Invalid"
				};		
								
				for ( U32 i = 0; i < graph->m_cmdList.GetNumCommands(); ++i )
				{
					const Animation::C_Command& cmd = graph->m_cmdList.GetCommand( i );
					
					switch ( cmd.type )
					{
						case Animation::E_CommandType_BlendCycle:
						{
							str.Printf( "Blend %s, %s, W:%.3f, T:%.3f (Frame:%.1f)",
								blendModes[cmd.blendCycle.blendMode],
								cmd.blendCycle.cycleInstance->GetCycleName().GetString(),
								cmd.blendCycle.weight,
								fmodf(cmd.blendCycle.time * Maths::Reciprocal(cmd.blendCycle.cycleInstance->GetDuration()), 1.f ),
								fmodf(cmd.blendCycle.time, cmd.blendCycle.cycleInstance->GetDuration()) * cmd.blendCycle.cycleInstance->GetFrameRate()
							);
							
							if (m_gui.TreeNode(++id, str.Chars(), false))
							{
								m_gui.BeginChildren();

								str.Printf("Size: %.1f Kb", cmd.blendCycle.cycleInstance->GetCycleMemoryUsage() / 1024.f);
								m_gui.Label(str.Chars());
								str.Printf("No. Frames: %.0f", cmd.blendCycle.cycleInstance->GetDuration() * cmd.blendCycle.cycleInstance->GetFrameRate());
								m_gui.Label(str.Chars());
								str.Printf("Frame Rate: %.0f fps", cmd.blendCycle.cycleInstance->GetFrameRate());
								m_gui.Label(str.Chars());
								str.Printf("Duration: %.2f secs", cmd.blendCycle.cycleInstance->GetDuration());
								m_gui.Label(str.Chars());
								
								m_gui.EndChildren();
							}
													
							break;
						}
						case Animation::E_CommandType_PopAndBlendPose:
						{
							str.Printf( "Pop (%s)", blendModes[cmd.popPose.blendMode] );
							m_gui.Label(str.Chars());
							break;
						}
						case Animation::E_CommandType_PushCurrentPose:
						{
							str.Printf( "Push (%.3f)", cmd.pushPose.weight );
							m_gui.Label(str.Chars());
							break;
						}
							
						case Animation::E_CommandType_MirrorPose:
						{
							str.Printf( "MirrorPose (%c)", "XYZ"[cmd.mirrorPose.poseMirror->GetMirrorAxis()] );
							m_gui.Label(str.Chars());
							break;
						}
						
						case Animation::E_CommandType_SetIkTargetFromJoint:
							str.Printf( "SetIkTargetFromJoint %d %s%s %s (%d)",
								cmd.setIkTargetFromJoint.targetIndex,
								cmd.setIkTargetFromJoint.setRotation ? "R" : ".",
								cmd.setIkTargetFromJoint.setPosition ? "T" : ".",
								gClipPreviewState->GetGlobalSkeleton().GetJointName(cmd.setIkTargetFromJoint.jointIndex).GetString(),
								cmd.setIkTargetFromJoint.jointIndex
							);
							m_gui.Label(str.Chars());
							break;
								
						case Animation::E_CommandType_EvaluateIkChain:
							str.Printf( "EvaluateIkChain");
							m_gui.Label(str.Chars());
							break;
					
							
						default:
							str = "? Unknown ?";
							break;
					}
				}
				m_gui.EndChildren();

			}
		} // if graph
		
	
		// Pose related information
		if (m_gui.TreeNode('POSE', "Pose", false))
		{
			m_gui.BeginChildren();

			//
			// Custom Curves
			//
		
			U32 numCurves = pose.GetNumCustomScalars() + pose.GetNumCustomVector3s() + pose.GetNumCustomVector4s();
			if (numCurves > 0)
			{
				if (m_gui.TreeNode('CURV' + 0, str.Printf("Custom Curves (%d)", numCurves).Chars(), false))
				{
					m_gui.BeginChildren();
					
					if (pose.GetNumCustomScalars() > 0)
					{
						if (m_gui.TreeNode('CURV' + 1, str.Printf("Scalar (%d)", pose.GetNumCustomScalars()).Chars(), false))
						{
							m_gui.BeginChildren();
						
							for ( U32 i = 0; i < pose.GetNumCustomScalars(); ++i )
							{
								str.Printf( "%s: %.2f", pose.GetCustomScalarName(i).GetString(), static_cast<F32>(pose.GetCustomScalar(i)) );
								m_gui.Label(str.Chars());
							}		
							
							m_gui.EndChildren();
						}
					}
					
					if (pose.GetNumCustomVector3s() > 0)
					{
						if (m_gui.TreeNode('CURV' + 2, str.Printf("Vector3 (%d)", pose.GetNumCustomVector3s()).Chars(), false))
						{
							m_gui.BeginChildren();
							for ( U32 i = 0; i < pose.GetNumCustomVector3s(); ++i )
							{
								Maths::StorageVector3 value;
								Maths::Store(static_cast<Maths::Vector3>(pose.GetCustomVector3(i)), &value);
								str.Printf( "%s: (%.2f,%.2f,%.2f)", pose.GetCustomVector3Name(i).GetString(), value.m_Data.x, value.m_Data.y, value.m_Data.z );
								m_gui.Label(str.Chars());
							}		
							m_gui.EndChildren();
						}
					}
					
					if (pose.GetNumCustomVector4s() > 0)
					{
						if (m_gui.TreeNode('CURV' + 3, str.Printf("Vector4 (%d)", pose.GetNumCustomVector4s()).Chars(), false))
						{
							m_gui.BeginChildren();
							for ( U32 i = 0; i < pose.GetNumCustomVector4s(); ++i )
							{
								Maths::StorageVector4 value;
								Maths::Store(static_cast<Maths::Vector4>(pose.GetCustomVector4(i)), &value);
								
								str.Printf( "%s: (%.2f,%.2f,%.2f,%.2f)", pose.GetCustomVector4Name(i).GetString(), value.m_Data.x, value.m_Data.y, value.m_Data.z, value.m_Data.w );
								
								// TODO: save cursor pos
								m_gui.BeginGroup(GroupFlags::LayoutHorizontal | GroupFlags::AlignTop | GroupFlags::AlignLeft);
								m_gui.ColourSwatch(16.f, 16.f, Maths::RGBA( value.m_Data.x, value.m_Data.y, value.m_Data.z, value.m_Data.w ));
								m_gui.Label(str.Chars());
								m_gui.EndGroup();
							}
							m_gui.EndChildren();
						}
					}
					
					m_gui.EndChildren();
				}
			}
			//
			// Trajectory
			//
			
			if (pose.HasTrajectoryTransform())
			{
				if (m_gui.TreeNode('TRAJ', "Trajectory", false))
				{
					m_gui.BeginChildren();
					
					U32 id = 'TRAJ';
					
					if (m_gui.TreeNode(++id, "Transform", false))
					{
						m_gui.BeginChildren();
						GuiShowJointTransform(pose.GetTrajectoryTransform());
						m_gui.EndChildren();
					}

					if (m_gui.TreeNode(++id, "DeltaTransform", false))
					{
						m_gui.BeginChildren();
						GuiShowJointTransform(pose.GetTrajectoryDeltaTransform());
						m_gui.EndChildren();
					}
					
					gClipPreviewState->freezeRoot = m_gui.CheckBox(++id, "Freeze", gClipPreviewState->freezeRoot);
					gClipPreviewState->absoluteRootMotion = m_gui.CheckBox(++id, "Absolute", gClipPreviewState->absoluteRootMotion);
					
					if (!gClipPreviewState->absoluteRootMotion)
					{
						if (m_gui.Button(++id, "Reset To Origin"))
						{
							gClipPreviewState->rootTransform.SetIdentity();
						}
					}

					m_gui.EndChildren();
				}
			}

			// Selected joint information.
			if (gClipPreviewState->GetSelectedJointIndex() >= 0)
			{
				U32 id = 'SELJ';

				if (m_gui.TreeNode(id, "Selected Joint", false))
				{
					m_gui.BeginChildren();				
					m_gui.Label(str.Printf("Name: %s", pose.GetJointTransformName(gClipPreviewState->GetSelectedJointIndex()).GetString()).Chars());
					m_gui.Label(str.Printf("Index: %d", gClipPreviewState->GetSelectedJointIndex()).Chars());

					++id;
					if (m_gui.TreeNode(id, "Local Transform", false))
					{
						m_gui.BeginChildren();
						GuiShowJointTransform(pose.GetJointTransform(gClipPreviewState->GetSelectedJointIndex()));
						m_gui.EndChildren();
					}
					++id;
					if (m_gui.TreeNode(id, "World Transform", false))
					{
						m_gui.BeginChildren();
						GuiShowJointTransform(gClipPreviewState->GetModelJointTransform(gClipPreviewState->GetSelectedJointIndex()));
						m_gui.EndChildren();
					}
					++id;
					if (m_gui.TreeNode(id, "Local Base Pose Transform", false))
					{
						m_gui.BeginChildren();
						GuiShowJointTransform(gClipPreviewState->GetGlobalSkeleton().GetLocalBasePoseJointTransform(gClipPreviewState->GetSelectedJointIndex()));
						m_gui.EndChildren();
					}
					++id;
					if (m_gui.TreeNode(id, "Inverse World Base Pose Transform", false))
					{
						m_gui.BeginChildren();
						Animation::C_JointTransform invWorldBasePose(gClipPreviewState->GetGlobalSkeleton().GetInverseWorldBasePoseMatrix(gClipPreviewState->GetSelectedJointIndex()));
						GuiShowJointTransform(invWorldBasePose);
						m_gui.EndChildren();
					}

					m_gui.EndChildren();
				}
			}

			m_gui.EndChildren();
		}

		// options
		if (m_gui.TreeNode('OPTN', "Options", false))
		{
			m_gui.BeginChildren();
			// display stuff
			if (m_gui.TreeNode('DISP', "Display", false))
			{
				U32 id = 'DISP';
				m_gui.BeginChildren();
				gClipPreviewState->showSkeleton = m_gui.CheckBox(++id, "Skeleton", gClipPreviewState->showSkeleton);
				gClipPreviewState->showJointNames = m_gui.CheckBox(++id, "Bone Names", gClipPreviewState->showJointNames);
				++id;
				if (gClipPreviewState->showJointNames)
				{
					gClipPreviewState->showOnlySelectedJointName = m_gui.CheckBox(id, "Only Show Selected Bone's Name", gClipPreviewState->showOnlySelectedJointName);
				}
				gClipPreviewState->showSkinnedMesh = m_gui.CheckBox(++id, "Mesh", gClipPreviewState->showSkinnedMesh);
				gClipPreviewState->showWireframe= m_gui.CheckBox(++id, "Wireframe", gClipPreviewState->showWireframe);
				gClipPreviewState->showWorldAxes = m_gui.CheckBox(++id, "World Axes", gClipPreviewState->showWorldAxes);
				m_gui.EndChildren();
			}

			U32 id = 'SCEN';
			if (m_gui.TreeNode(id, "Scene", false))
			{
				
				m_gui.BeginChildren();

				++id;
				C_ResourceCache::Instance().GetSceneConfig().flipX = m_gui.CheckBox(id, "Flip X", C_ResourceCache::Instance().GetSceneConfig().flipX);
				++id;
				C_ResourceCache::Instance().GetSceneConfig().flipY = m_gui.CheckBox(id, "Flip Y", C_ResourceCache::Instance().GetSceneConfig().flipY);
				++id;
				C_ResourceCache::Instance().GetSceneConfig().flipZ = m_gui.CheckBox(id, "Flip Z", C_ResourceCache::Instance().GetSceneConfig().flipZ);
				++id;
				C_ResourceCache::Instance().GetSceneConfig().removeNamespaces = m_gui.CheckBox(id, "Remove Namespaces", C_ResourceCache::Instance().GetSceneConfig().removeNamespaces);
				++id;
				bool removeNulls = m_gui.CheckBox(id, "Remove Nulls", C_ResourceCache::Instance().GetSceneConfig().excludedNodeNames.Match("_null"));
				++id;
				if (removeNulls)
				{
					C_ResourceCache::Instance().GetSceneConfig().excludedNodeNames.Add("*_null");
				}
				else
				{
					C_ResourceCache::Instance().GetSceneConfig().excludedNodeNames.Remove("*_null");
				}

				++id;
				if (m_gui.Button(id, "Reload All"))
				{
					gClipPreviewState->requestReloadAll = true;
				}

				m_gui.EndChildren();
			}
			m_gui.EndChildren();
		}
		
		// technical info
		if (m_gui.TreeNode('TCHI', "Stats", false))
		{
			U32 id = 'TCHI';
			m_gui.BeginChildren();
			
			m_gui.Label(str.Printf("Pose Data Size: %.1f Kb", pose.GetPoseLayout()->GetPoseDataSize() / 1024.f).Chars());
			m_gui.Label(str.Printf("Frame Rate: %.0f fps", gClipPreviewState->GetFrameRate()).Chars());
			
			/*
			static F32 sliderValue = 0.f;
			sliderValue = m_gui.HorizontalSlider(++id, 100.f, 0.f, 1000.f, sliderValue);
			m_gui.Label(str.Printf("Slider value: %f", sliderValue).Chars());
			*/
			
			m_gui.EndChildren();
		}
			
		m_gui.EndChildren();
	}
	
	m_gui.EndGroup();
	
	//
	// Camera button in bottom right hand corner.
	//
	{
		const CHAR8* kCameraModeNames[ E_CameraMode_Count ] =
		{
			"Orbit",
			"Orbit Selected",
			"Camera On Joint"
		};
		
		m_gui.BeginGroup(GroupFlags::LayoutHorizontal | GroupFlags::AlignRight | GroupFlags::AlignTop, m_viewportWidth - 5.f, m_viewportHeight - 25.f);
		
		m_gui.SetFixedWidth(130.f);
		if (m_gui.Button('CAMR', kCameraModeNames[ gClipPreviewState->GetCameraMode() ]))
		{
			// toggle camera mode? 
			C_GlobalSignals::Instance().cameraChanged((E_CameraMode)( (gClipPreviewState->GetCameraMode() + 1) % E_CameraMode_Count ));
		}
		
		m_gui.EndGroup();
	}	
}

Maths::Point3 LoadPoint3(const MathsStorage::Float3& v)
{
	return Maths::Load((const Maths::StoragePoint3*)&v);
}

Maths::Vector3 LoadVector3(const MathsStorage::Float3& v)
{
	return Maths::Load((const Maths::StorageVector3*)&v);
}

Maths::Normal3 LoadNormal3(const MathsStorage::Float3& v)
{
	return Maths::Load((const Maths::StorageNormal3*)&v);
}


Maths::RGBA Lerp(const Maths::RGBA& from, const Maths::RGBA& to, F32 amount)
{
	S32 k = static_cast<U32>(amount * 256.f);
	S32 r = from.R() + ((((S32)to.R() - from.R()) * k) >> 8);
	S32 g = from.G() + ((((S32)to.G() - from.G()) * k) >> 8);
	S32 b = from.B() + ((((S32)to.B() - from.B()) * k) >> 8);
	return Maths::RGBA(r, g, b, 255);
}

void C_PreviewSection::RenderConstraints( S32 selectedJointIndex )
{
	if (!gClipPreviewState->showConstraints) return;
	
	DbUtil::I_DebugRender& debugRender = *DbUtil::DefaultDebugRenderSys()->Immediate();
	const Animation::C_ObjectTransform modelWorldTransform( gClipPreviewState->rootTransform );
	
	debugRender.SetDepthEnable(false);
	
	const Maths::RGBA kWhite(0xff, 0xff, 0xff, 0xff);
	const Maths::RGBA kYellow(0xff, 0xff, 0x00, 0xff);
	const Maths::RGBA kOrange(0xe0, 0xa5, 0x00, 0xff);
	const Maths::RGBA kLightBlue(0x21, 0xc5, 0xfb, 0xff);
	const Maths::RGBA kBlue(0x00, 0x00, 0xff, 0xff);
	const Maths::RGBA kGreen(0x00, 0xff, 0x10, 0xff);
	const Maths::RGBA kRed(0xcd, 0x25, 0x06, 0xff);
	
	const F32 kArrowHeadSize = 0.002f;
		
	for (U32 i = 0, count = gClipPreviewState->selectedConstraints.Size(); i < count; ++i)
	{
		U32 index = gClipPreviewState->selectedConstraints[i];
		
		const AnimationTools::C_ExportConstraint* c = gClipPreviewState->constraints.GetConstraint(index);
		
		if (c == NULL) continue;

		Animation::C_ObjectTransform resultTransform = m_transformBuffer[c->nodeIndex] * modelWorldTransform;
		Maths::Point3 resultPos = resultTransform.GetPosition();
		
		switch (c->GetType())
		{
			case Animation::E_ConstraintType_Aim:
			{
				const AnimationTools::C_ExportAimConstraint* aim = static_cast<const AnimationTools::C_ExportAimConstraint*>(c);
				
				Animation::C_ObjectTransform targetTransform = m_transformBuffer[aim->targetNodeIndex] * modelWorldTransform;
				Maths::Point3 targetPos = targetTransform.GetPosition();
				
				Maths::Vector3 upVector = LoadVector3(aim->referenceUpVector);
				
				const Maths::RGBA kUpVectorColour = kLightBlue;
				
				switch (aim->upVectorType)
				{
					case Animation::E_UpVectorType_ModelSpace:
						// Constant model space vector.
						debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(upVector), 0.03f, kArrowHeadSize, kUpVectorColour);						
						break;
						
					case Animation::E_UpVectorType_ReferenceJointSpace:
					{
						// Up vector defined in local space of up vector reference joint.
						Animation::C_ObjectTransform refTransform = m_transformBuffer[aim->upVectorReferenceNodeIndex] * modelWorldTransform;
						upVector = refTransform.TransformVector(upVector);
						debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(upVector), 0.03f, kArrowHeadSize, kUpVectorColour);						
						debugRender.Draw3dArrow(Maths::HVec3(refTransform.GetPosition()), Maths::HVec3(upVector), 0.03f, kArrowHeadSize, kUpVectorColour);
						break;
					}
						
					case Animation::E_UpVectorType_AimAtReferenceJoint:
					{
						// Up vector calculated by aiming at the origin of the up vector reference joint.
						assert(aim->upVectorReferenceNodeIndex >= 0);
						Animation::C_ObjectTransform refTransform = m_transformBuffer[aim->upVectorReferenceNodeIndex] * modelWorldTransform;
						upVector = refTransform.GetPosition() - resultPos;
						debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(upVector), 1.f, kArrowHeadSize, kUpVectorColour);						
						break;
					}
				}
			
				debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(targetPos - resultPos), 1.f, kArrowHeadSize, kRed);
				
				//debugRender.Draw3dSphere(Maths::HVec3(resultPos), 0.01f, kGreen);
				
				break;
			}			
			case Animation::E_ConstraintType_Orientation:
			{
				// Draw arrows from input joints just to indicate where they are.
				// TODO: Draw large axes for input joints? Visualise limits? Offset?
				const AnimationTools::C_ExportOrientationConstraint* ori = static_cast<const AnimationTools::C_ExportOrientationConstraint*>(c);
				
				Animation::C_ObjectTransform input1Transform = m_transformBuffer[ori->inputNodeIndex1] * modelWorldTransform;
				Animation::C_ObjectTransform input2Transform = m_transformBuffer[ori->inputNodeIndex2] * modelWorldTransform;
				
				Maths::Point3 inputPos1 = input1Transform.GetPosition();
				Maths::Point3 inputPos2 = input2Transform.GetPosition();
								
				Maths::VF32 length1 = Maths::Length(resultPos - inputPos1);
				Maths::VF32 length2 = Maths::Length(resultPos - inputPos2);

				debugRender.Draw3dCross(Maths::HVec3(inputPos1), kArrowHeadSize, kYellow);
				debugRender.Draw3dCross(Maths::HVec3(inputPos2), kArrowHeadSize, kYellow);

				debugRender.Draw3dArrow(Maths::HVec3(inputPos1), Maths::HVec3(resultPos - inputPos1), 1.f, kArrowHeadSize, kYellow);
				debugRender.Draw3dArrow(Maths::HVec3(inputPos2), Maths::HVec3(resultPos - inputPos2), 1.f, kArrowHeadSize, kYellow);
				//debugRender.Draw3dSphere(Maths::HVec3(resultPos), 0.01f, kLightBlue);
			
				break;
			}
			
			
			case Animation::E_ConstraintType_Position:
			{
				// Draw arrows to final joint position without its local offset and then draw an arrow from this position
				// to the final joint position.
				const AnimationTools::C_ExportPositionConstraint* pos = static_cast<const AnimationTools::C_ExportPositionConstraint*>(c);
				
				Animation::C_ObjectTransform input1Transform = m_transformBuffer[pos->inputNodeIndex1] * modelWorldTransform;
				Animation::C_ObjectTransform input2Transform = m_transformBuffer[pos->inputNodeIndex2] * modelWorldTransform;
				
				Maths::Point3 targetPosition = Maths::MakePoint3(Maths::Lerp(input1Transform.GetTranslation(), input2Transform.GetTranslation(), Maths::MakeVF32(pos->blendFraction)));
												
				Maths::Point3 inputPos1 = input1Transform.TransformPoint(LoadPoint3(pos->inputNodeOffset1));
				Maths::Point3 inputPos2 = input2Transform.TransformPoint(LoadPoint3(pos->inputNodeOffset2));
				
				//Maths::Point3 resultPosNoOffset = resultTransform.GetPosition() - LoadVector3(pos->localOffset);
				
				
				S32 parentJointIndex = gClipPreviewState->GetGlobalSkeleton().GetJointParentIndex(pos->nodeIndex);
				Animation::C_ObjectTransform parentTransform = m_transformBuffer[parentJointIndex] * modelWorldTransform;
				
				Maths::Vector3 offset = parentTransform.TransformVector(LoadVector3(pos->localOffset));
				
				debugRender.Draw3dBox(Maths::HVec3(LoadPoint3(pos->minLimits)), Maths::HVec3(LoadPoint3(pos->maxLimits)), Maths::HMat43(parentTransform.AsMat43()), kLightBlue);
				
				debugRender.Draw3dCross(Maths::HVec3(inputPos1), kArrowHeadSize, kOrange);
				debugRender.Draw3dCross(Maths::HVec3(inputPos2), kArrowHeadSize, kOrange);
				
								
				debugRender.Draw3dArrow(Maths::HVec3(inputPos1), Maths::HVec3(targetPosition - inputPos1), 1.f, kArrowHeadSize, kOrange);
				debugRender.Draw3dArrow(Maths::HVec3(inputPos2), Maths::HVec3(targetPosition - inputPos2), 1.f, kArrowHeadSize, kOrange);
				debugRender.Draw3dArrow(Maths::HVec3(targetPosition), Maths::HVec3(offset), 1.f, kArrowHeadSize, kGreen);
				//debugRender.Draw3dCross(Maths::HVec3(resultPos), 0.01f, kLightBlue);
							
				break;
			}
			case Animation::E_ConstraintType_DrivenKeyRotation:
			{
				break;
			}
			case Animation::E_ConstraintType_DrivenKeyTranslation:
			{
				break;
			}
			case Animation::E_ConstraintType_Custom:
			{
				break;
			}
			case Animation::E_ConstraintType_DynamicAimJoint:
			{
				// Draw arrows to target joint.
				const AnimationTools::C_ExportDynamicAimJoint* aim = static_cast<const AnimationTools::C_ExportDynamicAimJoint*>(c);
				
				Maths::Normal3 axis = Maths::MakeNormal3(LoadVector3(aim->aimAxis));
				S32 dynamicJointIndex = gClipPreviewState->m_skeletonInstance.FindDynamicJoint(aim->nodeIndex);
				S32 targetNodeIndex = aim->targetNodeIndex;
				if (dynamicJointIndex >= 0)
				{
					targetNodeIndex = gClipPreviewState->m_skeletonInstance.GetDynamicAimJointTargetJointIndex(dynamicJointIndex);
				}
				
				if (targetNodeIndex >= 0)
				{
					Animation::C_ObjectTransform targetTransform = m_transformBuffer[targetNodeIndex] * modelWorldTransform;
					Maths::Point3 targetPos = targetTransform.GetPosition();
															
					Maths::Quaternion desiredRotation;
					
					{
						const Animation::C_JointTransform modelTargetTransform = m_transformBuffer[targetNodeIndex];
						const Maths::Point3 modelTargetPos = modelTargetTransform.TransformPoint(LoadPoint3(aim->targetNodeOffset));
						const Maths::Point3 worldTargetPos = modelWorldTransform.TransformPoint(modelTargetPos);
						
						Animation::C_JointTransform modelDynamicJointTransform = m_transformBuffer[aim->nodeIndex];
						Animation::C_JointTransform modelUpVectorJointTransform = m_transformBuffer[aim->upVectorNodeIndex];
						Maths::Point3 modelUpVectorTarget = modelUpVectorJointTransform.TransformPoint(LoadPoint3(aim->upVectorNodeOffset));
						const Maths::Vector3 modelAimVector = modelTargetPos - modelDynamicJointTransform.GetPosition();
						const Maths::Vector3 modelUpVector = modelUpVectorTarget - modelDynamicJointTransform.GetPosition();
						
						desiredRotation = Animation::CalculateAimRotation(LoadNormal3(aim->aimAxis), LoadNormal3(aim->upVector), modelAimVector, modelUpVector);
						
						debugRender.Draw3dMatrix(Maths::HMat43(Maths::HMat33(Maths::C_Quaternion(desiredRotation)), Maths::HVec3(resultPos)), 0.1f);
					}
		
					static Debug::Int gDebugNumDivisions("AnimationPreview.NumJointLimitDivisions", 8, Debug::Range(0, 128));
					const U32 numDivisions = gDebugNumDivisions;
					const F32 anglePerDivision = Maths::Pi2 / numDivisions;
					
					Maths::VF32 size = Maths::MakeVF32(0.1f);
					Maths::Normal3 axisX = Maths::Const<Maths::Normal3>::AxisX();
					Maths::Normal3 axisY = Maths::Const<Maths::Normal3>::AxisY();
					Maths::Normal3 axisZ = Maths::Const<Maths::Normal3>::AxisZ();
									
					Display::Device()->SetCullMode(Display::E_CullMode_None);
					
					F32 minPitch = aim->minPitch;
					F32 maxPitch = aim->maxPitch;
					F32 minHeading = aim->minHeading;
					F32 maxHeading = aim->maxHeading;
					
					// Use joint limits from current skeleton instance settings.
					if (dynamicJointIndex >= 0)
					{
						gClipPreviewState->m_skeletonInstance.GetDynamicAimJointLimits(dynamicJointIndex, &minPitch, &maxPitch, &minHeading, &maxHeading);
					}
										
					const Maths::Mat33 minPitchRot = MakeMat33_RotX(Maths::MakeVF32(minPitch));
					const Maths::Mat33 maxPitchRot = MakeMat33_RotX(Maths::MakeVF32(maxPitch));
					const Maths::Mat33 minHeadingRot = Maths::MakeMat33_RotY(Maths::MakeVF32(minHeading));
					const Maths::Mat33 maxHeadingRot = Maths::MakeMat33_RotY(Maths::MakeVF32(maxHeading));

					Maths::Vector3 minPitchAxis = axisZ * minPitchRot;
					Maths::Vector3 maxPitchAxis = axisZ * maxPitchRot;
					Maths::Vector3 minHeadingAxis = axisZ * minHeadingRot;
					Maths::Vector3 maxHeadingAxis = axisZ * maxHeadingRot;
					
					// Draw limit surface.
					Maths::Point3 prevMaxPitchPt;
					Maths::Point3 prevMinPitchPt;
					Maths::Point3 prevMaxHeadingPt;
					Maths::Point3 prevMinHeadingPt;
					
					debugRender.SetBlendMode(DbUtil::E_BlendMode_Alpha);
					
					for (U32 i = 0; i <= numDivisions; ++i)
					{
						// Draw fans for min and max pitch limits by iterating over heading.
						F32 k = (static_cast<F32>(i) / numDivisions);
						F32 pitchAngle = minPitch + k * (maxPitch - minPitch);
						F32 headingAngle = minHeading + k * (maxHeading - minHeading);
						
						Maths::Mat33 pitchRot = Maths::MakeMat33_RotX(Maths::MakeVF32(pitchAngle));
						Maths::Mat33 headingRot = Maths::MakeMat33_RotY(Maths::MakeVF32(headingAngle));
						
						Maths::Vector3 pitchDir = axisZ * pitchRot;
						Maths::Vector3 headingDir = axisZ * headingRot;
						
						// Top = heading dir rotated upwards.
						// Bottom = heading dir rotated downwards.

						#if 0
						Maths::Vector3 maxPitchDir   = (Maths::MakeVector3(Maths::X(headingDir), Maths::Y(maxPitchAxis), Maths::Z(headingDir)));
						Maths::Vector3 minPitchDir   = (Maths::MakeVector3(Maths::X(headingDir), Maths::Y(minPitchAxis), Maths::Z(headingDir)));
						Maths::Vector3 maxHeadingDir = (Maths::MakeVector3(Maths::X(maxHeadingAxis), Maths::Y(pitchDir), Maths::Z(pitchDir)));
						Maths::Vector3 minHeadingDir = (Maths::MakeVector3(Maths::X(minHeadingAxis), Maths::Y(pitchDir), Maths::Z(pitchDir)));
						#else
						// Heading rotated up to maximum pitch point.
						Maths::Vector3 maxPitchDir   = axisZ * maxPitchRot * headingRot;
						Maths::Vector3 minPitchDir   = axisZ * minPitchRot * headingRot;
						Maths::Vector3 maxHeadingDir = axisZ * pitchRot * maxHeadingRot;
						Maths::Vector3 minHeadingDir = axisZ * pitchRot * minHeadingRot;
						#endif
						
						Maths::Point3 maxPitchPt = resultPos + size * maxPitchDir * desiredRotation;
						Maths::Point3 minPitchPt = resultPos + size * minPitchDir * desiredRotation;
						Maths::Point3 maxHeadingPt = resultPos + size * maxHeadingDir * desiredRotation;
						Maths::Point3 minHeadingPt = resultPos + size * minHeadingDir * desiredRotation;
						
						if (i > 0)
						{
							Maths::RGBA colour = kLightBlue;
							colour.A(64);
							
							Maths::HVec3 points[4 * 3];
							
							Maths::HVec3* p = points;
							
							*p++ = Maths::HVec3(resultPos);
							*p++ = Maths::HVec3(maxPitchPt);
							*p++ = Maths::HVec3(prevMaxPitchPt);
							*p++ = Maths::HVec3(resultPos);
							*p++ = Maths::HVec3(minPitchPt);
							*p++ = Maths::HVec3(prevMinPitchPt);
							*p++ = Maths::HVec3(resultPos);
							*p++ = Maths::HVec3(maxHeadingPt);
							*p++ = Maths::HVec3(prevMaxHeadingPt);
							*p++ = Maths::HVec3(resultPos);
							*p++ = Maths::HVec3(minHeadingPt);
							*p++ = Maths::HVec3(prevMinHeadingPt);
							
							p = points;
							
							for (U32 tri = 0; tri < 4; ++tri)
							{
								debugRender.Draw3dTriangle(
									p[0],
									p[1],
									p[2],
									colour);
								p += 3;
							}

							p = points;
							for (U32 tri = 0; tri < 4; ++tri)
							{
								debugRender.Draw3dLine(
									p[1],
									p[2],
									kWhite);
								p += 3;
							}
						}
						
						if (i == 0 || i == numDivisions)
						{
							debugRender.Draw3dLine(
								Maths::HVec3(resultPos),
								Maths::HVec3(maxPitchPt),
								kWhite
							);
							debugRender.Draw3dLine(
								Maths::HVec3(resultPos),
								Maths::HVec3(minPitchPt),
								kWhite
							);

						}
						
						prevMaxPitchPt = maxPitchPt;
						prevMinPitchPt = minPitchPt;
						prevMaxHeadingPt = maxHeadingPt;
						prevMinHeadingPt = minHeadingPt;										
					}
					debugRender.SetBlendMode(DbUtil::E_BlendMode_Solid);
					Display::Device()->SetCullMode(Display::E_CullMode_Default);
				
					debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(targetPos - resultPos), 1.f, kArrowHeadSize, kOrange);
					debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(resultTransform.TransformVector(Maths::MakeVector3(0.f, 0.f, 0.1f))), 1.f, kArrowHeadSize, kBlue);
				}
				
				//S32 parentJointIndex = gClipPreviewState->GetGlobalSkeleton().GetJointParentIndex(aim->nodeIndex);
				//Animation::C_ObjectTransform parentTransform = m_transformBuffer[parentJointIndex] * modelWorldTransform;

				
				break;
			}
			
			case Animation::E_ConstraintType_DynamicSliderJoint:
			{
				const AnimationTools::C_ExportDynamicSliderJoint* slider = static_cast<const AnimationTools::C_ExportDynamicSliderJoint*>(c);
				//Animation::C_ObjectTransform targetTransform = m_transformBuffer[slider->nodeIndex] * modelWorldTransform;
								
				S32 parentJointIndex = gClipPreviewState->GetGlobalSkeleton().GetJointParentIndex(slider->nodeIndex);
				Animation::C_ObjectTransform parentTransform = m_transformBuffer[parentJointIndex] * modelWorldTransform;
				
				Maths::Normal3 axisZ = Maths::MakeNormal3(LoadVector3(slider->axis));
				Maths::Normal3 axisX, axisY;
				Maths::PerpendicularAxes(axisZ, &axisX, &axisY);
				
				axisX = parentTransform.TransformNormal(axisX);
				axisY = parentTransform.TransformNormal(axisY);
				axisZ = parentTransform.TransformNormal(axisZ);
				
				// Draw limits, specified in parent space.
				
				F32 minSlide = slider->minLimit;
				F32 maxSlide = slider->maxLimit;
				S32 dynamicJointIndex = gClipPreviewState->m_skeletonInstance.FindDynamicJoint(slider->nodeIndex);
				if (dynamicJointIndex >= 0)
				{
					gClipPreviewState->m_skeletonInstance.GetDynamicSliderJointLimits(dynamicJointIndex, &minSlide, &maxSlide);
				}
				
				Maths::Point3 minPos = parentTransform.GetPosition() + axisZ * Maths::MakeVF32(minSlide);
				Maths::Point3 maxPos = parentTransform.GetPosition() + axisZ * Maths::MakeVF32(maxSlide);
				
				Maths::VF32 size = Maths::MakeVF32(0.1f);
				
				debugRender.Draw3dLine(Maths::HVec3(minPos), Maths::HVec3(maxPos), kGreen);
				debugRender.Draw3dLine(Maths::HVec3(minPos - axisX * size), Maths::HVec3(minPos + axisX * size), kLightBlue);
				debugRender.Draw3dLine(Maths::HVec3(minPos - axisY * size), Maths::HVec3(minPos + axisY * size), kLightBlue);
				debugRender.Draw3dLine(Maths::HVec3(maxPos - axisX * size), Maths::HVec3(maxPos + axisX * size), kLightBlue);
				debugRender.Draw3dLine(Maths::HVec3(maxPos - axisY * size), Maths::HVec3(maxPos + axisY * size), kLightBlue);

				break;
			}
				
			default:
				break;
		}
		
		// Render final joint orientation.
		const F32 kAxisSize = 0.025f;
		const F32 kAxisArrowSize = kAxisSize * 0.2f;
		const Maths::RGBA kPureRed(0xff, 0x00, 0x00, 0xff);
		const Maths::RGBA kPureGreen(0x00, 0xff, 0x00, 0xff);
		const Maths::RGBA kPureBlue(0x00, 0x00, 0xff, 0xff);
		
		if ( selectedJointIndex != c->nodeIndex )
		{
			debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(Maths::Const<Maths::Vector3>::UnitX() * resultTransform.GetRotation()), kAxisSize, kAxisArrowSize, kPureRed);
			debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(Maths::Const<Maths::Vector3>::UnitY() * resultTransform.GetRotation()), kAxisSize, kAxisArrowSize, kPureGreen);
			debugRender.Draw3dArrow(Maths::HVec3(resultPos), Maths::HVec3(Maths::Const<Maths::Vector3>::UnitZ() * resultTransform.GetRotation()), kAxisSize, kAxisArrowSize, kPureBlue);
		}
	}
	
	debugRender.SetDepthEnable(true);
}

void C_PreviewSection::GuiShowJointTransform(const Animation::C_JointTransform& jointTransform)
{
	Maths::Point3 jointPosPoint;
	Maths::Quaternion jointRotQ;
	Maths::Vector3 jointScale;

	jointPosPoint = jointTransform.GetPosition();
	jointRotQ = jointTransform.GetRotation();
	jointScale = jointTransform.GetScale();

	Maths::StoragePoint3 jointPos;
	Maths::Store( jointPosPoint, &jointPos );
	Maths::StorageVec4F32 jointRot;
	Maths::Store( Maths::GetVec4F32(jointRotQ), &jointRot);
	Maths::StorageVector3 jointS;
	Maths::Store( jointScale, &jointS );

	Btl::String64 str;
	str.Printf( "Translation: (%.3f,%.3f,%.3f)",
		jointPos.m_Data.x, jointPos.m_Data.y, jointPos.m_Data.z);
	m_gui.Label(str.Chars());

	str.Printf( "Rotation: (%.3f,%.3f,%.3f,%.3f)",
		jointRot.m_Data.x, jointRot.m_Data.y, jointRot.m_Data.z, jointRot.m_Data.w);
	m_gui.Label(str.Chars());						

	str.Printf( "Scale: (%.3f,%.3f,%.3f)",
		jointS.m_Data.x, jointS.m_Data.y, jointS.m_Data.z );
	m_gui.Label(str.Chars());
}



