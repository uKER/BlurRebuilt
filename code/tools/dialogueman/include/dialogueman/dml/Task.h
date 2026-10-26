#ifndef DMAN_DML_TASK_H
#define DMAN_DML_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace Dml
{
	class C_Variable;

	/////////////////////////// C_Task \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_Task
	{
	public:
		virtual ~C_Task();
	
	public:
		wxString GetName() const;
		
	public:
		void SetInput(C_Variable* pInput, bool takeOwnership = false);
		C_Variable* GetInput();
		
		void SetOutput(C_Variable* pOutput, bool takeOwnership = false);
		C_Variable* GetOutput();

		void SetGlobals(C_Variable* pGlobals);
		C_Variable* GetGlobals();
		
		virtual bool Execute() = 0;

	public:
		virtual void Add(C_Task* pTask);
		virtual void Delete(const wxString& name);
		virtual int GetChildCount();

	public:
		virtual C_Task& FindChildTask(const wxString& name);
		virtual int FindChildTaskIndex(const wxString& name);
		virtual C_Task& operator[](const wxString& name);
		virtual C_Task& operator[](int index);

	protected:
		C_Task(const wxString& name);
		C_Variable& GetInputVariable(const wxString& inputName);

	private:
		C_Task(const C_Task& src);
		C_Task& operator=(const C_Task& rhs);

	private:
		wxString m_name;
		C_Variable* m_pInput;
		C_Variable* m_pOutput;
		C_Variable* m_pGlobals;
		bool m_takeInputOwnership;
		bool m_takeOutputOwnership;

	};

	/////////////////////////// C_NullTask \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_NullTask : public C_Task
	{
	public:
		C_NullTask(const wxString& name);
		virtual ~C_NullTask();

	public:
		virtual bool Execute();
	};

	/////////////////////////// C_CompositeTask \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_CompositeTask : public C_Task
	{
	public:
		C_CompositeTask(const wxString& name);
		virtual ~C_CompositeTask();
		
	public:
		virtual bool Execute() = 0;

	public:
		virtual void Add(C_Task* pTask);
		virtual void Delete(const wxString& name);
		virtual int GetChildCount();

	public:
		virtual C_Task& FindChildTask(const wxString& name);
		virtual int FindChildTaskIndex(const wxString& name);
		virtual C_Task& operator[](const wxString& name);
		virtual C_Task& operator[](int index);
	
	protected:
		Btl::Vector<C_Task*> m_tasks;
		C_NullTask m_nullTask;
		C_NullTask m_endTask;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_TASK_H
