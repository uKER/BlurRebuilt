#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif
			
#include "stdafx.h"
#include "UiMessageImp.h"
#include "Ui/UiLogger.h"
			
namespace UI
{
	UI::MessageRef Message::create(const char* messageId, bool confirmed)
	{
		return new MessageImp(messageId, confirmed);
	}
		
	MessageParam::MessageParam(const int type, const int len)
	: m_type(type)
	, m_len(len)
	{}

	MessageParam::~MessageParam() 
	{}

	void MessageParam::logOutput(int i) const
	{
		LOGINFO(g_Log, "\t%s[%d] type(%d) len(%d)\n", "UNKNOWN", i, m_type, m_len);
	}

	MessageParamChunk::MessageParamChunk(const void* ptr, const int len)
	: MessageParam(PARAM_TYPE_CHUNK, len)
	, m_ptr(new char[len])
	{
		memcpy(m_ptr, ptr, len);
	}

	MessageParamChunk::~MessageParamChunk()
	{
		delete[] m_ptr;
	}

	void MessageParamChunk::logOutput(int i) const
	{
		const char* chr = (const char*)m_ptr;
		LOGINFO(g_Log, "\t%s[%d] (len = %d, %02x.%02x.%02x.%02x...)\n", "CHUNK", i, m_len, chr[0], chr[1], chr[2], chr[3]);
	}

	MessageParamString::MessageParamString(const char* str, const int len)
	: MessageParam(PARAM_TYPE_STRING, len)
	, m_str(new char[len + 10])
	{
		strncpy(m_str, str, len);
		m_str[len-1] = '\0';
	}
	MessageParamString::~MessageParamString()
	{
		delete[] m_str;
	}
	void MessageParamString::logOutput(int i) const
	{
		LOGINFO(g_Log, "\t%s[%d] '%s'\n", "STRING", i, m_str);
	}

	MessageParamBoolean::MessageParamBoolean(const bool val)
	: MessageParam(PARAM_TYPE_BOOLEAN, sizeof(m_value))
	, m_value(val)
	{}
	void MessageParamBoolean::logOutput(int i) const
	{
		LOGINFO(g_Log, "\t%s[%d] val=%s\n", "BOOLEAN", i, m_value? "true" : "false");
	}

	MessageParamReal::MessageParamReal(const float val)
	: MessageParam(PARAM_TYPE_REAL, sizeof(m_value))
	, m_value(val)
	{}
	void MessageParamReal::logOutput(int i) const
	{
		LOGINFO(g_Log, "\t%s[%d] val=%f\n", "REAL", i, m_value);
	}

	MessageParamInteger::MessageParamInteger(const int val)
	: MessageParam(PARAM_TYPE_INTEGER, sizeof(m_value))
	, m_value(val)
	{}
	void MessageParamInteger::logOutput(int i) const
	{
		LOGINFO(g_Log, "\t%s[%d] val=%d\n", "INTEGER", i, m_value);
	}

	MessageImp::MessageImp(const char* messageId, bool confirmed)
	: Message(UI_MESSAGE_ID_UNKNOWN, confirmed)
	, m_params(new MessageParam*[4])
	, m_limit(4)
	, m_count(0)
	{			
		if(messageId != NULL)
		{
			// write the header
			// parameter[0] is the header name
			pushString(messageId);
		}
	}
	MessageImp::~MessageImp()
	{
		for(int i = 0; i < m_count; i ++)
		{
			delete m_params[i];
		}
		delete m_params;
	}

	// get parameters. type conversion will occur
	int	MessageImp::count() const 
	{ 
		return m_count; 
	}

	const char* MessageImp::messageType() const
	{
		return string(0);
	}		
	int MessageImp::type(int index) const
	{
		// get the stored parameter
		const MessageParam* p = param(index);
		if(!p) return PARAM_TYPE_NOT_SET;
		return p->m_type;
	}
		
	// number of bytes stored for the parameter
	int MessageImp::length(int index) const
	{
		// get the stored parameter
		const MessageParam* p = param(index);
		if(!p) return 0;
		return p->m_len;
	}

