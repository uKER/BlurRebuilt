#ifndef __LUX_SPATIALHIERARCHY_H__
#define __LUX_SPATIALHIERARCHY_H__

namespace LUX
{

// lux specific additional info about a primitive
struct PrimInfo
{
	U16	iRGB;
	int	iChart;
};

inline ost::Engine& operator << (ost::Engine& ar, PrimInfo &T)
{
	ar << T.iRGB << T.iChart;
    return ar;
}

inline ost::Engine& operator >> (ost::Engine& ar, PrimInfo &T)
{
	ar >> T.iRGB >> T.iChart;
    return ar;
}

/// LUX specific derivation of the spatial hierarchy, containing extra data for bounce lighting
class SpatialHierarchy : public SD::SpatialHierarchy
{
public:
					/// Build a spatial hierarchy from a given Aftk::C_Scene
	void			Build(Aftk::C_Scene *pAftkScene, LayerMode *pLayerModes = NULL);

					/// Serialize a spatial hierarchy to an ost::Engine class for loading/saving.
	void			write(ost::Engine &ar);
	void			read(ost::Engine &ar);

					/// Retrieve extra info for a particular primitive
	const PrimInfo	&GetAdditionalPrimInfo(int iPrim) const;

					/// Set extra info for a particular primitive
	void			SetAdditionalPrimInfo(int iPrim, const PrimInfo &info);

private:
	Ut::Vector<PrimInfo>	m_PrimInfo;
};

}; // namespace LUX


#endif
