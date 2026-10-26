#ifndef DMAN_DML_VARIABLE_H
#define DMAN_DML_VARIABLE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_MutantValue;

namespace Dml
{
	class C_SimpleVariable;

	/////////////////////////// C_Variable \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_Variable
	{
	public:
		virtual ~C_Variable();

	public:
		void SetContext(C_Variable* pContext);
		C_Variable* GetContext();
		
	public:
		wxString GetName() const;
		
	public:
		virtual void SetValue(const C_MutantValue& value);
		virtual C_MutantValue GetValue(bool solveRef = true) const;
		operator C_MutantValue() const;
		operator wxString() const;
		operator void*() const;
		
	public:
		virtual C_Variable& FindChildVariable(const wxString& name);
		virtual int FindChildVariableIndex(const wxString& name);
		virtual C_Variable& operator[](const wxString& name);
		virtual C_Variable& operator[](int index);

	public:
		class Iterator 
		{
		public:
			virtual Iterator& operator=(const Iterator& rhs)
			{
				if(this == &rhs)
				{
					return *this; 
				}

				m_pCurrVar = rhs.m_pCurrVar;
				return *this;
			}

			bool operator==(const Iterator& rhs)
			{
				return m_pCurrVar == rhs.m_pCurrVar;
			}

			bool operator!=(const Iterator& rhs)
			{
				return !(*this == rhs);
			}

			virtual Iterator& operator++(int) 
			{
				return *this;
			};

			C_Variable& operator*()  { return *m_pCurrVar; } 
			C_Variable* operator->() { return m_pCurrVar; }
				
		protected:
			Iterator(C_Variable* pCurrVar) :
				m_pCurrVar(pCurrVar)
			{}

			Iterator(const Iterator& src) :
				m_pCurrVar(src.m_pCurrVar)
			{}

			C_Variable* m_pCurrVar;

			friend class C_Variable;
		};

	public:
		virtual void Add(C_Variable* pVariable, bool takeOwnership = true);
		virtual void Delete(const wxString& name);
		virtual int GetChildCount();

	public:
		virtual Iterator& Begin() 
		{ 
			if(!m_pIteratorToThis)
				m_pIteratorToThis = new Iterator(this);	

			return *m_pIteratorToThis; 
		}
		virtual Iterator& End() 
		{ 
			if(!m_pIteratorToThis)
				m_pIteratorToThis = new Iterator(this);	

			return *m_pIteratorToThis; 
		}

	public:
		static C_SimpleVariable NULL_VAR;
		static C_SimpleVariable END_VAR;
		static C_Variable& SolveReference(const wxString& refToSolve, C_Variable* pContext, bool stripMarkers = true);

	protected:
		C_Variable(const wxString& name);
		C_Variable& operator=(const C_Variable& rhs);
		
		
		wxString m_name;
		C_Variable* m_pContext;
		Iterator* m_pIteratorToThis;
	};

	/////////////////////////// C_SimpleVariable \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_SimpleVariable : public C_Variable
	{
	public:
		C_SimpleVariable(const wxString& name, const C_MutantValue& value);
		virtual ~C_SimpleVariable();

	public:
		virtual C_MutantValue GetValue(bool solveRef = true) const;
		virtual void SetValue(const C_MutantValue& value);
	
	private:
		C_MutantValue m_value;

		wxStringList GetVariableReferences(const wxString& lookIn) const;
	};

	/////////////////////////// C_CompositeVariable \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class C_CompositeVariable : public C_Variable
	{
	public:
		C_CompositeVariable(const wxString& name);
		virtual ~C_CompositeVariable();
	
	public:
		virtual C_Variable& FindChildVariable(const wxString& name);
		virtual int FindChildVariableIndex(const wxString& name);
		virtual C_Variable& operator[](int index);
		virtual C_Variable& operator[](const wxString& name);

	public:
		class CompositeIterator : public Iterator
		{
		public:
			CompositeIterator(C_CompositeVariable* pContainerVar, C_Variable* pCurrVar) :
				C_Variable::Iterator(pCurrVar),
				m_pContainerVar(pContainerVar)
			{}

			virtual CompositeIterator& operator=(const CompositeIterator& rhs)
			{
				if(this == &rhs)
				{
					return *this; 
				}

				m_pCurrVar = rhs.m_pCurrVar;
				m_pContainerVar = rhs.m_pContainerVar;
				return *this;
			}

			virtual Iterator& operator++(int) 
			{
				int currPos = m_pContainerVar->FindChildVariableIndex(m_pCurrVar->GetName());
				int nextPos = currPos + 1;
				
				if(nextPos < m_pContainerVar->m_variables.Size())
				{
					C_Variable& nextVar = (*m_pContainerVar)[nextPos]; 
					if(m_pContainerVar->GetContext()) //give our context
						nextVar.SetContext(m_pContainerVar->GetContext());

					m_pCurrVar = &nextVar;
				}
				else
				{
					m_pCurrVar = &C_Variable::END_VAR;
				}
				return *this;
			};

		private:
			C_CompositeVariable* m_pContainerVar;
		};

	public:
		virtual void Add(C_Variable* pVariable, bool takeOwnership = true);
		virtual void Delete(const wxString& name);
		virtual int GetChildCount();

	public:
		virtual CompositeIterator& Begin() 
		{ 
			if(m_pIteratorToBegin)
			{
				delete m_pIteratorToBegin;
			}

			if(m_variables.Size() == 0)
				return End();

			C_Variable*	pBeginVar = m_variables[0];
			if(GetContext()) //give our context 
				pBeginVar->SetContext(GetContext());

			m_pIteratorToBegin = new CompositeIterator(this, pBeginVar); 
			
			return *m_pIteratorToBegin;
		}
		virtual CompositeIterator& End() 
		{ 
			if(!m_pIteratorToEnd)
			{
				m_pIteratorToEnd = new CompositeIterator(this, &C_Variable::END_VAR);
			}
			return *m_pIteratorToEnd; 
		}
	
	private:
		Btl::Vector<C_Variable*> m_variables;
		Btl::Map<C_Variable*, bool> m_varToTakeOwnership;
			CompositeIterator* m_pIteratorToBegin;
		CompositeIterator* m_pIteratorToEnd;
	};
	
} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_VARIABLE_H
