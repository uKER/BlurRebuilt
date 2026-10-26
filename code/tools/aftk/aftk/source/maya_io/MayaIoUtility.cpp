#include "stdafx.h"
#include <Maya/MCommandResult.h>
#include <Maya/MFnEnumAttribute.h>
#include <Maya/MFnMatrixAttribute.h>

namespace MayaIo
{
	void (*g_pOutputFunc)(Aftk::E_OutputChannel channel, const std::string& text);


	U32 g_progressMax;
	U32 g_progress;


	void SetProgressMax(U32 max)
	{
		g_progressMax = max;
	}

	F32 GetProgressPercent()
	{
		return F32((F32)g_progress / (F32)g_progressMax)*100.0f;
	}

	U32 GetProgress()
	{
		return g_progress;
	}

	void SetProgress(U32 progress)
	{
		g_progress = progress;
	}

	void AdvanceProgress()
	{
		g_progress++;
	}

	std::string GetFirstToken(const std::string &input)
	{
		if (input.size() == 0)
		{
			return std::string("");
		}
		U32 index;
		for (index = 0; index < input.size(); index++)
		{
			char ch = input[index];
			if (isspace(ch))
			{
				break;
			}
		}
		return input.substr(0,index);
	}

	bool StripNodeNameFromPlugName(const std::string& inputName, std::string& outputName)
	{
		//Look thru our list of filtered names to determine if we are to export this plug.
		//First, remove the namespace of the plug. To do this we look for the first '.' char.
		
		S32 pos = (S32)inputName.find(".");
		if (pos != std::string::npos)
		{
			pos++;
			if (pos >= (S32)inputName.size())
			{
				return false;
			}
			U32 charsToCopy = (U32)(inputName.size() - pos);
			outputName = inputName.substr(pos, charsToCopy);
			
			return true;
		}

		return false;
	}

	bool StripNodeNameAndIndexFromPlugName(const std::string& inputName, std::string& outputName)
	{
		StripNodeNameFromPlugName(inputName, outputName);
		//Look thru our list of filtered names to determine if we are to export this plug.
		//First, remove the namespace of the plug. To do this we look for the first '.' char.

		std::string _inputName(outputName);
		
		S32 pos = (S32)_inputName.find("].");
		if (pos != std::string::npos)
		{
			pos+=2;
			if (pos >= (S32)_inputName.size())
			{
				return false;
			}
			U32 charsToCopy = (U32)(_inputName.size() - pos);
			outputName = _inputName.substr(pos, charsToCopy);
			
			return true;
		}

		return false;
	}

	bool BuildCorrectChildName(const std::string& parentName, const std::string& inputName, std::string& outputName)
	{
		//Look thru our list of filtered names to determine if we are to export this plug.
		//First, remove the namespace of the plug. To do this we look for the first '.' char.

		//Exclude parental name
		U32 charsToCopy = (U32)inputName.size()-(U32)parentName.size();
		U32 startIndex = (U32)parentName.size();
		outputName = inputName.substr(startIndex, charsToCopy);

		if (outputName.size() == 0)
		{
			return true;
		}

		//Lowercase first char
		outputName[0] = tolower(outputName[0]);

		return true;
	}

	MString comment(const MString& text)
	{
		MString	result("//");
		result += text;

		return result;
	}


	//
	// Convert a string into a quoted, printable string.
	//
	MString quote(const MString& str)
	{
		const char* cstr = str.asChar();
		int	strLen = str.length();
		int i;

		MString	result("\"");

		for (i = 0; i < strLen; i++)
		{
			int c = cstr[i];

			if (isprint(c))
			{
				//
				// Because backslash and double-quote have special meaning
				// within a printable string, we have to turn those into escape
				// sequences.
				//
				switch (c)
				{
					case '"':
						result += "\\\"";
					break;

					case '\\':
						result += "\\\\";
					break;

					default:
						result += MString((const char*)&c, 1);
					break;
				}
			}
			else
			{
				//
				// Convert non-printable characters into escape sequences.
				//
				switch (c)
				{
					case '\n':
						result += "\\n";
					break;

					case '\t':
						result += "\\t";
					break;

					case '\b':
						result += "\\b";
					break;

					case '\r':
						result += "\\r";
					break;

					case '\f':
						result += "\\f";
					break;

					case '\v':
						result += "\\v";
					break;

					case '\007':
						result += "\\a";
					break;

					default:
					{
						//
						// Encode it as an octal escape sequence.
						//
						char buff[5];
						sprintf(buff, "\\%.3o", c);
						result += MString(buff, 4);
					}
				}
			}
		}

		//
		// Add closing quote.
		//
		result += "\"";

		return result;
	}



