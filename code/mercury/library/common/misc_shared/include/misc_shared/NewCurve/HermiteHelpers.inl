void ConvertLegacyHermiteTangents(Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots)
{
	if (knots.Size() == 0)
		return;
	Btl::Vector<F32> positions;
	positions.Resize(knots.Size());
	F32 physicalLength;

	// work out physical length
	{
		Maths::HF32 length;
		Maths::HVec3 prevValue = knots[0].value;

		for(S32 i = 1; i < knots.Size(); ++i)
		{
			Maths::HVec3 nextValue = knots[i].value;
			length += Maths::Length(prevValue - nextValue);
			prevValue = nextValue;
		}
		physicalLength = length;
	}

	// work out positions
	{
		Maths::HF32 totalLength = physicalLength;
		Maths::HF32 currentLength = 0.0f;
		Maths::HF32 position = 0.0f;

		Maths::HVec3 prevValue = knots[0].value;
		positions[0] = 0.0f;
		
		for(S32 i = 1; i < knots.Size(); ++i)
		{
			Maths::HVec3 nextValue = knots[i].value;
			Maths::HF32 length = Maths::Length(prevValue - nextValue);
			prevValue = nextValue;
			currentLength += length;

			positions[i] = currentLength / totalLength;
		}
	}

	for (S32 i = 0; i < knots.Size()-1; ++i)
	{
		Maths::HF32 period = positions[i+1] - positions[i];
		Curve::C_HermiteSplineKnot<Maths::HVec3>& knot0 = knots[i];
		Curve::C_HermiteSplineKnot<Maths::HVec3>& knot1 = knots[i+1];
		knot0.tangentOut = knot0.tangentOut * period;
		knot1.tangentIn = knot1.tangentIn * period;

		Maths::HVec3 ctrl0 = knot0.value + knot0.tangentOut;
		Maths::HVec3 ctrl1 = knot1.value - knot1.tangentIn;

		knot0.tangentOut = Curve::Helpers::ControlPointToOutTangent(knot0.value, ctrl0, knot0.tangentScale);
		knot1.tangentIn = Curve::Helpers::ControlPointToInTangent(knot1.value, ctrl1, knot1.tangentScale);
	}
	knots[0].tangentIn = knots[0].tangentOut;
	knots[knots.Size()-1].tangentOut = knots[knots.Size()-1].tangentIn;
}