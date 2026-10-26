#ifndef AFTK_OBJECT_H
#define AFTK_OBJECT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class I_Plug;
	class C_NativeIPlug;

	class I_Object
	{
	public:
		///\brief Destructor.
		///
		virtual ~I_Object();

		///\brief Get this objects's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this object. For example, spline, camera actor etc.
		///
		///@return object's type enum.
		virtual E_ObjType GetType() const = 0;

		///\brief Returns true if this object implements the given type.
		///
		///@param The object type to test against.
		///@return true if we implement the given object type.
		virtual bool HasType(E_ObjType type) const = 0;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const = 0;

		///\brief Retruns the name of this object.
		///
		///@return object name.
		virtual const std::string& GetName() const = 0;

		virtual std::string GetPath() const = 0;

		U32 GetNumConnectedPlugs() const;

		const Ut::UtVector<Ut::DynamicVector<I_Plug*> >& GetConnectedPlugs() const;

	protected:
		bool AddConnectedPlugPtr(I_Plug* pPlug);

		void DeleteConnectedPlugPtr(I_Plug* pPlug);
		//Track everything connected to this node so that we can dettach ourselves during as
		//part of our destructor.
		Ut::UtVector<Ut::DynamicVector<I_Plug*> > m_connectedPlugs;

		FRIEND_CLASS I_Plug;
		FRIEND_CLASS C_NativeIPlug;
	};
}

#endif