	Aftk::C_Status ConnectPlugs(
		MFnDependencyNode sourceNode, 
		MString sourcePlugName, 
		MFnDependencyNode destinationNode,
		MString destinationPlugName)
	{
		//Connect two give plugs on two given nodes.
		Aftk::C_Status status;
		MStatus mStatus;
		MPlug sourcePlug = sourceNode.findPlug(sourcePlugName, &mStatus);
		MCHECK_BEGIN << "Could not find source plug" MCHECK_ERROR_RETURN_END_1;
		MPlug destinationPlug = destinationNode.findPlug(destinationPlugName, &mStatus);
		MCHECK_BEGIN << "Could not find destination plug" MCHECK_ERROR_RETURN_END_1;
		
		MDGModifier mdgModifier;
		mStatus = mdgModifier.connect(sourcePlug, destinationPlug); 
		MCHECK_BEGIN << "Could not connect plugs" MCHECK_ERROR_RETURN_END_1;
		
		return Aftk::C_Status::E_Status_Success;
	}


	template<class T>
	MStatus ExportNumericData(
		MFnNumericAttribute& mFnNumericAttribute,
		const MPlug& mayaPlug,
		T& aftkAttribute)
	{
		switch (mFnNumericAttribute.unitType())
		{
			case MFnNumericData::kDouble:
			{
				F64 val;
				mayaPlug.getValue(val);
				aftkAttribute.SetValue(val);
				break;
			}

			case MFnNumericData::kFloat:
			{
				F32 val;
				mayaPlug.getValue(val);
				aftkAttribute.SetValue(val);
				break;
			}

			case MFnNumericData::kByte:
			case MFnNumericData::kChar:
			{
				char val;
				mayaPlug.getValue(val);
				aftkAttribute.SetValue(val);
				break;
			}

			case MFnNumericData::kInt:
			case MFnNumericData::kShort:
			{
				S32 val;
				mayaPlug.getValue(val);
				aftkAttribute.SetValue(val);
				break;
			}

			case MFnNumericData::kBoolean:
			{
				bool val;
				mayaPlug.getValue(val);
				aftkAttribute.SetValue(val);
				break;
			}

			default:
				break;
		}
		
		return MStatus::kSuccess;
	}

