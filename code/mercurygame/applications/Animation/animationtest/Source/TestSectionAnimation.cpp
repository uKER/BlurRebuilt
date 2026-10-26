#include "stdafx.h"
#include "TestSectionAnimation.h"

#include <Render/Debug/DebugRender.h>
#include <animation/common/JointTransform.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/PoseFields.h>
#include <Render/Render/GridRender.h>
#include <Display/Render/DebugRender.h>
#include <core/containers/String.h>
#include <camera2.h>
#include <framework/view/View.h>
#include <framework/application/UserControls.h>
#include <framework/application/ApplicationBase.h>
#include <camera2/stock/SimpleChaseCamera.h>
#include "SkeletonRenderer.h"

Debug::Logger& gLog = Debug::GetLogger( "mercurygame.animation" );

Debug::Bool gDrawJointSpheres("Skeleton.DrawJointSpheres", false);
Debug::Bool gDrawBones("Skeleton.DrawBones", true);

void Draw3dArrow( const Maths::HVec3& pos, const Maths::HVec3& dir, F32 axisSize, F32 arrowSize, const Maths::RGBA& colour )
{
	Maths::HVec3 normDir = dir.SafeNormalised().Normal();
	Maths::HVec3 tip = pos + normDir * axisSize;
	Display::BasicRender()->DrawColoured3DLine( pos, tip, colour );
}

void RenderJoint(DbUtil::I_DebugRender& debugRenderer,
				 const Maths::C_Quaternion& rotation,
				 const Maths::HVec3& translation,
				 U32 alpha = 255,
				 F32 size = 0.02f)
{
	Maths::HMat43 worldMatrix(Maths::HMat33(rotation), translation);

	// Draw axes.
	const Maths::RGBA kRed(255, 0, 0, alpha);
	const Maths::RGBA kGreen(0, 255, 0, alpha);
	const Maths::RGBA kBlue(0, 0, 255, alpha);

	F32 axisSize = size * 1.2f;
	F32 arrowSize = axisSize * 0.2f;
	Draw3dArrow(worldMatrix.Translation(), worldMatrix.AX(), axisSize, arrowSize, kRed);
	Draw3dArrow(worldMatrix.Translation(), worldMatrix.AY(), axisSize, arrowSize, kGreen);
	Draw3dArrow(worldMatrix.Translation(), worldMatrix.AZ(), axisSize, arrowSize, kBlue);

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

		Display::BasicRender()->DrawColoured3DLine(worldMatrix.Translation() + sphereRadius * startA,
								 worldMatrix.Translation() + sphereRadius * endA,
								 kGrey);
		Display::BasicRender()->DrawColoured3DLine(worldMatrix.Translation() + sphereRadius * startB,
								 worldMatrix.Translation() + sphereRadius * endB,
								 kGrey);
	}

	//debugRenderer.Draw3dSphere(worldMatrix.Translation(), sphereRadius, kGrey);
}

