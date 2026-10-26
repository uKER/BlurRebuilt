#ifndef AFTK_BLOB_H
#define AFTK_BLOB_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{

//Container for  binary large object data
class C_BlobData
{
public:
	///\brief Default Constructor.
	///
	C_BlobData();

	///\brief Copy Constructor.
	///
	C_BlobData(const C_BlobData& blob);

	///\brief Destructor.
	///
	~C_BlobData();

	///\brief Assignment operator.
	///
	///@param rhs Class to copy.
	///@return Reference to this class.
	const C_BlobData& operator=(const C_BlobData &rhs);

	///\brief Equality test operator.
	///
	///@param rhs The class to test against.
	///@return bool Test result.
	bool operator== (const C_BlobData &rhs) const;

	///\brief Inequality test operator.
	///
	///@param rhs The object to test against.
	///@return bool Test result.
	bool operator!= (const C_BlobData &rhs) const;

	///\brief Creates a copy of the BLOb data
	///
	///@return a pointer to the newly created data
	C_BlobData* Clone() const;

	///\brief Set BLOb data
	///
	///@param size The size of the BLOb.
	///@param pData A pointer to the data.
	///@return Aftk status indicating success or failure.
	Aftk::C_Status SetData(U32 size, void* pData);

	///\brief Resizes BLOb data.  Allocates a fresh block of memory & copies existing blob data (if any) into it.
	///
	///@param size The size of the BLOb.
	///@return Aftk status indicating success or failure.
	Aftk::C_Status Resize(U32 size);

	///\brief Get size of BLOb data
	///
	///@return Pointer to data.
	U32 GetSize() const;

	///\brief Get a pointer to BLOb data
	///
	///@return Pointer to data.
	void* GetDataPtr() const;

private:
	U32		m_size;
	void	*m_pData;
};


}

#endif
