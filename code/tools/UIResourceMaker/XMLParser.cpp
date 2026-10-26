#include "stdafx.h"

#include "CommandLine.h"
#include "XMLParser.h"
#include "Fontmaker.h"
#include "Config.h"

XMLParser gXMLParser;

void XMLParser::GetNameandValue(DOM_Node attr,DOMString *name,DOMString *value)
{
	*name=attr.getNodeName();
	*value=attr.getNodeValue();
}

void XMLParser::ProcessFonts(char *filename)
{
	char name[128];
	char export_path[128];
	char overide_name[128];
	char texture_name[128];
	char texture_compression[128];

	int texture_mip_level=1;
    const char*              xmlFile = 0;
    DOMParser::ValSchemes    valScheme = DOMParser::Val_Auto;
    bool                     doNamespaces    = false;
	DOMString nodename;
	DOMString attrname;
	DOMString attrval;
	bool write_out=false;


    DOMParser parser;
    parser.setValidationScheme(valScheme);
    parser.setDoNamespaces(doNamespaces);

    // And create our error handler and install it
    DOMCountErrorHandler errorHandler;
    parser.setErrorHandler(&errorHandler);

	parser.parse(filename);

    if (errorHandler.getSawErrors())
    {
		gCommandLine.AddLineArg(TC_RED,"Errors in XML file %s",filename);        
    }
    else
    {
		gFontMaker.StartFontMaker(filename);

		gCommandLine.AddLineArg(TC_BLACK,"Processing Fonts XML file %s ...",filename);        

		gCommandLine.AddLine(TC_BLUE,"");
		gCommandLine.AddLine(TC_BLUE,"Building Font List ...");
		gCommandLine.AddLine(TC_BLUE,"");

		int i=0;

        DOM_Document doc = parser.getDocument();
		DOM_NodeList nodelist = doc.getChildNodes();
		DOM_Node node;

		node=ReadHeader(&nodelist,NULL);

		nodelist = node.getChildNodes();
		for(int b=0;b<(int)nodelist.getLength();b++)
		{
			write_out=false;
			overide_name[0]=NULL;
			DOM_Node node=nodelist.item(b);
			if(node.getNodeType() == DOM_Node::ELEMENT_NODE)
			{
				nodename=node.getNodeName();
				sprintf(name,"%s",nodename.transcode());

				if(nodename.equals("BUNDLE"))
				{
					DOM_NamedNodeMap attributes = node.getAttributes();
					int count=attributes.getLength();
					for(int c=0;c<count;c++)
					{
						GetNameandValue(attributes.item(c),&attrname,&attrval);
						if(attrname.equals("Name"))
						{
							sprintf(name,"%s",attrval.transcode());
							gFontMaker.SetBundle(name);
						}
						if(attrname.equals("Export"))
						{
							sprintf(export_path,"%s",attrval.transcode());
							gFontMaker.SetBundleExportPath(export_path);
						}
					}

					DOM_NodeList nodelist1 = node.getChildNodes();

					for(int a=0;a<(int)nodelist1.getLength();a++)
					{
						DOM_Node node=nodelist1.item(a);
						if(node.getNodeType() == DOM_Node::ELEMENT_NODE)
						{
							nodename=node.getNodeName();
							sprintf(name,"%s",nodename.transcode());

							if(nodename.equals("FONTSET"))
							{
								DOM_NamedNodeMap attributes = node.getAttributes();
								int count=attributes.getLength();
								for(int c=0;c<count;c++)
								{
									GetNameandValue(attributes.item(c),&attrname,&attrval);
									if(attrname.equals("Name"))
									{
										sprintf(name,"%s",attrval.transcode());
										write_out=true;
									}
								}

								DOM_NodeList nodelist2 = node.getChildNodes();
								for(int d=0;d<(int)nodelist2.getLength();d++)
								{
									node=nodelist2.item(d);

									if(node.getNodeType() == DOM_Node::ELEMENT_NODE) 
									{
										nodename=node.getNodeName();

										if(nodename.equals("FAMILY"))
										{
											DOM_NamedNodeMap attributes = node.getAttributes();
											for(int c=0;c<(int)attributes.getLength();c++)
											{
												GetNameandValue(attributes.item(c),&attrname,&attrval);

												if(attrname.equals("FamilyName"))
												{
													gFontMaker.SetFontName(attrval.transcode());
												}
											}
										}
										else if(nodename.equals("TEXTURE"))
										{
											DOM_NamedNodeMap attributes = node.getAttributes();
											for(int c=0;c<(int)attributes.getLength();c++)
											{
												GetNameandValue(attributes.item(c),&attrname,&attrval);

												if(attrname.equals("File"))
												{
													strcpy(texture_name,attrval.transcode());
												}
												if(attrname.equals("Compression"))
												{
													strcpy(texture_compression,attrval.transcode());
												}
												if(attrname.equals("Levels"))
												{
													texture_mip_level=atoi(attrval.transcode());
												}
												if(attrname.equals("OverideName"))
												{
													strcpy(overide_name,attrval.transcode());
												}
											}
										}
										else if(nodename.equals("SIZE"))
										{
											DOM_NamedNodeMap attributes = node.getAttributes();
											for(int c=0;c<(int)attributes.getLength();c++)
											{
												GetNameandValue(attributes.item(c),&attrname,&attrval);

												if(attrname.equals("Pixels"))
												{
													gFontMaker.SetSize(atoi(attrval.transcode()));
												}
												else if(attrname.equals("Weight"))
												{
													gFontMaker.SetFontWeight(atoi(attrval.transcode()));
												}
											}
										}
										else if(nodename.equals("GLYPHS"))
										{
											int start=0;
											char filename[128];
											UIFONT_GLYPH_FILE_TYPE type;

											DOM_NamedNodeMap attributes = node.getAttributes();
											for(int c=0;c<(int)attributes.getLength();c++)
											{
												GetNameandValue(attributes.item(c),&attrname,&attrval);

												if(attrname.equals("File"))
												{
													strcpy(filename,attrval.transcode());
												}
												else if(attrname.equals("Type"))
												{
													if(attrval.equals("INI"))
													{
														type=UIFONT_GLYPH_FILE_TYPE_INI;
													}
													else if(attrval.equals("XML"))
													{
														type=UIFONT_GLYPH_FILE_TYPE_XML;
													}
												}
												else if(attrname.equals("Start"))
												{
													start=atoi(attrval.transcode());
												}
											}

											gFontMaker.FillGlyphTable(filename,start,type);
										}
										else if(nodename.equals("STYLE"))
										{
											gFontMaker.SetItalics(false);
											gFontMaker.SetDropShadow(false);
											gFontMaker.SetOutline(0);
											gFontMaker.SetAntiAliased(false);
											gFontMaker.SetGlow(false);
											gFontMaker.SetGlowGamma(2.0f);

											DOM_NamedNodeMap attributes = node.getAttributes();
											for(int c=0;c<(int)attributes.getLength();c++)
											{
												GetNameandValue(attributes.item(c),&attrname,&attrval);

												if(attrname.equals("Italic") && attrval.equals("true"))
												{
													gFontMaker.SetItalics(true);
												}
												else if(attrname.equals("DropShadow") && attrval.equals("true"))
												{
													gFontMaker.SetDropShadow(true);
												}
												else if(attrname.equals("Glow") && attrval.equals("true"))
												{
													gFontMaker.SetGlow(true);
												}
												else if(attrname.equals("GlowGamma"))
												{
													gFontMaker.SetGlowGamma((float)atof(attrval.transcode()));;
												}
												else if(attrname.equals("Outline"))
												{
													gFontMaker.SetOutline((int)atoi(attrval.transcode()));;
												}
												else if(attrname.equals("Extrude"))
												{
													gFontMaker.SetExtrude((int)atoi(attrval.transcode()));;
												}
												else if(attrname.equals("Antialiased") && attrval.equals("true"))
												{
													gFontMaker.SetAntiAliased(true);
												}
												else if(attrname.equals("Blobby") && attrval.equals("true"))
												{
													gFontMaker.SetBlobby(true);
												}
												else if(attrname.equals("IntensityOutline") && attrval.equals("true"))
												{
													gFontMaker.SetIntensityOutline(true);
												}
												else if(attrname.equals("BlackAndWhite"))
												{
													gFontMaker.SetBlackAndWhite(attrval.equals("true"));
												}
											}
										}
										else if(nodename.equals("REGION"))
										{
											DOM_NamedNodeMap attributes = node.getAttributes();
											for(int c=0;c<(int)attributes.getLength();c++)
											{
												GetNameandValue(attributes.item(c),&attrname,&attrval);

												if(attrname.equals("Language"))
												{
													if(attrval.equals("Japanese"))
													{
														gFontMaker.SetLanguage(UIFONT_LANGUAGE_JAPANESE);
													}
													else if(attrval.equals("Chinese"))
													{
														gFontMaker.SetLanguage(UIFONT_LANGUAGE_CHINESE);
													}
													else if(attrval.equals("Korean"))
													{
														gFontMaker.SetLanguage(UIFONT_LANGUAGE_KOREAN);
													}
													else if(attrval.equals("European"))
													{
														gFontMaker.SetLanguage(UIFONT_LANGUAGE_EUROPEAN);
													}
												}
											}
										}
										else if(nodename.equals("SPACING"))
										{
											DOM_NamedNodeMap attributes = node.getAttributes();
											for(int c=0;c<(int)attributes.getLength();c++)
											{
												GetNameandValue(attributes.item(c),&attrname,&attrval);

												if(attrname.equals("Horizontal"))
												{
													gFontMaker.SetHorizontalSpacing(atoi(attrval.transcode()));
												}
												else if(attrname.equals("Vertical"))
												{
													gFontMaker.SetVerticalSpacing(atoi(attrval.transcode()));
												}
												else if(attrname.equals("TopOffset"))
												{
													gFontMaker.SetTopOffset((float)atof(attrval.transcode()));
												}
												else if(attrname.equals("SizeOveride"))
												{
													gFontMaker.SetSizeOveride((float)atof(attrval.transcode()));
												}
											}
										}
									}
								}

								if(write_out) gFontMaker.CreateFontFiles(name,texture_name,overide_name,texture_compression,texture_mip_level);
							}//fontset element node
						}//is element node
					}//fontset array

					gFontMaker.EndBundle();
				}//is bundle node
			}//is element node
		}//nodelist
	}

	gFontMaker.WriteOutTotalBundles();
}

