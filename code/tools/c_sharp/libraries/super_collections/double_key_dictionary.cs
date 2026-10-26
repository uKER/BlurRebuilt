using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace SuperCollections
{
	/// TODO: Make exposed properties 'readonly'.
	/// <summary>
	/// This class provides a dictionary that can have up to 2 keys per value.
	/// values can be keyed by key1 or key2 or both. A value does not have to have both keys present.
	/// </summary>
	/// <typeparam name="Tkey1">The type of the first key.</typeparam>
	/// <typeparam name="Tkey2">The type of the second key.</typeparam>
	/// <typeparam name="Tvalue">The type of the value.</typeparam>
	public class DoubleKeyDictionary<Tkey1, Tkey2, Tvalue>
	{
		/// <summary>
		/// For every value in the dictionary we have a reference to either or both of the keys that refer to it.
		/// => value is used as a key into look up it's keys.
		/// </summary>
		private Dictionary<Tvalue, DictionaryKeys> _valuesToKeysLookup = new Dictionary<Tvalue, DictionaryKeys>();

#if reference
		ICollection<int> a()
		{
			List<int> a = new List<int>();
			Dictionary<int, int> d;
			return new System.Collections.ObjectModel.ReadOnlyCollection<int>(a);

		}
#endif

		/// <summary>
		/// This is simply a dictionary relating key1 to its value.
		/// </summary>
		public Dictionary<Tkey1, Tvalue> key1_dict = new Dictionary<Tkey1, Tvalue>();

		/// <summary>
		/// This is simply a dictionary relating key2 to its value.
		/// </summary>
		public Dictionary<Tkey2, Tvalue> key2_dict = new Dictionary<Tkey2, Tvalue>();

		public DoubleKeyDictionary()
		{
		}

		/// <summary>
		/// Gets the values in the double key dictionary as an ICollection.
		/// </summary>
		public ICollection<Tvalue> Values
		{
			get { return (ICollection<Tvalue>) _valuesToKeysLookup.Keys; }
		}

		/// <summary>
		/// Given a value that may be in the dictionary, returns the keys associated with it.
		/// </summary>
		/// <param name="_value"></param>
		/// <returns></returns>
		public DictionaryKeys GetKeysFromValue(Tvalue _value)
		{
			DictionaryKeys keys = null;

			if(_valuesToKeysLookup.TryGetValue(_value, out keys) != true) // If this value doesn't exist under either key ...
				return null;
			return keys;
		}

		/// <summary>
		/// Add a value into our dictionary keyed by key1.
		/// </summary>
		/// <param name="key1">The key.</param>
		/// <param name="_value">The value.</param>
		public void AddKey1(Tkey1 key1, Tvalue _value)
		{
			// First try and add the key-value pair into key1_dict.
			key1_dict.Add(key1, _value); // Throws an ArgumentException if the key already exists.

			// Next either find or create the DictionaryKeys entry for this value.
			DictionaryKeys keys = null;

			if(_valuesToKeysLookup.TryGetValue(_value, out keys) != true) // If this value doesn't exist under either key ...
			{
				keys = new DictionaryKeys(); // ... Then create the keys entry and add it to the values dictionary.
				_valuesToKeysLookup.Add(_value, keys);
			}
			keys._key1 = key1; // Set the key1 value for the value.
		}

		/// <summary>
		/// Add a value into our dictionary keyed by key2.
		/// See AddKey1 for comments.
		/// </summary>
		/// <param name="key2">The key.</param>
		/// <param name="_value">The value.</param>
		public void AddKey2(Tkey2 key2, Tvalue _value)
		{
			key2_dict.Add(key2, _value);

			DictionaryKeys keys = null;

			if(_valuesToKeysLookup.TryGetValue(_value, out keys) != true)
			{
				keys = new DictionaryKeys();
				_valuesToKeysLookup.Add(_value, keys);
			}
			keys._key2 = key2;
		}

		/// <summary>
		/// Look up a value associated with key1.
		/// </summary>
		/// <param name="key1">key.</param>
		/// <returns>value.</returns>
		public Tvalue FindValueByKey1(Tkey1 key1)
		{
			return key1_dict[key1];
		}

		/// <summary>
		/// Look up a value associated with key2.
		/// </summary>
		/// <param name="key2">key.</param>
		/// <returns>value.</returns>
		public Tvalue FindValueByKey2(Tkey2 key2)
		{
			return key2_dict[key2];
		}


		public bool TryValueFindByKey1(Tkey1 key1, out Tvalue _value)
		{
			return key1_dict.TryGetValue(key1, out _value);
		}

		public bool TryValueFindByKey2(Tkey2 key2, out Tvalue _value)
		{
			return key2_dict.TryGetValue(key2, out _value);
		}

		public bool ContainsKey1(Tkey1 key1)
		{
			return key1_dict.ContainsKey(key1);
		}

		public bool ContainsKey2(Tkey2 key2)
		{
			return key2_dict.ContainsKey(key2);
		}


		/// <summary>
		/// Removes a dictionary entry by key1.
		/// </summary>
		/// <param name="key1">key</param>
		public void RemoveKey1(Tkey1 key1)
		{
			Tvalue _value = key1_dict[key1]; // Get the value from key1.

			DictionaryKeys keys = _valuesToKeysLookup[_value]; // Get key1 and key2 from key1.

//			Debug.Assert(key1 == keys._key1);							// Cannot test for equality of generic types, even if the compiler knows they are the same type.

			_valuesToKeysLookup.Remove(_value); // Remove the key reference from the value entry.
			key1_dict.Remove(key1); // Remove the key.
			// 
//			if(keys._key2 != default(Tkey2))							// Cannot test for equality ...
			try
			{
				key2_dict.Remove(keys._key2); // Throws an exception if the key does not exist.
			}
			catch
			{
			}
		}

		/// <summary>
		/// Removes a dictionary entry by key2.
		/// See RemoveKey1 for comments.
		/// </summary>
		/// <param name="key2">key</param>
		public void RemoveKey2(Tkey2 key2)
		{
			Tvalue _value = key2_dict[key2];

			DictionaryKeys keys = _valuesToKeysLookup[_value];

//			Debug.Assert(key2 == keys._key2);

			_valuesToKeysLookup.Remove(_value);
			key2_dict.Remove(key2);
//			if(keys._key1 != default(Tkey1))
			try
			{
				key1_dict.Remove(keys._key1);
			}
			catch
			{
			}
		}

		#region Nested type: DictionaryKeys

		public class DictionaryKeys
		{
			public Tkey1 _key1;
			public Tkey2 _key2;
		}

		#endregion
	}
}