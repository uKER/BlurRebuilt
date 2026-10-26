#include "stdafx.h"

void GameLoadDebugVariables()
{
	FILE *fh;
	
	if (fh = fopen("GAME:\\debug_values.xml", "r"))
	{
		char text_buffer[1024];

		// Skip the <Values>
		fgets(text_buffer, lengthof(text_buffer), fh);

		while (!feof(fh))
		{
			fgets(text_buffer, lengthof(text_buffer), fh);
			
			if (!strncmp(text_buffer, "</Values>", 9))
				break;

			// Parse the line...
			char* startOfName = strstr(text_buffer, "name=\"");
			char* startOfType = strstr(text_buffer, "type=\"");
			char* startOfValue = strstr(text_buffer, "value=\"");
			if (startOfName && startOfType && startOfValue)
			{
				char* name = startOfName + 6;
				int nameLength(0);
				for (const char *cptr=name; *cptr != '\"'; cptr++, nameLength++);

				char* type = startOfType + 6;
				int typeLength(0);
				for (const char *cptr=type; *cptr != '\"'; cptr++, typeLength++);

				char* value = startOfValue + 7;
				int valueLength(0);
				for (const char *cptr=value; *cptr != '\"'; cptr++, valueLength++);

				name[nameLength] = '\0';
				type[typeLength] = '\0';
				value[valueLength] = '\0';

				// Find the variable and set it's value
				for (Debug::Iterator it; it; ++it)
				{
					if (stricmp(it->Name(), name) != 0)
						continue;

					switch (it->Type())
					{
					case Debug::kVtBool:
						{
							if (!strcmp(type, "bool"))
							{
								Debug::Bool& var = (Debug::Bool&)*it;

								if (stricmp(value, "true") == 0)
									var = true;
								else if (stricmp(value, "false") == 0)
									var = false;
								else
									var = (atoi(value) != 0);
							}
							else assert(false);
							break;
						}

					case Debug::kVtInt:
						{
							if (!strcmp(type, "int"))
							{
								Debug::Int& var = (Debug::Int&)*it;
								var = atoi(value);
							}
							else assert(false);
							break;
						}

					case Debug::kVtFloat:
						{
							if (!strcmp(type, "float"))
							{
								Debug::Float& var = (Debug::Float&)*it;
								var = (float)atof(value);
							}
							else assert(false);
							break;
						}

					case Debug::kVtEnum:
						{
							if (!strcmp(type, "enum"))
							{
								Debug::EnumBase& var = (Debug::EnumBase&)*it;
								var.SetValueByName(value);
							}
							else assert(false);
							break;
						}
					
					case Debug::kVtColour:
						{
							if (!strcmp(type, "colour"))
							{
								float r, g, b, a;
								sscanf(value, "%f %f %f %f", &r, &g, &b, &a);

								Debug::Colour& var = (Debug::Colour&)*it;
								var.SetValue(r, g, b, a);
							}
							else assert(false);
							break;
						}

					case Debug::kVtVector3:
					case Debug::kVtFunction:
						break;

					default:
						assert(false);
					}

					break;
				}
			}
		}
		fclose(fh);
	}
}

void GameSaveDebugVariables()
{
	FILE *fh;
	
	if (fh = fopen("GAME:\\debug_values.xml", "w"))
	{
		fputs("<Values>\n", fh);

		for (Debug::Iterator it; it; ++it)
		{
			if (it->Flags() & Debug::kVfPersist)
			{
				char text_buffer[1024];

				sprintf_s(text_buffer, lengthof(text_buffer), "\t<Variable name=\"%s\" ", it->Name());
				fputs(text_buffer, fh);

				// Save me please
				switch (it->Type())
				{
				case Debug::kVtBool:
				{
					const Debug::Bool& var = (Debug::Bool&)*it;
					sprintf_s(text_buffer, lengthof(text_buffer), "type=\"bool\" value=\"%s\"", var.Value() ? "true" : "false");
					break;
				}
				case Debug::kVtInt:
				{
					const Debug::Int& var = (Debug::Int&)*it;
					sprintf_s(text_buffer, lengthof(text_buffer), "type=\"int\" value=\"%d\"", var.Value());
					break;
				}
				case Debug::kVtFloat:
				{
					const Debug::Float& var = (Debug::Float&)*it;
					sprintf_s(text_buffer, lengthof(text_buffer), "type=\"float\" value=\"%f\"", var.Value());
					break;
				}
				case Debug::kVtEnum:
				{
					const Debug::EnumBase& var = (Debug::EnumBase&)*it;
					sprintf_s(text_buffer, lengthof(text_buffer), "type=\"enum\" value=\"%s\"", var.ValueAsName());
					break;
				}
				case Debug::kVtColour:
				{
					const Debug::Colour& var = (Debug::Colour&)*it;
					sprintf_s(text_buffer, lengthof(text_buffer), "type=\"colour\" value=\"%f\"", var->r, var->g, var->b, var->a);
					break;
				}

				// Yet to do
		//kVtString,
		//kVtFunction,
		//kVtVector3,
		//kVtColour

				default:
					assert(false);
				}

				fputs(text_buffer, fh);
				fputs("/>\n", fh);
			}
		}
		fputs("</Values>\n", fh);
		fclose(fh);
	}
}

void InitialiseDebugVariables()
{
	// startup Xnet first
	XNetStartupParams xnsp;
	ZeroMemory(&xnsp, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(xnsp);
#ifndef CFG_GOLD
	// flags are ignored when linking with secure Xenon libraries..
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
#endif
	if (NO_ERROR != XNetStartup(&xnsp))
		assert(!"XNetStartup failed.");

	gGameLoadDebugVariables = GameLoadDebugVariables;
	gGameSaveDebugVariables = GameSaveDebugVariables;
}