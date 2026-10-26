#ifndef CS_GEN_TEXT_OUTPUT_H
#define CS_GEN_TEXT_OUTPUT_H

namespace TexAtlas
{
	class TextOutput
	{
	public:
		static void DoOutput(void (*pMessageCallbackFunc)(const std::string& text), const std::string& text)
		{
			if (pMessageCallbackFunc)
			{
				pMessageCallbackFunc(text);
			}
		}

		static void DoOutput(void (*pMessageCallbackFunc)(const std::string& text), const std::stringstream& text)
		{
			if (pMessageCallbackFunc)
			{
				pMessageCallbackFunc(text.str());
			}
		}

		static void DoHeader(void (*pMessageCallbackFunc)(const std::string& text))
		{
			std::stringstream ss;
			ss << "-==| Crop-Sheet Generator (CSGen) " << CS_GEN_VERSION << " Created |==-\n";
			DoOutput(pMessageCallbackFunc, ss);
		}
	};
}

#endif