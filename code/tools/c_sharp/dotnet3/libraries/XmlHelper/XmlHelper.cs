using System;
using System.Xml;
using System.Xml.Serialization;
using System.Collections;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Collections.Generic;

// TODO: Replace XmlNode references with XmlAttribute references where applicable.

//[assembly: ComVisible(false)]
namespace Global
{
	/// <summary>
	/// Summary description for XmlHelper.
	/// </summary>

	public sealed class XmlHelper
	{
		#region Xml helper functions.
		////////////////////////////////////////////////////////////////////////////////
		/// XML helper functions.
		////////////////////////////////////////////////////////////////////////////////

		private XmlHelper() { }

		public static string FullNodePath(XmlNode node, string delim)
		{
			if (delim == null)
				delim = "/";

			if (node.ParentNode != null && !(node.ParentNode is XmlDocument))
				return FullNodePath(node.ParentNode, delim) + delim + node.Name;
			else
				return node.Name;
		}

		public static XmlElement GetNodeFromPath(XmlDocument xDoc, string strPath)
		{
			if (xDoc == null)
				return null;

			string[] split = strPath.Split(new Char[] { '/' });

			XmlNode xElement = xDoc as XmlNode;

			foreach (string s in split)
			{
				xElement = xElement[s];

				if (xElement == null)
					return null;
			}
			return xElement as XmlElement;
		}

		public static string FullNodePathByKey(XmlNode node, XmlNode xRoot, string strKey, string delim)
		{
			if (delim == null)
				delim = ".";

			string strRetval = null;

			while (node != null && !(node is XmlDocument))
			{
				string strKeyValue = XmlHelper.GetKeyValue(node, strKey);
				if (strKeyValue != null)
				{
					if (strRetval == null)
						strRetval = strKeyValue;
					else
						strRetval = strKeyValue + delim + strRetval;
				}

				else
					strRetval = "MISSING KEY VALUE" + strRetval;

				if (node == xRoot)
					break;

				node = node.ParentNode;
			}
			return strRetval;
		}

		/// <summary>
		/// Gets the value associated with a key. Returns null if the key is not found.
		/// </summary>
		/// <param name="node"></param>
		/// <param name="strKey"></param>
		/// <returns>The value, or null if the key is not found.</returns>
		public static string GetKeyValue(XmlNode node, string strKey)
		{
			string strRetval = null;
			if(node != null)
			{
				XmlNode attr = node.Attributes.GetNamedItem(strKey);
				if(attr != null)
				{
					strRetval = attr.Value;
				}
			}
			return strRetval;
		}

