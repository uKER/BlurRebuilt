
#ifndef UI_UIADOBEGROUP_H
#define UI_UIADOBEGROUP_H

#include "ui/Illustrator/UIAdobeElement.h"
#include "ui/Illustrator/UIAdobePolygon.h"

#include <btl/Containers/containers.h>

namespace UI
{

class C_AdobeShape;

/// @addtogroup Illustrator
/// @{

//-------------------------------------------------------------------
// Class : C_AdobeGroup
//-------------------------------------------------------------------
/// Container class to store adobe elements.
class C_AdobeGroup : public C_AdobeElement
{	
public:
	/// UtVector that can store pointers to adobe elements.
	typedef Btl::Vector< C_AdobeElement* >	AdobeElementList;

public:
	C_AdobeGroup();

	~C_AdobeGroup();																	///< @a

public:
							/// Remove but doesnt delete all of the elements in the list.
	void					RemoveElements();

							/// Removes and deallocates memory.
	void					DeleteElements();

	void					AddElement( C_AdobeElement* pElement );							///< @a

	void					SetCapacity( S32 capacity );									///< @a

	const C_AdobeElement*	GetElementPtr( const S32 index ) const;							///< @a

	const C_AdobeElement*	GetElementPtrByName( const char* name ) const;					///< @a

	const S32				GetSize() const				{ return m_ElementList.Size(); }	///< @a

	const S32				GetNumOfElements() const	{ return m_ElementList.Size(); }	///< @a

	const S32				GetNumOfShapes() const		{ return m_NumberOfShapes; }		///< @a

	const S32				GetNumOfGroups() const		{ return m_NumberOfGroups; }		///< @a

							/// Returns the group in order found. Not by index because the group can contain
							/// shapes and groups.
	const C_AdobeGroup*		GetGroup( const S32 group ) const;

							/// Returns the group matching the name given. Returns null if it cant be found.
	const C_AdobeGroup*		GetGroupByName( const char* name ) const;

							/// Returns the shape in order found. Not by index because the group can contain
							/// shapes and groups.
	const C_AdobeShape*		GetShape( const S32 shape ) const;

public:
	// Make this public for ease of access.
	C_AdobeGroup*		m_pParent;														///< @a

private:
	AdobeElementList	m_ElementList;													///< @a
	S32					m_NumberOfShapes;
	S32					m_NumberOfGroups;
};

/// @}

}// namespace UI

#endif // UI_UIADOBEGROUP_H
