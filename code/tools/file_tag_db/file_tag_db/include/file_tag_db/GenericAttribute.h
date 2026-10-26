#ifndef FILE_TAG_DB_GENERIC_ATTRIBUTE_H
#define FILE_TAG_DB_GENERIC_ATTRIBUTE_H

namespace FileTag
{
	class C_GenericAttribute
	{
	public:
		enum E_DataType
		{
			E_DataType_Null		= 0,
			E_DataType_S32		= 1,
			E_DataType_S64		= 2,
			E_DataType_F32		= 3,
			E_DataType_F64		= 4,
			E_DataType_Boolean	= 5,
			E_DataType_String	= 6,
			E_DataType_StringEnum = 7,
			E_DataType_StringFilename = 8
		};

		///\brief Constructor.
		///
		C_GenericAttribute();

		///\brief Destructor.
		///
		~C_GenericAttribute();

		///\brief Copy constructor.
		///
		///@param rhs Class to copy.
		C_GenericAttribute(const C_GenericAttribute &rhs); 

		///\brief assignment operator.
		///
		///@param rhs Class to copy.
		///@return Reference to this class.
		const C_GenericAttribute& operator=(const C_GenericAttribute &rhs);

		///\brief Equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator==(const C_GenericAttribute &rhs) const;

		///\brief Negated equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator!= (const C_GenericAttribute &rhs) const;

		///\brief Less than operator.
		///
		///This test is required in order to be able to store generic attributes within a sorted container.
		///
		///@param rhs The class to test against
		///@return bool Test result.
		bool operator<(const C_GenericAttribute &rhs) const;

		///\brief Overwrites the contents of the generic attribute using another generic attribute.
		///
		///@param rhs The generic attribute to copy.
		void SetValue(const C_GenericAttribute& rhs);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(S32 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(S64 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(F32 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(F64 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(bool value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Util::C_String& value, E_DataType stringDataType);

		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		const Util::C_String& GetName() const;

		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		Util::C_String& GetName();

		///\brief Get the value of this attribute.
		///
		///@param value Reference to variable to be filled with this attribute's value.
		void GetValue(S32& value) const;

		void GetValue(S64& value) const;

		void GetValue(F32& value) const;

		void GetValue(F64& value) const;

		void GetValue(bool& value) const;

		void GetValue(Util::C_String& value) const;

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool etc.
		E_DataType GetDataType() const;

		///\brief Clear the contents of this attribute.
		///
		void Clear();

	private:
		void _OneTimeInit();

		void _SetValue(void* pValue, E_DataType type);

		Util::C_String	m_name;
		E_DataType		m_type;
		void*			m_pValue;
	};
}

#endif
