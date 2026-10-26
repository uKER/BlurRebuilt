#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>
#include <map>
#include <vector>

std::string Path_GetFileName(std::string file);
std::string Path_GetPath(std::string file);

inline bool isAlpha(char c) 
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline bool isNum(char c)
{
	return c >= '0' && c <= '9';
}

inline bool isCName(char c) 
{
	return isAlpha(c) || c == '_';
}

inline bool isCNameNum(char c) 
{
	return isCName(c) || isNum(c);
}

inline bool isCNum(char c) 
{
	return isNum(c) || c == '.';
}

inline bool isCOperator(char c)
{
	return	c == ':' ||	c == '+' ||	c == '-' ||	
		c == '*' ||	c == '/' ||	c == '%' ||	
		c == '<' ||	c == '>' ||	c == '&' ||	
		c == '|' ||	c == '^' ||	c == '~' ||
		c == '?' ||	c == '!' ||	c == ',' || 
		c == '=' ||	c == ';';
}

inline bool isCBracketStart(char c)
{
	return c == '(' || c == '[' || c == '{';
}

inline bool isCBracketEnd(char c)
{
	return c == ')' || c == ']' || c == '}';
}

inline bool isWS(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline char makeLC(char c)
{
	return c<'A' || c>'Z' ? c : (c-'A')+'a';
}

// Comparison of hashes.
// http://www.cse.yorku.ca/~oz/hash.html
// http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-talk/142054
// I did a test myself out of curiosity.
// Out of 477072 words taken from the spiderman 3 code base there where 24 clashes.
// This one is good enough for our purposes.
inline unsigned long string_hash_sdbm(const char *str,unsigned long hash=1)
{
	// Johan: Slight modification. Start with 1 to avoid "" to become 0 which is a special case.
	int c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

class hString {
	class hash2string_c: public std::map<unsigned long,std::vector<const char *> > {
	public:
		hash2string_c() {}
		~hash2string_c();

		const char *add(const char *szStr);
	};

	static hash2string_c hash2string;

	const char *szStr;
public:
	hString(): szStr(NULL) {
	}
	explicit hString(const char *_szStr,bool /*bAlreadyHashedOveride*/) { szStr = _szStr; }
	explicit hString(const char *_szStr) { *this = _szStr; }
	explicit hString(char c) { char _szStr[2] = {c,0}; *this = _szStr; } 
	
	hString LC() const {
		return hString(lc_c_str(),true);
	}

	const char *c_str() const {
		return szStr;
	}

	const char *lc_c_str() const {
		return ((char **)szStr)[-1];
	}

	bool empty() const { return (szStr == NULL || szStr[0] == 0); }

	hString operator =(const char *_szStr) {
		if(_szStr) {
			szStr = hash2string.add(_szStr);
		} else {
			szStr = NULL;
		}
		return *this;
	}
	hString operator =(const hString str) {
		szStr = str.szStr;
		return *this;
	}

	bool operator !=(const hString b) const {
		return this->szStr != b.szStr;
	}

	bool operator ==(const hString b) const {
		return this->szStr == b.szStr;
	}

	bool operator <(const hString b) const {
		return c_str() < b.c_str();
	}

	bool operator <=(const hString b) const {
		return c_str() <= b.c_str();
	}

	bool operator >(const hString b) const {
		return c_str() > b.c_str();
	}

	bool operator >=(const hString b) const {
		return c_str() >= b.c_str();
	}

};

#endif
