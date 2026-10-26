// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdUtilities/bdBase64.h>

const bdNChar8          fillchar = '=';

                        // 00000000001111111111222222
                        // 01234567890123456789012345
static const bdNChar8 *cvt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

                        // 22223333333333444444444455
                        // 67890123456789012345678901
                          "abcdefghijklmnopqrstuvwxyz"

                        // 555555556666
                        // 234567890123
                          "0123456789+/";

bdUInt findIndex(bdNChar8 c)
{
	bdUInt i = 0;
	while (cvt[i] != c)
	{
		i++;
	}
	return i;
}

void BD_CALL bdBase64::encode(const bdByte8* data, bdNChar8* dest, bdUInt length)
{
	bdUInt		i;
    bdNChar8    c;
 
	bdUInt destIndex = 0;
    for (i = 0; i < length; ++i)
    {
        c = (data[i] >> 2) & 0x3f;
        dest[destIndex] = cvt[(bdUInt)c];
		destIndex++;
        c = (data[i] << 4) & 0x3f;
        if (++i < length)
            c |= (data[i] >> 4) & 0x0f;

        //ret.append(1, cvt[c]);
		dest[destIndex] = cvt[(bdUInt)c];
		destIndex++;
        if (i < length)
        {
            c = (data[i] << 2) & 0x3f;
            if (++i < length)
                c |= (data[i] >> 6) & 0x03;

            //ret.append(1, cvt[c]);
			dest[destIndex] = cvt[(bdUInt)c];
			destIndex++;
        }
        else
        {
            ++i;
            //ret.append(1, fillchar);
			dest[destIndex] = fillchar;
			destIndex++;
        }

        if (i < length)
        {
            c = data[i] & 0x3f;
            //ret.append(1, cvt[c]);
			dest[destIndex] = cvt[(bdUInt)c];
			destIndex++;
        }
        else
        {
            //ret.append(1, fillchar);
			dest[destIndex] = fillchar;
			destIndex++;
        }
    }
	dest[destIndex] = 0;
}

bdUInt BD_CALL bdBase64::decode(const bdNChar8* data, bdByte8* dest, bdUInt length)
{
	bdUInt		i;
    bdNChar8    c;
    bdNChar8    c1;
  
	bdUInt destIndex = 0;
    for (i = 0; i < length; ++i)
    {
        c = (bdNChar8) findIndex(data[i]);
        ++i;
        c1 = (bdNChar8) findIndex(data[i]);
        c = (c << 2) | ((c1 >> 4) & 0x3);

        //ret.append(1, c);
		dest[destIndex] = c;
		destIndex++;
        if (++i < length)
        {
            c = data[i];
            if (fillchar == c)
                break;

            c = (bdNChar8) findIndex(c);
            c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
			dest[destIndex] = c1;
			destIndex++;
            //ret.append(1, c1);
        }

        if (++i < length)
        {
            c1 = data[i];
            if (fillchar == c1)
                break;

            c1 = (bdNChar8) findIndex(c1);
            c = ((c << 6) & 0xc0) | c1;
            dest[destIndex] = c;
			destIndex++;
        }
    }
	return destIndex;
}

