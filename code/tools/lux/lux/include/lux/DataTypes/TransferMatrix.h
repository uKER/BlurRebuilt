#ifndef __LUX_DATATYPES_TRANSFERMATRIX_H__
#define __LUX_DATATYPES_TRANSFERMATRIX_H__

// datatype definition for a monochromatix SH light transfer matrix
// This can be used to specialize an iterator, e.g. light transfer nodes or light transfer textures

namespace LUX
{

namespace DataTypes
{

float3 TransferMatrix_GetSphereRand();

#define COEFF (ORDER*ORDER)

// template argument ORDER containing the SH order
// the number of coefficients is ORDER*ORDER
template<int ORDER>
class TransferMatrix : public BaseType
{
public:
	float v[COEFF][COEFF];

	/// default constructor
	TransferMatrix()
	{
	}

	TransferMatrix(float f)
	{
		for (int i=0; i<COEFF*COEFF; i++)
			v[0][i] = 0;
	}

	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		// the input datatype is an SH vector of the same order as this matrix
		// this SH vector represents the direction of incoming light
		SHVector<ORDER> shSample = sample.vColour;

		// evaluate SH for the direction of outgoing light
		SHVector<ORDER> shDir = sample.vDir;

		// TODO: use proper maths to do the integration rather than sampling 
		const int nSamples = 100;
		for (int iSample=0; iSample<nSamples; iSample++)
		{
			// pick a random SH direction
			float3 vDir = TransferMatrix_GetSphereRand();

			// evaluate SH for the sampling direction
			SHVector<ORDER> sampleDir = vDir;

			// evaluate input SH for the sampling direction
			float f = DotProduct(shSample, sampleDir);
			if (f<0)
				continue;

			for (int x=0; x<4; x++)
			{
				for (int y=0; y<4; y++)
				{
					// triple product of the incoming light dir * emit light dir * scale
					v[x][y] += f * shDir.v[x] * sampleDir.v[y] * (1.f/(float)nSamples);
				}
			}	
		}
	}

	/// force all elements to a value (usually 0)
	TransferMatrix operator =(float f)
	{
		for (int i=0; i<COEFF*COEFF; i++)
				v[0][i] = f;
		return *this;
	}

	/// scale an item
	TransferMatrix operator *(float fScale) const
	{
		TransferMatrix tmp;
		for (int i=0; i<COEFF*COEFF; i++)
				tmp.v[0][i] = v[0][i] * fScale;
		return tmp;
	}

	/// add two items
	TransferMatrix operator +(const TransferMatrix &i2) const
	{
		TransferMatrix tmp;
		for (int i=0; i<COEFF*COEFF; i++)
			tmp.v[0][i] = v[0][i] + i2.v[0][i];
		return tmp;
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return (COEFF*COEFF);
	}

	/// apply gamma correction to this item - not applicable
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
	}

	/// get the content of this item
	void GetData(float*&p, ContextDataType &context)
	{
		for (int i=0; i<COEFF*COEFF; i++)
			p[i] = v[0][i];
		p += COEFF*COEFF;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, TransferMatrix &t)
	{
		for (int i=0; i<COEFF*COEFF; i++)
			ar >> t.v[0][i];
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, TransferMatrix &t)
	{
		for (int i=0; i<COEFF*COEFF; i++)
			ar << t.v[0][i];
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#undef COEFF

#endif
