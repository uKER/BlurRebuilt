// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdString/bdString.h>

#include <bdPlatform/bdPlatformString/bdPlatformString.h>



// the reference count is set to 1 to ensure we never try to free this data.
static bdInt g_emptyStringData[4] = {1,0,0,0};
static bdNChar8* g_emptyString = reinterpret_cast<bdNChar8*>(
								reinterpret_cast<bdByte8*>(&g_emptyStringData)
								+ sizeof(bdStringData));

extern bdStringData* getEmptyStringData()
{
	*g_emptyString = '\0';
	return reinterpret_cast<bdStringData*>(g_emptyStringData);
}

bdString::bdString()
{
	initialize();
}

bdString::bdString(const bdString& s)
{
	m_string = s.m_string;
	addReference(getStringData());
}

bdString::bdString(const bdNChar8 *const s)
{	
	const bdUWord length = bdStrlen(s);
	if (length > 0)
	{
		allocateBuffer(length);
		bdMemcpy(m_string, s, length+1);
	}
	else
	{
		initialize();
	}
}

bdString::~bdString()
{
	removeReference(getStringData());
}

bdString& bdString::operator=(const bdString& s)
{
	// check for self assignment
	if (m_string != s.m_string)
	{
		// forget about the old me
		removeReference(getStringData());

		m_string = s.m_string;
		addReference(getStringData());
	}
	return *this;
}

bdString& bdString::operator=(const bdNChar8 *const s)
{
	// need to create a new buffer if this one is shared or too small
	const bdUWord length = bdStrlen(s);
	
	if (getStringData()->m_referenceCount > 1 || !enoughCapacity(length))
	{
		// forget about the old me
		removeReference(getStringData());
		allocateBuffer(length);
	}
	else
	{
		getStringData()->m_length = length;
	}

	bdMemcpy(m_string, s, length+1);
	
	return *this;
}

bdBool bdString::operator==(const bdString& s) const
{
	const bdBool areEqual = bdStrcmp(m_string, s.m_string) == 0;
	return areEqual;
}

bdBool bdString::operator==(const bdNChar8 *const s) const
{
	const bdBool areEqual = bdStrcmp(m_string, s) == 0;
	return areEqual;
}

bdBool bdString::operator!=(const bdString& s) const
{
	const bdBool areNotEqual = bdStrcmp(m_string, s.m_string) != 0;
	return areNotEqual;
}

bdBool bdString::operator!=(const bdNChar8 *const s) const
{
	const bdBool areNotEqual = bdStrcmp(m_string, s) != 0;
	return areNotEqual;
}

bdString bdString::operator+(const bdString& s) const
{
	const bdUWord thisLength = getStringData()->m_length; 
	const bdUWord sLength = s.getStringData()->m_length;

	const bdUWord newLength = thisLength + sLength;

	bdString newString;

	if (newLength > 0)
	{		
		newString.allocateBuffer(newLength);
		bdMemcpy(newString.m_string, m_string, thisLength);
		bdMemcpy(newString.m_string+thisLength, s.m_string, sLength+1);		
	}

	return newString;	
}

bdString bdString::operator+(const bdNChar8 *const s) const
{
	const bdUWord thisLength = getStringData()->m_length; 
	const bdUWord sLength = bdStrlen(s);

	const bdUWord newLength = thisLength + sLength;

	bdString newString;

	if (newLength > 0)
	{		
		newString.allocateBuffer(newLength);
		bdMemcpy(newString.m_string, m_string, thisLength);
		bdMemcpy(newString.m_string+thisLength, s, sLength+1);		
	}

	return newString;
}

bdString& bdString::operator+=(const bdString& s)
{
	const bdUWord length = s.getStringData()->m_length;

	if (length > 0)
	{
		const bdUWord newLength = length + getStringData()->m_length;

		// need to create a new buffer if this one is shared or too small
		if (getStringData()->m_referenceCount > 1 || !enoughCapacity(newLength))
		{
			bdStringData *oldStringData = getStringData();
			const bdUWord oldLength = oldStringData->m_length;

			allocateBuffer(newLength);
			bdMemcpy(m_string, oldStringData->getString(), oldLength);
			bdMemcpy(m_string + oldLength, s.m_string, length+1);

			// forget about the old me
			removeReference(oldStringData);
		}
		else
		{
			const bdUWord currentStringLength = getStringData()->m_length;
			bdMemcpy(m_string + currentStringLength, s.m_string, length+1);
			getStringData()->m_length += length;
		}
	}

	return *this;
}