	// get integer (if possible)
	int	MessageImp::integer(int index) const
	{
		// get the stored parameter
		const MessageParam* p = param(index);
		if(!p) return 0;

		// convert parameter to the type we want
		switch(p->m_type)
		{
		default:
		case PARAM_TYPE_NOT_SET:
		case PARAM_TYPE_CHUNK:
		case PARAM_TYPE_STRING:
			{
				assert(!"expecting number type");
				return 0;
			}
		case PARAM_TYPE_BOOLEAN:
			{
				assert(!"expecting bool type");
				return (int)boolean(index);
			}
		case PARAM_TYPE_REAL:
			{
				assert(!"expecting real type");
				return (int) real(index);
			}
		case PARAM_TYPE_INTEGER:
			{
				MessageParamInteger* cast = (MessageParamInteger*)p;
				return cast->m_value;
			}
		}
	}

	const char* MessageImp::string(int index) const
	{
		// get the stored parameter
		const MessageParam* p = param(index);
		if(!p) return 0;

		// convert parameter to the type we want
		switch(p->m_type)
		{
		default:
		case PARAM_TYPE_NOT_SET:
		case PARAM_TYPE_BOOLEAN:
		case PARAM_TYPE_REAL:
		case PARAM_TYPE_INTEGER:
		case PARAM_TYPE_CHUNK:
			{
				assert(!"expecting string type");
				return "";
			}
		case PARAM_TYPE_STRING:
			{
				MessageParamString* cast = (MessageParamString*)p;
				return cast->m_str;
			}
		}
	}

	bool MessageImp::boolean(int index) const
	{
		// get the stored parameter
		const MessageParam* p = param(index);
		if(!p) return 0;

		// convert parameter to the type we want
		switch(p->m_type)
		{
		default:
		case PARAM_TYPE_NOT_SET:
		case PARAM_TYPE_CHUNK:
		case PARAM_TYPE_STRING:
			{
				assert(!"expecting number type");
				return 0;
			}
		case PARAM_TYPE_BOOLEAN:
			{
				MessageParamBoolean* cast = (MessageParamBoolean*)p;
				return cast->m_value;
			}
		case PARAM_TYPE_REAL:
			{
				assert(!"expecting real type");
				return (bool) (real(index) != 0.0f);
			}
		case PARAM_TYPE_INTEGER:
			{
				assert(!"expecting int type");
				return (bool) (integer(index) != 0);
			}
		}
	}

	float MessageImp::real(int index) const
	{
		// get the stored parameter
		const MessageParam* p = param(index);
		if(!p) return 0;

		// convert parameter to the type we want
		switch(p->m_type)
		{
		default:
		case PARAM_TYPE_NOT_SET:
		case PARAM_TYPE_CHUNK:
		case PARAM_TYPE_STRING:
			{
				assert(!"expecting number type");
				return 0.0f;
			}
		case PARAM_TYPE_BOOLEAN:
			{
				assert(!"expecting bool type");
				return (float)boolean(index);
			}
		case PARAM_TYPE_REAL:
			{
				MessageParamReal* cast = (MessageParamReal*)p;
				return cast->m_value;
			}
		case PARAM_TYPE_INTEGER:
			{
				assert(!"expecting int type");
				return (float)integer(index);
			}
		}
	}

	const void* MessageImp::chunk(int index) const
	{
		// get the stored parameter
		const MessageParam* p = param(index);
		if(!p) return 0;

		// convert parameter to the type we want
		switch(p->m_type)
		{
		default:
		case PARAM_TYPE_NOT_SET:
		case PARAM_TYPE_BOOLEAN:
		case PARAM_TYPE_REAL:
		case PARAM_TYPE_INTEGER:
			{
				assert(!"expecting int/bool/float type");
				return NULL;
			}
		case PARAM_TYPE_STRING:
			{
				assert(!"expecting string type");
				return (const void*)string(index);
			}
		case PARAM_TYPE_CHUNK:
			{
				MessageParamChunk* cast = (MessageParamChunk*)p;
				return cast->m_ptr;
			}
		}
	}

	void MessageImp::pushBool(const bool value)
	{
		pushParam(new MessageParamBoolean(value));
	}

	void MessageImp::pushInteger(const int value)
	{
		pushParam(new MessageParamInteger(value));
	}

