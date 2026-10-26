#ifndef ANIMATION_TOOLS_SCENE_H
#define ANIMATION_TOOLS_SCENE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/utility/ScopedPointer.h>
#include <btl/containers/vector.h>
#include <animation/tools/SceneToGameTransform.h>
#include <animation/tools/PatternList.h>
#include <animation/tools/Status.h>


namespace Sys
{
	class I_BinaryStream;
}

namespace Aftk
{
	class C_Scene;
	class C_DynamicPlug;
}

namespace Animation
{
	class C_ObjectTransform;
	class C_JointTransform;
	class C_Skeleton;
}

namespace AnimationTools
{
	class C_ExportConstraints;
	
	void ConvertAftkMatrix( Maths::Mat43& lhs, const Aftk::Matrix44& rhs );

	void RemoveAftkMatrixZFlip( Aftk::Matrix44& matrix );
	
	/**
	Animated scene configuration.
	*/
	class C_SceneConfig
	{
	public:
		C_SceneConfig():
			worldOrigin(0.f, 0.f, 0.f),
			scaleFactor(1.f),
			oversampling(1.f),
			namespaceDelimiter(':'),
			importAnimatedMaterials(true),
			importAnimatedNodeAttributes(true),
			excludeSkinnedMeshes(true),
			removeNamespaces(false),
			flipX(true),
			flipY(false),
			flipZ(false)
		{
		}
		
		MathsStorage::Float3 worldOrigin;			///< Point to use as world origin, default (0,0,0).
		C_PatternList worldOriginNodeNames;			///< Name patterns for nodes to use as custom world origin (overriding the above origin value and including rotation).
		
		C_PatternList rootNodeNames;				///< Patterns used to identity the animated scene/skeleton root node. If empty then scene root is used.
		C_PatternList dynamicAimJointNames;			///< Patterns used to identify dynamic aim joints.
		C_PatternList dynamicSliderJointNames;		///< Patterns used to identify dynamic slider joints.
		C_PatternList excludedNodeNames;			///< Patterns used to identify joints that should be excluded.
		C_PatternList referenceNodeNames;			///< Patterns used to identify non-animated reference markers.
		C_PatternList helperNodeNames;				///< Patterns used to identify helper nodes that must keep their animation, even if it's constant and matches the base pose.
		C_PatternList excludedAttributeNames;		///< Patterns used to identity animated node attributes that should be ignored.
		C_PatternList highFidelityRootNodeNames;	///< Patterns used to identity sub-trees of nodes that should be kept at high fidelity, e.g. facial or camera bones.
		F32			  scaleFactor;					///< Global scene scale factor.
		F32			  oversampling;					///< Oversampling factor, applied to frame rate.
		CHAR8		  namespaceDelimiter;			///< Namespace delimiter, usually ':'
		bool		  importAnimatedMaterials;		///< If true, animated material parameters will be imported as custom curves.
		bool		  importAnimatedNodeAttributes;	///< If true, animated node attributes will be imported as custom curves.
		bool		  excludeSkinnedMeshes;			///< If true (the default), skinned mesh transforms will be excluded (they are constant and set to bind pose).
		bool		  removeNamespaces;				///< If true, namespace prefixes of the form "<name>:" will be removed from node names. Default is false.
		bool		  flipX;						///< If true, game coordinate system has X axis flipped.
		bool		  flipY;						///< If true, game coordinate system has Y axis flipped.
		bool		  flipZ;						///< If true, game coordinate system has Z axis flipped.
		
		bool operator == ( const C_SceneConfig& rhs ) const;

		bool HasNonZeroWorldOrigin() const
		{
			return worldOrigin.x != 0.f || worldOrigin.y != 0.f || worldOrigin.z != 0.f;
		}
	};
	
	
	/**
	Interface for filtering out nodes of interest in an Aftk::C_Scene.
	*/
	class I_Filter
	{
	public:
		virtual ~I_Filter() {}
		
		/// Returns true if the specified node passes the filter.
		virtual bool IsValidNode( const Aftk::C_SceneNode* node ) const = 0;
	};
	
