#ifndef DMAN_EXECUTE_DML_H
#define DMAN_EXECUTE_DML_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace ExecuteDML
{
	class C_Options
	{
	public:
		C_Options()
		{
			SetRecord(NULL);
			SetDMLFile("");
		}

		C_Record* GetRecord()  const { return m_pRecord; }
		wxString  GetDMLFile() const { return m_dmlFile; }

		void SetRecord(C_Record* pRecord) 
		{ 
			m_pRecord = pRecord; 
		}

		void SetDMLFile(const wxString& dmlFile) 
		{ 
			m_dmlFile = dmlFile; 
		}
						
	private:
		C_Record* m_pRecord;
		wxString  m_dmlFile;
	};

	bool Do(const C_Options& options);
}
}

#endif //DMAN_EXECUTE_DML_H