void RenderBone(DbUtil::I_DebugRender& debugRenderer,
				const Maths::C_Quaternion& rootRotation,
				const Maths::HVec3& rootTranslation,
				const Maths::HVec3& tipTranslation,
				U32 alpha = 255,
				F32 size = 0.005f)
{
	if (!gDrawBones) return;

	Maths::HMat43 rootWorldMatrix(Maths::HMat33(rootRotation), rootTranslation);

	// Draw pyramid.
	static const F32 kEps = 0.001f;

	S32 g = (190 * alpha) / 255;

	Maths::RGBA kGrey(g, g, g, alpha);

	Maths::HVec3 rootToTip = tipTranslation - rootTranslation;
	
	if ( rootToTip.LengthSquared() < kEps )
	{
		return;
	}

	Maths::C_Normal baseU = (rootToTip ^ Maths::Constants<Maths::HVec3>::UnitAxisPositiveY()).SafeNormalised(Maths::C_Normal::AxisPositiveX()).Normal();
	Maths::C_Normal baseV = (rootToTip ^ baseU).SafeNormalised(Maths::C_Normal::AxisPositiveY()).Normal();

	Maths::HVec3 basePos = rootWorldMatrix.Translation() + rootToTip.SafeNormalised().Normal() * size;
	
#if 1
	Maths::HVec3 base0 = basePos + baseU * size - baseV * size;
	Maths::HVec3 base1 = basePos + baseU * size + baseV * size;
	Maths::HVec3 base2 = basePos - baseU * size + baseV * size;
	Maths::HVec3 base3 = basePos - baseU * size - baseV * size;

	Display::BasicRender()->DrawColoured3DLine(
		base0,
		tipTranslation,
		kGrey
	);

	Display::BasicRender()->DrawColoured3DLine(
		base1,
		tipTranslation,
		kGrey
	);

	Display::BasicRender()->DrawColoured3DLine(
		base2,
		tipTranslation,
		kGrey
	);

	Display::BasicRender()->DrawColoured3DLine(
		base3,
		tipTranslation,
		kGrey
	);
		
#else
	debugRenderer.Draw3dTriangleOutline(basePos + baseU * size - baseV * size,
										basePos + baseU * size + baseV * size,
										tipTranslation,
										kGrey);
	debugRenderer.Draw3dTriangleOutline(basePos - baseU * size - baseV * size,
										basePos + baseU * size - baseV * size,
										tipTranslation,
										kGrey);
	debugRenderer.Draw3dTriangleOutline(basePos - baseU * size + baseV * size,
										basePos - baseU * size - baseV * size,
										tipTranslation,
										kGrey);
	debugRenderer.Draw3dTriangleOutline(basePos + baseU * size + baseV * size,
										basePos - baseU * size + baseV * size,
										tipTranslation,
										kGrey);
#endif
}

void RenderSimpleBone(
	const Maths::C_Quaternion& rootRotation,
	const Maths::HVec3& rootTranslation,
	const Maths::HVec3& tipTranslation,
	U32 alpha = 255,
	F32 size = 0.005f
)
{
	if (!gDrawBones) return;

	// Draw triangle.
	static const F32 kEps = 0.001f;

	S32 g = (190 * alpha) / 255;

	Maths::RGBA kGrey(g, g, g, alpha);

	Maths::HVec3 rootToTip = tipTranslation - rootTranslation;
	
	if ( rootToTip.LengthSquared() < kEps )
	{
		return;
	}

	//Maths::C_Normal baseU = (rootToTip ^ Maths::Constants<Maths::HVec3>::UnitAxisPositiveY()).SafeNormalised(Maths::C_Normal::AxisPositiveX()).Normal();
	Maths::C_Normal baseU = Maths::C_Normal::AxisPositiveX() * rootRotation;

	Maths::HVec3 basePos = rootTranslation + rootToTip.SafeNormalised().Normal() * size;
	
	Maths::HVec3 base0 = basePos + baseU * size;
	Maths::HVec3 base1 = basePos - baseU * size;

	Display::BasicRender()->DrawColoured3DLine(
		base0,
		tipTranslation,
		kGrey
	);

	Display::BasicRender()->DrawColoured3DLine(
		base1,
		tipTranslation,
		kGrey
	);

	Display::BasicRender()->DrawColoured3DLine(
		base0,
		base1,
		kGrey
	);
		
}

void RenderVerySimpleBone(
	const Maths::HVec3& rootTranslation,
	const Maths::HVec3& tipTranslation,
	const Maths::RGBA colour
)
{
	if (!gDrawBones) return;
	Display::BasicRender()->DrawColoured3DLine(
		rootTranslation,
		tipTranslation,
		colour
	);
}

void DrawHighlight(
	DbUtil::I_DebugRender& debugRenderer,
	const Maths::HVec3& pos,
	const Maths::RGBA& colour
)
{
	debugRenderer.Draw3dSphereOutline( pos, 0.05f, colour );
}

