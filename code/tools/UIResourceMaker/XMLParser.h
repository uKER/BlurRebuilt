#pragma once

#include <sax/ErrorHandler.hpp>
#include <util/PlatformUtils.hpp>
#include <sax/SAXParseException.hpp>
#include <parsers/DOMParser.hpp>
#include <dom/DOM_DOMException.hpp>
#include <dom/DOM_NamedNodeMap.hpp>
#include <iostream>

class XMLParser
{
public:
	XMLParser(){};

	void ParseFile(char *filename);

	char *GetFlowFileName(){return flowfilename;};

	void ProcessFonts(char *filename);

private:
	DOM_Node ReadHeader(DOM_NodeList *nodelist,int *xml_type);

	void BuildConfig(DOM_NodeList *nodelist);

	void GetNameandValue(DOM_Node attr,DOMString *name,DOMString *value);

	char flowfilename[128];
};

extern XMLParser gXMLParser;
















// ---------------------------------------------------------------------------
//  Simple error handler deriviative to install on parser
// ---------------------------------------------------------------------------
class DOMCountErrorHandler : public ErrorHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DOMCountErrorHandler();
    ~DOMCountErrorHandler();


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getSawErrors() const;


    // -----------------------------------------------------------------------
    //  Implementation of the SAX ErrorHandler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& e);
    void error(const SAXParseException& e);
    void fatalError(const SAXParseException& e);
    void resetErrors();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMCountErrorHandler(const DOMCountErrorHandler&);
    void operator=(const DOMCountErrorHandler&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSawErrors
    //      This is set if we get any errors, and is queryable via a getter
    //      method. Its used by the main code to suppress output if there are
    //      errors.
    // -----------------------------------------------------------------------
    bool    fSawErrors;
};


// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class StrX
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    StrX(const XMLCh* const toTranscode)
    {
        // Call the private transcoding method
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        delete [] fLocalForm;
    }


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const char* localForm() const
    {
        return fLocalForm;
    }

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocalForm
    //      This is the local code page form of the string.
    // -----------------------------------------------------------------------
    char*   fLocalForm;
};

//inline ostream& operator<<(ostream& target, const StrX& toDump)
//{
    //target << toDump.localForm();
    //return target;
//}

inline bool DOMCountErrorHandler::getSawErrors() const
{
    return fSawErrors;
}
