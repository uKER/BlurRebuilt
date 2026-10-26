#include "stdafx.h"

//------------------------------------------------------------------------------
//  Make
//------------------------------------------------------------------------------
bool Audio::Script::Make(const char* pchName, const char* pchSource, const char* pchPrefix)
{
    Release();

    try
    {
        m_iCurrLine = 0;

        Init();
        Lex(pchSource);
        Parse();
        Validate(pchPrefix);
        Create();
	}
	catch (Error& err)
	{
		printf("Script error in '%s': %s" , pchName, err.What());

        if (err.When())
		    printf(" at line %d", err.When());

	    printf("\n");

		system("pause 0");

        return false;
	}

    CleanUp();

    return true;
}

//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------
void Audio::Script::Release()
{
    CleanUp();

    if (m_pwCode)
    {
        delete[] m_pwCode;
        m_pwCode = NULL;
    }
}

//------------------------------------------------------------------------------
//  CleanUp
//------------------------------------------------------------------------------
void Audio::Script::CleanUp()
{
    m_iCurrLine = 0;

    for (Audio::Iterator<Token*> i1 = m_lTokens.first(); !i1.isPastEnd(); i1.advance())
        delete i1.retrieve();

    m_lTokens.makeEmpty();

    for (Audio::Iterator<Label*> i2 = m_lLabels.first(); !i2.isPastEnd(); i2.advance())
        delete i2.retrieve();

    m_lLabels.makeEmpty();

    for (Audio::Iterator<Instruction*> i3 = m_lInstructions.first(); !i3.isPastEnd(); i3.advance())
        delete i3.retrieve();

    m_lInstructions.makeEmpty();

    for (int i = 0; i < (int)m_lMessages.size(); i++)
        delete m_lMessages[i];

    m_lMessages.resize(0);
}

//------------------------------------------------------------------------------
//  Init
//  create message list
//------------------------------------------------------------------------------
void Audio::Script::Init()
{
    //  init valid messages

	m_lMessages.addEntry(new Message("OneShotSound", "PLAY", OneShotSound::ePlay));
    m_lMessages.addEntry(new Message("OneShotSound", "STOP", OneShotSound::eStop));

    m_lMessages.addEntry(new Message("LoopingSound", "PLAY", LoopingSound::ePlay));
    m_lMessages.addEntry(new Message("LoopingSound", "STOP", LoopingSound::eStop));

//    m_lMessages.addEntry(new Message("StreamingSound", "PLAY", StreamingSound::ePlay));
//    m_lMessages.addEntry(new Message("StreamingSound", "STOP", StreamingSound::eStop));

	m_lMessages.addEntry(new Message("Timer", "START", Timer::eStart));
    m_lMessages.addEntry(new Message("Timer", "STOP", Timer::eStop));
    m_lMessages.addEntry(new Message("Timer", "RESET", Timer::eReset));

    m_lMessages.addEntry(new Message("StaticRandomInt", "GENERATE_NEW", StaticRandomInt::eGenerateNew));
    m_lMessages.addEntry(new Message("StaticRandomInt", "REGENERATE", StaticRandomInt::eRegenerate));

	m_lMessages.addEntry(new Message("DynamicRandomInt", "GENERATE_NEW", DynamicRandomInt::eGenerateNew));
    m_lMessages.addEntry(new Message("DynamicRandomInt", "REGENERATE", DynamicRandomInt::eRegenerate));

	m_lMessages.addEntry(new Message("StaticRandomFloat", "REGENERATE", StaticRandomFloat::eRegenerate));

	m_lMessages.addEntry(new Message("DynamicRandomFloat", "REGENERATE", DynamicRandomFloat::eRegenerate));

	m_lMessages.addEntry(new Message("Toggle", "TOGGLE", Toggle::eToggle));
	m_lMessages.addEntry(new Message("Toggle", "TURN_ON", Toggle::eTurnOn));
    m_lMessages.addEntry(new Message("Toggle", "TURN_OFF", Toggle::eTurnOff));

	m_lMessages.addEntry(new Message("PeakHold", "RESET", PeakHold::eReset));

	m_lMessages.addEntry(new Message("SampleAndHold", "SAMPLE", SampleAndHold::eSample));
}