void RenderSkeletonJoint(
	DbUtil::I_DebugRender& renderer,
	U32 jointIndex,
	const Animation::C_Skeleton& skeleton,
	const Animation::C_JointTransform* worldTransforms,
	const Maths::HMat43& modelWorldMatrix,
	const Maths::RGBA& colour,
	bool showBoneNames,
	F32 size = 0.02f
)
{
	Maths::HVec3 jointTranslation = Maths::HVec3( worldTransforms[jointIndex].GetTranslation() ) * modelWorldMatrix;

#if 0
	RenderJoint(renderer,
				Maths::C_Quaternion( worldTransforms[jointIndex].GetRotation() ),
				jointTranslation ,
				255,
				size);
#endif

	if ( showBoneNames )
	{
		Display::FontRender()->Draw3D(
			skeleton.GetJointName( jointIndex ).GetString(),
			Maths::HVec3( worldTransforms[ jointIndex ].TransformPoint( 0.02f * Maths::Const< Maths::Point3 >::UnitY() ) ),
			0.1f,
			Maths::RGBA( 0xff, 0xff, 0xff )
		);
		//renderer.SetDepthEnable( true );
	}

	S32 parentIndex = skeleton.GetJointParentIndex(jointIndex);
	if (parentIndex >= 0)
	{
		
		Maths::HVec3 parentTranslation = Maths::HVec3( worldTransforms[parentIndex].GetTranslation() ) * modelWorldMatrix;

		//Maths::C_Quaternion parentRotation = Maths::C_Quaternion( worldTransforms[parentIndex].GetRotation() );
		//RenderSimpleBone(
		//	parentRotation,
		//	parentTranslation,
		//	jointTranslation,
		//	alpha,
		//	size
		//);
		RenderVerySimpleBone( parentTranslation, jointTranslation, colour );
		
	}
}

Debug::Int	gGridSize("Preview.Grid.Size", 25, Debug::Range(0, 50));

void RenderGrid(const Maths::HVec3& origin,
				const Display::C_Texture2D* texture = 0,
				const Maths::RGBA& lineColour = Maths::RGBA(255,255,255), // Maths::RGBA(128,128,128),
				const Maths::RGBA& floorColour = Maths::RGBA(55,56,77)
				)
{
	PERF_EVENT("RenderGrid");

	Display::Device()->SetDepthWriteEnable( Display::E_Bool_True );

	F32 fGridSize = static_cast<F32>(gGridSize);

	F32 ox = floorf(origin.X());
	F32 oy = 0.f; // origin.Y();
	F32 oz = floorf(origin.Z());
	
	
#if 0

	F32 verts[4][3] =
	{
		{ ox - fGridSize, oy, oz - fGridSize },
		{ ox - fGridSize, oy, oz + fGridSize },
		{ ox + fGridSize, oy, oz + fGridSize },
		{ ox + fGridSize, oy, oz - fGridSize }
	};
	
	F32 uvs[6][2] =
	{
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		
		{ 0.f, 0.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f }
	};
	
	for ( U32 i = 0; i < 6; ++i )
	{
		uvs[ i ][ 0 ] *= fGridSize;
		uvs[ i ][ 1 ] *= fGridSize;
	}
	
	Display::Device()->SetTextureLodBias( 0, -20 );

	Display::BasicRender()->DrawTextured3DTriangle(
		Maths::HVec3( verts[0] ),
		Maths::HVec3( verts[1] ),
		Maths::HVec3( verts[2] ),
		(Display::C_Float2*) &uvs[0],
		lineColour,
		texture
	);
		
	Display::BasicRender()->DrawTextured3DTriangle(
		Maths::HVec3( verts[0] ),
		Maths::HVec3( verts[2] ),
		Maths::HVec3( verts[3] ),
		(Display::C_Float2*) &uvs[3],
		lineColour,
		texture
	);



#else

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
					
	Display::Device()->SetDepthWriteEnable( Display::E_Bool_False );
	Display::Device()->SetDepthEnable( Display::E_Bool_False );
	
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
	Display::Device()->SetDepthEnable( Display::E_Bool_True );

#endif

}




