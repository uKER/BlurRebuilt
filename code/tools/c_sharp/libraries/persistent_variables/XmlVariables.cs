using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Global;

namespace XmlVariables_NS
{
	public class XmlVariables
	{
		readonly XmlDocument m_variablesDocument;

		public XmlVariables(XmlDocument variablesDocument)
		{
			m_variablesDocument = variablesDocument;
		}

		public string Get(string strVarName)
		{
			try
			{
				XmlElement xVars = m_variablesDocument["root/vars"];

				XmlElement xVar = XmlHelper.FindNodeByKeyValue(xVars, "name", strVarName);
				if(xVar != null)
				{
					return XmlHelper.GetKeyValue(xVar, "value");
				}
			}
			catch
			{
			}

			return null;
		}

		public void Set()
		{
			throw new System.NotImplementedException();
		}
	}
}