//------------------------------------------------------------------------------
//  Lex
//  create token list from char stream
//------------------------------------------------------------------------------
void Audio::Script::Lex(const char* pchText)
{
    char* pchCurr = (char*)pchText;

    while (pchCurr)
    {
        Token* pToken = new Token(m_iCurrLine+1);
        pchCurr = GetToken(pchCurr, &pToken->GetName());

        if (pchCurr)
   		    m_lTokens.insert(pToken, m_lTokens.last());
        else
            delete pToken;
    }
}

//------------------------------------------------------------------------------
//  Parse
//  create instruction and label lists from token list
//------------------------------------------------------------------------------
void Audio::Script::Parse()
{
    m_iOffset = 0;
    Label* pLabel = NULL;

	for (Audio::Iterator<Token*> i = m_lTokens.first(); !i.isPastEnd(); i.advance())
	{
		Token* pToken = i.retrieve();

        //  if label
        if (pToken->GetName().at(pToken->GetName().length()-1) == ':')
        {
            pLabel = new Label;

            pLabel->GetName() = pToken->GetName().substr(0, pToken->GetName().length()-1);

   		    m_lLabels.insert(pLabel, m_lLabels.last());
        }
        //  else opcode
        else 
        {
            Instruction* pInstruction = NULL;

            //  if 3 token opcode
            if (pToken->GetName().compare(std::string("SEND")) == 0)
            {
                pInstruction = new Instruction(pToken->GetLine());
                pInstruction->SetOffset(m_iOffset);
                pInstruction->GetOpCodeString() = pToken->GetName();

                //  1st param
                i.advance();
                pToken = i.retrieve();
                pInstruction->GetParam1String() = pToken->GetName();

                //  2nd param
                i.advance();
                pToken = i.retrieve();
                pInstruction->GetParam2String() = pToken->GetName();

   		        m_lInstructions.insert(pInstruction, m_lInstructions.last());
                m_iOffset += 3;
            }
            //  else 2 token opcode
            else if ((pToken->GetName().compare(std::string("PUSH")) == 0) ||
                (pToken->GetName().compare(std::string("PULL")) == 0) ||
                (pToken->GetName().compare(std::string("JUMP")) == 0) ||
                (pToken->GetName().compare(std::string("JEQ")) == 0) ||
                (pToken->GetName().compare(std::string("JNEQ")) == 0) ||
                (pToken->GetName().compare(std::string("JGT")) == 0) ||
                (pToken->GetName().compare(std::string("JNGT")) == 0) ||
                (pToken->GetName().compare(std::string("JLT")) == 0) ||
                (pToken->GetName().compare(std::string("JNLT")) == 0) ||
                (pToken->GetName().compare(std::string("EXEC")) == 0) ||
                (pToken->GetName().compare(std::string("ADD")) == 0) ||
                (pToken->GetName().compare(std::string("SUB")) == 0) ||
                (pToken->GetName().compare(std::string("MUL")) == 0) ||
                (pToken->GetName().compare(std::string("DIV")) == 0) ||
                (pToken->GetName().compare(std::string("CALL")) == 0))
            {
                pInstruction = new Instruction(pToken->GetLine());
                pInstruction->SetOffset(m_iOffset);
                pInstruction->GetOpCodeString() = pToken->GetName();

                //  1st param
                i.advance();
                pToken = i.retrieve();
                pInstruction->GetParam1String() = pToken->GetName();

   		        m_lInstructions.insert(pInstruction, m_lInstructions.last());
                m_iOffset += 2;
            }
            //  else 1 token opcode
            else if ((pToken->GetName().compare(std::string("COMPARE")) == 0) ||
                (pToken->GetName().compare(std::string("DISCARD")) == 0) ||
                (pToken->GetName().compare(std::string("DESTRUCT")) == 0) ||
                (pToken->GetName().compare(std::string("EXIT")) == 0))
            {
                pInstruction = new Instruction(pToken->GetLine());
                pInstruction->SetOffset(m_iOffset);
                pInstruction->GetOpCodeString() = pToken->GetName();

   		        m_lInstructions.insert(pInstruction, m_lInstructions.last());
                m_iOffset ++;
            }
            else
                throw Error("Unknown script instruction", pToken->GetLine());

            //  previous token was a label
            if (pLabel)
            {
                //  set label's instruction
                pLabel->SetInstruction(pInstruction);
                pLabel = NULL;
            }
        }
	}
}

