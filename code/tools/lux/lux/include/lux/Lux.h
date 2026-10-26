#ifndef LUX_H
#define LUX_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}
namespace Progress
{
	class I_ProgressBar;
};

class LightingPass;

namespace LUX
{
	class LightingModel;
	class SamplePoints;

	typedef bool ProgressCallbackProc(void *pThis, float fProgress, char *pszDescription);

	class Lux
	{
	public:
		///
		///Default constructor.
		Lux();
		///
		///Destructor
		~Lux();
		///
		///Light the given scene
		bool							Light(Progress::I_ProgressBar *);
		///
		void							SetAftkScene(Aftk::C_Scene *pAftkScene);
		///
		void							SetSpatialHierarchy(LUX::SpatialHierarchy* pHierachy);
		///
		void							SetLightingModelPropertiesPtr(LightingModelProperties* pProperties);
		///
		void							SetInputSceneFileName(const std::string& fileName);
		void							SetOutputSceneFileName(const std::string& fileName);
		void							SetLightingModelFileName(const std::string& fileName);

		void							SetLightMapFolder(const std::string& folder);
		void							SetLightMapFilePrefix(const std::string& prefix);
		void							SetLightMapFileType(LUX::LuxLightMapFileTypeEnum type);
		///
		Aftk::C_Scene*					GetAftkScene();
		///
		const LUX::SpatialHierarchy*	GetSpatialHierarchyPtr() const;
		///
		const LightingModelProperties*	GetLightingModelPropertiesPtr() const;
		LightingModelProperties*		GetLightingModelPropertiesPtr();

		const LightingModel*			GetLightingModelPtr() const;

		const std::string&				GetInputSceneFileName() const;
		const std::string&				GetOutputSceneFileName() const;
		const std::string&				GetLightingModelFileName() const;

		const std::string&				GetLightMapFolder() const;
		const std::string&				GetLightMapFilePrefix() const;
		LUX::LuxLightMapFileTypeEnum	GetLightMapFileType() const;
		SamplePoints					*GetSamplePointsPtr() const;
		
		void							GetLayerUsage(SD::SpatialHierarchy::LayerMode *piLayerUsage);

		bool							ReadProjectProperties(const std::string& fileName);
		bool							WriteProjectProperties(const std::string& fileName) const;

		bool							WriteLightingModelProperties(const std::string& filename) const;
		bool							ReadLightingModelProperties();

		void							SetProgressCallback(void *pThis, ProgressCallbackProc *p);
		bool							DoProgressCallback(float f, char *psz) const;

		int								GetNumPasses();
		void							SetCurrentPass(int i);

		const Ut::Vector<LightingPass*>	*GetLightingPassVector() const;
		Ut::BitArray128					GetLightingPassBits(int iPass) const;

		///
		///Clears up any memory allocated by this class
		void Clear();

		mutable int						m_iAtlasOffset;

	private:
		///
		///Copy constructor
		Lux(const Lux& rhs);

		void Reset();

		void							CreateLightingModelFromXML(XERCES_CPP_NAMESPACE::DOMNode *pNode);
		void							ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode);
		void							ParseAdditionalXMLData(XERCES_CPP_NAMESPACE::DOMNode *pNode);

		int								m_iCurrentPass;

		Aftk::C_Scene*						m_pAftkScene;
		bool							m_bCommandLineMode;

		LUX::SpatialHierarchy*			m_pHierarchy;
		LightingModelProperties*		m_pProperties;

		void							*m_ProgressContext;
		ProgressCallbackProc			*m_ProgressCallback;
		LightingModel*					m_pLightingModel;

		std::string						m_lightMapFolder;
		std::string						m_lightMapFilePrefix;
		LUX::LuxLightMapFileTypeEnum	m_lightMapFileType;

		std::string						m_inputSceneFileName;
		std::string						m_outputSceneFileName;
		std::string						m_lightingModelFileName;

		SamplePoints					*m_pSamplePoints;
		Ut::Vector<LightingPass*>		m_LightingPasses;

    	//Pointer to a function that provides a logging facility.
		void (*m_pLogCallbackFunc)(const std::string& text);
	};
}

#endif
