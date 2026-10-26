#include "AudioPch.h"

//------------------------------------------------------------------------------
//  Make
//------------------------------------------------------------------------------
bool Audio::ScriptBuilder::Make(const char* pchName, const char* pchSource, ComponentData* pCompData)
{
    Release();

    try
    {
        m_iCurrLine = 0;

        Init();
        Lex(pchSource);
        Parse();
        Validate(pCompData);
        Create();
	}
	catch (Error& err)
	{
		if (! errorCallback)
		{
			printf("ScriptBuilder error in '%s': %s" , pchName, err.What());

			if (err.When())
				printf(" at line %d", err.When());

			printf("\n");

			system("pause 0");

			return false;
		}
		else
		{
		    CleanUp();
			return errorCallback(err.When(), err.What(), err.Where(), (char*)pchName);
		}
	}

    CleanUp();

    return true;
}

//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------
void Audio::ScriptBuilder::Release()
{
    CleanUp();

    if (m_pCode)
    {
        delete[] m_pCode;
        m_pCode = NULL;
    }
}

//------------------------------------------------------------------------------
//  CleanUp
//------------------------------------------------------------------------------
void Audio::ScriptBuilder::CleanUp()
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

    for (int i = 0; i < (int)m_lMessages.Size(); i++)
        delete m_lMessages[i];

    m_lMessages.Clear();
}