	/**
	Interpolation modes.
	By default all inter-keyframe interpolation is linear, except for integer
	custom curves which use constant (nearest) interpolation.
	*/	
	enum E_InterpolationMode
	{
		E_InterpolationMode_Linear	= 0,
		E_InterpolationMode_Constant,
		E_InterpolationMode_Count
	};


	/**
	A wrapper around an Aftk scene acting as an animated data source.
	
	C_CycleBuilder etc. should use this class and never have to deal with the Aftk.
	
	The scene hierarchy can be interpreted as a skeleton. Nodes are sorted in order
	of ascending depth in the hierarchy, such that the parent of node X has an index
	less than the index of X. 
	
	The hierarchy can be interpreted as a skeleton. Node transforms can be queried
	in various non-interchangeable formats, currently C_JointTransform (rotation, translation),
	C_ObjectTransform (rotation, translation, scale) and Maths::Mat43

	Types of data:
	
		- Nodes (transforms).
		- Objects: scale, rotation, translation.
		- Reference markers (transforms, extracted from hierarchy)
		- Custom curves (scalar curves, extracted from nodes or special nodes)
		- Material properties (extracted from materials in the scene, whatever they may be).
	
	*/
	class C_Scene
	{
	public:

		C_Scene();
		
		~C_Scene();

		/// Import a scene.
		C_Status Import( Aftk::C_Scene& aftkScene, const C_SceneConfig& sceneConfig );
		
		/// Import animated data from the specified list of scene nodes.
		C_Status Import( Aftk::C_Scene& aftkScene, const Btl::Vector<Aftk::C_SceneNode*>& sceneNodes, const C_SceneConfig& sceneConfig );

		/// Import a scene using the specified node filter.
		C_Status Import( Aftk::C_Scene& aftkScene, const C_SceneConfig& sceneConfig, const I_Filter& filter );
		
		/// Get the scene config used to import this scene.
		const C_SceneConfig& GetSceneConfig() const;
		
		// Scene to game transform goes here.
		const C_SceneToGameTransform& GetSceneToGameTransform() const;
		
		/// Get number of frames in scene.
		U32 GetNumFrames() const;
		
		/// Get frame rate, in frames per second.
		F32 GetFrameRate() const;
		
		/// Set the current frame.
		void SetCurrentFrame( U32 frame );
		
		//
		// Nodes.
		//
				
		U32 GetNumNodes() const;
				
		S32 GetNodeParentIndex( U32 nodeIndex ) const;
		U32 GetNodeDepth( U32 nodeIndex ) const;
		
		U32 GetMaxNodeChainLength( U32 nodeIndex ) const;
		F32 GetMaxNodeChainDistance( U32 nodeIndex ) const;
		
		const CHAR8* GetNodeName( U32 nodeIndex ) const;
		
		S32 GetNodeIndexByAftkSceneNode( const Aftk::C_SceneNode* aftkSceneNode ) const;
		S32 GetNodeIndexByNearestAftkSceneNode( const Aftk::C_SceneNode* aftkSceneNode ) const;
				
		void GetCurrentLocalNodeTransform( U32 nodeIndex, Animation::C_JointTransform& outLocalTransform ) const;
		void GetCurrentLocalNodeTransform( U32 nodeIndex, Animation::C_ObjectTransform& outLocalTransform ) const;
		void GetCurrentLocalNodeTransform( U32 nodeIndex, Maths::Mat43& outLocalTransform ) const;

		void GetCurrentWorldNodeTransform( U32 nodeIndex, Animation::C_JointTransform& outWorldTransform ) const;
		void GetCurrentWorldNodeTransform( U32 nodeIndex, Animation::C_ObjectTransform& outWorldTransform ) const;
		void GetCurrentWorldNodeTransform( U32 nodeIndex, Maths::Mat43& outWorldTransform ) const;

		void GetLocalNodeBindPoseTransform( U32 nodeIndex, Animation::C_JointTransform& outLocalTransform ) const;
		void GetLocalNodeBindPoseTransform( U32 nodeIndex, Animation::C_ObjectTransform& outLocalTransform ) const;
		void GetLocalNodeBindPoseTransform( U32 nodeIndex, Maths::Mat43& outLocalTransform ) const;

