#pragma once

namespace LUX
{

class RenderChannel : public BaseObject
{
public:
	DECLARE_PERSISTENCE(RenderChannel);

	typedef enum Target
	{
        None, Red, Green, Blue, RGB, Alpha,		// generic RGBA
		Sun, Sky,								// sun and sky channels for The Club
		AnimatedOn, AnimatedOff					// animated channels for PGR4 (iSubChannel stores the animation settings index)
	};

					RenderChannel() { };				// default constructor for serialization
					RenderChannel(RenderChannel &ref);	// copy constructor
					RenderChannel(bool bDefault);		// create the "default" renderchannel with default settings

	void			ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode);

	// settings
	Target			m_target;
	int				m_iSubChannel;
	std::string		m_name;

	void write(ost::Engine &ar)
	{
		ar << m_name;
		ar << m_target;
		ar << m_iSubChannel;
	}
	void read(ost::Engine &ar)
	{
		ar >> m_name;
		ar >> *(int*)&m_target;
		ar >> m_iSubChannel;
	}
};

}
