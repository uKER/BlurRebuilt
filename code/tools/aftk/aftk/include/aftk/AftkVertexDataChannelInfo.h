///Stores information related to the mesh vertex channels.
#ifndef AFTK_VERTEX_DATA_CHANNEL_INFO_H
#define AFKT_VERTEX_DATA_CHANNLE_INFO_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_VertexDataChannelInfo
	{
	public:
		///\brief Constructor.
		///
		C_VertexDataChannelInfo()
		{
			m_dataType = E_GenericAttributeType_Null;
		}

		///\brief Destructor.
		///
		~C_VertexDataChannelInfo()
		{
		}

		///\brief Copy constructor.
		///
		///@param rhs the channel info to copy.
		C_VertexDataChannelInfo(const C_VertexDataChannelInfo& rhs)
		{
			*this = rhs;
		}

		///\brief Asignment operator.
		///
		///@param rhs the channel info to copy.
		///@return A reference to this object.
		const C_VertexDataChannelInfo& operator=(const C_VertexDataChannelInfo& rhs)
		{
			m_name = rhs.m_name;
			m_dataType = rhs.m_dataType;

			return *this;
		}

		///\brief Sets the name of this data channel info.
		///
		///@param name The channel name.
		void SetName(const std::string& name)
		{
			m_name = name;
		}

		///\brief Sets the data type stored by the channel.
		///
		///@return The data type.
		void SetDataType(E_GenericAttributeType type)
		{
			m_dataType = type;
		}

		///\brief Returns the name of this channel.
		///
		///@return The channel name.
		const std::string& GetName() const
		{
			return m_name;
		}

		///\brief Returns the data type stored by the channel.
		///
		///@return The data type.
		E_GenericAttributeType GetDataType() const
		{
			return m_dataType;
		}

		bool operator== (const C_VertexDataChannelInfo& rhs) const
		{
			if ((m_name != rhs.m_name) || (m_dataType != rhs.m_dataType))
			{
				return false;
			}

			return true;
		}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		std::string					m_name;
		E_GenericAttributeType	m_dataType;
	#endif
	};
}
#endif