DOM_Node XMLParser::ReadHeader(DOM_NodeList *nodelist,int *xml_type)
{
	DOM_Node node;
	char text[128];
	int i=0;

	do
	{
		node=nodelist->item(i++);

		DOMString nodename=node.getNodeName();

		if(xml_type)
		{
			if(nodename.equals("Config")) *xml_type=1;
		}

		DOM_NamedNodeMap attributes = node.getAttributes();
		int count=attributes.getLength();

		for(int a=0;a<count;a++)
		{
			DOM_Node attr=attributes.item(a);

			DOMString attrname=attr.getNodeName();
			DOMString attrval=attr.getNodeValue();

			sprintf(text,"%s: %s",attrname.transcode(),attrval.transcode());
			gCommandLine.AddLine(TC_BLACK,text);
		}

	}while(node.getNodeType() != DOM_Node::ELEMENT_NODE);

	return node;
}

void XMLParser::BuildConfig(DOM_NodeList *nodelist)
{
	char xml[64];
	char name[64];
	char dir[64];

	DOM_Node node;

	gCommandLine.AddLine(TC_BLUE,"");
	gCommandLine.AddLine(TC_BLUE,"Building Resources ...");
	gCommandLine.AddLine(TC_BLUE,"");

	for(int z=0;z<(int)nodelist->getLength();z++)
	{
		node=nodelist->item(z);

		if(node.getNodeType() == DOM_Node::ELEMENT_NODE) 
		{
			DOM_NamedNodeMap attributes = node.getAttributes();
			int count=attributes.getLength();

			DOMString nodename=node.getNodeName();

			for(int a=0;a<count;a++)
			{
				DOM_Node attr=attributes.item(a);

				DOMString attrname=attr.getNodeName();
				if(attrname.equals("Name"))
				{
					DOMString attrval=attr.getNodeValue();
					strcpy(name,attrval.transcode());
				}
				else if(attrname.equals("File"))
				{
					DOMString attrval=attr.getNodeValue();
					strcpy(xml,attrval.transcode());
				}
				else if(attrname.equals("Directory"))
				{
					DOMString attrval=attr.getNodeValue();
					strcpy(dir,attrval.transcode());
				}
			}

			if(nodename.equals("BUILD"))
			{
				strcpy(gConfig.mBuildFile,xml);
			}
			else if(nodename.equals("TEXT"))
			{
				strcpy(gConfig.mTextDirectory,dir);
				gFontMaker.SetTextDir(gConfig.mTextDirectory);
			}
			else if(nodename.equals("TEXTURE_EXPORT"))
			{
				strcpy(gConfig.mTextureExport,dir);
				gFontMaker.SetTextureDir(gConfig.mTextureExport);
			}
			else if(nodename.equals("PC_EXPORT"))
			{
				strcpy(gConfig.mPCExport,dir);
			}
			else if(nodename.equals("SERVER_EXPORT"))
			{
				strcpy(gConfig.mServerExport,dir);
			}
			else if(nodename.equals("EDITOR"))
			{
				strcpy(gConfig.mEditorExe,xml);
			}
			else if(nodename.equals("BUNDLER"))
			{
				strcpy(gConfig.mBundlerExe,xml);
			}
		}
	}
}

