////////////////////////////////////////////////////////////////////////////
//	Created		: 11.05.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MEMORY_WRITER_INLINE_H_INCLUDED
#define XRAY_MEMORY_WRITER_INLINE_H_INCLUDED

namespace xray {
namespace memory {

inline void i_writer::w_u32(u32 d)
{	w(&d,sizeof(u32));	}

inline void i_writer::w_u16(u16 d)
{	w(&d,sizeof(u16));	}

inline void i_writer::w_u8(u8 d)
{	w(&d,sizeof(u8));	}

inline void i_writer::w_s32(s32 d)
{	w(&d,sizeof(s32));	}

inline void i_writer::w_s16(s16 d)
{	w(&d,sizeof(s16));	}

inline void i_writer::w_s8(s8 d)
{	w(&d,sizeof(s8));	}

inline void i_writer::w_float(float d)
{	w(&d,sizeof(float));}

inline void i_writer::w_stringZ(const char *p)
{	w(p,(u32)strlen(p)+1);					}

inline void i_writer::w_fvector3(const math::float3 &v)	
{	w(&v, sizeof(math::float3));	}

inline void i_writer::w_fvector2(const xray::math::float2 &v)	
{	w(&v,sizeof(xray::math::float2));	}


} // namespace memory
} // namespace xray

#endif // #ifndef XRAY_MEMORY_WRITER_INLINE_H_INCLUDED