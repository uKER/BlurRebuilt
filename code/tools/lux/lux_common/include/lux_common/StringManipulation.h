#ifndef STRING_MANIPULATION_H
#define STRING_MANIPULATION_H

namespace StringUtility
{
	template <class T, int N>
	void StringToVector(const std::string& input, UtMaths::BaseVector<T, N>& output) 
	{
		std::string delimiters = " ";

		// Skip delimiters at beginning.
		std::string::size_type lastPos = input.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		std::string::size_type pos     = input.find_first_of(delimiters, lastPos);

		U32 elementIndex = 0;
		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			if (elementIndex == output.SIZE)
			{
				break;
			}
			// Found a token, add it to the vector.
			std::string token = input.substr(lastPos, pos - lastPos);
			std::stringstream ss;
			ss << token;
			ss >> output[elementIndex];
			// Skip delimiters.  Note the "not_of"
			lastPos = input.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = input.find_first_of(delimiters, lastPos);
			elementIndex++;
		}
	}
}

#endif