using namespace Framework;

C_TestSectionAnimation::C_TestSectionAnimation(const Util::C_String& name) :
	C_TestSection(name),
	m_renderTaskPending(false)
{
	if ( !Display::CreateTexture2DFromFile( "Textures/grid.dds", &m_gridTexture ) )
	{
		//DebugBreak();
	}
	//Display::CreateTexture2DFromFile( "Textures/bone.dds", &m_boneTexture );

	//m_cycleSkeletonMappings[ 0 ].Create( m_cycles[ 0 ], m_skeleton );
	
	m_actorList.Clear();
	
#ifdef PLATFORM_PS3
	#define NUM_ACTORS 512
#else
	#define NUM_ACTORS 256
#endif
	
	for ( U32 i = 0; i < NUM_ACTORS; ++i )
	{
		m_actorList.AddActor();
	}
	
	if ( m_actorList.GetNumActors() > 0 )
	{
		m_actorList.GetActor( 0 ).m_isPlayerControlled = true;
	}
		
	Framework::C_View& view = GetSplitscreenView( 0 );
	m_cameraTypeIndex = 0;
	view.DebugFeed().SetDebugType( Cam2::E_FlyCamera );
	
	if ( Scheduler::Active() )
	{
		m_skeletonRenderGroup = Scheduler::CreateGroup();
		
		m_actorList.AddUpdateDependencies( m_skeletonRenderGroup );
	}
}

C_TestSectionAnimation::~C_TestSectionAnimation()
{
	m_dynamicVertexBuffer.Destroy();
}

static bool reloadResourcesFlag = false;

void SetReloadResourcesFlag( void* )
{
	reloadResourcesFlag = true;	
}

Debug::Function gReloadResources( "Animation.ReloadResources", SetReloadResourcesFlag, 0 );
Debug::Float gTimeScale( "Animation.TimeScale", 1.f, Debug::Range( 0.f, 10.f ) );
Debug::Bool gPause( "Animation.Pause", false );
Debug::Bool gShowBoneNames( "Animation.ShowBoneNames", false );
Debug::Float gRunDamping( "Animation.RunDamping", 0.1f, Debug::Range( 0.f, 1.f ) );
Debug::Float gControlDamping( "Animation.ControlDamping", 0.1f, Debug::Range( 0.f, 1.f ) );

bool C_TestSectionAnimation::OnCreate()
{
	m_dynamicVertexBuffer.Create(
		2 * sizeof( Display::C_BasicRenderColouredVertex3D )
		* m_actorList.GetNumActors() * ( m_actorList.GetNumJointsPerActor() - 1 )
	);
	
	Display::C_VertexFormat vf;
	vf.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	vf.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
	m_pSkeletonVertexDecl = Display::Device()->CreateVertexDeclaration( vf );
	
	// Force update before render.
	m_actorList.StartUpdate( 0.f );
	m_actorList.FinishUpdate();
	
	return true;
}

void C_TestSectionAnimation::OnDestroy()
{
	m_dynamicVertexBuffer.Destroy();
}

