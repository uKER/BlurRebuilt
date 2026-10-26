#ifndef DMAN_GENERATE_SPLITS_H
#define DMAN_GENERATE_SPLITS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_Record;

namespace GenerateSplits
{
	static const int SPLIT_SIZE = 200;

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

	void FindEnd(const Xt::DomArray::IndexIterator begin, Xt::DomArray::IndexIterator& end);
}

}

#endif //DMAN_GENERATE_SPLITS_H