//------------------------------------------------------------------------------
//  Init
//  create message list
//------------------------------------------------------------------------------
void Audio::ScriptBuilder::Init()
{
    //  init valid messages

	m_lMessages.PushBack(new Message(Component::eEventOutput, "EXEC", EventOutput::ExecuteMsg));

	m_lMessages.PushBack(new Message(Component::eParameterIn, "RESET", ParameterIn::ResetMsg));

	m_lMessages.PushBack(new Message(Component::eMonoOneShotSound, "PLAY", MonoOneShotSound::PlayMsg));
    m_lMessages.PushBack(new Message(Component::eMonoOneShotSound, "STOP", MonoOneShotSound::StopMsg));
	m_lMessages.PushBack(new Message(Component::eMonoOneShotSound, "PRIME", MonoOneShotSound::PrimeMsg));
    m_lMessages.PushBack(new Message(Component::eMonoOneShotSound, "FLUSH", MonoOneShotSound::FlushMsg));

	m_lMessages.PushBack(new Message(Component::eMonoLoopingSound, "PLAY", MonoLoopingSound::PlayMsg));
    m_lMessages.PushBack(new Message(Component::eMonoLoopingSound, "STOP", MonoLoopingSound::StopMsg));
	m_lMessages.PushBack(new Message(Component::eMonoLoopingSound, "PRIME", MonoLoopingSound::PrimeMsg));
    m_lMessages.PushBack(new Message(Component::eMonoLoopingSound, "FLUSH", MonoLoopingSound::FlushMsg));

	m_lMessages.PushBack(new Message(Component::eMonoOneShotSoundTable, "PLAY", MonoOneShotSoundTable::PlayMsg));
    m_lMessages.PushBack(new Message(Component::eMonoOneShotSoundTable, "STOP", MonoOneShotSoundTable::StopMsg));
	m_lMessages.PushBack(new Message(Component::eMonoOneShotSoundTable, "PRIME", MonoOneShotSoundTable::PrimeMsg));
    m_lMessages.PushBack(new Message(Component::eMonoOneShotSoundTable, "FLUSH", MonoOneShotSoundTable::FlushMsg));

	m_lMessages.PushBack(new Message(Component::eStereoOneShotSound, "PLAY", StereoOneShotSound::PlayMsg));
    m_lMessages.PushBack(new Message(Component::eStereoOneShotSound, "STOP", StereoOneShotSound::StopMsg));
	m_lMessages.PushBack(new Message(Component::eStereoOneShotSound, "PRIME", StereoOneShotSound::PrimeMsg));
    m_lMessages.PushBack(new Message(Component::eStereoOneShotSound, "FLUSH", StereoOneShotSound::FlushMsg));

	m_lMessages.PushBack(new Message(Component::eStereoOneShotSoundTable, "PLAY", StereoOneShotSoundTable::PlayMsg));
    m_lMessages.PushBack(new Message(Component::eStereoOneShotSoundTable, "STOP", StereoOneShotSoundTable::StopMsg));
	m_lMessages.PushBack(new Message(Component::eStereoOneShotSoundTable, "PRIME", StereoOneShotSoundTable::PrimeMsg));
    m_lMessages.PushBack(new Message(Component::eStereoOneShotSoundTable, "FLUSH", StereoOneShotSoundTable::FlushMsg));

	m_lMessages.PushBack(new Message(Component::eStereoLoopingSound, "PLAY", StereoLoopingSound::PlayMsg));
    m_lMessages.PushBack(new Message(Component::eStereoLoopingSound, "STOP", StereoLoopingSound::StopMsg));
	m_lMessages.PushBack(new Message(Component::eStereoLoopingSound, "PRIME", StereoLoopingSound::PrimeMsg));
    m_lMessages.PushBack(new Message(Component::eStereoLoopingSound, "FLUSH", StereoLoopingSound::FlushMsg));

    m_lMessages.PushBack(new Message(Component::eRandom, "REGENERATE", Random::RegenerateMsg));

	m_lMessages.PushBack(new Message(Component::eTimer, "START", Timer::StartMsg));
    m_lMessages.PushBack(new Message(Component::eTimer, "STOP", Timer::StopMsg));
    m_lMessages.PushBack(new Message(Component::eTimer, "RESET", Timer::ResetMsg));
	m_lMessages.PushBack(new Message(Component::eTimer, "ZERO", Timer::ZeroMsg));

	m_lMessages.PushBack(new Message(Component::eMetronome, "START", Metronome::StartMsg));
    m_lMessages.PushBack(new Message(Component::eMetronome, "STOP", Metronome::StopMsg));
    m_lMessages.PushBack(new Message(Component::eMetronome, "RESET", Metronome::ResetMsg));

	m_lMessages.PushBack(new Message(Component::eSampleAndHold, "SAMPLE", SampleAndHold::SampleMsg));

	m_lMessages.PushBack(new Message(Component::eDebugPrint, "PRINT", DebugPrint::PrintMsg));

	m_lMessages.PushBack(new Message(Component::eSelector, "SELECT", Selector::SelectMsg));
	m_lMessages.PushBack(new Message(Component::eSelector, "RESET", Selector::ResetMsg));

	m_lMessages.PushBack(new Message(Component::eMonoCompressor, "ENABLE", MonoCompressor::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoCompressor, "BYPASS", MonoCompressor::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoDelay, "ENABLE", MonoDelay::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoDelay, "BYPASS", MonoDelay::BypassMsg));
	m_lMessages.PushBack(new Message(Component::eMonoDelay, "RESET", MonoDelay::ResetMsg));

	m_lMessages.PushBack(new Message(Component::eMonoDistortion, "ENABLE", MonoDistortion::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoDistortion, "BYPASS", MonoDistortion::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoFeedbackDelay, "ENABLE", MonoFeedbackDelay::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoFeedbackDelay, "BYPASS", MonoFeedbackDelay::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoGain, "ENABLE", MonoGain::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoGain, "BYPASS", MonoGain::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoHighPass, "ENABLE", MonoHighPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoHighPass, "BYPASS", MonoHighPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoHighShelf, "ENABLE", MonoHighShelf::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoHighShelf, "BYPASS", MonoHighShelf::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoLipSync, "ENABLE", MonoLipSync::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoLipSync, "BYPASS", MonoLipSync::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoLowPass, "ENABLE", MonoLowPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoLowPass, "BYPASS", MonoLowPass::BypassMsg));


	m_lMessages.PushBack(new Message(Component::eMonoSpectralReverb, "ENABLE", MonoSpectralReverb::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoSpectralReverb, "BYPASS", MonoSpectralReverb::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoSpectralEnvelope, "ENABLE", MonoSpectralEnvelope::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoSpectralEnvelope, "BYPASS", MonoSpectralEnvelope::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoSpectralEnvelope, "ENABLE", StereoSpectralEnvelope::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoSpectralEnvelope, "BYPASS", StereoSpectralEnvelope::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoLowShelf, "ENABLE", MonoLowShelf::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoLowShelf, "BYPASS", MonoLowShelf::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoPeakEQ, "ENABLE", MonoPeakEQ::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoPeakEQ, "BYPASS", MonoPeakEQ::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoPeakLevel, "ENABLE", MonoPeakLevel::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoPeakLevel, "BYPASS", MonoPeakLevel::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoPhaser, "ENABLE", MonoPhaser::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoPhaser, "BYPASS", MonoPhaser::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoResonantLowPass, "ENABLE", MonoResonantLowPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoResonantLowPass, "BYPASS", MonoResonantLowPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMono24dBResonantLowPass, "ENABLE", Mono24dBResonantLowPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMono24dBResonantLowPass, "BYPASS", Mono24dBResonantLowPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurround24dBResonantLowPass, "ENABLE", Surround24dBResonantLowPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurround24dBResonantLowPass, "BYPASS", Surround24dBResonantLowPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoRMSLevel, "ENABLE", MonoRMSLevel::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoRMSLevel, "BYPASS", MonoRMSLevel::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoOscilloscope, "ENABLE", MonoOscilloscope::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoOscilloscope, "BYPASS", MonoOscilloscope::BypassMsg));
	m_lMessages.PushBack(new Message(Component::eMonoOscilloscope, "RESET", MonoOscilloscope::ResetMsg));

	m_lMessages.PushBack(new Message(Component::eStereoRMSLevel, "ENABLE", StereoRMSLevel::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoRMSLevel, "BYPASS", StereoRMSLevel::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoSaturator, "ENABLE", MonoSaturator::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoSaturator, "BYPASS", MonoSaturator::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoBitCrusher, "ENABLE", MonoBitCrusher::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoBitCrusher, "BYPASS", MonoBitCrusher::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundBitCrusher, "ENABLE", SurroundBitCrusher::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundBitCrusher, "BYPASS", SurroundBitCrusher::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoFlanger, "ENABLE", MonoFlanger::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoFlanger, "BYPASS", MonoFlanger::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundFlanger, "ENABLE", SurroundFlanger::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundFlanger, "BYPASS", SurroundFlanger::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eMonoRingModulator, "ENABLE", MonoRingModulator::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eMonoRingModulator, "BYPASS", MonoRingModulator::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundRingModulator, "ENABLE", SurroundRingModulator::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundRingModulator, "BYPASS", SurroundRingModulator::BypassMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundRingModulator, "RESYNCINPHASE", SurroundRingModulator::ReSyncInPhaseMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundRingModulator, "RESYNCOUTPHASE", SurroundRingModulator::ReSyncOutPhaseMsg));

	m_lMessages.PushBack(new Message(Component::eStereoCompressor, "ENABLE", StereoCompressor::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoCompressor, "BYPASS", StereoCompressor::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoDistortion, "ENABLE", StereoDistortion::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoDistortion, "BYPASS", StereoDistortion::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoFeedbackDelay, "ENABLE", StereoFeedbackDelay::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoFeedbackDelay, "BYPASS", StereoFeedbackDelay::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoHighPass, "ENABLE", StereoHighPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoHighPass, "BYPASS", StereoHighPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundHighPass, "ENABLE", SurroundHighPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundHighPass, "BYPASS", SurroundHighPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoLowPass, "ENABLE", MonoLowPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoLowPass, "BYPASS", MonoLowPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoPeakEQ, "ENABLE", StereoPeakEQ::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoPeakEQ, "BYPASS", StereoPeakEQ::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoPitchShift, "ENABLE", StereoPitchShift::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoPitchShift, "BYPASS", StereoPitchShift::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoResonantLowPass, "ENABLE", StereoResonantLowPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoResonantLowPass, "BYPASS", StereoResonantLowPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStereoSaturator, "ENABLE", StereoSaturator::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eStereoSaturator, "BYPASS", StereoSaturator::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundCompressor, "ENABLE", SurroundCompressor::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundCompressor, "BYPASS", SurroundCompressor::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundDelay, "ENABLE", SurroundDelay::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundDelay, "BYPASS", SurroundDelay::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundHighShelf, "ENABLE", SurroundHighShelf::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundHighShelf, "BYPASS", SurroundHighShelf::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundLowPass, "ENABLE", MonoLowPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundLowPass, "BYPASS", MonoLowPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundLowShelf, "ENABLE", SurroundLowShelf::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundLowShelf, "BYPASS", SurroundLowShelf::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundPeakEQ, "ENABLE", SurroundPeakEQ::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundPeakEQ, "BYPASS", SurroundPeakEQ::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundResonantLowPass, "ENABLE", SurroundResonantLowPass::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundResonantLowPass, "BYPASS", SurroundResonantLowPass::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundRMSLevel, "ENABLE", SurroundRMSLevel::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundRMSLevel, "BYPASS", SurroundRMSLevel::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eSurroundSaturator, "ENABLE", SurroundSaturator::EnableMsg));
	m_lMessages.PushBack(new Message(Component::eSurroundSaturator, "BYPASS", SurroundSaturator::BypassMsg));

	m_lMessages.PushBack(new Message(Component::eStateMachine, "INCREMENT", StateMachine::IncrementMsg));
	m_lMessages.PushBack(new Message(Component::eStateMachine, "DECREMENT", StateMachine::DecrementMsg));
	m_lMessages.PushBack(new Message(Component::eStateMachine, "CHANGE", StateMachine::ChangeMsg));
	m_lMessages.PushBack(new Message(Component::eStateMachine, "RESET", StateMachine::ResetMsg));

	m_lMessages.PushBack(new Message(Component::eMessageDelay, "DELAY", MessageDelay::DelayMsg));
	m_lMessages.PushBack(new Message(Component::eMessageDelay, "CANCEL", MessageDelay::CancelMsg));

	m_lMessages.PushBack(new Message(Component::eImpactFilter, "IMPACT", ImpactFilter::ImpactMsg));
	m_lMessages.PushBack(new Message(Component::eImpactFilter, "RESET", ImpactFilter::ResetMsg));
}

