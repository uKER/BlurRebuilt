#ifndef __ENDIANSWITCH_INL__
#define __ENDIANSWITCH_INL__

#if !defined(COMPILER_VC)
inline U16 _byteswap_ushort(U16 i)
{
	return (((i >> 8) & 0x00ff) | ((i << 8) & 0xff00));
}
inline int _byteswap_ulong(int i)
{
   return (((u&0x000000FF)<<24) + ((u&0x0000FF00)<<8) + ((u&0x00FF0000)>>8) + ((u&0xFF000000)>>24));
}
inline U64 _byteswap_uint64(U64 i)
{
	return (_bswap(i&0xffffffff)<<32) + (_bswap((i&0xffffffff)>>32));
}
#endif
inline U8 _byteswap_dummy(U8 i)
{
	return i;
}

#endif
