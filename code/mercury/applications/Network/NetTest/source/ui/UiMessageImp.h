#ifndef UIMESSAGEIMP_H
#define UIMESSAGEIMP_H

#ifdef _MSC_VER
#pragma once
#endif

#include "UiMessage.h"

namespace UI
{
	class MessageParam
	{
	public:
		MessageParam(const int type, const int len);
		virtual ~MessageParam();
		virtual void logOutput(int i) const;

		const int m_type;
		const int m_len;
	};

	class MessageParamChunk: public MessageParam
	{
	public:
		MessageParamChunk(const void* ptr, const int len);
		~MessageParamChunk();
		virtual void logOutput(int i) const;
		void* m_ptr;
	};

	class MessageParamString: public MessageParam
	{
	public:
		MessageParamString(const char* str, const int len);
		~MessageParamString();
		virtual void logOutput(int i) const;
		char* m_str;
	};

	class MessageParamBoolean: public MessageParam
	{
	public:
		MessageParamBoolean(const bool val);
		virtual void logOutput(int i) const;
		const bool m_value;
	};

	class MessageParamReal: public MessageParam
	{
	public:
		MessageParamReal(const float val);
		virtual void logOutput(int i) const;
		const float m_value;
	};

	class MessageParamInteger: public MessageParam
	{
	public:
		MessageParamInteger(const int val);
		virtual void logOutput(int i) const;
		const int m_value;
	};

	BD_REFERENCE(MessageImp);
	class MessageImp: public Message
	{
	public:
		MessageImp(const char* message, bool confirmed=false);
		~MessageImp();

		// get parameters. type conversion will occur
		virtual const char*		messageType()		const;
		virtual void			logOutput()			const;		
		virtual int				count()				const;
		virtual int				length	(int index) const;
		virtual int				type	(int index) const;
		virtual int				integer	(int index) const;
		virtual bool			boolean	(int index) const;
		virtual float			real	(int index) const;
		virtual const char*		string	(int index) const;
		virtual const void*		chunk	(int index) const;
		virtual UI::MessageRef	message	(int index) const;
				
		virtual void pushMessage	(UI::MessageRef msg);
		virtual void pushBool		(const bool value);
		virtual void pushInteger	(const int value);
		virtual void pushReal		(const float value);
		virtual void pushChunk		(const void* ptr, const int len);
		virtual void pushString		(const char* string);
		
		virtual UI::MessageRef	copy() const;

	private:
		const MessageParam* param(int index) const;
		void pushParam(MessageParam* param);

		MessageParam**	 m_params;
		int				 m_limit;
		int				 m_count;
	
	};
}; // namespace UI

#endif //UIMESSAGEIMP_H