//------------------------------------------------------------------------------
//  Lex
//  create token list from char stream
//------------------------------------------------------------------------------
void Audio::ScriptBuilder::Lex(const char* pchText)
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
void Audio::ScriptBuilder::Parse()
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
            if (pToken->GetName().compare(std::string("SEND")) == 0 ||
            pToken->GetName().compare(std::string("DIFF")) == 0)
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
//	GetParam1CompOrFloat
//
//		If a script command has one param that can be either a component or a numeric value,
//		(eg PUSH, ADD, SUB etc), this function will retrieve the value of that param, whether
//		it is a component, float value, or constant
//------------------------------------------------------------------------------
bool Audio::ScriptBuilder::GetParam1CompOrFloat(Instruction* pInstruction, Script::OpCodes eCode, const char* strCommand, ComponentData* pCompData)
{
	bool bValidFloat = true;
	U32 u32NumStops = 0;
	for (U32 i = 0; i < strlen(pInstruction->GetParam1String().data()); i++)
	{
		if (pInstruction->GetParam1String().data()[i] < '0' || pInstruction->GetParam1String().data()[i] > '9')
		{
			if (pInstruction->GetParam1String().data()[i] == '.')
				u32NumStops++;
			else if (! (pInstruction->GetParam1String().data()[i] == '-' && i == 0))
			{
				bValidFloat = false;
				break;
			}
		}
	}

	float f32PushValue = 0.f;

	if (! bValidFloat)
	{
		char strTempString2[256] = {0};
		char* strCompName = (char*)(pInstruction->GetParam1String().data());

		U32 u32GroupsToUnnest = 0;
		NamedGroupLevel* pNamedGroup = pCompData->GetGroupLevel();

		while(strCompName[0] == ':')
		{
			u32GroupsToUnnest++;
			strCompName = &(strCompName[1]);
		}

		while (u32GroupsToUnnest && pNamedGroup)
		{
			u32GroupsToUnnest--;
			pNamedGroup = pNamedGroup->GetParent();
		}

		
		if (pNamedGroup)
			sprintf(strTempString2, "%s.%s", pNamedGroup->GetName(), strCompName);
		else
			sprintf(strTempString2, "%s", strCompName);

		for (U32 i = 0; i < m_apConstant.Size() && (! bValidFloat); i++)
		{
			if (m_apConstant[i]->IsCalled(strTempString2))
			{
				bValidFloat = true;
				f32PushValue = m_apConstant[i]->GetValue();
			}
		}
		if (! bValidFloat)
		{
			char strError[256] = {0};
			sprintf(strError, "Unknown component/ invalid float in %s", strCommand);
		    throw Error(strError, pInstruction->GetLine());
			return false;
		}
	}
	else
		f32PushValue = (float)(atof(pInstruction->GetParam1String().data()));

	pInstruction->SetOpCode(eCode);
	pInstruction->SetParam1(f32PushValue);

	return true;
}