void C_TestSectionAnimation::OnUpdate()
{
	F32 dt = Application().GetTimer().GetFrameTime();
	
	dt *= gTimeScale;
	
	if ( gPause )
	{
		dt = 0.f;
	}

	if ( m_actorList.GetNumActors() > 0 )
	{
		m_actorList.GetFocusActor().m_targetDirX = Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_FlyMoveX );
		m_actorList.GetFocusActor().m_targetDirZ = Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_FlyMoveZ );
		m_actorList.GetFocusActor().m_runDamping = gRunDamping;
		m_actorList.GetFocusActor().m_dirDamping = gControlDamping;
	}
	
	m_actorList.StartUpdate( dt );
	

	Framework::C_View& view = GetSplitscreenView(0);
	view.DebugFeed().SetInput( GetFlyCameraControls(0) );
	
	if ( Input::GetGameInput(0)->GetButtonAutoRepeat(E_Button_ChangeCamera, 10, 5) )
	{
		static const Cam2::E_CameraType kCameraTypes[] =
		{
			Cam2::E_FlyCamera,
			//Cam2::E_ChaseCamera,
			//Cam2::E_CharacterCamera,
			//Cam2::E_RelativeCamera,
			Cam2::E_RelativeFlyCamera,
			Cam2::E_SimpleChaseCamera
		};
		
		static const U32 kNumCameraTypes = sizeof(kCameraTypes) / sizeof(kCameraTypes[0]);
		
		m_cameraTypeIndex = ( m_cameraTypeIndex + 1 ) % kNumCameraTypes;
		
		view.DebugFeed().SetDebugType( kCameraTypes[ m_cameraTypeIndex ] );
	}

	//const Cam2::C_Camera& camera = view.DebugFeed().Live();

	//switch ( camera.Type() )
	//{
	//	case Cam2::E_SimpleChaseCamera:
	//	{
	//		Cam2::C_SimpleChaseCamera chaseCamera;
	//		//Cam2::C_SimpleChaseCamera& chaseCamera = static_cast<Cam2::C_SimpleChaseCamera&>( camera );
	//		chaseCamera.SetLookAround( Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_FlyRotateX ) * Maths::Pi );
	//		view.DebugFeed().CopyFrom( chaseCamera );
	//	}
	//}

	if ( m_actorList.GetNumActors() > 0 )
	{
		const C_Actor& actor = m_actorList.GetFocusActor();
	
		Cam2::C_CamObj targetObject( actor.GetWorldPos(), actor.GetWorldRotation() );
		view.DebugFeed().SetObject( targetObject );
	}
	
	if ( reloadResourcesFlag )
	{
		m_actorList.ReloadResources();
		reloadResourcesFlag = false;
	}
}

Debug::Float gDebugTextX( "AnimationTest.DebugTextX", 50.f, Debug::Range(0.f, 1280.f) );
Debug::Float gDebugTextY( "AnimationTest.DebugTextY", 50.f, Debug::Range(0.f, 800.f) );