	void MessageImp::pushReal(const float value)
	{
		pushParam(new MessageParamReal(value));
	}

	void MessageImp::pushChunk(const void* ptr, const int len)
	{
		pushParam(new MessageParamChunk(ptr, len));
	}

	void MessageImp::pushString(const char* string)
	{
		pushParam(new MessageParamString(string, strlen(string)+1));
	}

	UI::MessageRef MessageImp::copy() const
	{
		UI::MessageRef msg = Message::create(NULL);

		for(int i = 0; i < count(); i ++)
		{
			const MessageParam* p = param(i);
			
			switch(p->m_type)
			{
			default:
			case PARAM_TYPE_NOT_SET:
				{
					continue;
				}
			case PARAM_TYPE_CHUNK:
				{
					msg->pushChunk(chunk(i), length(i));
					break;
				}
			case PARAM_TYPE_STRING:
				{
					msg->pushString(string(i));
					break;
				}
			case PARAM_TYPE_BOOLEAN:
				{
					msg->pushBool(boolean(i));
					break;
				}
			case PARAM_TYPE_REAL:
				{
					msg->pushReal(real(i));
					break;
				}
			case PARAM_TYPE_INTEGER:
				{
					msg->pushInteger(integer(i));
					break;
				}
			}
		}
		return msg;
	}

	UI::MessageRef MessageImp::message(int index) const
	{
		if(index >= count()) return NULL;

		UI::MessageRef msg = Message::create(NULL);

		for(int i = index; i < count(); i ++)
		{
			const MessageParam* p = param(i);
			
			switch(p->m_type)
			{
			default:
			case PARAM_TYPE_NOT_SET:
				{
					assert(false);
					continue;
				}
			case PARAM_TYPE_CHUNK:
				{
					msg->pushChunk(chunk(i), length(i));
					break;
				}
			case PARAM_TYPE_STRING:
				{
					msg->pushString(string(i));
					break;
				}
			case PARAM_TYPE_BOOLEAN:
				{
					msg->pushBool(boolean(i));
					break;
				}
			case PARAM_TYPE_REAL:
				{
					msg->pushReal(real(i));
					break;
				}
			case PARAM_TYPE_INTEGER:
				{
					msg->pushInteger(integer(i));
					break;
				}
			}
		}
		return msg;
	}
				
	void MessageImp::pushMessage(UI::MessageRef msg)
	{
		if(!msg) return;

		for(int i = 0; i < msg->count(); i ++)
		{
			const MessageParam* p = msg->param(i);
			
			switch(p->m_type)
			{
			default:
			case PARAM_TYPE_NOT_SET:
				{
					continue;
				}
			case PARAM_TYPE_CHUNK:
				{
					pushChunk(msg->chunk(i), msg->length(i));
					break;
				}
			case PARAM_TYPE_STRING:
				{
					pushString(msg->string(i));
					break;
				}
			case PARAM_TYPE_BOOLEAN:
				{
					pushBool(msg->boolean(i));
					break;
				}
			case PARAM_TYPE_REAL:
				{
					pushReal(msg->real(i));
					break;
				}
			case PARAM_TYPE_INTEGER:
				{
					pushInteger(msg->integer(i));
					break;
				}
			}
		}
	}		

	void MessageImp::pushParam(MessageParam* p)
	{
		if(m_count >= m_limit)
		{
			m_limit += (m_limit / 2) + 2;
			MessageParam* *params = new MessageParam*[m_limit];
		
			for(int i = 0; i < m_count; i ++)
				params[i] = m_params[i];
			delete m_params;

			m_params = params;
		}
		m_params[m_count++] = p;
	}

	const MessageParam* MessageImp::param(int index) const
	{
		// sanity check
		if(index < 0 || index >= m_count) 
		{
			assert(false);
			return NULL;
		}
		return m_params[index];
	}

	void MessageImp::logOutput() const
	{
		if(m_count <= 0) return;

		LOGINFO(g_Log, "UI::Message('%s') id(%d), count(%d)\n", messageType(), messageId(), count());

		for(int i = 1; i < m_count; i ++)
		{
			// get the stored parameter
			const MessageParam* p = param(i);
			p->logOutput(i);
		}
	}

};



#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

