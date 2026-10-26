namespace XercesUtility
{
	class StrX
	{
	public :
		// -----------------------------------------------------------------------
		//  Constructors and Destructor
		// -----------------------------------------------------------------------
		StrX(const XMLCh* const toTranscode)
		{
			// Call the private transcoding method
			fLocalForm = XERCES_CPP_NAMESPACE::XMLString::transcode(toTranscode);
		}

		~StrX()
		{
			XERCES_CPP_NAMESPACE::XMLString::release(&fLocalForm);
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
}
