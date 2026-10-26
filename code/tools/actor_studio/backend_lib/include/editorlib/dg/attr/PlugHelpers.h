#ifndef EDITOR_LIB_PLUG_HELPERS_H
#define EDITOR_LIB_PLUG_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/Attr/PlugBase.h>

namespace EditorLib
{
	namespace PlugHelpers
	{
		C_NullPlug* CreateCompoundVec3(
			I_DgNode* pParent,
			void(*pOnChangedCallback)(I_Plug*),
			E_PlugType type,
			const char* name, 
			F32 x = 0, F32 y = 0, F32 z = 0);

		template<typename DATA_TYPE>
		INLINE C_StaticPlug<DATA_TYPE>* CreateScalar(
			I_DgNode* pParent,
			void(*pOnChangedCallback)(I_Plug*),
			E_PlugType type,
			const char* name,
			const DATA_TYPE& value);

		template <class T, int N>
		INLINE void SetCompoundValue(C_NullPlug& plug, const UtMaths::BaseVector<T, N>& value, bool triggerTopLevelParentOnChanged = true);

		template <class T, int N>
		INLINE void GetCompoundValue(const I_Plug& plug, UtMaths::BaseVector<T, N>& value, bool update = true);

		///\brief Get the type of compound object this plug represents.
		///
		///This function is obviously only valid if this is a compound plug.
		///
		///@return The compound plug type.
		E_CompoundType GetCompoundType(const I_Plug& plug);

		const I_DgNode* GetTopLevelParent(const I_Plug& plug);

		I_DgNode* GetTopLevelParent(I_Plug& plug);

		void SetMatrixComponents(
			C_NullPlug& scalePlug, 
			C_NullPlug& shearPlug, 
			C_NullPlug& rotatePlug, 
			C_NullPlug& translatePlug, 
			E_RotationOrder order, 
			const Matrix44& matrix,
			bool triggerTopLevelParentOnChanged = true);

		void SetMatrix(
			Matrix44& matrix,
			const C_NullPlug& scalePlug, 
			const C_NullPlug& shearPlug, 
			const C_NullPlug& rotatePlug, 
			const C_NullPlug& translatePlug, 
			E_RotationOrder order,
			bool update = true);

		void SetRotationMatrix(
			Matrix44& matrix,
			const C_NullPlug& rotatePlug, 
			E_RotationOrder order,
			bool update = true);

		void SetRotationMatrixComponents(
			C_NullPlug& rotatePlug, 
			E_RotationOrder order,
			const Matrix44& matrix,
			bool triggerTopLevelParentOnChanged = true);

		bool AreRelated(I_Plug* l, I_Plug* r);

		#include "PlugHelpers.inl"
	}
}

#endif
