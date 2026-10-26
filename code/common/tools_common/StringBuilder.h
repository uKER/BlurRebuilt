#ifndef TU_STRING_BUILDER_H
#define TU_STRING_BUILDER_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Tu
{
	class StringBuilder
	{
	public:
		StringBuilder()
		{
		}

		~StringBuilder()
		{
		}

		void Clear()
		{
			m_string.clear();
		}

		StringBuilder& operator<<(const char* string) 
		{
			m_string += string;
			return *this;
		}

		StringBuilder& operator<<(const std::string& string) 
		{
			m_string += string;
			return *this;
		}

		StringBuilder& operator<<(const U8 var) 
		{
			std::ostringstream stream;
			stream << var;
			m_string += stream.str();

			return *this;
		}

		StringBuilder& operator<<(const U16 var) 
		{
			std::ostringstream stream;
			stream << var;
			m_string += stream.str();

			return *this;
		}

		StringBuilder& operator<<(const U32 var) 
		{
			std::ostringstream stream;
			stream << var;
			m_string += stream.str();

			return *this;
		}

		StringBuilder& operator<<(const S8 var) 
		{
			std::ostringstream stream;
			stream << var;
			m_string += stream.str();

			return *this;
		}

		StringBuilder& operator<<(const S16 var) 
		{
			std::ostringstream stream;
			stream << var;
			m_string += stream.str();

			return *this;
		}

		StringBuilder& operator<<(const S32 var) 
		{
			std::ostringstream stream;
			stream << var;
			m_string += stream.str();

			return *this;
		}

		StringBuilder& operator<<(const F32 var) 
		{
			std::ostringstream stream;
			stream << var;
			m_string += stream.str();

			return *this;
		}

		StringBuilder& operator<<(const F64 var) 
		{
			std::ostringstream stream;
			stream << var;
			m_string += stream.str();

			return *this;
		}
/*
		template <class T, int N>
		StringBuilder& operator <<(const UtMaths::BaseVector<T, N>& value)
		{
			for (U32 i = 0; i < value.SIZE; i++)
			{
				*this << value[i];
				if (i < value.SIZE-1)
				{
					*this << " ";
				}
			}

			return*	this;
		}

		template <class T, int N>
		StringBuilder& operator <<(const UtMaths::BasicMatrix<T, N>& value) 
		{
			for (U32 i = 0; i < value.SIZE; i++)
			{
				*this << "Row ";
				*this << i;
				*this << " = [";
				*this << value[i];
				if (i < value.SIZE-1)
				{
					*this << "]\n";
				}
			}
			*this << "]\n";

			return*	this;
		}
*/
		const std::string& Text() const
		{
			return m_string;
		}

	private:
		std::string	m_string;
	};
}

#endif
