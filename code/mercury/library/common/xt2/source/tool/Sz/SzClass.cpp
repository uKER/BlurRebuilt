#include "Common/XtPch.h"
#include <Xt/XtTool.h>

// SzConst(sz) -> Make constant string (read only)
// SzError(sz) -> Make error flagged string (read only), use ExtractError(sz) to get pointer to char*, otherwise just returns empty string

//
// Sz<Char> helpers
//

template<typename Type> inline Type Max(const Type& lhs, const Type& rhs)
{
	return (rhs > lhs) ? rhs : lhs;
}

template<typename Type> inline Type Min(const Type& lhs, const Type& rhs)
{
	return (lhs > rhs) ? rhs : lhs;
}

template<typename Type> inline Type SzAlignUpper(Type value, Type alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

void* SzAlloc(size_t count)
{
	return new char[count];
}

void SzFree(void* p)
{
	delete [] (char*)p;
}

//
// Sz<Char> class
//

template<typename Char>
void Sz<Char>::Set(const Sz<Char>* sz)
{
	if (_d.IsExternal())
	{
		External* ex = _d.ex;

		if (sz->_d.IsExternal())
		{
			_d.SetEx(sz->_d.ex);
			_d.ex->Acquire();
		}
		else
			_d = sz->_d;

		ex->Release();
	}
	else
	{
		if (sz->_d.IsExternal())
		{
			_d.SetEx(sz->_d.ex);
			_d.ex->Acquire();
		}
		else
			_d = sz->_d;
	}
}

template<typename Char>
bool Sz<Char>::Set(const Char* text)
{
	return Set(text, text ? SzLength(text) : 0);
}

template<typename Char>
bool Sz<Char>::Set(const Char* text, size_t length)
{
	if (length <= Const::kInternalCapacity)
	{
		if (_d.IsExternal())
		{
			External* ex = _d.ex;

			SzMemCpy(_d.InText(), text, length);

			ex->Release();
		}
		else
			SzMemMove(_d.InText(), text, length);

		_d.SetInLength(length);

		return true;
	}

	if (_d.IsExternal() && (_d.ex->references == 1) && (length <= _d.ex->capacity) && ((length + 15) >= (_d.ex->capacity / 2)))
	{
		SzMemMove(_d.ex->text, text, length);

		_d.SetExLength(length);

		return true;
	}

	External* ex = External::Create(length);

	if (!ex)
		return false;

	SzMemCpy(ex->text, text, length);

	if (_d.IsExternal())
		_d.ex->Release();

	_d.SetEx(ex);

	return true;
}

template<typename Char>
void Sz<Char>::Clear()
{
	if (_d.IsExternal())
		_d.ex->Release();

	_d.Init();
}

template<typename Char>
Char* Sz<Char>::Capture(size_t length, bool persistContent)
{
	if (length <= Const::kInternalCapacity)
	{
		if (_d.IsExternal())
		{
			External* ex = _d.ex;

			if (persistContent)
				SzMemCpy(_d.InText(), ex->text, Min(length, 1 + ex->length));

			ex->Release();
		}

		_d.SetInLength(length);

		return _d.InText();
	}

	if (_d.IsExternal() && (_d.ex->references == 1) && (length <= _d.ex->capacity) && ((length + 15) >= (_d.ex->capacity / 2)))
	{
		_d.SetExLength(length);

		return _d.ex->text;
	}

	External* ex = External::Create(length);

	if (!ex)
		return 0;

	if (persistContent)
		SzMemCpy(ex->text, Text(), Min(length, 1 + Length()));

	if (_d.IsExternal())
		_d.ex->Release();

	_d.SetEx(ex);

	return ex->text;
}

template<typename Char>
void Sz<Char>::Truncate(size_t length)
{
	if (length < Length())
	{
		if (length <= Const::kInternalCapacity)
		{
			if (_d.IsExternal())
			{
				External* ex = _d.ex;

				SzMemCpy(_d.InText(), ex->text, Min(length, ex->length));

				ex->Release();
			}

			_d.SetInLength(length);
		}
		else if (_d.IsExternal() && (_d.ex->references == 1) && (length <= _d.ex->capacity) && ((length + 15) >= (_d.ex->capacity / 2)))
		{
			_d.SetExLength(length);
		}
		else if (External* ex = External::Create(length))
		{
			SzMemCpy(ex->text, Text(), Min(length, Length()));

			if (_d.IsExternal())
				_d.ex->Release();

			_d.SetEx(ex);
		}
		else
		{
			_d.SetExLength(length);
		}
	}
}

template<typename Char>
typename Sz<Char>::External* Sz<Char>::External::Create(size_t length)
{
	size_t bytes = SzAlignUpper((((length + 15 + 1) & ~0xF) * sizeof(Char)) + Const::kExternalSize, (size_t)16);

	External* ex = (External*)SzAlloc(bytes);

	if (!ex)
		return 0;

	ex->references		= 1;
	ex->length			= length;
	ex->capacity		= ((bytes - Const::kExternalSize) / sizeof(Char)) - 1;
	ex->text[length]	= '\0';

	return ex;
}

template<typename Char>
void Sz<Char>::External::Acquire()
{
	InterlockedIncrement((LONG*)&references);
}

template<typename Char>
void Sz<Char>::External::Release()
{
	if (InterlockedDecrement((LONG*)&references) <= 0)
		SzFree(this);
}

template Sz<char>;
template Sz<wchar_t>;

//
// Sz::SzConst
//

template<typename Char>
Sz<Char> SzConst(const Char* text)
{
	return Sz<Char>(text);
}

template Sz<char> SzConst(const char* text);
template Sz<wchar_t> SzConst(const wchar_t* text);

// Sz::SzError
//
//

template<typename Char>
Sz<Char> SzError(SzErrorCode errorCode)
{
	return Sz<Char>();
}

template Sz<char> SzError(SzErrorCode errorCode);
template Sz<wchar_t> SzError(SzErrorCode errorCode);

//
// Sz::SzLength
//

template<typename Char>
size_t SzLength(const Char* text)
{
	size_t length = 0;

	if (text)
	{
		for(; *text; text++)
			length++;
	}

	return length;
}

template size_t SzLength(const char* text);
template size_t SzLength(const wchar_t* text);

//
// Sz::Concat
//

template<typename Char> Sz<Char>
Concat(const Char* lText, size_t lLength, const Char* rText, size_t rLength)
{
	size_t length = lLength + rLength;

	Sz<Char> out;

	Char* text = out.Capture(length, false);

	if (!text)
		return SzError<Char>(kSzOkay);

	SzMemCpy(text, lText, lLength);
	SzMemCpy(text + lLength, rText, rLength);

	return out;
}

template Sz<char> Concat(const char* lText, size_t lLength, const char* rText, size_t rLength);
template Sz<wchar_t> Concat(const wchar_t* lText, size_t lLength, const wchar_t* rText, size_t rLength);

//
// Sz::ConcatN
//

template<typename Char>
Sz<Char> ConcatN(int count, const Char** strings, const size_t* lengths)
{
	size_t length = 0;

	for(int i = 0; i < count; i++)
		length += lengths[i];

	Sz<Char> out;

	Char* text = out.Capture(length, false);

	if (!text)
		return SzError<Char>(kSzOkay);

	for(int i = 0; i < count; i++)
	{
		size_t sl = lengths[i];

		SzMemCpy(text, strings[i], sl);

		text += sl;
	}

	return out;
}

template Sz<char> ConcatN(int count, const char** strings, const size_t* lengths);
template Sz<wchar_t> ConcatN(int count, const wchar_t** strings, const size_t* lengths);

//
// Sz::Append
//

template<typename Char>
bool Append(Sz<Char>& text, const Sz<Char>& suffix)
{
	size_t		textLength		= text.Length();
	size_t		suffixLength	= suffix.Length();
	const Char* suffixText		= suffix.Text();

	Char* ptr = text.Capture(textLength + suffixLength, true);

	if (!ptr)
		return false;

	SzMemCpy(ptr + textLength, suffixText, suffixLength);

	return true;
}

template<typename Char>
bool Append(Sz<Char>& text, const Char* suffixText, size_t suffixLength)
{
	size_t textLength = text.Length();

	Char* ptr = text.Capture(textLength + suffixLength, true);

	if (!ptr)
		return false;

	SzMemCpy(ptr + textLength, suffixText, suffixLength);

	return true;
}

template bool Append(Sz<char>& text, const Sz<char>& suffix);
template bool Append(Sz<wchar_t>& text, const Sz<wchar_t>& suffix);

template bool Append(Sz<char>& text, const char* suffixText, size_t suffixLength);
template bool Append(Sz<wchar_t>& text, const wchar_t* suffixText, size_t suffixLength);