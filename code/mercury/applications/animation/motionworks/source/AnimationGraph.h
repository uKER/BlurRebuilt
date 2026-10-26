#ifndef ANIMATIONGRAPH_H
#define ANIMATIONGRAPH_H

#include <animation/common/BlendModes.h>
#include <animation/cycle/CycleInstance.h>
#include <animation/deferred/CommandList.h>
#include <animation/pose/PoseMirror.h>

// for C_SceneConfig and C_CycleParameters
#include <animation/tools/Scene.h>
#include <animation/tools/CycleBuilder.h>

#include <btl/containers/set.h>

class C_ResourceCache;

namespace Animation
{
	class C_CommandList;
	class C_Skeleton;
	class C_Cycle;
	class C_MemoryBlock;
	class C_MemoryBlockReader;
	class C_MemoryBlockWriter;
	class C_PoseMirror;
}

namespace AnimationGraph
{
	/*
		Global context
			Skeleton
			Model->world transform
			Input variables?
		Local context, passed from parents to children.
			Blend weight
			Blend mode
			Time
			Mirroring?
	*/
	
	class C_Context;
	
	enum E_InputVarType
	{
		E_InputVarType_InputVar	= 0,
		E_InputVarType_Constant,
		E_InputVarType_SineWave,
		E_InputVarType_Controller,
		E_InputVarType_Timeline,
		E_InputVarType_Count
	};	
	
	
	
	class C_InputVar
	{
	public:
		C_InputVar( E_InputVarType varType ):
			m_type(varType)
		{}
	
		virtual F32 GetValue( const C_Context& context)	{ return 0.f; }
		
		virtual void SaveState(Animation::C_MemoryBlockWriter& out) const {}
		virtual void LoadState(Animation::C_MemoryBlockReader& in) {}
		
		E_InputVarType GetType() const	{ return m_type; }
	
		XtSz8		m_name;
		
	private:
		E_InputVarType	m_type;
	};
	
	class C_ConstantVar: public C_InputVar
	{
	public:
		C_ConstantVar(): C_InputVar( E_InputVarType_Constant ) {}
	
		virtual F32 GetValue( const C_Context& context )	{ return m_value; }
	
		F32	m_value;
	};
	
	class C_SineWaveVar: public C_InputVar
	{
	public:
		C_SineWaveVar():
			C_InputVar( E_InputVarType_SineWave ),
			m_phase(0.f),
			m_updateCounter(0),
			m_value(0.f)
		{
		}
	
		virtual F32 GetValue( const C_Context& context );
		
		virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
		virtual void LoadState(Animation::C_MemoryBlockReader& in);
		
		F32		m_frequency;
		F32		m_scale;
		F32		m_offset;
		F32		m_phaseOffset;
		bool	m_clamp;
		XtRangeF32 m_clampRange;
		
	private:
		F32		m_phase;
		U32		m_updateCounter;
		F32		m_value;
	
	};
	
	class C_TimelineVar: public C_InputVar
	{
	public:
		C_TimelineVar():
			C_InputVar(E_InputVarType_Timeline) {}
			
		virtual F32 GetValue( const C_Context& context );
			
	};
	
	enum E_ControllerInputType
	{
		E_ControllerInputType_ForwardBack,
		E_ControllerInputType_LeftRight,
		E_ControllerInputType_Action1,
		E_ControllerInputType_Action2,
		E_ControllerInputType_Heading
	};
	
	class C_ControllerInputVar: public C_InputVar
	{
	public:
		C_ControllerInputVar():
			C_InputVar( E_InputVarType_Controller )
		{}
	
		virtual F32 GetValue( const C_Context& context );
		
		E_ControllerInputType	m_input;
	};
	
	class C_Context;
	
	enum E_NodeType
	{
		E_NodeType_Node	= 0,
		E_NodeType_Cycle,
		E_NodeType_Blend,
		E_NodeType_LayeredBlend,
		E_NodeType_Mirror,
		E_NodeType_TimeController,
		E_NodeType_Result,
		E_NodeType_Count
	};
	
	//
	// Base class for animation graph nodes.
	//
	class C_Node
	{
	public:
		C_Node( E_NodeType type );
		
		virtual ~C_Node();
	
		virtual void LoadResources(C_ResourceCache& rc) {}
		
		virtual void Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight )=0;
		
		virtual void SetPhase( const C_Context& context, F32 phase );
		F32 GetPhase() const;
		virtual void UpdateTime( const C_Context& context, F32 deltaTime );
		virtual F32 GetFrequency( const C_Context& context );
		void SetFrequency( const C_Context& context, F32 f );
		
		//void SetWeight( F32 w )		{ m_weight = w; }
		//F32 GetWeight()				{ return m_weight; }
		
