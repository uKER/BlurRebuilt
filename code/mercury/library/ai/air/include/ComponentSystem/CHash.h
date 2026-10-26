#ifndef __CHASH_H
#define __CHASH_H

#ifdef _MSC_VER
#pragma once
#endif

#define DONT_RECORD_STRING

#include <string>

namespace airacingtech
{
	class CHash
	{
	public:
		CHash();
		CHash(unsigned int hashValue);
		CHash(const char *pString);
		CHash(const CHash &rhs);
		CHash &operator=(const CHash &rhs);

		void Set(const unsigned int hash);

		void CreateHash(const char *pString)
		{
			mHashValue = MakeHash(pString);	
		}

#ifndef DONT_RECORD_STRING
		const std::string &GetHashName() const {return m_HashName;}
#endif

		bool IsValid() const;
		operator unsigned int()	{ return mHashValue; }

		const bool operator<(const CHash &rhs) const;
		const bool operator>(const CHash &rhs) const;
		const bool operator<=(const CHash &rhs) const;
		const bool operator>=(const CHash &rhs) const;
		const bool operator==(const CHash &rhs) const;
		const bool operator!=(const CHash &rhs) const;

		

	private:
		void Mix(unsigned int& a, unsigned int& b, unsigned int& c);

		unsigned int MakeHash(const char *pString);
		unsigned int MakeHashTwo(const char *k);
		unsigned int mHashValue;

#ifndef DONT_RECORD_STRING
		std::string m_HashName;
#endif
	};
}
#endif //__CHASH_H