//------------------------------------------------------------------------------
//  Validate
//  check semantics, get enumerated elements
//------------------------------------------------------------------------------
void Audio::Script::Validate(const char* pchPrefix)
{
	for (Audio::Iterator<Instruction*> i = m_lInstructions.first(); !i.isPastEnd(); i.advance())
	{
		Instruction* pInstruction = i.retrieve();

        //  SEND [com] [mes]
        if (pInstruction->GetOpCodeString().compare(std::string("SEND")) == 0)
        {
            pInstruction->SetOpCode(eSend);

            //  get component
            int iCom = GetComponentIndex(std::string(pchPrefix) + pInstruction->GetParam2String());

            if (iCom != -1)
                pInstruction->SetParam1(iCom);
            else
                throw Error("Unknown component in SEND", pInstruction->GetLine());

            //  get message
            Message* pMessage = GetMessage(std::string(pchPrefix) + pInstruction->GetParam2String(), pInstruction->GetParam1String());

            if (pMessage)
                pInstruction->SetParam2(pMessage->GetEnum());
            else
                throw Error("Unknown message in SEND", pInstruction->GetLine());
        }
        //  PUSH [com] / PUSH [val]
        else if (pInstruction->GetOpCodeString().compare(std::string("PUSH")) == 0)
        {
            //  get component
            int iCom = GetComponentIndex(std::string(pchPrefix) + pInstruction->GetParam1String());

            if (iCom != -1)
            {
                pInstruction->SetOpCode(ePush);
                pInstruction->SetParam1(iCom);
            }
            else
                throw Error("Unknown component in PUSH", pInstruction->GetLine());
        }
        //  PULL [com]
        else if (pInstruction->GetOpCodeString().compare(std::string("PULL")) == 0)
        {
            //  get component
            int iCom = GetComponentIndex(std::string(pchPrefix) + pInstruction->GetParam1String());

            if (iCom == -1)
                throw Error("Unknown component in PULL", pInstruction->GetLine());
			else if (GetComponentType(std::string(pchPrefix) + pInstruction->GetParam1String()) != "Variable")
				throw Error("Trying to PULL a component other than a Variable", pInstruction->GetLine());
			else
            {
                pInstruction->SetOpCode(ePull);
                pInstruction->SetParam1(iCom);
            }
        }
        //  JUMP/JEQ/JNEQ/JGT/JNGT/JLT/JNLT [label]
        else if ((pInstruction->GetOpCodeString().compare(std::string("JUMP")) == 0) ||
            (pInstruction->GetOpCodeString().compare(std::string("JEQ")) == 0) ||
            (pInstruction->GetOpCodeString().compare(std::string("JNEQ")) == 0) ||
            (pInstruction->GetOpCodeString().compare(std::string("JLT")) == 0) ||
            (pInstruction->GetOpCodeString().compare(std::string("JNLT")) == 0) ||
            (pInstruction->GetOpCodeString().compare(std::string("JGT")) == 0) ||
            (pInstruction->GetOpCodeString().compare(std::string("JNGT")) == 0))
        {
            //  get label
            int iLabel = GetLabelOffset(pInstruction->GetParam1String());

            if (iLabel != -1)
            {
                if (pInstruction->GetOpCodeString().compare(std::string("JEQ")) == 0)
                    pInstruction->SetOpCode(eJumpEqual);
                else if (pInstruction->GetOpCodeString().compare(std::string("JNEQ")) == 0)
                    pInstruction->SetOpCode(eJumpNotEqual);
                else if (pInstruction->GetOpCodeString().compare(std::string("JLT")) == 0)
                    pInstruction->SetOpCode(eJumpLessThan);
                else if (pInstruction->GetOpCodeString().compare(std::string("JNLT")) == 0)
                    pInstruction->SetOpCode(eJumpNotLessThan);
                else if (pInstruction->GetOpCodeString().compare(std::string("JGT")) == 0)
                    pInstruction->SetOpCode(eJumpGreaterThan);
                else if (pInstruction->GetOpCodeString().compare(std::string("JNGT")) == 0)
                    pInstruction->SetOpCode(eJumpNotGreaterThan);
                else // JUMP
                    pInstruction->SetOpCode(eJump);

                pInstruction->SetParam1(iLabel);           
            }
            //  else get immediate
            else
                throw Error("Unknown label in JUMP", pInstruction->GetLine());
        }
        //  EXEC [script]
        else if (pInstruction->GetOpCodeString().compare(std::string("EXEC")) == 0)
        {
            //  get script
            int iScript = GetScriptIndex(std::string(pchPrefix) + pInstruction->GetParam1String());

            if (iScript != -1)
            {
                pInstruction->SetOpCode(eExecute);
                pInstruction->SetParam1(iScript);
            }
            else
            {
                throw Error("Unknown script in EXEC", pInstruction->GetLine());
            }
        }
        //  CALL [func]
        else if (pInstruction->GetOpCodeString().compare(std::string("CALL")) == 0)
        {
            //  get function
            int iFunction = GetFunctionIndex(pInstruction->GetParam1String());

            if (iFunction != -1)
            {
                pInstruction->SetOpCode(eCall);
                pInstruction->SetParam1(iFunction);
            }
            else
            {
                throw Error("Unknown script in CALL", pInstruction->GetLine());
            }
        }
        //  COMPARE
        else if (pInstruction->GetOpCodeString().compare(std::string("COMPARE")) == 0)
            pInstruction->SetOpCode(eCompare);
        //  DISCARD
        else if (pInstruction->GetOpCodeString().compare(std::string("DISCARD")) == 0)
            pInstruction->SetOpCode(eDiscard);
        //  DESTRUCT
        else if (pInstruction->GetOpCodeString().compare(std::string("DESTRUCT")) == 0)
            pInstruction->SetOpCode(eDestruct);
        //  ADD
        else if (pInstruction->GetOpCodeString().compare(std::string("ADD")) == 0)
		{
            //  get component
            int iCom = GetComponentIndex(std::string(pchPrefix) + pInstruction->GetParam1String());
            if (iCom != -1)
            {
				pInstruction->SetOpCode(eAdd);
				pInstruction->SetParam1(iCom);
            }
            else
                throw Error("Unknown component in ADD", pInstruction->GetLine());
		}
        //  SUB
        else if (pInstruction->GetOpCodeString().compare(std::string("SUB")) == 0)
		{
            int iCom = GetComponentIndex(std::string(pchPrefix) + pInstruction->GetParam1String());
            if (iCom != -1)
            {
				pInstruction->SetOpCode(eSubtract);
				pInstruction->SetParam1(iCom);
            }
            else
                throw Error("Unknown component in SUB", pInstruction->GetLine());
		}
        //  MUL
        else if (pInstruction->GetOpCodeString().compare(std::string("MUL")) == 0)
		{
            int iCom = GetComponentIndex(std::string(pchPrefix) + pInstruction->GetParam1String());
            if (iCom != -1)
            {
				pInstruction->SetOpCode(eMultiply);
				pInstruction->SetParam1(iCom);
            }
            else
                throw Error("Unknown component in MUL", pInstruction->GetLine());
		}
        //  DIV
        else if (pInstruction->GetOpCodeString().compare(std::string("DIV")) == 0)
		{
            int iCom = GetComponentIndex(std::string(pchPrefix) + pInstruction->GetParam1String());
            if (iCom != -1)
            {
				pInstruction->SetOpCode(eDivide);
				pInstruction->SetParam1(iCom);
            }
            else
                throw Error("Unknown component in DIV", pInstruction->GetLine());
		}
        //  EXIT
        else if (pInstruction->GetOpCodeString().compare(std::string("EXIT")) == 0)
            pInstruction->SetOpCode(eExit);
        //  BREAK
        else if (pInstruction->GetOpCodeString().compare(std::string("BREAK")) == 0)
            pInstruction->SetOpCode(eBreak);
		else
			throw Error("Unknown opcode in script", pInstruction->GetLine());
	}
}