	Aftk::C_Status MayaPlugToAftkDynamicPlug(
		C_Importer* pImporter,
		const MPlug& mayaPlug,
		Aftk::C_DynamicPlug& aftkPlug)
	{
		Aftk::C_Status status;
		MStatus mStatus;
		std::string strippedPlugName;
		StripNodeNameFromPlugName(mayaPlug.name().asChar(), strippedPlugName);
		aftkPlug.SetName(strippedPlugName);

		//Import children and bail if we can.
		if (mayaPlug.isCompound())
		{
			//We could not figure out a more tidy way of exporting this data, so export it
			//as a regular compound attribute.
			for (U32 childIndex = 0; childIndex < mayaPlug.numChildren(); childIndex++)
			{
				MPlug childPlug = mayaPlug.child(childIndex, &mStatus);
				if (!mStatus)
					continue;

				Aftk::C_DynamicPlug* pAftkChildPlug = new Aftk::C_DynamicPlug();
				aftkPlug.AddChildPtr(pAftkChildPlug);

				status = MayaPlugToAftkDynamicPlug(
					pImporter,
					mayaPlug.child(childIndex),
					*pAftkChildPlug);
				AFTK_CHECK_ERROR_RETURN;
			}

			return Aftk::C_Status::E_Status_Success;
		}

		if (mayaPlug.isArray(&mStatus))
		{
			MCHECK_ERROR_RETURN_1;
			return Aftk::C_Status::E_Status_Success;
		}
		MCHECK_ERROR_RETURN_1;

		MFn::Type apiType = mayaPlug.attribute().apiType();
		if (apiType == MFn::kTypedAttribute)
		{
			MFnTypedAttribute mFnTypedAttribute(mayaPlug.attribute(), &mStatus);
			if (!mStatus) return Aftk::C_Status::E_Status_Success;
		
			switch (mFnTypedAttribute.attrType())
			{
				case MFnData::kIntArray:
				{
					int attrInt;
					mayaPlug.getValue(attrInt);
					aftkPlug.SetValue(attrInt);
					break;
				}
				case MFnData::kDoubleArray:
				{
					double attrDouble;
					mayaPlug.getValue(attrDouble);
					aftkPlug.SetValue(attrDouble);
					break;
				}
				case MFnData::kStringArray:
				{
					break;
				}
				
				case MFnData::kString:
				{
					if (mayaPlug.isArray())
					{
						break;
					}
					MFnStringData stringData(mFnTypedAttribute.object(), &mStatus);
					if (!mStatus)
					{
						std::string stdString;
						#ifdef DEBUG_MATERIALS
						g_pluginDebugManager << "Warning: could not acquire material string: stage 1\n";
						#endif
	// The code below crashes Maya
	//					MString attrString;
	//					mStatus = mayaPlug.getValue(attrString);
	//					stdString = attrString.asChar();

						std::string command;
						MObject theNodeObject;
						if (mayaPlug.isChild())
						{
							theNodeObject = mayaPlug.parent().node(&mStatus);
							MCHECK_ERROR_RETURN_1;
						}
						else
						{
							theNodeObject = mayaPlug.node(&mStatus);
							MCHECK_ERROR_RETURN_1;
						}

						MFnDependencyNode fnTheNode(theNodeObject, &mStatus);
						MCHECK_ERROR_RETURN_1;

						command += "getAttr ";
						command += fnTheNode.name().asChar();
						command += ".";
						command += strippedPlugName;
						MStringArray resultArray;

						mStatus = MGlobal::executeCommand(command.c_str(), resultArray, false, false);
						if (mStatus)
						{
							if (resultArray.length() > 0)
							{
								stdString = resultArray[0].asChar();
								aftkPlug.SetValue(stdString);
								#ifdef DEBUG_PRINT
								g_pluginDebugManager << "String dyn attr. " << stdString << "\n";
								#endif
							}
						}
					}
					break;
				}

				case MFnData::kMatrix:
				{
					Aftk::Matrix44 val;
					MFnMatrixData mFnMatrixData(mFnTypedAttribute.object());
					MMatrix mayaMatrix = mFnMatrixData.matrix();
					
					F32 tempMatrix1[4][4];
					mayaMatrix.get(tempMatrix1);
					Aftk::Vector4F32 ax1(tempMatrix1[0][0],tempMatrix1[0][1],tempMatrix1[0][2],0);
					Aftk::Vector4F32 ay1(tempMatrix1[1][0],tempMatrix1[1][1],tempMatrix1[1][2],0);
					Aftk::Vector4F32 az1(tempMatrix1[2][0],tempMatrix1[2][1],tempMatrix1[2][2],0);
					Aftk::Vector4F32 aw1(tempMatrix1[3][0],tempMatrix1[3][1],tempMatrix1[3][2],1);

					val.At<0>() = ax1;
					val.At<1>() = ay1;
					val.At<2>() = az1;
					val.At<3>() = aw1;
					aftkPlug.SetValue(val);
					break;
				}

				case MFnData::kNumeric:
				{
					MFnNumericAttribute mFnNumericAttribute(mFnTypedAttribute.object());
					ExportNumericData(mFnNumericAttribute, mayaPlug, aftkPlug);
					break;
				}
				default:
					break;
			}
		}
		else if (apiType == MFn::kNumericAttribute)
		{
			MFnNumericAttribute mFnNumericAttribute(mayaPlug.attribute(), &mStatus);
			MCHECK_ERROR_RETURN_1;
			ExportNumericData(mFnNumericAttribute, mayaPlug, aftkPlug);
		}
		else if (apiType == MFn::kEnumAttribute)
		{
			MFnEnumAttribute fnEnumAttribute(mayaPlug.attribute(), &mStatus);
			MCHECK_ERROR_RETURN_1;

			S32 enumValue;
			mayaPlug.getValue(enumValue);
			MString enumString = fnEnumAttribute.fieldName(enumValue, &mStatus);
			if (mStatus)
				aftkPlug.SetValue(std::string(enumString.asChar()));
			else
			{
				AFTK_DO_BEGIN << "Invalid enum index for plug " << mayaPlug.name().asChar() AFTK_DO_WARNING_END;
				aftkPlug.SetValue("");
			}
		}
		else if (apiType == MFn::kMatrixAttribute)
		{
			MObject oMatrix;
			mStatus = mayaPlug.getValue(oMatrix);
			MCHECK_ERROR_RETURN_1;

			Aftk::Matrix44 val;
			MFnMatrixData mFnMatrixData(oMatrix, &mStatus);
			MCHECK_ERROR_RETURN_1;
			MMatrix mayaMatrix = mFnMatrixData.matrix();
			
			F32 tempMatrix1[4][4];
			mayaMatrix.get(tempMatrix1);
			Aftk::Vector4F32 ax1(tempMatrix1[0][0],tempMatrix1[0][1],tempMatrix1[0][2],0);
			Aftk::Vector4F32 ay1(tempMatrix1[1][0],tempMatrix1[1][1],tempMatrix1[1][2],0);
			Aftk::Vector4F32 az1(tempMatrix1[2][0],tempMatrix1[2][1],tempMatrix1[2][2],0);
			Aftk::Vector4F32 aw1(tempMatrix1[3][0],tempMatrix1[3][1],tempMatrix1[3][2],1);

			val.At<0>() = ax1;
			val.At<1>() = ay1;
			val.At<2>() = az1;
			val.At<3>() = aw1;
			aftkPlug.SetValue(val);
		}
		else
		{
			//We really have no idea how to handle the data for this plug.
			//We assume that the content of this plug is NULL.
			#ifdef DEBUG_MATERIALS
			g_pluginDebugManager << "Warning - unknown attribute fn type\n";
			#endif
		}

		// Attribute may be driven
		{
			Aftk::I_Object* pObject = C_ConnectedDgImport::ImportDirectlyConnectedPlug(
				pImporter,
				mayaPlug,
				status);
			CHECK_ERROR_RETURN;
			if (pObject)
				aftkPlug.SetSourceConnectionPtr(pObject);
		}

		return Aftk::C_Status::E_Status_Success;
	}

