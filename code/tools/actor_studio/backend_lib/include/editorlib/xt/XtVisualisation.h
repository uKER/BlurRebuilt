#ifndef EDITOR_LIB_XT_VISUALISATION_H
#define EDITOR_LIB_XT_VISUALISATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

#include <Btl/Utility/String.h>

#include <EditorLib/Misc/Status.h>

namespace Sys
{
	class C_XmlDomElement;
}

namespace EditorLib
{
	enum XtVisualisationType
	{
		XtVisualisationUnknown,
		XtVisualisationMesh,
		XtVisualisationSprite		
	};

	struct XtVisMeshInfo
	{
		XtVisMeshInfo();
		XtVisMeshInfo(const XtVisMeshInfo& r);
		XtVisMeshInfo(const Btl::String& meshGuid);
		XtVisMeshInfo(const Btl::String& stringKey, U32 aftkUniqueId);

		void GenerateAftkUniqueMeshGuid(Btl::String& stringKey);

		inline bool operator==(const XtVisMeshInfo& r) const
		{
			return stringKey == r.stringKey && aftkUniqueId == r.aftkUniqueId;
		}

		const XtVisMeshInfo& operator=(const XtVisMeshInfo& r);

		Btl::String	stringKey;
		U32			aftkUniqueId;
	};

	struct XtVisualisationDef
	{
		XtVisualisationDef() {}

		XtVisualisationType visualisationType;
		Btl::String		filePath;
		F32 scale;
		Btl::String		id;

		XtVisualisationDef* Clone();

		inline bool operator==(const XtVisualisationDef& r) const
		{
			return filePath == r.filePath && id == r.id;
		}

	private:
		XtVisualisationDef(const XtVisualisationDef& rhs);
		const XtVisualisationDef& operator=(const XtVisualisationDef& rhs);
	};

	struct C_XtVisualisationRootFolder
	{
		C_XtVisualisationRootFolder()
		{
			abSync = false;
		}

		C_XtVisualisationRootFolder(const C_XtVisualisationRootFolder& rhs)
		{
			*this = rhs;
		}

		const C_XtVisualisationRootFolder& operator=(const C_XtVisualisationRootFolder& rhs)
		{
			path = rhs.path;
			abSync = rhs.abSync;
			return *this;
		}

		Btl::String path;
		bool abSync;
	};

	class C_XtVisualisationData
	{
	public:
		C_XtVisualisationData();
		~C_XtVisualisationData();

		C_Status RegisterDefinitions(const char* pConfigPath);
		C_Status UnregisterDefinitions();

		C_Status RescanFolders();

		const XtVisualisationDef* GetXtVisualisationDef(const I_DgNode*) const;
		void BuildRootFolderList(Btl::Vector<C_XtVisualisationRootFolder>& folderList) const;
		void BuildResourceFileList(Btl::Vector<Btl::String>& fileList) const;
		const XtVisualisationDef* FindByFilePath(const char *pOriginalFilePath) const;

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			void Connect();
			void Disconnect();

			void OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent e);

			void OnAftkRefLoadedSlot(
				const Btl::String& originalFilePath, 
				const Aftk::C_Scene& aftkScene, 
				const EditorLib::C_SceneNode& sceneNode, 
				const EditorLib::AftkDgMap& aftkDgMap);
			void OnAftkRefUnloadedSlot(
				const Btl::String& originalFilePath);

			FRIEND_CLASS C_XtVisualisationData;
		};

		void AddMapping(const char* pId, XtVisualisationType visualisationType, const char* fileName, const char* scale);
		void ParseVisualisation(const char* pId, const Sys::C_XmlDomElement* pVisualisation);
		void ParseVisualisationFolder(const Sys::C_XmlDomElement* pVisualisationFolder);
		void ParseVisualisationFolder(const Btl::String& root, const Sys::C_DirectoryInfo& directoryInfo, bool recursive);
		void RemapVisualisationByExtActors(const Btl::String& originalFilePath);
		void RemapVisualisationByExtActor(
			XtVisualisationDef* pDef,
			const Btl::String& originalFilePath,
			const Btl::String& xml);
		void RevertRemap();

		typedef Btl::OpenHashMap<Btl::String, XtVisualisationDef*, Btl::HashKeyOpBtlString> 
			VisualisationMap;

		struct C_State
		{
			~C_State();
			C_State* Clone() const;

			Btl::Vector<C_XtVisualisationRootFolder>			
												visualisationFolders;
			Btl::Vector<XtVisualisationDef*>	definitions;
			VisualisationMap					mappingById;
			VisualisationMap					mappingByFilePath;
		};

		C_State*							m_pPristineState;
		C_State*							m_pState;
		Btl::String							m_configFilePath;
		C_Slots								m_slots;
	};
}

#endif