//------------------------------------------------------------------------------
//	GetParam2CompOrFloat
//
//		If a script command has one param that can be either a component or a numeric value,
//		(eg PUSH, ADD, SUB etc), this function will retrieve the value of that param, whether
//		it is a component, float value, or constant
//------------------------------------------------------------------------------
bool Audio::ScriptBuilder::GetParam2CompOrFloat(Instruction* pInstruction, Script::OpCodes eCode, const char* strCommand, ComponentData* pCompData)
{
	bool bValidFloat = true;
	U32 u32NumStops = 0;
	for (U32 i = 0; i < strlen(pInstruction->GetParam2String().data()); i++)
	{
		if (pInstruction->GetParam2String().data()[i] < '0' || pInstruction->GetParam2String().data()[i] > '9')
		{
			if (pInstruction->GetParam2String().data()[i] == '.')
				u32NumStops++;
			else if (! (pInstruction->GetParam2String().data()[i] == '-' && i == 0))
			{
				bValidFloat = false;
				break;
			}
		}
	}

	float f32PushValue = 0.f;

	if (! bValidFloat)
	{
		char strTempString2[256] = {0};
		char* strCompName = (char*)(pInstruction->GetParam2String().data());

		U32 u32GroupsToUnnest = 0;
		NamedGroupLevel* pNamedGroup = pCompData->GetGroupLevel();

		while(strCompName[0] == ':')
		{
			u32GroupsToUnnest++;
			strCompName = &(strCompName[1]);
		}

		while (u32GroupsToUnnest && pNamedGroup)
		{
			u32GroupsToUnnest--;
			pNamedGroup = pNamedGroup->GetParent();
		}

		
		if (pNamedGroup)
			sprintf(strTempString2, "%s.%s", pNamedGroup->GetName(), strCompName);
		else
			sprintf(strTempString2, "%s", strCompName);

		for (U32 i = 0; i < m_apConstant.Size() && (! bValidFloat); i++)
		{
			if (m_apConstant[i]->IsCalled(strTempString2))
			{
				bValidFloat = true;
				f32PushValue = m_apConstant[i]->GetValue();
			}
		}
		if (! bValidFloat)
		{
			char strError[256] = {0};
			sprintf(strError, "Unknown component/ invalid float in %s", strCommand);
		    throw Error(strError, pInstruction->GetLine());
			return false;
		}
	}
	else
		f32PushValue = (float)(atof(pInstruction->GetParam2String().data()));

	pInstruction->SetOpCode(eCode);
	pInstruction->SetParam2(f32PushValue);

	return true;
}

