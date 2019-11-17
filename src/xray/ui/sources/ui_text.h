#ifndef UI_TEXT_H_INCLUDED
#define UI_TEXT_H_INCLUDED

#include "ui_font.h"
#include "ui_window.h"

class ui_text :	public ui::text,
				public ui_window
{
public:
						ui_text			( ui_world& w );
	virtual				~ui_text		();
	virtual void		set_font		(enum_font fnt);
	virtual void		set_color		(u32 clr);
	virtual void		set_text		(pcstr text);
	virtual pcstr		get_text		() const						{return m_text_.c_str();};
	virtual void		set_align		(enum_text_align al);
	virtual void		set_text_mode	(enum_text_mode tm);
	virtual float2		measure_string	() const;
	virtual void		fit_height_to_text();
	virtual void		split_and_set_text(pcstr str, float const width, pcstr& ret_str);

	virtual void		draw			(xray::render::world& w);
	virtual window*		w				()								{return this;}
protected:
			u32			get_text_length	()	const						{return m_text_length;}
			void		draw_internal	(xray::render::world& w, int const& idx1, int const& idx2, u32 const& sel_clr);
			bool		on_text_changed	(window* w, int p1, int p2);

	shared_string						m_text_;
	ui_font const*						m_font;
	enum_text_align						m_align;
	enum_text_mode						m_mode;
	u32									m_color;
	u16									m_text_length;
	ui_world&							m_ui_world;
};
#endif //UI_TEXT_H_INCLUDED