#ifndef SZ_H
#define SZ_H

#ifdef _MSC_VER
#pragma once
#endif

//
// SzMem
//

template<typename Char>
void SzMemCpy(Char* dst, const Char* src, size_t count)
{
	memcpy(dst, src, count * sizeof(Char));
}

template<typename Char>
void SzMemMove(Char* dst, const Char* src, size_t count)
{
	memmove(dst, src, count * sizeof(Char));
}

//
// SzErrorCode
//

enum SzErrorCode
{
	kSzOkay				= 0,
	kSzOutOfMemory		= -1,
	kSzInvalidParameter	= -2
};

//
// Sz<Char>
//

template<typename Char>
class Sz
{
public:
	inline Sz()
	{
		_d.Init();
	}

	inline Sz(const Sz<Char>& sz)
	{
		_d.Init();
		Set(&sz);
	}

	inline Sz(const Char* text)
	{
		_d.Init();

		if (!Set(text))
		{
			assert(0);
		}
	}

	inline Sz(const Char* text, size_t length)
	{
		_d.Init();

		if (!Set(text, length))
		{
			assert(0);
		}
	}

	inline ~Sz()
	{
		if (_d.IsExternal())
			_d.ex->Release();
	}

public:
	inline Sz<Char>& operator=(const Sz<Char>& sz)
	{
		Set(&sz);
		return *this;
	}

	inline Sz<Char>& operator=(const Char* text)
	{
		if (!Set(text))
		{
			assert(0);
		}

		return *this;
	}

public:
	inline size_t Length() const
	{
		return _d.IsExternal() ? _d.ex->length : _d.InLength();
	}

	inline const Char* Text() const
	{
		return _d.IsExternal() ? _d.ex->text : _d.InText();
	}

	inline operator const Char*() const
	{
		return Text();
	}

	inline Char operator[](size_t index) const
	{
		if (_d.IsExternal())
			return (index < _d.ex->length) ? _d.ex->text[index] : '\0';
		else
			return (index < _d.InLength()) ? _d.InText()[index] : '\0';
	}

	inline Char operator[](int index) const
	{
		return operator[]((size_t)index);
	}

public:
	void Set(const Sz<Char>* sz);
	bool Set(const Char* text);
	bool Set(const Char* text, size_t length);
	void Clear();

public:
	Char* Capture(size_t length, bool persistContent);
	void Truncate(size_t length);

private:
	struct External
	{
		size_t	references;
		size_t	length;
		size_t	capacity;
		Char	text[1];

	public:
		static External* Create(size_t length);
		void Acquire();
		void Release();
	};

	struct Const
	{
		static const size_t kDataSize			= 16;
		static const size_t kDataInSize			= (kDataSize - sizeof(struct External*)) / sizeof(Char);
		static const size_t kInternalOverflow	= (kDataSize / sizeof(Char)) - 1;
		static const size_t kInternalCapacity	= kInternalOverflow - 1;
		static const size_t kExternalSize		= (size_t)&((External*)0)->text - (size_t)&((External*)0)->references;
	};

	struct Data
	{
		Char		in[Const::kDataInSize];
		External*	ex;

	public:
		inline void Init()						{ in[0] = 0; in[1] = '\0';						}
		inline bool IsExternal() const			{ return in[0] == Const::kInternalOverflow;		}
		inline Char* InText()					{ return &in[1];								}
		inline const Char* InText() const		{ return &in[1];								}
		inline size_t InLength() const			{ return in[0];									}
		inline void SetInLength(size_t length)	{ in[0] = (Char)length; in[length + 1] = '\0';	}
		inline void SetEx(External* ex_)		{ in[0] = Const::kInternalOverflow; ex = ex_;	}
		inline void SetExLength(size_t length)	{ ex->length = length; ex->text[length] = '\0';	}
	};

private:
	Data _d;
};

//
// Sz8 / Sz16
//

typedef Sz<char> Sz8;
typedef Sz<wchar_t> Sz16;

//
// Helpers
//

