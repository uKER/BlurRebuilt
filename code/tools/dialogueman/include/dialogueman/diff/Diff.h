#ifndef DMAN_DIFF_H
#define DMAN_DIFF_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

typedef struct 
{
	enum Status
	{
		Status_Added,
		Status_Deleted,
		Status_Changed,
		Status_NoChange,
	};
	int lineNo;
	wxString line;
	Status status;
} diffLine;

class C_Diff
{
public:
	C_Diff(const wxString& leftFile, const wxString& rightFile) : 
	  m_leftFile(leftFile), m_rightFile(rightFile) {}
	
	C_Diff(const std::vector< wxString >& leftLines, const std::vector< wxString >& rightLines) :
	  m_leftLines(leftLines), m_rightLines(rightLines), m_leftFile(wxT("")), m_rightFile(wxT(""))  {}
	
	std::vector< std::pair<int, int> > Compare();
	std::vector< std::pair<int, int> > GetMatchedLines()
	{
		return m_matchedLines;
	}

	std::vector< diffLine > GetLeftDiff()
	{
		return m_leftDiff;
	}
	std::vector< diffLine > GetRightDiff()
	{
		return m_rightDiff;
	}

	typedef std::vector< std::pair<int, int> >::iterator MatchedLinesIt;

private:
	const wxString m_leftFile;
	const wxString m_rightFile;

	std::vector< wxString >	   m_leftLines;
	std::vector< int >		   m_leftLinesHashed;
	std::vector< diffLine >	   m_leftDiff;
	std::vector< diffLine >	   m_rightDiff;

	int m_addedLines;
	int m_deletedLines;
	int m_changedLines;

	std::vector< wxString > m_rightLines;
	std::vector< int >		m_rightLinesHashed;
	
	std::vector< std::pair<int, int> > m_matchedLines;

	void PrepareDiff();
	void HashLines();
	int HashString(const wxString& str);
};

// Longest common subsequence table. 
// @see http://www.ics.uci.edu/~eppstein/161/960229.html
class C_LCSTable
{
public:
	C_LCSTable(int rows, int cols); 
	
	void Set(int row, int col, int value)
	{
		if( (0 <= row && row < m_rows) &&
			(0 <= col && col < m_cols))
		{
			m_values[row][col] = value;
		}
	}

	int Get(int row, int col)
	{
		if( (0 <= row && row < m_rows) &&
			(0 <= col && col < m_cols))
		{
			return m_values[row][col];
		}
		return -1;
	}

	virtual ~C_LCSTable();

private:
	C_LCSTable(const C_LCSTable& src); //disallow
	C_LCSTable& operator=(const C_LCSTable& rhs); //disallow
	
	int m_rows;
	int m_cols;
	int** m_values;
};

} //namespace DMan

#endif //DMAN_DIFF_H