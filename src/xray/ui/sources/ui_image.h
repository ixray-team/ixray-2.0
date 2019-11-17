#ifndef UI_IMAGE_H_INCLUDED
#define UI_IMAGE_H_INCLUDED

#include "ui_window.h"

class ui_image :	public ui::image,
					public ui_window
{
public:
						ui_image		();
	virtual				~ui_image		();
	virtual void		init_texture	(pcstr texture_name);
	virtual void		draw			(xray::render::world& w);
	virtual void		set_color		(u32 clr)						{m_color = clr;};
	virtual window*		w				()								{return this;}
protected:
	u32					m_color;
	float4				m_tex_coords;// ready (uv) format, lt-rb 
	int					m_point_type;//temporary
};

#endif //UI_IMAGE_H_INCLUDED