void XMLParser::ParseFile(char *filename)
{
	XMLPlatformUtils::Initialize();
	gConfig.Default();

    DOMParser::ValSchemes    valScheme = DOMParser::Val_Auto;
    bool                     doNamespaces    = false;
	int XmlType=-1;

    DOMParser parser;
    parser.setValidationScheme(valScheme);
    parser.setDoNamespaces(doNamespaces);

    DOMCountErrorHandler errorHandler;
    parser.setErrorHandler(&errorHandler);

	parser.parse(filename);

    if (errorHandler.getSawErrors())
    {
        gCommandLine.AddLine(TC_RED,"Errors in XML file");
    }
    else
    {
        DOM_Document doc = parser.getDocument();
		DOM_NodeList nodelist = doc.getChildNodes();
		DOM_Node node;

		//Read in the header
		node=ReadHeader(&nodelist,&XmlType);
		gCommandLine.AddLine(TC_BLACK,"");

		if(XmlType == 1)
		{
			strcpy(flowfilename,filename);

			gCommandLine.AddLine(TC_BLACK,"Processing XML ...");

			//Now Sort out the Screens
			nodelist = node.getChildNodes();

			BuildConfig(&nodelist);
		}
		else
		{
			gCommandLine.AddLine(TC_RED,"Error - Unknown Flow XML format");
		}
	}
}

DOMCountErrorHandler::DOMCountErrorHandler() :

    fSawErrors(false)
{
}

DOMCountErrorHandler::~DOMCountErrorHandler()
{
}

void DOMCountErrorHandler::error(const SAXParseException& e)
{
    fSawErrors = true;

	char text[128];
	sprintf(text,"Error at Line: %d",e.getLineNumber());
	gCommandLine.AddLine(TC_RED,text);
}

void DOMCountErrorHandler::fatalError(const SAXParseException& e)
{
    fSawErrors = true;

	char text[128];
	sprintf(text,"Error at Line: %d",e.getLineNumber());
	gCommandLine.AddLine(TC_RED,text);
}

void DOMCountErrorHandler::warning(const SAXParseException& e)
{
	char text[128];
	sprintf(text,"Error at Line: %d",e.getLineNumber());
	gCommandLine.AddLine(TC_RED,text);
}

void DOMCountErrorHandler::resetErrors()
{
}
