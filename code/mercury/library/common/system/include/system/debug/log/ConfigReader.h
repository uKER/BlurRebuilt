#ifndef __SYSTEM_LOG_CONFIGREADER_H__
#define __SYSTEM_LOG_CONFIGREADER_H__

namespace Debug
{

// callbacks for reading the ini file using XMLParser.cpp
class XMLConfigFile : public Sys::C_XmlContent
{
public:
	bool					StartDocument() { return true; }
	bool					EndDocument() { return true; }

	bool					StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);
	bool					EndElement(const Util::C_String& name) { return true; };

private:
							/// parse a logger
	void					ProcessLoggerElement(Debug::Logger &logger, const Sys::C_XmlAttributeList &attributes);

							/// parse a subscriber
	void					ProcessSubscriberElement(const Sys::C_XmlAttributeList &attributes);
};

}

#endif
