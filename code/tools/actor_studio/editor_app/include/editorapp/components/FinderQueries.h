#ifndef EDITOR_WX_XT_FINDER_QUERIES_H
#define EDITOR_WX_XT_FINDER_QUERIES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_FinderDialog;

	/////////////////////////// QeAllActors \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class QeAllActors : public Xt::QueryElement
	{
	public:
		virtual void Process(Xt::QueryContext* pContext, const Xt::QueryResult& source);
	};

	/////////////////////////// QeByActorName \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class QeByActorName : public Xt::QueryElement
	{
	public:
		QeByActorName(const XtString& name);

	public:
		virtual void Process(Xt::QueryContext* pContext, const Xt::QueryResult& source);

	private:
		XtString m_name;
	};

	/////////////////////////// QeByActorType \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class QeByActorType : public Xt::QueryElement
	{
	public:
		QeByActorType(LtStruct* pType);

	public:
		virtual void Process(Xt::QueryContext* pContext, const Xt::QueryResult& source);

	private:
		LtStruct* m_pType;
	};

	/////////////////////////// QeByActorFieldExpression \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class QeByActorFieldExpression : public Xt::QeFieldExpression
	{
	public:
		QeByActorFieldExpression(LtStruct* pType, Xt::FieldPath* pPath, Xt::FieldExpressionCmp cmp, const XtString& low, const XtString& high);

	public:
		virtual void Process(Xt::QueryContext* pContext, const Xt::QueryResult& source);
	};

	/////////////////////////// LogResults \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class LogResults : public Xt::QueryElement
	{

	public:
		LogResults();

	public:
		virtual void Process(Xt::QueryContext* pContext, const Xt::QueryResult& source);
	};

	/////////////////////////// SinkActorResults \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class SinkActorResults : public Xt::QueryElement
	{
	public:
		SinkActorResults(C_FinderDialog* pFinderDlg);

	public:
		virtual void Process(Xt::QueryContext* pContext, const Xt::QueryResult& source);	
	
	private:
		C_FinderDialog* m_pFinderDlg;
	};
}

#endif