bdString& bdString::operator+=(const bdNChar8 *const s)
{
	const bdUWord length = bdStrlen(s);

	if (length > 0)
	{
		const bdUWord newLength = length + getStringData()->m_length;

		// need to create a new buffer if this one is shared or too small
		if (getStringData()->m_referenceCount > 1 || !enoughCapacity(newLength))
		{
			bdStringData *oldStringData = getStringData();
			const bdUWord oldLength = oldStringData->m_length;

			allocateBuffer(newLength);
			bdMemcpy(m_string, oldStringData->getString(), oldLength);
			bdMemcpy(m_string + oldLength, s, length+1);

			// forget about the old me
			removeReference(oldStringData);
		}
		else
		{
			const bdUWord currentStringLength = getStringData()->m_length;
			bdMemcpy(m_string + currentStringLength, s, length+1);
			getStringData()->m_length += length;
		}
	}

	return *this;
}

bdString& bdString::operator+=(const bdNChar8 c)
{
	bdNChar8 ch[2] = {BD_NULL, BD_NULL};
	ch[0] = c;
	return operator +=(ch);
}

bdString::operator bdString::bdLPCTSTR() const
{
	return m_string;
}

bdBool bdString::findFirst(const bdNChar8 c, 
				 bdUWord& i) const
{
	for(bdUWord a = 0; a < getLength(); a++)
	{
		if(m_string[a] == c)
		{
			i = a;
			return true;
		}
	}

	return  false;
}

bdString bdString::getSection(const bdUWord begin,
							  bdUWord end) const
{
	const bdUWord length = getLength();

	bdString newString;

	if (end > length)
	{
		end = length;
	}

	if (begin < end)
	{
		const bdUWord numToCopy = end - begin;

		newString.allocateBuffer(numToCopy);
		bdMemcpy(newString.m_string, m_string + begin, numToCopy);
		newString.m_string[numToCopy] = '\0';		
	}

	return newString;
}
bdUWord bdString::getLength() const
{
	return getStringData()->m_length;
}

const bdNChar8* bdString::getBuffer() const
{
	return m_string;
}

bdStringData* bdString::getStringData() const
{
	return reinterpret_cast<bdStringData*>(m_string) - 1;
}

void bdString::initialize()
{
	m_string = getEmptyStringData()->getString();
	addReference(getStringData());
}

void bdString::addReference(bdStringData* stringData) const
{
	stringData->m_referenceCount++;
}

void bdString::removeReference(bdStringData* stringData) const
{
	stringData->m_referenceCount--;
	if (stringData->m_referenceCount == 0)
	{
		freeBuffer(stringData);
	}
}

void bdString::allocateBuffer(const bdUWord length)
{
	// Calculate how many BD_CAPACITY_INCREMENT we need to store size 
	// characters.
	const bdUWord requiredSize = length + 1;

	bdUWord numChunks = requiredSize / BD_CAPACITY_INCREMENT;
	if (requiredSize % BD_CAPACITY_INCREMENT)
	{
		numChunks++;
	}

	const bdUWord bufferSize = numChunks * BD_CAPACITY_INCREMENT; 
	
	bdNChar8 *const buffer = bdAllocate<bdNChar8>(bufferSize + sizeof(bdStringData));
	
	bdStringData *stringData = reinterpret_cast<bdStringData*>(buffer);
	stringData->m_referenceCount = 1;
	stringData->m_capacity = bufferSize;
	stringData->m_length = length;

	m_string = stringData->getString();
}

void bdString::freeBuffer(bdStringData* stringData) const
{
	bdNChar8* data = reinterpret_cast<bdNChar8*>(stringData);
	bdDeallocate<bdNChar8>(data);
}

bdBool bdString::enoughCapacity(const bdUWord length) const
{
	return getStringData()->m_capacity >= length+1;
}


