#ifndef __SYSTEM_SHAREDSTRING_H__
#define __SYSTEM_SHAREDSTRING_H__

namespace SharedString
{

/// instance of a static shared string
class String
{
	friend class StringReferenceCounter;
public:
	String() {}

	String(const String &other);

	const char *Chars() const { return m_psz; }
	operator const char* () const { return m_psz; }

	template<typename T>
	void Serialise(T &ar)
	{
		int n = strlen(m_psz);
		ar << n;
		for (int i=0; i<n; i++)
			ar << m_psz[i];
	}

private:
	String(const char *psz);

	const char *m_psz;
};

/// constructs a globally shared string from the string that was passed in
const String GetString(const char *psz);

template<typename T>
void Deserialise(T &ar, String &s)
{
	Util::C_String tmp;
	ar >> tmp;
	s = GetString(tmp);
}

}

#endif