	//TODO: This is only used by the speed tree export - we need to refactor to roll the dynamic plug
	//and attr exporting into one code path rather than this disgusting hackery.
	Aftk::C_Status MayaPlugToAftkGenericAttribute(
		const MPlug& mayaPlug,
		Aftk::C_GenericAttribute& aftkGenericAttribute)
	{
		Aftk::C_Status status;
		MStatus mStatus;
		std::string strippedPlugName;
		StripNodeNameFromPlugName(mayaPlug.name().asChar(), strippedPlugName);
		aftkGenericAttribute.SetName(strippedPlugName);

		if (mayaPlug.isCompound(&mStatus))
		{
			MCHECK_ERROR_RETURN_1;
			return Aftk::C_Status::E_Status_Success;
		}
		MCHECK_ERROR_RETURN_1;
		if (mayaPlug.isArray(&mStatus))
		{
			MCHECK_ERROR_RETURN_1;
			return Aftk::C_Status::E_Status_Success;
		}
		MCHECK_ERROR_RETURN_1;

		if (mayaPlug.attribute().apiType() == MFn::kTypedAttribute)
		{
			MFnTypedAttribute mFnTypedAttribute(mayaPlug.attribute(), &mStatus);
			if (!mStatus) return Aftk::C_Status::E_Status_Success;
		
			switch (mFnTypedAttribute.attrType())
			{
				case MFnData::kIntArray:
				{
					int attrInt;
					mayaPlug.getValue(attrInt);
					aftkGenericAttribute.SetValue(attrInt);
					break;
				}
				case MFnData::kDoubleArray:
				{
					double attrDouble;
					mayaPlug.getValue(attrDouble);
					aftkGenericAttribute.SetValue(attrDouble);
					break;
				}
				case MFnData::kStringArray:
				{
					break;
				}
				
				case MFnData::kString:
				{
					if (mayaPlug.isArray())
					{
						break;
					}
					MFnStringData stringData(mFnTypedAttribute.object(), &mStatus);
					if (!mStatus)
					{
						std::string stdString;
						#ifdef DEBUG_MATERIALS
						g_pluginDebugManager << "Warning: could not acquire material string: stage 1\n";
						#endif
	// The code below crashes Maya
	//					MString attrString;
	//					mStatus = mayaPlug.getValue(attrString);
	//					stdString = attrString.asChar();

						std::string command;
						MObject theNodeObject;
						if (mayaPlug.isChild())
						{
							theNodeObject = mayaPlug.parent().node(&mStatus);
							MCHECK_ERROR_RETURN_1;
						}
						else
						{
							theNodeObject = mayaPlug.node(&mStatus);
							MCHECK_ERROR_RETURN_1;
						}

						MFnDependencyNode fnTheNode(theNodeObject, &mStatus);
						MCHECK_ERROR_RETURN_1;

						command += "getAttr ";
						command += fnTheNode.name().asChar();
						command += ".";
						command += strippedPlugName;
						MStringArray resultArray;

						mStatus = MGlobal::executeCommand(command.c_str(), resultArray, false, false);
						if (mStatus)
						{
							if (resultArray.length() > 0)
							{
								stdString = resultArray[0].asChar();
								aftkGenericAttribute.SetValue(stdString);
								#ifdef DEBUG_PRINT
								g_pluginDebugManager << "String dyn attr. " << stdString << "\n";
								#endif
							}
						}
					}
					break;
				}

				case MFnData::kMatrix:
				{
					Aftk::Matrix44 val;
					MFnMatrixData mFnMatrixData(mFnTypedAttribute.object());
					MMatrix mayaMatrix = mFnMatrixData.matrix();
					
					F32 tempMatrix1[4][4];
					mayaMatrix.get(tempMatrix1);
					Aftk::Vector4F32 ax1(tempMatrix1[0][0],tempMatrix1[0][1],tempMatrix1[0][2],0);
					Aftk::Vector4F32 ay1(tempMatrix1[1][0],tempMatrix1[1][1],tempMatrix1[1][2],0);
					Aftk::Vector4F32 az1(tempMatrix1[2][0],tempMatrix1[2][1],tempMatrix1[2][2],0);
					Aftk::Vector4F32 aw1(tempMatrix1[3][0],tempMatrix1[3][1],tempMatrix1[3][2],1);

					val.At<0>() = ax1;
					val.At<1>() = ay1;
					val.At<2>() = az1;
					val.At<3>() = aw1;
					aftkGenericAttribute.SetValue(val);
					break;
				}
				case MFnData::kNumeric:
				{
					MFnNumericAttribute mFnNumericAttribute(mFnTypedAttribute.object());
					ExportNumericData(mFnNumericAttribute, mayaPlug, aftkGenericAttribute);
					break;
				}
				default:
					break;
			}
		}
		else if (mayaPlug.attribute().apiType() == MFn::kNumericAttribute)
		{
			MFnNumericAttribute mFnNumericAttribute(mayaPlug.attribute());
			ExportNumericData(mFnNumericAttribute, mayaPlug, aftkGenericAttribute);
		}
		else
		{
			//We really have no idea how to handle the data for this plug.
			//We assume that the content of this plug is NULL.
		}

		return Aftk::C_Status::E_Status_Success;
	}

