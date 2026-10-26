#ifndef FLOATVALIDATOR_H
#define FLOATVALIDATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "wx/valgen.h"

class C_FloatValidator: public wxValidator
{
public:
	explicit C_FloatValidator(float* value):
		m_value(value),
		m_hasRange(false)
	{
	}

	C_FloatValidator(float* value, float minValue, float maxValue):
		m_value(value),
		m_minValue(minValue),
		m_maxValue(maxValue),
		m_hasRange(true)
	{
	}

	C_FloatValidator(const C_FloatValidator& other)
	{
		m_value = other.m_value;
		m_minValue = other.m_minValue;
		m_maxValue = other.m_maxValue;
		m_hasRange = other.m_hasRange;
	}

	virtual wxObject* Clone() const
	{
		return new C_FloatValidator(*this);
	}

	virtual bool TransferFromWindow()
	{
		if (GetWindow()->IsKindOf(CLASSINFO(wxTextCtrl)))
		{
			wxString str = ((wxTextCtrl*)GetWindow())->GetValue();
			double value;
			if (str.ToDouble(&value))
			{
				*m_value = static_cast<float>(value);
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}
	
	virtual bool TransferToWindow()
	{
		if (GetWindow()->IsKindOf(CLASSINFO(wxTextCtrl)))
		{
			((wxTextCtrl*)GetWindow())->SetValue(wxString::Format("%.2g", *m_value));
			return true;
		}
		return false;
	}

	virtual bool Validate(wxWindow* parent)
	{
		if (!GetWindow()->IsEnabled())
		{
			return true;
		}
		
		if (!GetWindow()->IsKindOf(CLASSINFO(wxTextCtrl)))
		{
			return true;
		}

		bool ok = true;
		wxString errorMsg;
		wxString str = ((wxTextCtrl*)GetWindow())->GetValue();

		double value;
		if (str.ToDouble(&value))
		{
			float fValue = static_cast<float>(value);
			if (m_hasRange)
			{
				if (fValue < m_minValue || fValue > m_maxValue)
				{
					ok = false;
					errorMsg = wxString::Format("Number must be in the range %g to %g", m_minValue, m_maxValue);
				}
			}
		}
		else
		{
			ok = false;
			errorMsg = "Not a valid floating point number";
		}

		if (!ok)
		{
			GetWindow()->SetFocus();
			::wxMessageBox(errorMsg, _("Invalid Number"), wxOK | wxICON_EXCLAMATION, parent);
		}
		return ok;
	}

private:
	float*	m_value;
	float	m_minValue;
	float	m_maxValue;
	bool	m_hasRange;
};

#endif
