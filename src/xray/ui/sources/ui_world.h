#ifndef UI_WORLD_H_INCLUDED
#define UI_WORLD_H_INCLUDED

#include <xray/ui/world.h>
#include "ui_font.h"

namespace xray {

namespace input {
	struct world;
} // namespace input

namespace render {
	struct world;
} // namespace render

namespace ui{
struct engine;
}//namespace ui
} // namespace xray

//class ui_dialog_holder;

class ui_world :public ui::world,
				private boost::noncopyable
{
public:
							ui_world				(engine& engine, input::world& input, render::world& render);
	virtual					~ui_world				();
	virtual	void			tick					();
	virtual	void			clear_resources			();
	virtual ui::window*		create_window			();
	virtual ui::dialog*		create_dialog			();
	virtual ui::text*		create_text				();
	virtual ui::text_edit*	create_text_edit		();
	virtual ui::image*		create_image			();
	virtual ui::scroll_view*	create_scroll_view	();
	virtual void			destroy_window			(ui::window* w);


	virtual void			on_device_reset			();
	void					client_to_screen_scaled (float2 const& src, float2& dst);
	void					align_pixel				(float2 const& src, float2& dst);
	float2 const&			base_screen_size		() const								{return m_base_screen_size;}
	
//	ui_dialog_holder&		get_current_holder		()										{return *m_dialog_holder;}
	font_manager&			get_font_manager		()										{return m_font_manager;}
	input::world&			input					()										{return m_input;}
	timing::timer const&	timer					() const								{return m_timer;}
private:
	engine&					m_engine;
	input::world&			m_input;
	render::world&	m_render;
	float2					m_base_screen_size;
	font_manager			m_font_manager;
//	ui_dialog_holder*		m_dialog_holder;
	timing::timer			m_timer;
}; // ui_class world

#endif // #ifndef UI_WORLD_H_INCLUDED