//------------------------------------------------------------------------------
//  Create
//------------------------------------------------------------------------------
void Audio::Script::Create()
{
    int iOffset = 0;

    if (m_pwCode)
        delete[] m_pwCode;

    m_pwCode = new WORD[m_iOffset];

    //  create instruction table
	for (Audio::Iterator<Instruction*> i = m_lInstructions.first(); !i.isPastEnd(); i.advance())
	{
        Instruction* pInstruction = i.retrieve();

        m_pwCode[iOffset] = DoEndian(pInstruction->GetOpCode());
        iOffset++;

        if (pInstruction->GetParam1() != 0xffff)
        {
            m_pwCode[iOffset] = DoEndian(pInstruction->GetParam1());
            iOffset++;
        }

        if (pInstruction->GetParam2() != 0xffff)
        {
            m_pwCode[iOffset] = DoEndian(pInstruction->GetParam2());
            iOffset++;
        }
    }
}

//------------------------------------------------------------------------------
//  GetComponentIndex
//------------------------------------------------------------------------------
int Audio::Script::GetComponentIndex(std::string& strName)
{
    //  scan for component
    for (Audio::Iterator<Script::ComponentInfo*> i = m_plComponentInfos->first(); !i.isPastEnd(); i.advance())
	{
        Script::ComponentInfo* pInfo = i.retrieve();

        if (strName.compare(pInfo->GetName()) == 0)
            return pInfo->GetIndex(); 
    }

    return -1;
}