		void GetInverseWorldNodeBindPoseTransform( U32 nodeIndex, Animation::C_JointTransform& outInverseWorldTransform ) const;
		void GetInverseWorldNodeBindPoseTransform( U32 nodeIndex, Animation::C_ObjectTransform& outInverseWorldTransform ) const;
		void GetInverseWorldNodeBindPoseTransform( U32 nodeIndex, Maths::Mat43& outInverseWorldTransform ) const;
		
		void GetWorldNodeBindPoseTransform( U32 nodeIndex, Animation::C_JointTransform& outWorldTransform ) const;
		void GetWorldNodeBindPoseTransform( U32 nodeIndex, Animation::C_ObjectTransform& outWorldTransform ) const;
		void GetWorldNodeBindPoseTransform( U32 nodeIndex, Maths::Mat43& outWorldTransform ) const;
		
		bool NodeHasRotationKeys( U32 nodeIndex ) const;
		bool NodeHasTranslationKeys( U32 nodeIndex ) const;
		bool NodeHasScaleKeys( U32 nodeIndex ) const;
		
		bool NodeHasRotationConstraint( U32 nodeIndex ) const;
		bool NodeHasTranslationConstraint( U32 nodeIndex ) const;
		bool NodeHasScaleConstraint( U32 nodeIndex ) const;
		
		//
		// Tests for discontinuities caused by stepped tangents etc. on the current frame.
		//
		bool NodeHasRotationDiscontinuity( U32 nodeIndex ) const;
		bool NodeHasTranslationDiscontinuity( U32 nodeIndex ) const;
		bool NodeHasScaleDiscontinuity( U32 nodeIndex ) const;

		bool NodeHasNamedParent(U32 nodeIndex, const CHAR8* parentNamePattern) const;
		bool NodeHasNamedParent(U32 nodeIndex, const C_PatternList& parentNamePatternList) const;
		bool IsHighFidelityNode(U32 nodeIndex) const;
		
		bool IsHelperNode( U32 nodeIndex ) const;
		
		void DebugPrintNodeHierarchy( Debug::Logger& logger ) const;		
	
		//
		// Custom scalar curves (extracted from node attributes).
		//
		
		U32 GetNumCustomCurves() const;
		const CHAR8* GetCustomCurveName( U32 customCurveIndex ) const;
		F32 GetCurrentCustomCurveValue( U32 customCurveIndex ) const;
		//void DebugPrintCustomCurves( Debug::Logger& logger ) const;
				
		//
		// Generic animated data parameters.
		// These can come from animated attributes on scene nodes or material
		// parameters on materials referenced by scene mesh nodes.
		//
		
		U32 GetNumCustomScalars() const;
		U32 GetNumCustomVector3s() const;
		U32 GetNumCustomVector4s() const;
		
		const CHAR8* GetCustomScalarName( U32 index ) const;
		const CHAR8* GetCustomVector3Name( U32 index ) const;
		const CHAR8* GetCustomVector4Name( U32 index ) const;
		
		F32 GetCurrentCustomScalarValue( U32 index ) const;
		void GetCurrentCustomVector3Value( U32 index, F32 outValue[3] ) const;
		void GetCurrentCustomVector4Value( U32 index, F32 outValue[4] ) const;
		
		E_InterpolationMode GetCustomScalarInterpolationMode( U32 index ) const;
		
		bool DoesCustomScalarHaveDiscontinuity( U32 index ) const;
		bool DoesCustomVector3HaveDiscontinuity( U32 index ) const;
		bool DoesCustomVector4HaveDiscontinuity( U32 index ) const;
		
		void DebugPrintCustomCurves( Debug::Logger& logger ) const;
				
		//
		// Reference transforms. These are typically static world space transforms that are not part of the node hierarchy.
		//
		U32 GetNumReferenceTransforms() const;
		
		const CHAR8* GetReferenceTransformName( U32 refTransformIndex ) const;
		
		/// Returns true if reference transform is animated.
		bool IsReferenceTransformAnimated( U32 refTransformIndex ) const;
		
