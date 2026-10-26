#ifndef EDITOR_PROPS_DG_COVER_NODE_PROP_NODE_CONTEXTS_H
#define EDITOR_PROPS_DG_COVER_NODE_PROP_NODE_CONTEXTS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	/*
	class C_ScaleDgCoverNodePropNodeContext : public I_VectorContext
	{
	public:
		C_ScaleDgCoverNodePropNodeContext();
		virtual ~C_ScaleDgCoverNodePropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};

	class C_RotateDgCoverNodePropNodeContext : public I_OrientationContext
	{
	public:
		C_RotateDgCoverNodePropNodeContext();
		virtual ~C_RotateDgCoverNodePropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& axis, F32 angle);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& axis, F32 angle);
	};

	//Move CoverNode property indirectly by transforming it's parent
	//(CoverNode properties do not have their own position)
	class C_MoveDgCoverNodePropNodeContext : public I_VectorContext
	{
	public:
		C_MoveDgCoverNodePropNodeContext();
		virtual ~C_MoveDgCoverNodePropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};
	*/
}

#endif
