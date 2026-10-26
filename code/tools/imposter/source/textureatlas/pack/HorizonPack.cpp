#include "ImposterPch.h"

#include "TextureAtlas/ImageDescription.h"
#include "TextureAtlas/Pack/HorizonPack.h"

#include <algorithm>

namespace TextureAtlas
{
extern Debug::Logger& g_Log;

template <typename ElemType>
void InsertAtIndex(Btl::Vector<ElemType>& v, ElemType& e, U32 index)
{
	v.PushBack();
	for (S32 i=v.Size()-1; i>(S32)index; --i)
		v[i] = v[i-1];

	v[index] = e;
}


static bool SortDescriptors(const C_InputImageDescription* l, const C_InputImageDescription* r)
{
	if (l->size.y == r->size.y)
		return l->size.x > r->size.x;
	else
		return l->size.y > r->size.y;
}


C_HorizonPack::Skyline::Span::Span()
: end(0)
, value(0)
{
}

C_HorizonPack::Skyline::Span::Span(U32 e, U32 v)
: end(e)
, value(v)
{
}

bool C_HorizonPack::Skyline::Span::operator< (const C_HorizonPack::Skyline::Span& s) const
{
	return end < s.end;
}
	
bool C_HorizonPack::Skyline::Span::operator> (const C_HorizonPack::Skyline::Span& s) const
{
	return end > s.end;
}

bool C_HorizonPack::Skyline::Span::operator== (const C_HorizonPack::Skyline::Span& s) const
{
	return (end==s.end && value==s.value);
}

bool C_HorizonPack::Skyline::Span::operator!= (const C_HorizonPack::Skyline::Span& s) const
{
	return (end!=s.end || value!=s.value);
}

C_HorizonPack::Skyline::Skyline(U32 width)
: horizon(0)
{
	spans.PushBack(Span(width, 0));
}

ImagePosition C_HorizonPack::Skyline::Position(const C_InputImageDescription* desc, U32 border) const
{
	U32 bestX=0, bestY=0x0000ffff;

	U32 currentX=0;
	for (U32 i=0; i<(U32)spans.Size(); ++i)
	{
		if (currentX+desc->size.x+border > spans.Back().end)
			break;

		if (spans[i].end-currentX >= desc->size.x+border && spans[i].value<bestY)
		{
			bestY = spans[i].value;
			bestX = currentX;

			currentX = spans[i].end;
		}
		else
		{
			//look ahead for minimum height for positioning the image
			U32 peak = i;

			for (U32 j=i; j<(U32)spans.Size(); ++j)
			{
				if (spans[j].value > spans[peak].value)
					peak = j;

				if (spans[j].end-currentX >= desc->size.x+border)
				{
					if (spans[peak].value < bestY)
					{
						bestY = spans[peak].value;
						bestX = currentX;
					}
					break;
				}
			}

			//set next iteration starting point
			i = peak;
			currentX = spans[peak].end;
		}
	}

	return ImagePosition((bestX>0) ? bestX+border : bestX, (bestY>0) ? bestY+border : bestY);
}


void C_HorizonPack::Skyline::UpdateSpans(U32 xoffset, U32 width, U32 height)
{
	BIZ_ASSERT(xoffset+width<=spans.Back().end);

	U32 sBegin=0xffffffff;
	for (U32 i=0; i<(U32)spans.Size(); ++i)
	{
		if (spans[i].end > xoffset)
		{
			S32 j=i+1;
			while (j<spans.Size())
			{
				if (spans[j].end < xoffset+width)
					spans.EraseByIndex(j);
				else
					break;
			}

			sBegin = i;
			break;
		}
	}

	BIZ_ASSERT(sBegin!=0xffffffff);

	Span current = spans[sBegin];

	if (sBegin>0 && xoffset>spans[sBegin-1].end)
	{
		spans[sBegin].end = xoffset;
		InsertAtIndex(spans, Span(xoffset+width, height), sBegin+1);
		sBegin++;
	}
	else
	{
		spans[sBegin].end = xoffset+width;
		spans[sBegin].value = height;
	}

	if (xoffset+width<current.end)
		InsertAtIndex(spans, current, sBegin+1);

	horizon = Maths::Max(horizon, height);
}




C_HorizonPack::C_HorizonPack()
{
}

bool C_HorizonPack::Pack(const InputImageDescArray& input, OutputImageDescArray& output, const ImageSizeArray& atlasSizes, U32 border)
{
	if (!input.Size() || !atlasSizes.Size())
		return true;

	Btl::Vector<Skyline> skylines;
	for (S32 i=0; i<atlasSizes.Size(); ++i)
		skylines.PushBack(Skyline(atlasSizes[i].x));

	Btl::Vector<const C_InputImageDescription*> workingList;
	for (S32 i=0; i<input.Size(); ++i)
		workingList.PushBack(&input[i]);

	std::sort(workingList.Begin(), workingList.End(), SortDescriptors);

	//fill each atlas in turn
	for (S32 i=0; i<skylines.Size(); ++i)
	{
		Btl::Vector<const C_InputImageDescription*>::Iterator it = workingList.Begin();
		U32 iterationCount = workingList.Size();
		while (workingList.Size() && iterationCount)
		{
			--iterationCount;
			if (it == workingList.End())
				it = workingList.Begin();

			U32 nImages = workingList.Size();

			ImagePosition p = skylines[i].Position(*it, border);

			if (p.y + (*it)->size.y <= skylines[i].horizon)
			{
				//ok to place this image as it causes no increase in the horizon..
				output.PushBack(C_OutputImageDescription((*it)->fileName, ImageRect(p.x, p.y, (*it)->size.x, (*it)->size.y), i));
				skylines[i].UpdateSpans(p.x, (*it)->size.x, p.y+(*it)->size.y);
				it = workingList.Erase(it);
			}
			else
			{
				if (p.y + (*it)->size.y > atlasSizes[i].y)
				{
					//cannot fit in the current atlas
					++it;
					continue;
				}

				ImagePosition bestPos = p;
				ImagePosition worstPos = p;
				Btl::Vector<const C_InputImageDescription*>::Iterator bestIt = it;
				Btl::Vector<const C_InputImageDescription*>::Iterator worstIt = it;

				if (p.y > 0)
				{
					//placing the image at the current position will result in an increase in the horizon
					//search for a better candidate...
					++it;
					while (it != workingList.End())
					{
						p = skylines[i].Position(*it, border);
						if (p.y + (*it)->size.y < bestPos.y + (*bestIt)->size.y)
						{
							bestPos = p;
							bestIt = it;
						}

						if (p.y + (*it)->size.y > worstPos.y + (*worstIt)->size.y && p.y + (*it)->size.y < atlasSizes[i].y)
						{
							worstPos = p;
							worstIt = it;
						}

						//early exit as there's space available in the first row
						if (p.y == 0)
							break;

						++it;
					}
				}

				BIZ_ASSERT(bestIt != workingList.End());
				BIZ_ASSERT(bestPos.y + (*bestIt)->size.y <= atlasSizes[i].y);
				{
					if (bestPos.y==0 || bestPos.y < skylines[i].horizon)
					{
						output.PushBack(C_OutputImageDescription((*bestIt)->fileName, ImageRect(bestPos.x, bestPos.y, (*bestIt)->size.x, (*bestIt)->size.y), i));
						skylines[i].UpdateSpans(bestPos.x, (*bestIt)->size.x, bestPos.y+(*bestIt)->size.y);
						//do not update the horizon if we're still positioning images in the first row
						if (bestPos.y==0)
							skylines[i].horizon = 0;

						workingList.Erase(bestIt);
						it = workingList.End();
					}
					else
					{
						//new image is entirely above the current horizon.
						//add the image that maximises the horizon growth
						output.PushBack(C_OutputImageDescription((*worstIt)->fileName, ImageRect(worstPos.x, worstPos.y, (*worstIt)->size.x, (*worstIt)->size.y), i));
						skylines[i].UpdateSpans(worstPos.x, (*worstIt)->size.x, worstPos.y+(*worstIt)->size.y);
						workingList.Erase(worstIt);
						it = workingList.End();
					}
				}
			}
		}
	}

	return workingList.Size() == 0;
}

}