//------------------------------------------------------------------------------
//  Validate
//  check semantics, get enumerated elements
//------------------------------------------------------------------------------
void Audio::ScriptBuilder::Validate(ComponentData* pCompData)
{
	for (Audio::Iterator<Instruction*> i = m_lInstructions.first(); !i.isPastEnd(); i.advance())
	{
		Instruction* pInstruction = i.retrieve();

        //  SEND [com] [mes]
        if (pInstruction->GetOpCodeString().compare(std::string("SEND")) == 0)
        {
			pInstruction->SetOpCode(Script::OpSend);

            //  get component
			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam2String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo != -1)
                pInstruction->SetParam1(u32CompInfo);
            else
                throw Error("Unknown component in SEND", pInstruction->GetLine());

            //  get message
            Message* pMessage = GetMessage(pInstruction->GetParam2String(), pInstruction->GetParam1String(), pCompData);

            if (pMessage)
                pInstruction->SetParam2((U32)(pMessage->GetEnum()));
            else
                throw Error("Unknown message in SEND", pInstruction->GetLine());
        }
        //  DIFF [val][com]
        else if (pInstruction->GetOpCodeString().compare(std::string("DIFF")) == 0)
		{
			pInstruction->SetOpCode(Script::OpCompareParams);

			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam1String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo != -1)
                pInstruction->SetParam1(u32CompInfo);
            else
                throw Error("Unknown component in DIFF", pInstruction->GetLine());

			GetParam2CompOrFloat(pInstruction, Script::OpCompareParams, "DIFF", pCompData);
		}
        //  PUSH [com] / PUSH [val]
        else if (pInstruction->GetOpCodeString().compare(std::string("PUSH")) == 0)
        {
            //  get component
			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam1String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo != -1)
            {
                pInstruction->SetOpCode(Script::OpPush);
                pInstruction->SetParam1(u32CompInfo);
            }
			// else get float value or constant
			else
				GetParam1CompOrFloat(pInstruction, Script::OpPushFloat, "PUSH", pCompData);
        }
        //  PULL [com]
        else if (pInstruction->GetOpCodeString().compare(std::string("PULL")) == 0)
        {
            //  get component
			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam1String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo == -1)
                throw Error("Unknown component in PULL", pInstruction->GetLine());
