#include "stdafx.h"
#include <dialogueman/Diff/Diff.h>
#include <algorithm>

namespace DMan
{	

std::vector< std::pair<int, int> > C_Diff::Compare()
{
	if(m_leftFile != wxT("") && m_rightFile != wxT("")) 
	{
		//load files into vector
	}
	
	HashLines();

	int m = m_leftLinesHashed.size();
	int n = m_rightLinesHashed.size();

	//Fill The LCS table
	C_LCSTable t(m + 1, n + 1);
	int i, j;
	for(i = 0; i < m + 1; i++)
	{
		t.Set(i, 0, 0);
	}

	for(j = 0; j < n + 1; j++)
	{
		t.Set(0, j, 0);
	}

	for(i = 1; i < m + 1; i++)
	{
		for(j = 1; j < n + 1; j++)
		{
			if(m_leftLinesHashed[i - 1] == m_rightLinesHashed[j - 1])
			{
				int val = t.Get(i - 1, j - 1) + 1;
				t.Set(i, j, val);
			}
			else
			{
				int val = std::max(t.Get(i, j -1), t.Get(i - 1, j));
				t.Set(i, j, val);
			}
		}
	}

	//Find the Matching Pairs by tracing back from the LCS Table
	i = m;
	j = n;
	int k = t.Get(m, n) - 1;
	while( i > 0 && j > 0 )
	{
		if( (t.Get(i, j) == t.Get(i - 1, j - 1) + 1) && 
			(m_leftLinesHashed[i - 1] == m_rightLinesHashed[j - 1]) )
		{
			m_matchedLines.push_back(std::make_pair(i - 1, j - 1));
			i--;
			j--;
			k--;
		}
		else if(t.Get(i - 1, j) > t.Get(i, j - 1))
		{
			i--;
		}
		else
		{
			j--;
		}
	}
		
	std::reverse(m_matchedLines.begin(), m_matchedLines.end());
	
	PrepareDiff();
		
	return m_matchedLines;
}

void C_Diff::PrepareDiff()
{
	m_addedLines = m_deletedLines = m_changedLines = 0;

	MatchedLinesIt it;
	int first, second, i;
	int preFirst, preSecond, blockFirst, blockSecond, bigBlock;
	diffLine dL;

	first = preFirst = second = preSecond = 0;

	for(it = m_matchedLines.begin(); it < m_matchedLines.end(); it++)
	{
		first = it->first;
		second = it->second;
		blockFirst = first - preFirst;
		blockSecond = second - preSecond;

		//Added Block
		if ((blockFirst == 1) && (blockSecond > 1))
		{
			for(i = 1; i < blockSecond; i++)
			{
				dL.lineNo = preSecond + i;
				dL.status = diffLine::Status_Added;
				m_leftDiff.push_back(dL);
				m_rightDiff.push_back(dL);
				m_addedLines++;
			}
		}
		//Deleted Block
		else if ((blockSecond == 1) && (blockFirst > 1))
		{
			for(i = 1; i < blockFirst; i++)
			{
				dL.lineNo = preFirst + i;
				dL.status = diffLine::Status_Deleted;
				m_leftDiff.push_back(dL);
				m_rightDiff.push_back(dL);
				m_deletedLines++;
			}
		}
		//Changed Block
		else
		{
			bigBlock = (blockFirst > blockSecond) ? blockFirst : blockSecond;
			if(bigBlock > 1)
			{
				for(i = 1; i < bigBlock; i++)
				{
					if (preFirst + i < first) 
					{
						dL.lineNo = preFirst + i;
						dL.status = diffLine::Status_Changed;
						m_changedLines++;
					}
					else						
					{
						dL.lineNo = preFirst + i;
						dL.status = diffLine::Status_Added;
						m_addedLines++;
					}
					m_leftDiff.push_back(dL);
					if (preSecond + i < second)
					{
						dL.lineNo = preSecond + i;
						dL.status = diffLine::Status_Changed;
					}
					else
					{
						dL.lineNo = preSecond + i;
						dL.status = diffLine::Status_Deleted;
						m_deletedLines++;
					}
					m_rightDiff.push_back(dL);
				}
			}
		}
		dL.lineNo = first;
		dL.status = diffLine::Status_NoChange;
		m_leftDiff.push_back(dL);

		dL.lineNo = second;
		dL.status = diffLine::Status_NoChange;
		m_rightDiff.push_back(dL);

		preFirst = first;
		preSecond = second;
	}
}

void C_Diff::HashLines()
{
	for(size_t i = 0; i < m_leftLines.size(); i++)
	{
		wxString clean = m_leftLines[i];
		clean.Replace(wxT(" "), wxT(""));
		int hash = HashString(clean);
		m_leftLinesHashed.push_back(hash);
	}

	for(size_t i = 0; i < m_rightLines.size(); i++)
	{
		wxString clean = m_rightLines[i];
		clean.Replace(wxT(" "), wxT(""));
		int hash = HashString(clean);
		m_rightLinesHashed.push_back(hash);
	}
}

int C_Diff::HashString(const wxString& str)
{
	int ret = 0;
	
	for (size_t i = 0; i < str.length() ;i++)
	{
		ret = ret + (str[i] * (i+1));
	}
	return ret;
}
		 
C_LCSTable::C_LCSTable(int rows, int cols) : m_rows(rows), m_cols(cols)
{
	m_values = new int* [m_rows];
	for(int i = 0; i < m_rows; i++)
	{
		m_values[i] = new int[cols];
	}
}

C_LCSTable::~C_LCSTable()
{
	for(int i = 0; i < m_rows; i++)
	{
		delete[] m_values[i];
	}
	delete[] m_values;
}

} //namespace DMan