	bool GetOptionValueString(const char* pStr, std::string &value)
	{
		char delimiter[] = "=";

		std::string tokenStd(pStr);

		size_t pos = tokenStd.find(delimiter);
		if (pos == std::string::npos)
		{
			return false;
		}
		value = tokenStd.substr(pos+1, tokenStd.length()-pos-1);
		return true;
	}


	bool GetOptionValueBool(const char* pStr, bool &value)
	{
		char delimiter[] = "=";

		std::string tokenStd(pStr);

		size_t pos = tokenStd.find(delimiter);
		if (pos == std::string::npos)
		{
			return false;
		}
		tokenStd = tokenStd.substr(pos+1, tokenStd.length()-pos-1);
		if (tokenStd == "false" || tokenStd == "0")
		{
			value = false;
			return true;
		}

		value = true;
		return true;
	}

	bool GetOptionValueF32(const char* pStr, F32 &value)
	{
		char delimiter[] = "=";

		std::string tokenStd(pStr);

		size_t pos = tokenStd.find(delimiter);
		if (pos == std::string::npos)
		{
			return false;
		}
		tokenStd = tokenStd.substr(pos+1, tokenStd.length()-pos-1);
		std::stringstream ss;
		ss << tokenStd;
		ss >> value;
		return true;
	}
}