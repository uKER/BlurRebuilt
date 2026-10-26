#ifndef EDITOR_PROPS_DG_ANIMATION_PROP_NODE_H
#define EDITOR_PROPS_DG_ANIMATION_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace Curve
{
	template<typename KNOT>
	struct C_MayaHermiteSplineKnotU;

	template<typename KNOT>
	class C_CurveData;

	template<typename A, typename B>
	class C_HermiteCtrlPt1DTanAngle;

	template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY = C_HermiteCtrlPt1DTanAngle<VECTYPE, STORAGE> >
	class C_MayaHermiteSpline;
}

namespace EditorLib
{
	class C_XtDirty;
	class C_XtScratchData;

	class C_DgAnimationPropNode : public C_DgXtPropNodeBase
	{
	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool IsNonXtDirty() const;


	//From C_DgAnimationPropNode
	public:
		typedef Curve::C_CurveData<Curve::C_MayaHermiteSplineKnotU<F32> > CurveDataType;
		typedef Curve::C_MayaHermiteSpline<F32, CurveDataType, Curve::C_HermiteCtrlPt1DTanAngle<F32, CurveDataType> > CurveType;

		class Object;
		class Channel;

		class AnimationCurve
		{
		public:
			enum E_AnimationCurve_Metadata
			{
				E_AnimationCurve_Metadata_None,
				E_AnimationCurve_Metadata_X,
				E_AnimationCurve_Metadata_Y,
				E_AnimationCurve_Metadata_Z
			};

			AnimationCurve(const AnimationCurve& curve);
			AnimationCurve(C_DgAnimationPropNode* propNode, Channel* channel);
			~AnimationCurve();

			E_AnimationCurve_Metadata metadata;

			Btl::String			name;
			Maths::RGBA			colour;
			LarvaField*			pField;

			CurveType*			pCurve;
			CurveDataType*		pCurveData;

			Channel*			pChannel;

			void Save();
			void Process();
			C_Status AddKeyFrame(F32 position, F32 value, F32 tangentIn, F32 tangentOut, U32* newKeyIndex=0, bool* isNewKey=0);
			C_Status RemoveKeyFrames(F32 startPosition, F32 endPosition);
			C_Status RemoveKeyFrame(U32 index);
			C_Status SmoothTangents(U32 index);

			float GetOffset();
			void SetOffset(float value);
			float Evaluate(float t);

			void ParseMetadataString(const Btl::String& metadataString);

			//maya style post process to filter out rotational wrapping
			C_Status EulerFilter();

			bool IsDirty();
			void ClearDirty();

			void XtDeserialise(Xt::DomElement* pElement);

		private:
			C_Status InternalRemoveKeyFrame(U32 index);

			float m_offset;
			C_DgAnimationPropNode*	m_pPropNode;
			bool					m_dirty;
		};

		class Channel
		{
		public:
			Channel(C_DgAnimationPropNode* propNode, Object* object);
			~Channel();

			enum E_AnimationChannel_Metadata
			{
				E_AnimationChannel_Metadata_None,
				E_AnimationChannel_Metadata_Translation,
				E_AnimationChannel_Metadata_Rotation,
				E_AnimationChannel_Metadata_Time
			};

			XtId							id;
			LtStruct*						xtType;
			Btl::String						name;
			bool							hasNameField;

			E_AnimationChannel_Metadata		metadata;

			Btl::Vector<AnimationCurve*>	curves;

			Btl::Vector<AnimationCurve*>& GetCurves();
			const Btl::Vector<AnimationCurve*>& GetCurves() const;
			Object* GetObject();
			void Delete(bool doDelete);
			bool IsDeleted();
			void ParseMetadataString(const Btl::String& metadataString);
			void ParseName(Xt::DomComponent* pChannelComponent, LtStruct* pChannelType);
			
			void SetName(Btl::String name);
			Btl::String GetName();

			void XtDeserialise(LtStruct* pChannelType, Xt::DomComponent* pComponent);

		private:
			bool					m_deleted;
			C_DgAnimationPropNode*	m_pPropNode;

			Object*					m_pObject;
		};

		class Object
		{
		//From Object
		public:
			Object(C_DgAnimationPropNode* propNode);
			~Object();

			C_Status AddChannel(LtStruct* type);
			C_Status RemoveChannel(Channel* pChannel);

			Btl::Vector<Channel*>& GetChannels();
			const Btl::Vector<Channel*>& GetChannels() const;

			C_Status ChangeActor(XtId newActorId);

			I_DgNode* GetActorDgNode();
			
			void Delete(bool doDelete);
			bool IsDeleted();

			void XtDeserialise(Xt::DomComponent* pComponent);
			void XtSave(Xt::DomComponent* pComponent);

			const Btl::String& GetName() const;
			const C_DgAnimationPropNode* GetPropNode() const;

			float GetTimelineValue(float timeline);

			XtId						id;

		private:
			C_DgAnimationPropNode*	m_pPropNode;

			Btl::Vector<Channel*>	m_Channels;

			XtId					m_ActorId;
			I_DgNode*				m_ActorDgNode;
			
			C_XtScratchData*		m_pCurrentPosition;
			Btl::String				m_Name;

			bool					m_deleted;
		};

		virtual const char*		GetTypeName() const;

		static const char*		GetStaticTypeName();

		static const char*		GetStaticDisplayName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		virtual E_ObjType GetType() const;

		virtual bool IsA(E_ObjType type) const;

		virtual bool IsVisible() const;

		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

		C_NodeId GetAnimationId();

		C_Status XtSave();

		C_Status AddObject();
		C_Status RemoveObject(Object* pChannel);

		Btl::Vector<Object*>& GetObjects();
		const Btl::Vector<Object*>& GetObjects() const;

		C_Status SetClipLength(F32 clipLength);
		F32 GetClipLength();

		C_Status SetQuantisation(U32 quantisation);
		U32 GetQuantisation();

	protected:
		C_DgAnimationPropNode();
		virtual ~C_DgAnimationPropNode();

		C_DgAnimationPropNode(const C_DgAnimationPropNode& rhs);
		const C_DgAnimationPropNode& operator=(const C_DgAnimationPropNode& rhs);

		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

		void UpdateCollisionSource();

		void UpdateCollisionInstance();

		void SetDefault();

		U32						m_Quantisation;
		F32						m_ClipLength;
		Btl::String				m_Name;

		Xt::DomMonitor			m_RefTargetMonitor;

		Btl::Vector<Object*>	m_Objects;
		Btl::Vector<Object*>	m_ObjectsToAdd;
	};
}

#endif