		public static bool GetKeyBool(XmlNode node, string strKey, bool bDefaultValue)
		{
			if(node != null)
			{
				XmlNode attr = node.Attributes.GetNamedItem(strKey);
				if(attr != null)
				{
					string strValue = attr.Value.ToLower();
					if(strValue == "true" || strValue == "yes" || strValue == "1" || strValue == "ok")
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			return bDefaultValue;
		}


		public static int GetKeyInt(XmlNode node, string strKey, int defaultValue)
		{
			if(node != null)
			{
				XmlNode attr = node.Attributes.GetNamedItem(strKey);
				if(attr != null)
				{
					string strValue = attr.Value;

					int result;
					if(int.TryParse(strValue, out result) == false)
						return defaultValue;
					else
						return result;
				}
			}
			return defaultValue;
		}

		public static float GetKeyFloat(XmlNode node, string strKey, float defaultValue)
		{
			if(node != null)
			{
				XmlNode attr = node.Attributes.GetNamedItem(strKey);
				if(attr != null)
				{
					string strValue = attr.Value;

					float result;
					if(float.TryParse(strValue, out result) == false)
						return defaultValue;
					else
						return result;
				}
			}
			return defaultValue;
		}
		
		public static bool GetKeyBool(XmlNode node, string strKey)
		{
			return GetKeyBool(node, strKey, false);
		}



		public static XmlElement FindNodeByKeyValue(XmlNodeList xmlList, string strName, string strValue)
		{
			return FindNodeByKeyValue(xmlList, strName, strValue, false);
		}

		public static XmlElement FindNodeByKeyValue(XmlNodeList xmlList, string strName, string strValue, bool bRecurse)
		{
			if (xmlList != null)
			{
				foreach (XmlNode n in xmlList)
				{
					if(n.NodeType == XmlNodeType.Element)
					{
//						if((n as XmlElement).GetAttribute(strName) == strValue)
//							return n;




						XmlNode xItem = n.Attributes.GetNamedItem(strName);
						if(xItem != null)
						{
							if(xItem.Value == strValue)
								return n as XmlElement;
						}

						if(bRecurse == true)
						{
							XmlNodeList xChildren = n.ChildNodes;
							if(xChildren != null)
							{
								XmlNode xNode = FindNodeByKeyValue(xChildren, strName, strValue, true);
								if(xNode != null)
								{
									return xNode as XmlElement;
									//								if(xItem.Value == strValue)
									//									return xNode;
								}
							}
						}
					}
				}
			}
			return null;
		}

		public static XmlElement FindNodeByKeyValue(XmlElement xmlElement, string strName, string strValue)
		{
			XmlNode xItem = xmlElement.Attributes.GetNamedItem(strName);

			if (xItem != null)
			{
				if (xItem.Value == strValue)
					return xmlElement;
			}

			return FindNodeByKeyValue(xmlElement.ChildNodes, strName, strValue, true);
		}

		/// <summary>
		/// Builds a List of all (or one) children of xParent that hold the provided key-value pair.
		/// If strValue is null, it builds a list of all matching keys regardless of the value.
		/// </summary>
		/// <param name="xParent">Xml node whose children are searched.</param>
		/// <param name="strKey">The key to search for.</param>
		/// <param name="strValue">The value the key must have, or 'null' to match all.</param>
		/// <param name="bRecurseChildren">Whether to recurse children.</param>
		/// <param name="bBailOutAfterFirstMatch">Whether we're only interested in the first match or not.</param>
		/// <returns>A list of matching children.</returns>
		public static List<XmlElement> FindElementsByKeyValue(XmlElement xParent, string strKey, string strValue, bool bRecurseChildren, bool bBailOutAfterFirstMatch, bool bCaseSensitiveOnValues)
		{
			Dictionary<string, string> kvp = new Dictionary<string, string>(1);

			kvp.Add(strKey, strValue);

			return FindElementsByKeyValues(xParent, kvp, bRecurseChildren, bBailOutAfterFirstMatch, bCaseSensitiveOnValues);
		}

		/// <summary>
		/// Searches xParent for child elements that have attributes that match each key in keyValuePairs, whose value matches each value in keyValuePairs.
		/// Recursion and case sensitivity on value matching is optional.
		/// If a keyValuePair value is null then a positive match is assumed for any attribute value.
		/// </summary>
		/// <param name="xParent">The XmlElement whose children are to be searched.</param>
		/// <param name="keyValuePairs">Key-value pairs representing attribute name-values to match.</param>
		/// <param name="bRecurseChildren">Whether to recurse children of the child nodes.</param>
		/// <param name="bBailOutAfterFirstMatch">True to only find the first match.</param>
		/// <param name="bCaseSensitiveOnValues">True if a case sensitive comparison is required for 'values'.</param>
		/// <returns>A list of XmlElements fitting the criteria or null.</returns>
		public static List<XmlElement> FindElementsByKeyValues(XmlElement xParent, Dictionary<string, string> keyValuePairs, bool bRecurseChildren, bool bBailOutAfterFirstMatch, bool bCaseSensitiveOnValues)
		{
			List<XmlElement> results = new List<XmlElement>();

			foreach(XmlNode xNode in xParent)
			{
				if(xNode.NodeType == XmlNodeType.Element)
				{
					bool bFoundMatch = true;

					foreach(KeyValuePair<string, string> kvp in keyValuePairs)
					{
						XmlNode xAttr = xNode.Attributes.GetNamedItem(kvp.Key);
						if(xAttr != null)
						{
							if(kvp.Value != null)
							{
								if(bCaseSensitiveOnValues == true)
								{
									if(xAttr.Value.ToLower() != kvp.Value.ToLower())
									{
										bFoundMatch = false;
										break;
									}
								}
								else
								{
									if(xAttr.Value != kvp.Value)
									{
										bFoundMatch = false;
										break;
									}
								}
							}
						}
						else
						{
							bFoundMatch = false;
							break;
						}
					}

					if(bFoundMatch == true)
					{
						results.Add(xNode as XmlElement);
						if(bBailOutAfterFirstMatch == true)
							break;
					}

					if(bRecurseChildren == true)
					{
						List<XmlElement> childResults = FindElementsByKeyValues(xNode as XmlElement, keyValuePairs, true, bBailOutAfterFirstMatch, bCaseSensitiveOnValues);

						foreach(XmlElement xElement in childResults)
						{
							results.Add(xElement);
							if(bBailOutAfterFirstMatch == true)
							break;
						}
					}
				}
			}

			if(bBailOutAfterFirstMatch == true)
			{
				Debug.Assert(results.Count < 2);
			}
			return results;
		}
		
		/// <summary>
		/// Builds a List of all elements of xList that hold the provided key-value pair.
		/// If strValue is null, it builds a list of all matching keys regardless of the value.
		/// </summary>
		/// <param name="xList">A list of Xml elements to search.</param>
		/// <param name="strKey">The key to search for.</param>
		/// <param name="strValue">The value the key must have, or 'null' to match all.</param>
		/// <returns>A list of matching elements.</returns>
		public static List<XmlElement> FindElementsByKeyValue(List<XmlElement>xList, string strKey, string strValue)
		{
			List<XmlElement> results = new List<XmlElement>();

			foreach(XmlElement xElement in xList)
			{
				XmlNode xAttr = xElement.Attributes.GetNamedItem(strKey);
				if(xAttr != null)
				{
					if(strValue == null || xAttr.Value == strValue)
						results.Add(xElement as XmlElement);
				}
			}
			return results;
		}

		public static bool AddAttribute(XmlNode node, string strKey, string strValue)
		{
			if (node != null)
			{
				XmlAttribute attr = node.OwnerDocument.CreateAttribute(strKey);
				attr.Value = strValue;
				node.Attributes.SetNamedItem(attr);
				return true;
			}
			return false;
		}

		public static XmlElement FindChildNode(XmlElement parent, string strName, string strKey, string strValue)
		{
			if (parent != null)
			{
				XmlNodeList childList = parent.SelectNodes(strName);

				if (childList != null)
				{
					foreach (XmlElement childElement in childList)
					{
						if (GetKeyValue(childElement, strKey) == strValue)
							return childElement;
					}
				}
			}
			return null;
		}

		/// <summary>
		/// Given a node list and a key, make a list of strings holding the value of every
		/// node that holds the key.
		/// </summary>
		/// <param name="nodeList">List of XML nodes</param>
		/// <param name="strKey">The key on which to search</param>
		/// <returns></returns>
		public static List<string>GetValueList(XmlNodeList nodeList, string strKey)
		{
			List<string>arrayResult = new List<string>();

			foreach (XmlNode n in nodeList)
			{
				if (n.Attributes.GetNamedItem(strKey) != null)
				{
					arrayResult.Add(n.Attributes.GetNamedItem(strKey).Value);
				}
			}
			return arrayResult;
		}

		public static XmlNode GetNodeFromKeyPath(XmlNode xCurrentNode, string strKey, string strPath)
		{
			strPath = strPath.Replace('\\', '/');

			if(strPath.Length > 0 && strPath[0] == '/')
			{
				xCurrentNode = xCurrentNode.OwnerDocument.DocumentElement;
				strPath.Remove(0, 1);
			}

			string[] split = strPath.Split(new Char[] { '/' });


			foreach (string s in split)
			{
				bool bFound = false;

				if(s == "..")
				{
					if(xCurrentNode.ParentNode != null)
						xCurrentNode = xCurrentNode.ParentNode;
					else
						return null;
				}
				else
				{
					foreach(XmlNode xChild in xCurrentNode)
					{
						string strValue = GetKeyValue(xChild, strKey);
						if(strValue == s)
						{
							xCurrentNode = xChild;
							bFound = true;
							break;
						}
					}
					if(bFound == false)
						return null;
				}
			}
			return xCurrentNode;
		}

		public static XmlNode RenameNode(XmlNode node, string namespaceURI, string qualifiedName)
		{
			if(node.NodeType == XmlNodeType.Element)
			{
				XmlElement oldElement = (XmlElement)node;
				XmlElement newElement = node.OwnerDocument.CreateElement(qualifiedName, namespaceURI);
				while (oldElement.HasAttributes)
				{
					newElement.SetAttributeNode(oldElement.RemoveAttributeNode(oldElement.Attributes[0]));
				}
				while (oldElement.HasChildNodes)
				{
					newElement.AppendChild(oldElement.FirstChild);
				}
				if (oldElement.ParentNode != null)
				{
					oldElement.ParentNode.ReplaceChild(newElement, oldElement);
				}
				return newElement;
			}
			else if(node.NodeType == XmlNodeType.Attribute)
			{
				XmlAttribute xOldAttr = (XmlAttribute)node;
				XmlAttribute xNewAttr = xOldAttr.OwnerDocument.CreateAttribute(qualifiedName);
				xNewAttr.Value = xOldAttr.Value;
				XmlElement xOwnerElement = xOldAttr.OwnerElement;
				
				xOwnerElement.Attributes.InsertAfter(xNewAttr, xOldAttr);
				xOwnerElement.Attributes.Remove(xOldAttr);

				return xNewAttr;
			}
			else
			{
				return null;
			}
		}

		public static bool RemoveAttribute(XmlElement xElement, string strKey)
		{
			XmlAttribute xItem = xElement.Attributes.GetNamedItem(strKey) as XmlAttribute;

			if(xItem != null)
			{
				xElement.Attributes.Remove(xItem);
				return true;
			}
			return false;
		}
		#endregion Xml helper functions.
	}
}
