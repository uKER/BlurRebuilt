#ifndef AFTK_GENERIC_ATTRIBUTE_H
#define AFTK_GENERIC_ATTRIBUTE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_BlobData;

	class C_GenericAttribute
	{
	public:

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

		///\brief Sets the name of this attribute.
		///
		///@param name Attribute name.
		void SetName(const std::string &name);

		///\brief Overwrites the contents of the generic attribute using another generic attribute.
		///
		///@param rhs The generic attribute to copy.
		void SetValue(const C_GenericAttribute& rhs);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(U8 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(U16 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(U32 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(U64 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(S8 value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(S16 value);

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
		void SetValue(const Vector4F64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4F32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4S32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4S64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4S16& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4S8& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4U32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4U64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4U16& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector4U8& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3F64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3F32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3S32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3S64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3S16& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3S8& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3U32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3U64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3U16& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector3U8& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2F64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2F32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2S64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2S32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2S16& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2S8& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2U64& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2U32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2U16& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Vector2U8& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Matrix44F32& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(bool value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const char* value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const std::string& value);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const C_BlobData& value);

		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		const std::string& GetName() const;

		///\brief Get the value of this attribute.
		///
		///@param value Reference to variable to be filled with this attribute's value.
		template <class T>
		void GetValue(T& value) const
		{
			value = *(T*)m_pValue;
		}

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool, Vector3F32 etc.
		E_GenericAttributeType GetDataType() const;

		U32 GetHash() const;

		static void Output(const C_GenericAttribute& genericAttribute);

		///\brief Clear the contents of this attribute.
		///
		virtual void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void SetValue(void* pValue, E_GenericAttributeType type)
		{
			m_pValue = pValue;
			m_type = type;
		}

		std::string					m_name;
		E_GenericAttributeType		m_type;
		void*						m_pValue;

		friend class C_NativeGenericAttribute;

	#endif
	};
}

#endif