void C_TestSectionAnimation::OnRender3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(57, 67, 79, 225) );
	
	PERF_EVENT( "TestSectionAnimation.Render" );
	
	// also kick off render tasks, although they won't start until the actor tasks have completed.
	StartRenderTasks();
	
	Maths::HVec3 targetActorPos;
	if ( m_actorList.GetNumActors() > 0 )
	{
		targetActorPos = m_actorList.GetFocusActor().GetWorldPos();
	}
	
	RenderGrid( targetActorPos, &m_gridTexture );
	
	FinishRenderTasks();
	
	RenderSkeletons();
		
	//static_assert(sizeof(Animation::C_RelativePtr<Animation::C_JointTransform>) == 4);
	
	if ( m_actorList.GetNumActors() > 0 )
	{
		const C_Actor& actor = m_actorList.GetFocusActor();
	
		const Animation::C_Pose& pose = m_actorList.GetFocusActor().GetPose();
	
		U32 numScalars = pose.GetNumCustomScalars();
		
		Maths::HVec2 pos( (F32) gDebugTextX, (F32) gDebugTextY );
		const Maths::HVec2 fontSize( 16.f, 16.f );
		
		Util::C_String str = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
		
		for ( U32 i = 0; i < numScalars; ++i )
		{
			//const CHAR8* name = pose.GetFieldName( Animation::E_PoseFieldType_CustomScalar, i ).GetString();
			const CHAR8* name = pose.GetCustomScalarName( i ).GetString();
			assert( name );
						
			
			const F32 value = pose.GetCustomScalar( i );
			//str.Printf( "%s: %.3f", name, *pose.GetField( Animation::E_PoseFieldType_CustomScalar, i ) );
			str.Printf( "%s: %.3f", name, value );
			
			Display::FontRender()->Draw2D(
				str.Chars(),
				pos,
				fontSize,
				Maths::RGBA( 255, 255, 255, 255 ),
				Display::E_FontRenderAlign_Lt
			);
			
			pos.Y( pos.Y() + fontSize.Y() );
		}
		
		for ( U32 i = 0; i < actor.m_eventLogger.GetNumEvents(); ++i )
		{
			S32 eventFieldIndex = actor.m_eventLogger.GetEventFieldIndex( i );
			
			if ( eventFieldIndex >= 0 )
			{
				const CHAR8* name = pose.GetCustomScalarName( eventFieldIndex ).GetString();
				assert( name );

				str.Printf( "%s: %s %s", name, actor.m_eventLogger.GetEventValue(i) ? "true" : "false", actor.m_eventLogger.HasChanged(i) ? "(changed)" : "" );
				
				Display::FontRender()->Draw2D(
					str.Chars(),
					pos,
					fontSize,
					Maths::RGBA( 255, 255, 255, 255 ),
					Display::E_FontRenderAlign_Lt
				);
				
				pos.Y( pos.Y() + fontSize.Y() );
			}

		}
		
		#if 0
		F32 x = Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_FlyRotateX );
		F32 y = Input::GetGameInput(0)->GetAxisSignedValue( E_Axis_FlyRotateY );
		str.Printf( "Axis X: %.3f Axis Y: %.3f Length: %.3f Sum: %.3f",
			x, y, Maths::Sqrt(x*x + y*y), Maths::Abs(x) + Maths::Abs(y) );
		
			Display::FontRender()->Draw2D(
				str.Chars(),
				pos,
				fontSize,
				Maths::RGBA( 255, 255, 255, 255 ),
				Display::E_FontRenderAlign_Lt
			);
		#endif
		
	}
	
	//Util::C_String str = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
	//str.Printf( "%s: %.3f", "TESTING_1_2_3", 100.f );

}

extern Debug::Bool gForceSingleThreaded;
extern Debug::Bool gEnablePpuTasks;
extern Debug::Int gNumActorsPerTask;