//			else if (GetComponentType(std::string(pchPrefix) + pInstruction->GetParam1String()) != Component::Types::eVariable)
			else if (m_apComponentData[u32CompInfo >> 16]->GetType() != Component::eVariable)
				throw Error("Trying to PULL a component other than a Variable", pInstruction->GetLine());
			else
            {
                pInstruction->SetOpCode(Script::OpPull);
				pInstruction->SetParam1(u32CompInfo);
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
            U32 iLabel = GetLabelOffset(pInstruction->GetParam1String());

            if (iLabel != -1)
            {
                if (pInstruction->GetOpCodeString().compare(std::string("JEQ")) == 0)
                    pInstruction->SetOpCode(Script::OpJumpEqual);
                else if (pInstruction->GetOpCodeString().compare(std::string("JNEQ")) == 0)
                    pInstruction->SetOpCode(Script::OpJumpNotEqual);
                else if (pInstruction->GetOpCodeString().compare(std::string("JLT")) == 0)
                    pInstruction->SetOpCode(Script::OpJumpLessThan);
                else if (pInstruction->GetOpCodeString().compare(std::string("JNLT")) == 0)
                    pInstruction->SetOpCode(Script::OpJumpNotLessThan);
                else if (pInstruction->GetOpCodeString().compare(std::string("JGT")) == 0)
                    pInstruction->SetOpCode(Script::OpJumpGreaterThan);
                else if (pInstruction->GetOpCodeString().compare(std::string("JNGT")) == 0)
                    pInstruction->SetOpCode(Script::OpJumpNotGreaterThan);
                else // JUMP
                    pInstruction->SetOpCode(Script::OpJump);

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
			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam1String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo != -1)
            {
                pInstruction->SetOpCode(Script::OpExecute);
				pInstruction->SetParam1(u32CompInfo);
            }
            else
            {
                throw Error("Unknown script in EXEC", pInstruction->GetLine());
            }
        }
        ////  CALL [func]
        //else if (pInstruction->GetOpCodeString().compare(std::string("CALL")) == 0)
        //{
        //    //  get function
        //    int iFunction = GetFunctionIndex(pInstruction->GetParam1String());

        //    if (iFunction != -1)
        //    {
        //        pInstruction->SetOpCode(Script::OpCall);
        //        pInstruction->SetParam1(iFunction);
        //    }
        //    else
        //    {
        //        throw Error("Unknown script in CALL", pInstruction->GetLine());
        //    }
        //}
        //  COMPARE
        else if (pInstruction->GetOpCodeString().compare(std::string("COMPARE")) == 0)
            pInstruction->SetOpCode(Script::OpCompare);
        //  DISCARD
        else if (pInstruction->GetOpCodeString().compare(std::string("DISCARD")) == 0)
            pInstruction->SetOpCode(Script::OpDiscard);
        //  DESTRUCT
        else if (pInstruction->GetOpCodeString().compare(std::string("DESTRUCT")) == 0)
            pInstruction->SetOpCode(Script::OpDestruct);
        //  ADD
        else if (pInstruction->GetOpCodeString().compare(std::string("ADD")) == 0)
		{
            //  get component
			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam1String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo != -1)
            {
				pInstruction->SetOpCode(Script::OpAdd);
				pInstruction->SetParam1(u32CompInfo);
            }
			else
				GetParam1CompOrFloat(pInstruction, Script::OpAddFloat, "ADD", pCompData);
		}
        //	SUB
        else if (pInstruction->GetOpCodeString().compare(std::string("SUB")) == 0)
		{
			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam1String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo != -1)
            {
				pInstruction->SetOpCode(Script::OpSubtract);
				pInstruction->SetParam1(u32CompInfo);
            }
            else
				GetParam1CompOrFloat(pInstruction, Script::OpSubtractFloat, "SUB", pCompData);
		}
        //	MUL
        else if (pInstruction->GetOpCodeString().compare(std::string("MUL")) == 0)
		{
			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam1String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo != -1)
            {
				pInstruction->SetOpCode(Script::OpMultiply);
				pInstruction->SetParam1(u32CompInfo);
            }
            else
				GetParam1CompOrFloat(pInstruction, Script::OpMultiplyFloat, "MUL", pCompData);
		}
        //	DIV
        else if (pInstruction->GetOpCodeString().compare(std::string("DIV")) == 0)
		{
			U32 u32CompInfo = GetComponentIndex(pInstruction->GetParam1String(), pCompData, pInstruction->GetLine());

			if (u32CompInfo != -1)
            {
				pInstruction->SetOpCode(Script::OpDivide);
				pInstruction->SetParam1(u32CompInfo);
            }
            else
				GetParam1CompOrFloat(pInstruction, Script::OpDivideFloat, "DIV", pCompData);
		}
        //  EXIT
        else if (pInstruction->GetOpCodeString().compare(std::string("EXIT")) == 0)
		{
            pInstruction->SetOpCode(Script::OpExit);
			m_bContainsAnExit = true;
		}
        //  BREAK
        else if (pInstruction->GetOpCodeString().compare(std::string("BREAK")) == 0)
            pInstruction->SetOpCode(Script::OpBreak);
		else
			throw Error("Unknown opcode in script", pInstruction->GetLine());
	}
}