//------------------------------------------------------------------------------
//  GetComponentType
//------------------------------------------------------------------------------
std::string Audio::Script::GetComponentType(std::string& strName)
{
    //  scan for component
    for (Audio::Iterator<Script::ComponentInfo*> i = m_plComponentInfos->first(); !i.isPastEnd(); i.advance())
	{
        Script::ComponentInfo* pInfo = i.retrieve();

        if (strName.compare(pInfo->GetName()) == 0)
            return pInfo->GetType(); 
    }

    return 0;
}

//------------------------------------------------------------------------------
//  GetScriptIndex
//------------------------------------------------------------------------------
int Audio::Script::GetScriptIndex(std::string& strName)
{
    //  scan for script
	for (int index = 0; index != m_pvScriptNames->size(); index++)
	{
        std::string& strScript = *m_pvScriptNames->at(index);

        if (strName.compare(strScript) == 0)
            return index;
    }

    return -1;
}

//------------------------------------------------------------------------------
//  GetFunctionIndex
//------------------------------------------------------------------------------
int Audio::Script::GetFunctionIndex(std::string& strName)
{
    //  scan for script
	for (int index = 0; index != m_pvFunctionNames->size(); index++)
	{
        std::string& strFunc = *m_pvFunctionNames->at(index);

        if (strName.compare(strFunc) == 0)
            return index;
    }

    return -1;
}

//------------------------------------------------------------------------------
//  GetLabelOffset
//------------------------------------------------------------------------------
int Audio::Script::GetLabelOffset(std::string& strName)
{
    //  scan for label
	for (Audio::Iterator<Label*> i = m_lLabels.first(); !i.isPastEnd(); i.advance())
	{
        Label* pLabel = i.retrieve();

        if (strName.compare(pLabel->GetName()) == 0)
            return pLabel->GetInstruction()->GetOffset();
    }

    return -1;
}

//------------------------------------------------------------------------------
//  GetMessage
//------------------------------------------------------------------------------
Audio::Script::Message* Audio::Script::GetMessage(std::string& strCom, std::string& strName)
{
    //  scan for message
    for (int m = 0; m < (int)m_lMessages.size(); m++)
    {
        ComponentInfo* pComInfo = NULL;

        Message* pMessage = m_lMessages[m];

		for (Audio::Iterator<Script::ComponentInfo*> i = m_plComponentInfos->first(); !i.isPastEnd(); i.advance())
		{
			ComponentInfo* pEntry = i.retrieve();

			if (pEntry->GetName().compare(strCom) == 0)
            {
				pComInfo = pEntry;
                break;
            }
		}

        if (pComInfo)
        {
            //  correct component
            if (pComInfo->GetType().compare(pMessage->GetComType()) == 0)
            {
                //  valid message
                if (strName.compare(pMessage->GetName()) == 0)
                    return pMessage;
            }
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------
//  GetToken
//------------------------------------------------------------------------------
char* Audio::Script::GetToken(char* pchText, std::string* pstrToken)
{
    char* pchCurr = pchText;

	char chQuotes = '\0';

	while ((*pchCurr != '\0') && ((*pchCurr == ' ') || (*pchCurr == '\t') || (*pchCurr == '\n') || (*pchCurr == '\r')))
    {
        if (*pchCurr == '\n')
            m_iCurrLine++;

		pchCurr++;
    }

	if (*pchCurr == '\0')
		return NULL;

	if ((*pchCurr == '\"') || (*pchCurr == '\''))
    {
		chQuotes = *pchCurr;
        pchCurr++;
    }

	//	scan till ws, end marker, or end quote
	while (((*pchCurr != ' ') && (*pchCurr != '\t') && (*pchCurr != '\n') && (*pchCurr != '\r')) || (chQuotes))
	{
        if (chQuotes && (*pchCurr == '\0'))
            throw Error("End of text stream within a token");

		if (*pchCurr == chQuotes)
        {
			chQuotes = '\0';
            pchCurr++;
            break;
        }

		*pstrToken += *pchCurr;

        pchCurr++;
	}

    return pchCurr;
}