		void GetCurrentWorldReferenceTransform( U32 refTransformIndex, Animation::C_JointTransform& outWorldTransform ) const;
		void GetCurrentWorldReferenceTransform( U32 refTransformIndex, Animation::C_ObjectTransform& outWorldTransform ) const;
		void GetCurrentWorldReferenceTransform( U32 refTransformIndex, Maths::Mat43& outWorldTransform ) const;
		
		void DebugPrintReferenceTransforms( Debug::Logger& logger ) const;

		//
		// Event flags.
		//
		
		//
		// Constraints.
		// 
		
		const C_ExportConstraints& GetConstraints() const;
	
	private:
	
		C_Status VerifyNodeNamesAreUnique( const Aftk::C_Scene& scene, const C_SceneConfig& sceneConfig ) const;
		const Aftk::C_SceneNode* GetRootNode( const Aftk::C_Scene& scene, const C_SceneConfig& sceneConfig ) const;
		const Aftk::C_SceneNode* FindCustomWorldOriginNode( const Aftk::C_Scene& scene, const C_SceneConfig& sceneConfig );
	
		void BuildHierarchy( const Aftk::C_SceneNode* root, S32 parentIndex, const C_SceneConfig& sceneConfig, const I_Filter& filter );
		void AddNode( const Aftk::C_SceneNode* aftkNode, S32 parentIndex, const C_SceneConfig& sceneConfig );
		
		bool IsValidNode( const Aftk::C_SceneNode* node, const C_SceneConfig& sceneConfig ) const;
		//Util::C_String GetCleanAftkSceneNodeName( const Aftk::C_SceneNode* node, const C_SceneConfig& sceneConfig ) const;
		
		C_Status ImportConstraints( const C_SceneConfig& sceneConfig );
		C_Status ImportAnimatedNodeAttributes( const C_SceneConfig& sceneConfig );
		C_Status ImportAnimatedMaterialParameters( const C_SceneConfig& sceneConfig );
		C_Status ImportReferenceTransforms( const C_SceneConfig& sceneConfig );
		void ProcessInfluenceNodes(Btl::Vector<const Aftk::C_SceneNode*>* outMissingInfluenceNodes);
	
		class C_Node
		{
		public:
			C_Node() {}
		
			C_Node( const Aftk::C_SceneNode* inAftkSceneNode,
				const Util::C_String& inName,
				const S32 inParentIndex
			):
				aftkSceneNode( inAftkSceneNode ),
				name( inName ),
				parentIndex( inParentIndex ),
				isHelper(false),
				isHighFidelity(false),
				isModelElement(false),
				isInfluenceNode(false)
			{
			}
				
			const Aftk::C_SceneNode*	aftkSceneNode;
			Util::C_String				name;
			S32							parentIndex;
			
			mutable U32		chainLength;
			mutable F32		distanceFromChainRoot;
			mutable bool	inChain;
			
			bool			isHelper;
			bool			isHighFidelity;
			bool			isModelElement;
			bool			isInfluenceNode;		// True if this node is used as a skinning influence.

			// Returns true if this node is the Aftk-created scene root node.
			bool IsAftkSceneRootNode() const
			{
				if (!aftkSceneNode) return false;
				return (aftkSceneNode == aftkSceneNode->GetScenePtr()->GetRootSceneNodePtr());
			}
		};
	
		C_SceneConfig			m_sceneConfig;
	
		Aftk::C_Scene*			m_aftkScene;
		C_SceneToGameTransform	m_sceneToGameTransform;
		
		Btl::ScopedPointer<C_ExportConstraints>	m_constraints;
		Btl::Vector<C_Node>		m_nodes;


		class C_ReferenceTransform
		{
		public:
			C_ReferenceTransform() {}
		
			C_ReferenceTransform( const Aftk::C_SceneNode* inAftkSceneNode,
				const Util::C_String& inName
			):
				aftkSceneNode( inAftkSceneNode ),
				name( inName )
			{
			}
				
			const Aftk::C_SceneNode*	aftkSceneNode;
			Util::C_String				name;
		};
		
		Btl::Vector<C_ReferenceTransform> m_referenceTransforms;
		
		class C_CurveSource
		{
		public:
			C_CurveSource(): m_refCount(0) {}
			virtual ~C_CurveSource()
			{
				BIZ_ASSERT(m_refCount == 0);
			}
		