void C_TestSectionAnimation::StartRenderTasks()
{
	FinishRenderTasks();
	
	const U32 numActors = m_actorList.GetNumActors();
	
	const U32 totalLines = numActors * ( m_actorList.GetNumJointsPerActor() - 1 );
	const U32 vertexCount = totalLines * 2;
	
	C_SkeletonVertex* pVertices = (C_SkeletonVertex*) m_dynamicVertexBuffer.Begin(
		Display::E_PrimitiveType_LineList,
		vertexCount,
		sizeof( C_SkeletonVertex ),
		m_pSkeletonVertexDecl
	);
	
	assert( ( ( (U32) pVertices ) & 0xf ) == 0 );
	
	m_renderTaskPending = false;
	
	if ( !pVertices )
	{
		LOGWARN( Debug::GetLogger( "animation" ), "Couldn't get vertex buffer (%d requested)", vertexCount );
		return;
	}

	PERF_EVENT( "TestSectionAnimation.StartRenderTasks" );
	
	
	if ( Scheduler::Active() && !gForceSingleThreaded )
	{
		m_renderTaskPending = true;
		C_SkeletonRenderParams params;
		params.m_vertices = pVertices;
		params.m_actors = m_actorList.GetActorPtr( 0 );
		
		U32 kMaxSkeletonsPerBatch = 16; // use debug variable to ensure that we don't get first/last group overlaps?
		U32 actorIndex = 0;
		
		while ( actorIndex < numActors )
		{
			U32 numSkeletonsInThisBatch = Maths::Min( kMaxSkeletonsPerBatch, numActors - actorIndex );
			
			U32 numVerticesInThisBatch = 2 * numSkeletonsInThisBatch * ( m_actorList.GetNumJointsPerActor() - 1 );
			
			params.m_vertexBufferSize = numVerticesInThisBatch * sizeof( C_SkeletonVertex );
			assert( ( params.m_vertexBufferSize & 0xf ) == 0 );
			params.m_numActors = numSkeletonsInThisBatch;
			params.m_firstActorIndex = actorIndex;
			
			// Kick off a render task.
			Scheduler::AddTask(
				SkeletonRenderModule,
				0, // input ptr
				0, // output ptr
				m_skeletonRenderGroup,	// group ID
				gEnablePpuTasks ? 0 : 1, // policy
				&params,
				sizeof( C_SkeletonRenderParams )
			);
							
			actorIndex += numSkeletonsInThisBatch;
			params.m_vertices += numVerticesInThisBatch;
		}
	}
	else
	{
		m_renderTaskPending = true;
		static const Maths::C_RGBA_F32 white( Maths::RGBA( 255, 255, 255, 255 ) );
		static const Maths::C_RGBA_F32 green( Maths::RGBA( 0, 255, 0, 255 ) );

		for ( U32 i = 0; i < numActors; ++i )
		{
			C_Actor& actor = m_actorList.GetActor( i );
			
			Animation::C_SkeletonPose skeletonPose( *actor.GetSkeleton(), actor.GetPose() );
					
			Maths::Mat43 modelWorldMatrix = Maths::MakeMat43( 
				Maths::MakeMat33( actor.GetWorldRotation() ),
				static_cast<Maths::Vector3>(actor.GetWorldPos())
			);
	
			skeletonPose.GetWorldJointMatrices( &actor.m_worldJointMatrices[ 0 ], actor.m_worldJointMatrices.Size(), modelWorldMatrix );
	
			RenderSkeletonVertices(
				pVertices,
				*actor.GetSkeleton(),
				&actor.m_worldJointMatrices[ 0 ],
				i == 0  ? green : white
			);
		}
	}
}


void C_TestSectionAnimation::FinishRenderTasks()
{
	// Wait for rendering tasks to finish.
	if ( m_renderTaskPending )
	{
		if ( Scheduler::Active() && !gForceSingleThreaded )
		{
			// Waiting for a group to finish that has dependencies that haven't finished is inefficient. Normally
			// the current thread helps out by working on tasks in the specified group that remain in the queue.
			// However, if the tasks in the specified group cannot execute because their dependencies haven't finished,
			// then the current thread just blocks until all dependencies have completed. It would be more efficient
			// if the current thread tried to work on the dependencies instead.
			
			// For efficiency, we manually force a flush on the dependencies here, which makes dependencies fairly
			// pointless!
			m_actorList.FinishUpdate();
			Scheduler::Flush( m_skeletonRenderGroup );
		}
		m_renderTaskPending = false;

		m_vertexBufferOffset = m_dynamicVertexBuffer.GetOffset();

		m_dynamicVertexBuffer.End();
	}
}

void C_TestSectionAnimation::RenderSkeletons()
{
	PERF_EVENT( "TestSectionAnimation.RenderSkeletons" );
	
	// Borrow the BasicRender shaders!
	Display::BasicRender()->BindColoured3D();

	Display::Device()->SetVertexBuffer(
		0,
		m_dynamicVertexBuffer.GetVertexBuffer(),
		m_vertexBufferOffset,
		m_dynamicVertexBuffer.GetVertexStride()
	);
	Display::Device()->SetVertexDeclaration( m_dynamicVertexBuffer.GetVertexDeclaration() );
	Display::Device()->DrawPrimitive( m_dynamicVertexBuffer.GetPrimitiveType(), 0, m_dynamicVertexBuffer.GetVertexCount() );
	
	m_dynamicVertexBuffer.Swap();
}

