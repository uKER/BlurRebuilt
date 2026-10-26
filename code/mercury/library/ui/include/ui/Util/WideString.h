
#ifndef UI_WIDESTRING_H
#define UI_WIDESTRING_H

#include <system/serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include "ui/Graphics/Localisation/Loc.h"
#include "ui/Util/WideStringReformat.h"

namespace UI
{

/*---------------------------------------------------------------------------------------
	Class : I_WideString

	Desc :	WCHAR base class.
---------------------------------------------------------------------------------------*/
class I_WideString
{
public:
	virtual						~I_WideString(){};

								/// Allows for random character access.
	virtual const WCHAR			operator[] ( const S32 index ) const = 0;

								/// Allows for random character access.
	virtual WCHAR				operator[] ( const S32 index ) = 0;

								/// Returns a const pointer to the unicode data.
	virtual const WCHAR*		Chars() const = 0;

								/// Returns a non const pointer to the unicode data. **Shouldnt be used!**
	virtual WCHAR*				NonConstChars() = 0;

								/// Return the length of the string.
	virtual U32					Length() const = 0;

								/// Return the size of the string. Max characters.
	virtual U32					Size() const = 0;

								/// Returns index to first char or -1 if it cant be found.
	S32							Find( const WCHAR* pStr ) const;

protected:
	void						SubString( const S32 start, const S32 count, I_WideString& buff ) const;
};


/*---------------------------------------------------------------------------------------
	Class : C_WideString

	Desc :	WCHAR wrapper class.
---------------------------------------------------------------------------------------*/
template < U32 m_Size >
class C_WideString : public I_WideString
{
public:
/*---------------------------------------------------------------------------------------
	Constructors
---------------------------------------------------------------------------------------*/
	
	/// Default constructor. Zero out string.
	C_WideString();

	// Do nothing.
	~C_WideString(){};

	/// Construct by string.
	C_WideString( const WCHAR* str );

	/// Construct by WCHAR
	C_WideString( const WCHAR c );
	
	/// Construct by string.
	C_WideString( const char* str );

	/// Construct from string id.
	C_WideString( C_TextStringID str );

/*---------------------------------------------------------------------------------------
	Copy constructor
---------------------------------------------------------------------------------------*/
	
	/// Copy constructor
	C_WideString( const I_WideString& str );

/*---------------------------------------------------------------------------------------
	Operator overloads.
---------------------------------------------------------------------------------------*/

								/// Cast to WCHAR
								operator const WCHAR* () const;

								/// Concatenation operator
    const C_WideString&			operator += ( const I_WideString& str );

								/// Concatenation operator
	const C_WideString&			operator += ( const WCHAR* str );

								/// Concatenation operator
	const C_WideString&			operator += ( const WCHAR c );
	
								/// Add operator
	const C_WideString			operator + ( const I_WideString& str );

								/// Add operator
	const C_WideString			operator + ( const WCHAR* str );

								/// Comparison operator
	bool						operator == ( const I_WideString& str ) const;

								/// Comparison operator
	bool						operator != ( const I_WideString& str ) const;

								/// Allows for random character access.
	const WCHAR					operator[] ( const S32 index ) const;

								/// Allows for random character access.
	WCHAR						operator[] ( const S32 index );

/*---------------------------------------------------------------------------------------
	Functions
---------------------------------------------------------------------------------------*/

	void						Serialise( Sys::BinarySerialiserAutoEndian& serial );
	void						Deserialise( Sys::BinaryDeserialiserAutoEndian& serial );

								/// Returns unicode chars as multibyte string.
	Util::C_String				AsString() const;

								/// Sets the first element to the end of string marker.
	void						Clear();

								/// Replaces the string.
	void						Copy( const WCHAR* str );

								/// Reformats the string.
	void						Format( const WCHAR* format, ... );

								/// Returns true if this equals the given string.
	bool						Compare( const I_WideString& str ) const;

								/// Returns true if this equals the given string.
	bool						Compare( const WCHAR* str ) const;

								/// Appends another string onto the end.
    void						Add( const I_WideString& str );

								/// Appends another string onto the end.
    void						Add( const WCHAR* str );

								/// Appends another string onto the end.
    void						Add( const WCHAR c );

								/// Returns a substring.
	C_WideString				SubString( const S32 start, const S32 count ) const;

								/// Inserts the strings at the start point.
	void						Insert( const WCHAR* str, const S32 pos );

								/// Inserts the char at the start point.
	void						Insert( const WCHAR c, const S32 pos );

								/// Replace the WCHAR at the given index.
	void						Replace( U32 index, const WCHAR c  );

								/// Replace all instance of one character with another
	void						ReplaceCharacter( const WCHAR toReplace, const WCHAR newC );

								/// Returns index to first char or -1 if it cant be found.
	//S32						Find( const WCHAR* pStr ) const; // Base class function

								/// Returns a const pointer to the unicode data.
	const WCHAR*				Chars() const;

								/// Returns a non const pointer to the unicode data. **Shouldnt be used!**
	WCHAR*						NonConstChars();

								/// Return the length of the string.
	U32							Length() const;

								/// Return the size of the string. Max characters.
	U32							Size() const;

/*---------------------------------------------------------------------------------------
	Private Data
---------------------------------------------------------------------------------------*/
private:   
    WCHAR						m_Chars[ m_Size ];
};


/*---------------------------------------------------------------------------------------
	Typedefs
---------------------------------------------------------------------------------------*/
typedef C_WideString<4>			C_WideString4;
typedef C_WideString<6>			C_WideString6;
typedef C_WideString<8>			C_WideString8;
typedef C_WideString<12>		C_WideString12;
typedef C_WideString<16>		C_WideString16;
typedef C_WideString<32>		C_WideString32;
typedef C_WideString<64>		C_WideString64;
typedef C_WideString<80>		C_WideString80;
typedef C_WideString<96>		C_WideString96;
typedef C_WideString<128>		C_WideString128;
typedef C_WideString<256>		C_WideString256;
typedef C_WideString<512>		C_WideString512;

/*---------------------------------------------------------------------------------------
	Non-member functions.
---------------------------------------------------------------------------------------*/
/// Will copy the src into the dest. If the src is bigger, then it will be truncated.
int WideStrFormat( WCHAR* str, const U32 size, const WCHAR* format, ... );

int WideStrArgs( WCHAR* str, const U32 size, const WCHAR* format, va_list va_args );

/// Copy the src into the dest truncating if its too big.
void WideStrCopy( WCHAR* dst, const U32 size, const WCHAR* src );

}; // namespace

#define UI_WIDESTRING_INL
#include "WideString.inl"
#undef UI_WIDESTRING_INL

#endif // UI_STRING_H