		//Animation::E_BlendMode	GetBlendMode() const	{ return m_blendMode; }
		//void SetBlendMode( Animation::E_BlendMode mode ) { m_blendMode = mode; }
		
		virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
		virtual void LoadState(Animation::C_MemoryBlockReader& in);
		
		E_NodeType GetType() const	{ return m_type; }
		

	protected:
		F32						m_phase;
		F32						m_frequency;
		//F32						m_weight;

		const E_NodeType		m_type;
	};
	
	//
	// Root of animation graph.
	//
	class C_ResultNode: public C_Node
	{
	public:
		C_ResultNode();
		
		virtual void LoadResources(C_ResourceCache& rc);
		virtual void Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight );
		virtual void UpdateTime( const C_Context& context, F32 deltaTime );
		virtual void SetPhase( const C_Context& context, F32 phase );
		virtual F32 GetFrequency( const C_Context& context );
		
	public:
	
		C_Node* m_input;
	};
	
	enum E_FieldMaskFlags
	{
		E_FieldMaskFlags_TransformRotation			= 1 << 0,
		E_FieldMaskFlags_TransformTranslation		= 1 << 1,
		E_FieldMaskFlags_TransformScale				= 1 << 2,
		E_FieldMaskFlags_TrajectoryRotation			= 1 << 3,
		E_FieldMaskFlags_TrajectoryTranslation		= 1 << 4,
		E_FieldMaskFlags_TrajectoryDeltaRotation	= 1 << 5,
		E_FieldMaskFlags_TrajectoryDeltaTranslation = 1 << 6,
		E_FieldMaskFlags_CustomScalar				= 1 << 7,
		E_FieldMaskFlags_CustomVector3				= 1 << 8,
		E_FieldMaskFlags_CustomVector4				= 1 << 9
	};
	
	
	//
	// Samples a cycle.
	//
	class C_CycleNode: public C_Node
	{
	public:
		C_CycleNode();
		
		virtual void LoadResources(C_ResourceCache& rc);
		
		virtual void Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight );
		
		virtual void UpdateTime( const C_Context& context, F32 deltaTime );

		virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
		virtual void LoadState(Animation::C_MemoryBlockReader& in);

	public:
		XtSz8							m_filename;
		XtSz8							m_subtractedFilename;
		F32								m_phaseOffset;
		F32								m_timeScale;


		// Import options.
		AnimationTools::C_SceneConfig		m_sceneConfig;
		AnimationTools::C_CycleParameters	m_cycleParams;
		U32								m_version;
		bool							m_worldSpace;
		
		bool							m_ignoreTrajectory;
		
		Animation::E_LoopMode			m_loopMode;

		Animation::C_CycleInstance		m_cycleInstance;
		const Animation::C_Cycle*		m_cycle;
		
		bool							m_useFieldMask;
		
		XtFlags<U32, E_FieldMaskFlags>	m_enabledFields;
		
	};	
	
	class C_BlendInput
	{
	public:
		
	public:
		C_Node*			m_input;
		F32				m_controlValue;
	};
	
	//
	// Blends together its inputs.
	//
	class C_BlendNode: public C_Node
	{
	public:
		C_BlendNode();
		
		virtual void LoadResources(C_ResourceCache& rc);
		virtual void Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight );
		virtual void SetPhase( const C_Context& context, F32 phase );
		virtual void UpdateTime( const C_Context& context, F32 deltaTime );		
		
		virtual F32 GetFrequency( const C_Context& context );
		void CalcBlend( const C_Context& context );

		virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
		virtual void LoadState(Animation::C_MemoryBlockReader& in);


	public:
		Animation::E_BlendMode	m_blendMode;
		XtArray<C_BlendInput>	m_inputs;
		C_InputVar*				m_controlInput;
		bool					m_wrap;
		
	private:
		S32 m_activeChild0;
		S32 m_activeChild1;
		F32			m_alpha;
	};
	
	class C_BlendLayer
	{
	public:
		Animation::E_BlendMode	blendMode;
		C_Node* input;
	};

	//
	// Evaluates its inputs in sequence and blends them together.
	//
	class C_LayeredBlendNode: public C_Node
	{
	public:
		C_LayeredBlendNode();
		
		virtual void LoadResources(C_ResourceCache& rc);
		virtual void Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight );
		virtual void SetPhase( const C_Context& context, F32 phase );
		virtual void UpdateTime( const C_Context& context, F32 deltaTime );		
		
		virtual F32 GetFrequency( const C_Context& context );

	public:
		C_Node*					m_baseLayer;
		XtArray<C_BlendLayer>	m_layers;
	};
	
	//
	// Mirrors all evaluated joints in an input pose.
	//
	class C_MirrorNode: public C_Node
	{
	public:
		C_MirrorNode();
		virtual ~C_MirrorNode();
		
		virtual void LoadResources(C_ResourceCache& rc);
		virtual void Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight );
		virtual void SetPhase( const C_Context& context, F32 phase );
		virtual void UpdateTime( const C_Context& context, F32 deltaTime );		
		
		virtual F32 GetFrequency( const C_Context& context );
		
	public:
		C_Node*					m_input;
		Animation::E_MirrorAxis	m_mirrorAxis;
		bool					m_enabled;
		
	private:
		Animation::C_PoseMirror m_poseMirror;
		const Animation::C_Skeleton* m_currentSkeleton;
	};


	//
	// Controls the time of 
	//
	class C_TimeControllerNode: public C_Node
	{
	public:
	
		C_TimeControllerNode();
		virtual ~C_TimeControllerNode();
		
		virtual void LoadResources(C_ResourceCache& rc);
		virtual void Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight );
		virtual void SetPhase( const C_Context& context, F32 phase );
		virtual void UpdateTime( const C_Context& context, F32 deltaTime );
		
		virtual F32 GetFrequency( const C_Context& context );

	private:
		void SetChildTime( const C_Context& context );
		
	public:
		C_Node*		m_input;
		C_InputVar*	m_controlInput;
		XtRangeF32	m_inputRange;
		XtRangeF32	m_timeRange;
	};


	
	//
	// Generates a base pose.
	//
	//class C_BasePoseNode: public C_Node
	//{
	//public:
	//
	//};
	
	class C_ControllerState
	{
	public:
		C_ControllerState():
			forwardBack(0.f),
			leftRight(0.f),
			action1(0.f),
			action2(0.f)
		{
		}
		
		F32	forwardBack;		// -1 = back, +1 = forward
		F32 leftRight;			// -1 = left, +1 = right
		F32 action1;
		F32 action2;
		F32 heading;	// Heading in degrees, clockwise with 0 = north, 90 = east etc.
		F32 turnAngle;	// change in heading?
	};
	
	typedef Btl::Set<Btl::String> C_StringSet;
	
	class C_Checksum16
	{
	public:
	
		C_Checksum16():
			m_checksum(0)
		{}
		
		C_Checksum16( U16 checksum ):
			m_checksum( checksum )
		{
		}
		
		void Clear()
		{
			m_checksum = 0;
		}
		
		void Update( const void* data, U32 numBytes )
		{
			const U8* dataBytes = static_cast<const U8*>(data);
		
			U8 a = m_checksum & 0xff;
			U8 b = (m_checksum >> 8) & 0xff;
			
			for ( U32 i = 0; i < numBytes; ++i )
			{
				a += dataBytes[i];
				b += a;
			}
			m_checksum = (b << 16) | a;
		}
		
		U16 AsU16() const	{ return m_checksum; }
		
		bool operator == ( const C_Checksum16& other ) const
		{
			return m_checksum == other.m_checksum;
		}
		
		bool operator != ( const C_Checksum16& other ) const
		{
			return m_checksum != other.m_checksum;
		}
	
	private:
	
		U16		m_checksum;
	};
	
	class C_Graph
	{
	public:
		C_Graph();
		~C_Graph();
		
		void UpdateControllerState( const C_ControllerState& state );
		void UpdateTime( F32 time );
		void SetTimelineValue( F32 value );
		bool Evaluate( const Animation::C_Skeleton* skeleton, Animation::C_Pose& pose, bool& outPoseInWorldSpace, bool enablePushPopElimination = true );
		
		void SaveState(Animation::C_MemoryBlock& dest) const;
		void LoadState(const Animation::C_MemoryBlock& src);
		
		static void XtCallback(XtEventType eventType, XtRecord* record, C_Graph* graph, XtCallbackContext* context);
		
		
				
	public:
		C_ResultNode* m_result;
		XtArray<C_Node*> m_nodes;
		XtArray<C_InputVar*> m_inputVars;
		
		Animation::C_CommandList m_cmdList;
		U32 m_updateCounter;
		F32 m_deltaTime;
		S32 m_globalTime;
		F32 m_timelineValue;
		
	public:
		static C_Graph* s_activeGraph;
		
		void UpdateCustomCurveNames();
		static C_Checksum16 m_customCurveChecksum;
		static C_StringSet m_customScalarCurveNames;
		static C_StringSet m_customVector3CurveNames;
		static C_StringSet m_customVector4CurveNames;
		
	
		C_ControllerState m_controllerState;
//		C_MirroredPose* m_mirroredPose;
	};
	
	C_Graph* CreateGraph();


}


#endif

