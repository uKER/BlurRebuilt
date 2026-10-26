
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H


class C_MessageHandler	
{
public:
	enum E_MessageType
	{
		E_MessageType_Error = 0,
		E_MessageType_Warning,
		E_MessageType_Info,
	};
public:
	static C_MessageHandler& Instance()			{ return m_sInstance; }

	void ShowMessage( const E_MessageType type, const char* str );

private:
	// Singleton instance.
	static C_MessageHandler	m_sInstance;
};

// Define some messages for ease of use
#define Str_ErrorMessage_UnsupportedCompoundShapeUse "Building a compound shape using a group is not supported and could cause loss of data!"


#endif // ERROR_HANDLER_H