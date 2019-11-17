#ifndef UI_FONT_H_INCLUDED
#define UI_FONT_H_INCLUDED

class ui_font : private boost::noncopyable
{
public:
							ui_font			();
							~ui_font		();
	void					init_font		(pcstr name);
protected:
	f32						m_height;
	float2					m_ts_size;
	u32						m_char_count;
	float3*					m_char_map;
public:
	float3 const&			get_char_tc		(u8 const& ch)	const		{return m_char_map[ch];}
	float3 					get_char_tc_ts	(u8 const& ch)	const;
	float const&			get_height		()	const					{return m_height;}
	float					get_height_ts	()	const;
};

class font_manager
{
	ui_font					m_font;
	void					initialize_fonts();
public:
							font_manager	();
							~font_manager	();
	ui_font const *			get_font		()	const	{return &m_font;}
	void					on_device_reset	();
};

f32 calc_string_length		(ui_font const& f, pcstr str);
f32 calc_string_length_n	(ui_font const& f, pcstr str, u16 const str_len);

#endif //UI_FONT_H_INCLUDED