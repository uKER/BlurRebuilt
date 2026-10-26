
#ifndef IPF_IPFDATA_H
#define IPF_IPFDATA_H

// Forward declarations.
namespace UI
{
class C_AdobePolygon;
class C_AdobeGroup;
class C_AdobeGradient;
}

namespace Ipf
{

// Forward
class C_IpfWriter;

/// Vector that stores all of the layers in the .ipf
typedef Btl::Vector< UI::C_AdobeGroup* >	AdobeLayerList;		

/// Vector that stores all of the gradients in the .ipf
typedef Btl::Vector< UI::C_AdobeGradient* >	AdobeGradientList;

//-------------------------------------------------------------------
// Class : C_IpfData
//-------------------------------------------------------------------
/// ipf container class.
class C_IpfData
{
public:
	C_IpfData();

	~C_IpfData();

								/// @a
	void						FlushData();

								/// @a
	void						FlushGradients();

								/// @a
	void						Destroy();

								/// @a
	void						AddLayer( const char* name );

								/// @a
	void						AddGroup( const char* name );

								/// @a
	void						CloseGroup();

								/// Returns true if there is some valid data.
	bool						HasDataToWrite();

								/// @a
	void						AddData( UI::C_AdobePolygon* pPolygon );

								/// @a
	void						AddGradient( UI::C_AdobeGradient* pGradient );
	
								/// @a
	const S32					GetNumberOfLayers()	const					{ return m_LayerList.Size(); }

								/// @a
	const S32					GetNumberOfGroups()	const					{ return m_NumGroups; }

								/// @a
	const S32					GetNumberOfShapes()	const					{ return m_NumShapes; }

								/// @a
	const S32					GetNumberOfGradients() const				{ return m_GradientList.Size(); }

	const UI::C_AdobeGroup*		GetCurrentLayer() const						{ return m_pCurrentLayer; }

	const UI::C_AdobeGroup*		GetLayer( const S32 sLayer ) const			{ return m_LayerList[sLayer]; }

	const UI::C_AdobeGradient*	GetGradient( const S32 sGradient ) const	{ return m_GradientList[sGradient]; }

private:
	friend class C_IpfWriter;

	AdobeLayerList				m_LayerList;			///< @a
	AdobeGradientList			m_GradientList;			///< @a

	UI::C_AdobeGroup*			m_pCurrentLayer;		///< Current layer new items belong to.
	UI::C_AdobeGroup*			m_pCurrentGroup;		///< Current groupd new items belong to.
	
	U32							m_CurrentLevel;			///< How far down the heirarchy we are.
	U32							m_NumGroups;			///< @a
	U32							m_NumShapes;			///< @a
};

}; // namespace Ipf

#endif // IPF_IPFDATA_H