template<typename Char> Sz<Char> SzConst(const Char* text);
template<typename Char> Sz<Char> SzError(SzErrorCode errorCode);
template<typename Char> size_t SzLength(const Char* text);

//
// String Operators
//

// +

template<typename Char>
inline Sz<Char> operator+(const Sz<Char>& lhs, const Sz<Char>& rhs)
{
	return Concat<Char>(lhs.Text(), lhs.Length(), rhs.Text(), rhs.Length());
}

template<typename Char>
inline Sz<Char> operator+(const Sz<Char>& lhs, const Char* rhs)
{
	return Concat<Char>(lhs.Text(), lhs.Length(), rhs, SzLength(rhs));
}

template<typename Char>
inline Sz<Char> operator+(const Char* lhs, const Sz<Char>& rhs)
{
	return Concat<Char>(lhs, SzLength(lhs), rhs.Text(), rhs.Length());
}

template<typename Char>
inline Sz<Char> operator+(const Sz<Char>& lhs, Char rhs)
{
	return Concat<Char>(lhs.Text(), lhs.Length(), &rhs, 1);
}

template<typename Char>
inline Sz<Char> operator+(Char lhs, const Sz<Char>& rhs)
{
	return Concat<Char>(&lhs, 1, rhs.Text(), rhs.Length());
}

// +=

template<typename Char>
inline Sz<Char>& operator+=(Sz<Char>& lhs, const Sz<Char>& rhs)
{
	if (!Append(lhs, rhs))
	{
		assert(0);
	}

	return lhs;
}

template<typename Char>
inline Sz<Char>& operator+=(Sz<Char>& lhs, const Char* rhs)
{
	if (!Append(lhs, Sz<Char>(rhs)))
	{
		assert(0);
	}

	return lhs;
}

template<typename Char>
inline Sz<Char>& operator+=(Sz<Char>& lhs, Char rhs)
{
	if (!Append(lhs, Sz<Char>(&rhs, 1)))
	{
		assert(0);
	}

	return lhs;
}

//
// String Manipulation
//

template<typename Char> Sz<Char> Concat(const Char* lText, size_t lLength, const Char* rText, size_t rLength);
template<typename Char> Sz<Char> ConcatN(int count, const Char** strings, const size_t* lengths);
template<typename Char> bool Append(Sz<Char>& text, const Sz<Char>& suffix);
template<typename Char> bool Append(Sz<Char>& text, const Char* suffixText, size_t suffixLength);

//
// String Operations
//

template<typename Char> bool SplitPath(const Char* fullPath, Sz<Char>& path, Sz<Char>& file, Sz<Char>& ext);
template<typename Char> Sz<Char> SplitFileExt(const Char* fullPath);
template<typename Char> bool CleanUpFilename(Sz<Char>& filename, bool useForwardSlashes);
template<typename Char> bool CleanUpPath(Sz<Char>& path, bool useForwardSlashes, bool addTrailingSlash);
template<typename Char> Sz<Char> SplitIntercaps(const Sz<Char>& text);
template<typename Char> Sz<Char> CleanUpLabel(const Sz<Char>& text);
template<typename Char> Sz<Char> FormatV(const Char* format, va_list vl);
template<typename Char> Sz<Char> Format(const Char* format, ...);
template<typename Char> Sz<Char> Fill(Char character, size_t count);
template<typename Char> Sz<Char> Fill(const Char* pattern, size_t count);
template<typename Char> Sz<Char> PadStart(const Sz<Char>& sz, Char character, size_t count);
template<typename Char> Sz<Char> PadStart(const Sz<Char>& sz, const Char* pattern, size_t count);
template<typename Char> Sz<Char> PadEnd(const Sz<Char>& sz, Char character, size_t count);
template<typename Char> Sz<Char> PadEnd(const Sz<Char>& sz, const Char* pattern, size_t count);
template<typename Char> Sz<Char> Substring(const Sz<Char>& sz, size_t i, size_t j);
template<typename Char> Sz<Char> Substring(const Sz<Char>& sz, size_t i);
template<typename Char> Sz<Char> Replace(const Sz<Char>&sz, Char from, Char to);
template<typename Char> int IndexOf(const Sz<Char>& sz, const Char* pattern);
template<typename Char> int LastIndexOf(const Sz<Char>& sz, const Char* pattern);
template<typename Char> int IndexOfStringI(const Sz<Char>& sz, const Char* substring);
template<typename Char> bool StartsWithI(const Sz<Char>& sz, const Char* substring);
template<typename Char> bool EndsWithI(const Sz<Char>& sz, const Char* substring);

