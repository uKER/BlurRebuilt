#ifndef AFTK_UT_NODE_FLAG_PARSER_H
#define AFTK_UT_NODE_FLAG_PARSER_H

namespace AftkUt
{
	enum E_ParamType
	{
		E_ParamType_S32,
		E_ParamType_Bool,
		E_ParamType_F32,
		E_ParamType_Enum,
		E_ParamType_String,
		E_ParamType_Link,
		E_ParamType_Colour,
		E_ParamType_Unknown,

		E_ParamType_Count
	};

	class C_ExtractedPropString
	{
	public:
		C_ExtractedPropString();

		bool GetParameterValue(
			const std::string& name, 
			S32& value) const;

		bool GetParameterValue(
			const std::string& name, 
			bool& value) const;

		bool GetParameterValue(
			const std::string& name, 
			F32& value) const;

		bool GetParameterValue(
			const std::string& name, 
			F32& rValue, 
			F32& gValue, 
			F32& bValue, 
			F32& aValue) const;

		bool GetParameterValue(
			const std::string& name, 
			std::string& value,
			E_ParamType* pType = 0) const;

		E_ParamType GetType(const char* str) const;
		const char* GetTypeName(E_ParamType type) const;
		
	protected:
		bool Find(const std::string &str) const;	// Find a string from the current pos
		S32 FindFirstOf(const std::string &str) const;
		void Reset()	const;
	
		static char*		m_ppParamTypes[10];
		mutable size_t		m_cursor;
		std::string			m_input;

	private:
		C_ExtractedPropString(const std::string InputString);		// Construct with string
		void SetString(const std::string InputString);
	};

	// the C_NodeFlagParser reloaded
	class C_NodeFlagParser: public C_ExtractedPropString
	{
	public:
		/// default constructor
		C_NodeFlagParser();

		C_NodeFlagParser(const std::string &input);

		/// construct with Aftk Node
		C_NodeFlagParser(const Aftk::C_SceneNode& node, bool* pStatus = 0, bool queryBaseInstance = false);

		/// construct with Aftk Node, flag string is update when destructor is called
		C_NodeFlagParser(Aftk::C_SceneNode& node, bool* pStatus = 0, bool queryBaseInstance = false);

		/// destructor
		~C_NodeFlagParser();

		/// set flag string to a node
		void StoreFlags(Aftk::C_SceneNode& node);
		
		/// Set flag name
		bool SetFlagGroupName(const std::string& name);

		/// Set flag category
		bool SetFlagGroupCategory(const std::string& category);

		/// Get flag name
		std::string	GetFlagGroupName() const;

		/// Get flag category
		std::string	GetFlagGroupCategory() const;

		bool SetParameterValue(
			const std::string& name, 
			S32 value);

		bool SetParameterValue(
			const std::string& name, 
			bool value);

		bool SetParameterValue(
			const std::string& name, 
			F32 value);

		bool SetParameterValue(
			const std::string& name, 
			F32 rValue, 
			F32 gValue, 
			F32 bValue, 
			F32 aValue);

		/// Set the value associated with a parameter
		bool SetParameterValue(
			const std::string& name, 
			const std::string& value,
			E_ParamType type);

		/// Delete a parameter by name
		bool DeleteParameter(const std::string& name);

		static void	DeleteFlags(Aftk::C_SceneNode& node);

		static void ClearAllFlags(Aftk::C_SceneNode& node);

		/// check whether the given node has flag data attached to it
		static bool IsFlagged(const Aftk::C_SceneNode& node, bool queryBaseInstance = false);

		static bool IsFlaggedBaseInstance(const Aftk::C_SceneNode& node);

		static bool IsFlaggedInstance(const Aftk::C_SceneNode& node);

		bool GetFlagString(
			std::string& flagString, 
			bool* pReadOnly = 0);

		/// get flag string from a node
		bool GetFlagString(
			const Aftk::C_SceneNode& node, 
			bool queryBaseInstance = false);

	private:
		enum E_Version
		{
			E_Version_1,
			E_Version_2,
			E_Version_3,

			E_Version_Unknown,
			E_Version_Count
		};

		/// construct with input string
		C_NodeFlagParser(
			const std::string &input,
			E_Version version, 
			bool* pStatus,
			bool queryBaseInstance);

		static bool	SetFlagString(Aftk::C_SceneNode& node, const std::string& string);

		/// Find a string from the current pos
		//bool Find(const std::string &str) const;

		/// Find first occurence of any character in the input string, starting from the current pos
		//S32	FindFirstOf(const std::string &str) const;

		void Initialise();

		bool ConvertFlags(E_Version version);

		static bool ConvertFlag1String(std::string& string);

		static bool ConvertFlag2String(std::string& string);

		static void	AddLegacyAttribute(Aftk::C_SceneNode& node, Aftk::C_GenericAttribute& attribute);

		static std::string GetBlindDataFlagString(const Aftk::C_SceneNode& node);

		bool ConvertFlagParam1(std::string& newParam) const;

		bool ConvertFlagParam2(std::string& newParam) const;

		static bool IsBlindDataFlagged(const Aftk::C_SceneNode& node);

		std::string GetValueForNameValuePair(const std::string& name, E_Version version) const;

		std::string GetTypeForFlag2(std::string& newParam) const;

		/// Set the flag string
		void SetFlagString(const std::string &input);

		bool DeleteParameter(const std::string& type, const std::string& name);

		static bool IsThisFlagged(const Aftk::C_SceneNode& node);

		static Aftk::C_SceneNode* GetInstanceSourceNodePtr(Aftk::C_SceneNode& node);

		static const Aftk::C_SceneNode* GetInstanceSourceNodePtr(const Aftk::C_SceneNode& node);

		/// Get the flag string
		static bool GetThisFlagString(	
			std::string& flagString, 
			Aftk::C_SceneNode& node);

		static bool GetFlagString(	
			std::string& flagString, 
			const Aftk::C_SceneNode& node,
			bool* pReadOnly,
			bool queryBaseInstance);

		static std::string	m_default;
		bool				m_bStringModified;
		Aftk::C_SceneNode*	m_pNode;
		bool				m_readOnly;
	};
}

#endif
