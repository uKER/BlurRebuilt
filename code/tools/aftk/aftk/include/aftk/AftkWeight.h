#ifndef AFTK_WEIGHT_H
#define AFTK_WEIGHT_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	///Stores a weight. A weight comprises an index into
	///a array of data and a weight for that data.
	template< class T >
	class C_Weight
	{
	public:
		///
		///\brief Constructor.
		///
		C_Weight()
		{
			Clear();
		}

		///
		///\brief Copy constructor.
		///
		///@param rhs The vertex weight to copy
		C_Weight(const C_Weight<T>& rhs)
		{
			*this = rhs;
		}

		///
		///\brief Destructor
		///
		~C_Weight()
		{
		}

		///
		///\brief Asignment operator
		///
		///@param rhs The vertex weight to copy
		///@return A reference to this vertex weight.
		const C_Weight& operator=(const C_Weight<T>& rhs)
		{
			m_index	= rhs.m_index;
			m_weight= rhs.m_weight;

			return *this;
		}

		///
		///\brief Sets the index of this vertex weight.
		///
		///Note that the index value is signed. A value of -1 indicates that a valid
		///weight index has not yet been set.
		///
		///@param index The weighting index.
		void SetIndex(S32 index)
		{
			m_index = index;
		}

		///
		///\brief Sets the weight value.
		///
		///@param weight The weighting value.
		void SetWeight(T weight)
		{
			m_weight = weight;
		}

		///
		///\brief Returns the vertex position index for this weight.
		///
		///Note that a signed value is returned. A value of -1 indicates that a valid
		///index has not yet been set.
		///
		///@return The vertex position index for this weight.
		S32 GetIndex() const
		{
			return m_index;
		}

		///
		///\brief Returns the weight.
		///
		///@return The weighting value.
		T	GetWeight() const
		{
			return m_weight;
		}

		///
		///\brief Clears the contents of this class to their default values.
		///
		///The index is set to -1.
		///The weight is set to 0.
		void Clear()
		{
			m_index	= -1;
			memset(&m_weight, 0, sizeof(T));
		}
		
	#ifndef DOXYGEN_SHOULD_SKIP_THIS

	private:
		S32	m_index;
		T	m_weight;

	#endif
	};
}
#endif
