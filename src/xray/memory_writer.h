////////////////////////////////////////////////////////////////////////////
//	Created		: 11.05.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MEMORY_WRITER_H_INCLUDED
#define XRAY_MEMORY_WRITER_H_INCLUDED

namespace xray {
namespace memory {

extern int XRAY_CORE_API  aaa_bbb_ccc;
#pragma warning(push)
#pragma warning(disable:4251)

class XRAY_CORE_API i_writer
{
public:
					i_writer		( memory::base_allocator* allocator );
	virtual			~i_writer	( );

	// kernel
	virtual void	seek	(u32 pos)						= 0;
	virtual u32		tell	()								= 0;

	virtual void	w		(const void* ptr, u32 count)	= 0;

	// generalized writing functions
//	void			w_u64	(u64 d)					{	w(&d,sizeof(u64));	}
	inline void			w_u32	(u32 d);
	inline void			w_u16	(u16 d);
	inline void			w_u8	(u8 d);
//	void			w_s64	(s64 d)					{	w(&d,sizeof(s64));	}
	inline void			w_s32	(s32 d);
	inline void			w_s16	(s16 d);
	inline void			w_s8	(s8 d);
	inline void			w_float	(float d);
	//void			w_string(const char *p)			{	w(p,(u32)xr_strlen(p));w_u8(13);w_u8(10);	}
	inline void			w_stringZ(const char *p);
	//void			w_stringZ(const shared_str& p) 	{	w(*p?*p:"",p.size());w_u8(0);		}
	//void			w_stringZ(shared_str& p)		{	w(*p?*p:"",p.size());w_u8(0);		}
	//void			w_stringZ(const xr_string& p)	{	w(p.c_str()?p.c_str():"",(u32)p.size());w_u8(0);	}
	//void			w_fcolor(const Fcolor &v)		{	w(&v,sizeof(Fcolor));	}
	//void			w_fvector4(const Fvector4 &v)	{	w(&v,sizeof(Fvector4));	}
	void			w_fvector3(const math::float3 &v);
	void			w_fvector2(const xray::math::float2 &v);
	//void			w_ivector4(const Ivector4 &v)	{	w(&v,sizeof(Ivector4));	}
	//void			w_ivector3(const Ivector3 &v)	{	w(&v,sizeof(Ivector3));	}
	//void			w_ivector2(const Ivector2 &v)	{	w(&v,sizeof(Ivector2));	}

	// generalized chunking
	u32				align		();
	void			open_chunk	(u32 type);
	void			close_chunk	();
	u32				chunk_size	();					// returns size of currently opened chunk, 0 otherwise
	void			w_chunk		(u32 type, void* data, u32 size);
	virtual bool	valid		()									{return true;}
	virtual	void	flush		() = 0;

private:
	vectora<u32>				m_chunk_pos;
protected:
	memory::base_allocator*		m_allocator;
};
#pragma warning(pop)

class XRAY_CORE_API writer : public i_writer
{
	u8*				data;
	u32				position;
	u32				mem_size;
	u32				file_size;
public:
	writer			( memory::base_allocator* allocator );
	bool			external_data;
	virtual			~writer();

	// kernel
	virtual void	w			(const void* ptr, u32 count);

	virtual void	seek		(u32 pos)	{position = pos;			}
	virtual u32		tell		() 			{return position;			}

	// specific
	inline u8*		pointer		()			{return data;				}
	inline u32		size		() const 	{return file_size;			}
	inline void		clear		()			{file_size=0; position=0;	}
	void			free_		()			;
	bool			save_to		(pcstr fn);
	virtual	void	flush		()			{ };
};

} // namespace memory
} // namespace xray

#include <xray/memory_writer_inline.h>
#endif // #ifndef XRAY_MEMORY_WRITER_H_INCLUDED