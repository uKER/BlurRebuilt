#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

// -------------
// C_Marketplace
// -------------
class C_Marketplace
{
public:
	C_Marketplace()
		: m_bEnumerating( false )
		, m_bPurchasing( false )
	{
	}

public:
	static C_Marketplace&				Instance()						{ return m_sInstance; }

public:
	bool								Enumerating()					{ return m_bEnumerating; }
	bool								Purchasing()					{ return m_bPurchasing; }


private:
	static C_Marketplace				m_sInstance;

private:
	bool								m_bEnumerating;
	bool								m_bPurchasing;

};

}; //namespace UI

#endif /* MARKETPLACE_H */
