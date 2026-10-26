//generic template for converting integer based numbers to string. itoa limited to 32b numbers. 
//Extensions are MS only, so a simple version for all types and signs is implemented here. 

template <typename T>
String unsignedIntegerToCString( T value) 
{
	static const int N = sizeof(T)*8;
	const T base = 10;
	char result[N]; 
											
	//We assign the characters to the end first, as the mod operation later 
	//gives us the least significant numbers first. Therefore we will index into 
	//result backwards

	char* out = &result[N-1];	
	*out = 0; //terminating null
	out--;

	T quotient = value;
	
	do {		
		//take remainder from mod, and use to index array of 
		//chars representing base10 numbers. 
		*out = "0123456789"[ quotient % base ];	
		//Index backwards into the result buffer
		--out;	
		//divide by our base (10) to get the next digit.
		quotient /= base;
	} while ( quotient );
	
	out++;
	
	return String(out);
}
	
template <typename T>
String signedIntegerToCString( T value) 
{
	static const int N = sizeof(T)*8;
	const T base = 10;
	char result[N]; 
											
	//We assign the characters to the end first, as the mod operation later 
	//gives us the least significant numbers first. Therefore we will index into 
	//result backwards

	char* out = &result[N-1];	
	*out = 0; //terminating null
	out--;

	bool bNeg; //is this a negative number?
	(value < 0)?(bNeg=true):(bNeg=false);

	T quotient;

	(bNeg)?(quotient = value*-1):(quotient = value);
	
	do {		
		//take remainder from mod, and use to index array of 
		//chars representing base10 numbers. 
		*out = "0123456789"[ quotient % base ];	
		//Index backwards into the result buffer
		--out;	
		//divide by our base (10) to get the next digit.
		quotient /= base;
	} while ( quotient );
	
	//if negative, add the minus sign, otherwise we've indexed back too far, so increment.

	if (bNeg)
	{
		(*out = '-');
	}
	else
	{
		(out++);
	}
	
	return String(out);
}

inline String& operator << (String& string, const String& in)
{
	string += in;
	return string;
}

inline String& operator << (String& string, const U8 in)
{
	return string << unsignedIntegerToCString(in);
}

inline String& operator << (String& string, const S8 in)
{
	return string << signedIntegerToCString(in);
}

inline String& operator << (String& string, const U16 in)
{
	return string << unsignedIntegerToCString(in);
}

inline String& operator << (String& string, const S16 in)
{
	return string << signedIntegerToCString(in);
}

inline String& operator << (String& string, const U32 in)
{
	return string << unsignedIntegerToCString(in);
}

inline String& operator << (String& string, const S32 in)
{
	return string << signedIntegerToCString(in);
}

inline String& operator << (String& string, const U64 in)
{
	return string << unsignedIntegerToCString(in);
}

inline String& operator << (String& string, const S64 in)
{
	return string << signedIntegerToCString(in);
}

inline String& operator << (String& string, const F32 in)
{
	char numBuf[_CVTBUFSIZE];
	double d = (double)in;
	_gcvt_s( numBuf, _CVTBUFSIZE, d, 5 );
	string = string + numBuf ;
	return string;
}

inline String& operator << (String& string, const F64 in)
{
	char numBuf[_CVTBUFSIZE];
	double d = (double)in;
	_gcvt_s( numBuf, _CVTBUFSIZE, d, 10 );
	string = string + numBuf;
	return string;
}

inline String& operator << (String& string, const char* in)
{
	string = string + String(in);
	return string;
}

inline String& operator << (String& string, const char in)
{
	string = string + String(in);
	return string;
}