Sz8 WcToMb(const Sz16& sz);
Sz16 MbToWc(const Sz8& sz);

//
// String operators
//

// Sz8

inline bool operator==(const Sz8& lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) == 0; }
inline bool operator==(const Sz8& lhs, const char* rhs)	{ return strcmp(lhs, rhs) == 0; }
inline bool operator==(const char* lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) == 0; }

inline bool operator!=(const Sz8& lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) != 0; }
inline bool operator!=(const Sz8& lhs, const char* rhs)	{ return strcmp(lhs, rhs) != 0; }
inline bool operator!=(const char* lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) != 0; }

inline bool operator<(const Sz8& lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) < 0; }
inline bool operator<(const Sz8& lhs, const char* rhs)	{ return strcmp(lhs, rhs) < 0; }
inline bool operator<(const char* lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) < 0; }

inline bool operator>(const Sz8& lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) > 0; }
inline bool operator>(const Sz8& lhs, const char* rhs)	{ return strcmp(lhs, rhs) > 0; }
inline bool operator>(const char* lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) > 0; }

inline bool operator<=(const Sz8& lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) <= 0; }
inline bool operator<=(const Sz8& lhs, const char* rhs)	{ return strcmp(lhs, rhs) <= 0; }
inline bool operator<=(const char* lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) <= 0; }

inline bool operator>=(const Sz8& lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) >= 0; }
inline bool operator>=(const Sz8& lhs, const char* rhs)	{ return strcmp(lhs, rhs) >= 0; }
inline bool operator>=(const char* lhs, const Sz8& rhs)	{ return strcmp(lhs, rhs) >= 0; }

// Sz16

inline bool operator==(const Sz16& lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) == 0; }
inline bool operator==(const Sz16& lhs, const wchar_t* rhs)	{ return lstrcmpW(lhs, rhs) == 0; }
inline bool operator==(const wchar_t* lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) == 0; }

inline bool operator!=(const Sz16& lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) != 0; }
inline bool operator!=(const Sz16& lhs, const wchar_t* rhs)	{ return lstrcmpW(lhs, rhs) != 0; }
inline bool operator!=(const wchar_t* lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) != 0; }

inline bool operator<(const Sz16& lhs, const Sz16& rhs)		{ return lstrcmpW(lhs, rhs) < 0; }
inline bool operator<(const Sz16& lhs, const wchar_t* rhs)	{ return lstrcmpW(lhs, rhs) < 0; }
inline bool operator<(const wchar_t* lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) < 0; }

inline bool operator>(const Sz16& lhs, const Sz16& rhs)		{ return lstrcmpW(lhs, rhs) > 0; }
inline bool operator>(const Sz16& lhs, const wchar_t* rhs)	{ return lstrcmpW(lhs, rhs) > 0; }
inline bool operator>(const wchar_t* lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) > 0; }

inline bool operator<=(const Sz16& lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) <= 0; }
inline bool operator<=(const Sz16& lhs, const wchar_t* rhs)	{ return lstrcmpW(lhs, rhs) <= 0; }
inline bool operator<=(const wchar_t* lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) <= 0; }

inline bool operator>=(const Sz16& lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) >= 0; }
inline bool operator>=(const Sz16& lhs, const wchar_t* rhs)	{ return lstrcmpW(lhs, rhs) >= 0; }
inline bool operator>=(const wchar_t* lhs, const Sz16& rhs)	{ return lstrcmpW(lhs, rhs) >= 0; }

#endif // SZ_H
