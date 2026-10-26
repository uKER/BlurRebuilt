#ifndef LUX_GlobalIllumination_MODEL_PROPORTIES
#define LUX_GlobalIllumination_MODEL_PROPORTIES

namespace LUX
{
	class GlobalIlluminationProperties : public LightingModelProperties
	{
	public:
		///
		///Default constructor.
		GlobalIlluminationProperties();
		///
		///Copy constructor.
		GlobalIlluminationProperties(const GlobalIlluminationProperties& rhs);
		///
		///Destructor.
		virtual ~GlobalIlluminationProperties();

		virtual bool						Write(const std::string& fileName) const;

		virtual bool						Read(const std::string& fileName);

		///
		void								SetMethod(LuxModelSimpleMethodEnum method);

		///

		///
		LuxModelSimpleMethodEnum			GetMethod() const;

		virtual LuxModelTypeEnum GetType() const;

		///
		virtual GlobalIlluminationProperties* GetClonePtr() const;
		///

		///
		void Clear();

		void Reset();

		virtual void ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode);

		LuxModelSimpleMethodEnum			m_method;
		int									m_iForceGothamChannels;
		bool								m_bForceShooterChannels;
		bool								m_bBounceLighting;
		bool								m_bOptimize;
		float								m_fBouncePathsPerLight;
		bool								m_bFixSeams;
		bool								m_bFixAllSeams;
		float								m_fCutoff;

	public:
		int									m_iBounceChannel;
	};
}

#endif
