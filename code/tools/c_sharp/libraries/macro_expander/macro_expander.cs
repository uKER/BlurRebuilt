using System;
using System.Collections.Generic;
using System.Text;

namespace MacroExpander
{
	public class MacroExpander
	{
		private Dictionary<string, string> m_DictExpand;

		public MacroExpander()
		{
			m_DictExpand = new Dictionary<string, string>();
		}

		public bool AddPair(string strMacro, string strExpansion)
		{
			if(m_DictExpand.ContainsKey(strMacro))
			{
				m_DictExpand.Remove(strMacro);
				//return false;
			}

			m_DictExpand.Add(strMacro, strExpansion);

			return true;
		}

		public bool AddDictionary(Dictionary<string, string> dictionary)
		{
			if(dictionary != null)
			{
				foreach(KeyValuePair<string, string> kvp in dictionary)
				{
					this.AddPair(kvp.Key, kvp.Value);
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		public bool Clear()
		{
			m_DictExpand.Clear();
			return true;
		}

		public bool Remove(string strMacro)
		{
			if(m_DictExpand.ContainsKey(strMacro))
			{
				m_DictExpand.Remove(strMacro);
				return true;
			}
			else
				return false;
		}

		public string Expand(string strSource, bool bExpandEnvironmentVariables)
		{
			string strRetval = strSource;

			if(strSource != null)
			{
				if(m_DictExpand != null)
				{
					foreach(KeyValuePair<string, string> kvp in m_DictExpand)
					{
						strRetval = strRetval.Replace(kvp.Key, kvp.Value);
					}
				}

				if(bExpandEnvironmentVariables == true)
				{
					strRetval = Environment.ExpandEnvironmentVariables(strRetval);
				}
			}

			return strRetval;
		}

		public Dictionary<string, string> GetDictionary()
		{
			return m_DictExpand;
		}
	}
}