//------------------------------------------------------------------------------
//  Create
//------------------------------------------------------------------------------
void Audio::ScriptBuilder::Create()
{
    int iOffset = 0;

    if (m_pCode)
        delete[] m_pCode;

    m_pCode = new U32[m_iOffset];

    //  create instruction table
	for (Audio::Iterator<Instruction*> i = m_lInstructions.first(); !i.isPastEnd(); i.advance())
	{
        Instruction* pInstruction = i.retrieve();

        m_pCode[iOffset] = pInstruction->GetOpCode();
        iOffset++;

        if (pInstruction->GetParam1() != 0xffffffff)
        {
            m_pCode[iOffset] = pInstruction->GetParam1();
            iOffset++;
        }

        if (pInstruction->GetParam2() != 0xffffffff)
        {
            m_pCode[iOffset] = pInstruction->GetParam2();
            iOffset++;
        }
    }
}

//------------------------------------------------------------------------------
//	GetComponentIndex
//------------------------------------------------------------------------------
U32 Audio::ScriptBuilder::GetComponentIndex(std::string& strName, ComponentData* pCompData, U32 u32LineNumber)
{
	NamedGroupLevel* pNamedGroup = pCompData->GetGroupLevel();

	U16 u16OutputIndex = 0;

	ComponentStringBreakdown breakdown;

	bool bOk = AnalyzeComponentNameCommon(strName, pNamedGroup, breakdown);

	if (! bOk)
	{
		char strError[256] = {0};
		sprintf(strError, "Error processing output string in script for component %s", breakdown.strNameIncludingGroup);
	    throw Error(strError, u32LineNumber);
		return 0xffffffff;
	}

	if (strlen(breakdown.strOutputString))
		u16OutputIndex = atoi(breakdown.strOutputString);

    //  scan for component
	for (U32 i = 0; i < m_apComponentData.Size(); i++)
	{
		if (m_apComponentData[i]->GetName())
		{
			if (strcmp(m_apComponentData[i]->GetName(), breakdown.strNameIncludingGroup) == 0)
			{
				return (i << 16) | u16OutputIndex;
			}
		}
	}

	return 0xffffffff;
}

