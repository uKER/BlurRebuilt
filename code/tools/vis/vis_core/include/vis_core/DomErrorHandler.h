#ifndef __DOM_ERROR_HANDLER_H__
#define __DOM_ERROR_HANDLER_H__

#include <xercesc/dom/DOMErrorHandler.hpp>

class LuxDOMErrorHandler : public DOMErrorHandler
{
public:
	LuxDOMErrorHandler(std::string filename) :
		bError(false),
		m_filename(filename)
	{
	}
	bool handleError(const XERCES_CPP_NAMESPACE::DOMError &domError)
	{
		char *pError = XERCES_CPP_NAMESPACE::XMLString::transcode(domError.getMessage());
		LOGERROR( g_Log, "%s(%d) : %s\n", m_filename.c_str(), domError.getLocation()->getLineNumber(), pError  );
		XERCES_CPP_NAMESPACE::XMLString::release(&pError);
		bError = true;
		return true;
	}

	bool bError;
	std::string m_filename;
};

#endif
