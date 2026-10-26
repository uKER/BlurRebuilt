
#ifndef UI_ADOBEELEMENT_H
#define UI_ADOBEELEMENT_H

#include "ui/Illustrator/UIAdobeDefines.h"

namespace UI
{

// Forward declarations.
class C_AdobeGroup;

/// @addtogroup Illustrator
/// @{


//-------------------------------------------------------------------
// Class : C_AdobeElement
//-------------------------------------------------------------------
/// Base class for Adobe elements such as shapes and groups.
class C_AdobeElement
{
public:
	C_AdobeElement(){};

	/// Construct by element type.
	/// @param type - element type.
	C_AdobeElement( Ipf::E_ElementType type )
		: m_ElementType( type ),
		  m_Level( 0 ),
		  m_pParent( NULL )
	{};

	virtual ~C_AdobeElement() {};	 ///< @a


public:
	
	virtual void				Setup( U32 level )
								{
									m_Level = level;
								}

	virtual void				Setup( const char* name )
								{
									SetName( name );
								}

	virtual void				Setup( U32 level, const char* name )
								{
									m_Level = level;
									SetName( name );
								}

	const Ipf::E_ElementType	GetElementType() const	{ return m_ElementType; }	///< @a
								
	const U32					GetLevel() const		{ return m_Level; }			///< @a

	const C_AdobeGroup*			GetParent()	const		{ return m_pParent; }		///< @a

	const Util::C_String&		GetParentName()	const	{ return m_ParentName; }	///< @a

	const Util::C_String&		GetName() const			{ return m_Name; }			///< @a

	void						SetName( const char* name )					{ m_Name = Util::C_String( name ).Lower(); }	/// @a
	void						SetName( const Util::C_String& name )		{ m_Name = name.Lower(); }						/// @a

								/// @a
	void						AttachParent( C_AdobeGroup* pParent, const char* name )
								{ 
									m_pParent = pParent;
									m_ParentName = name;
								}	

protected:
	Util::C_String				m_Name;				///< Name of this element.

	Ipf::E_ElementType			m_ElementType;		///< @a
	U32							m_Level;			///< Layers will have a level of 0.

	C_AdobeGroup*				m_pParent;			///< Pointer to parent. Group or Layer.
	Util::C_String				m_ParentName;		///< Group name containing this element.
};

/// @}

} // namespace UI

#endif // UI_ADOBEELEMENT_H