//------------------------------------------------------------------------------
//  GetComponentType
//------------------------------------------------------------------------------
U32 Audio::ScriptBuilder::GetComponentType(std::string& strName)
{
    //  scan for component
	for (U32 i = 0; i < m_apComponentData.Size(); i++)
	{
		if (strcmp(m_apComponentData[i]->GetName(), strName.data()) == 0)
			return m_apComponentData[i]->GetType();
	}

    return 0;
}

//------------------------------------------------------------------------------
//  GetLabelOffset
//------------------------------------------------------------------------------
int Audio::ScriptBuilder::GetLabelOffset(std::string& strName)
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
Audio::ScriptBuilder::Message* Audio::ScriptBuilder::GetMessage(std::string& strCom, std::string& strName, ComponentData* pCompData)
{
	U32 u32CompInfo = GetComponentIndex(strCom, pCompData);

	if (u32CompInfo == -1)
		return NULL;

    //  scan for message
    for (int m = 0; m < (int)m_lMessages.Size(); m++)
    {
        Message* pMessage = m_lMessages[m];

        //	correct component
		if (m_apComponentData[u32CompInfo >> 16]->GetType() == pMessage->GetComType())
		{
			//	valid message
			if (strName.compare(pMessage->GetName()) == 0)
				return pMessage;
		}
	}

    return NULL;
}

//------------------------------------------------------------------------------
//  GetToken
//------------------------------------------------------------------------------
char* Audio::ScriptBuilder::GetToken(char* pchText, std::string* pstrToken)
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

//------------------------------------------------------------------------------
//  ContainsAnExit
//------------------------------------------------------------------------------
bool Audio::ScriptBuilder::ContainsAnExit()
{
	return m_bContainsAnExit;
}
