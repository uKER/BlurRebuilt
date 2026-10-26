#ifndef DMAN_GENERATE_HASHES_H
#define DMAN_GENERATE_HASHES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_Record;

namespace GenerateHashes
{
	class C_Options
	{
	public:
		C_Options()
		{
			SetRecord(NULL);
		}

		C_Record* GetRecord() const { return m_pRecord; }

		void SetRecord(C_Record* pRecord) 
		{ 
			m_pRecord = pRecord; 
		}
						
	private:
		C_Record* m_pRecord;
	};

	bool Do(const C_Options& options);
}

}

#endif //DMAN_GENERATE_HASHES_H