			virtual void Evaluate( F32* outValue ) const=0;
			
			virtual bool HasDiscontinuity( F32 currentTime ) const=0;
			
			void AddRef() const
			{
				m_refCount++;
			}
			
			void DecRef() const
			{
				if (0 == --m_refCount)
				{
					delete this;
				}
			}
		private:
			mutable S32		m_refCount;
		};
		
		class C_PlugCurveSource: public C_CurveSource
		{
		public:
			C_PlugCurveSource(const Aftk::I_Plug& plug)
			{
				plugs.Resize(1);
				plugs[0] = &plug;
			}
			
			C_PlugCurveSource(const Aftk::I_Plug** itsPlugs, U32 numPlugs)
			{
				plugs.Resize(numPlugs);
				for ( U32 i = 0; i < numPlugs; ++i )
				{
					plugs[i] = itsPlugs[i];
				}
			}
			
			virtual void Evaluate( F32* outValue ) const;
			
			virtual bool HasDiscontinuity( F32 currentTime ) const;
			
			Btl::Vector<const Aftk::I_Plug*>	plugs;
		};
		
		class C_CameraFovCurveSource;
		
		class C_CustomCurve
		{
		public:
			C_CustomCurve( const Util::C_String itsName, const C_CurveSource* itsSource ):
				name(itsName),
				source(itsSource)
			{
				if (source) source->AddRef();
			}
			
			C_CustomCurve():
				source(NULL)
			{
			}
			
			C_CustomCurve(const C_CustomCurve& other):
				name(other.name),
				source(other.source)
			{
				if (source) source->AddRef();				
			}
			
			~C_CustomCurve()
			{
				if (source) source->DecRef();
			}
			
			void GetCurrentValue( F32* outValue ) const
			{
				if (source)
					source->Evaluate(outValue);
			}
			
			bool HasDiscontinuity( F32 currentTime ) const
			{
				if (source)
					return source->HasDiscontinuity( currentTime );
				else
					return false;
			}
			
			const C_CurveSource*	source;
			Util::C_String	name;
			
		private:
			void operator=(const C_CustomCurve& rhs); // intentionally unimplemented
		};

		class C_CustomScalarCurve: public C_CustomCurve
		{
		public:
			C_CustomScalarCurve() {}
	
			C_CustomScalarCurve( const Util::C_String& name, const C_CurveSource* source ):
				C_CustomCurve( name, source )
			{
			}

			C_CustomScalarCurve( const Aftk::I_Plug* srcPlug, const Util::C_String& name ):
				C_CustomCurve( name, new C_PlugCurveSource(&srcPlug, 1) )
			{
			}
		};
		
		class C_CustomVector3Curve: public C_CustomCurve
		{
		public:
			C_CustomVector3Curve() {}
			
			C_CustomVector3Curve( const Aftk::I_Plug* srcPlugs[3], const Util::C_String& name ):
				C_CustomCurve( name, new C_PlugCurveSource(srcPlugs, 3) )
			{
			}
		};
		
		class C_CustomVector4Curve: public C_CustomCurve
		{
		public:
			C_CustomVector4Curve() {}
			
			C_CustomVector4Curve( const Aftk::I_Plug* srcPlugs[4], const Util::C_String& name ):
				C_CustomCurve( name, new C_PlugCurveSource(srcPlugs, 4) )
			{}
		};				

		Btl::Vector<C_CustomScalarCurve> m_customScalarCurves;
		Btl::Vector<C_CustomVector3Curve> m_customVector3Curves;
		Btl::Vector<C_CustomVector4Curve> m_customVector4Curves;
		
		U32						m_numFrames;
		F32						m_frameRate;
	};
	
	//
	// These methods shouldn't really live here...
	//

	/// Serialise the scene hierarchy as a skeleton into a stream.
	C_Status SerialiseSkeleton( const C_Scene& scene, Sys::I_BinaryStream& stream );
	
	/// Deserialise a skeleton.
	C_Status DeserialiseSkeleton( Animation::C_Skeleton& outSkeleton, Sys::I_BinaryStream& stream );

}

#endif
