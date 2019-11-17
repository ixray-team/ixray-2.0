////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ui_window.h"
#include "pch.h"
#include "ui_window.h"
#include <xray/render/base/world.h>
#include <xray/render/base/ui_renderer.h>

void client_to_screen(const ui_window& w, float2& src_and_dest)
{
	window* parent_window = w.get_parent();
	while(parent_window)
	{
		src_and_dest	+= parent_window->get_position();
		parent_window	= parent_window->get_parent();
	}
}

ui_window::ui_window()
:m_position(0.0f,0.0f),
m_size(0.0f,0.0f),
m_b_visible(false),
m_parent(NULL),
m_b_orphan(true),
m_b_focused(false),
m_b_tab_stop(false)
{}

ui_window::~ui_window()
{
	remove_all_childs			();
}

void ui_window::set_parent(window* w)
{
	m_parent				= w;
	process_event			(ev_parent_changed, 0, 0);
}

void ui_window::set_focused(bool val)
{
	if(m_b_focused!=val)
	{
		m_b_focused			= val;
		process_event		(ev_focus, (val)?1:0, 0);
	}
}

void ui_window::add_child(window* w, bool adopt)
{
	w->set_parent			(this);
	w->set_orphan			(adopt);

	m_childs.push_back		(w);
}

void ui_window::remove_all_childs()
{
	// optimize next
	while(!m_childs.empty())
	{
		remove_child(m_childs.back());
	}
}

void ui_window::remove_child(window* w)
{
	window_list_it it		= std::find(m_childs.begin(), m_childs.end(), w);
	m_childs.erase			(it);

	if(w->get_orphan())
		DELETE				(w);
	else
		w->set_parent		(NULL);
}

void ui_window::draw(xray::render::world& render_world)
{
	window_list_it it		= m_childs.begin();
	window_list_it it_e		= m_childs.end();
	for(;it!=it_e;++it)
	{
		window* w = *it;
		if(w->get_visible())
			w->draw			(render_world);
	}
#if 0
	int prim_type			= 1; // 0-tri-list, 1-line-list
	int point_type			= 1;
	
	float2 pos				= get_position();
	float2 const& size		= get_size();
	client_to_screen		(*this, pos);		//need absolute position

	u32 clr = (m_b_focused)?0xffff0000:0xff00ff00;
	xray::render::ui::command* cmd = render_world.ui().create_command(5, prim_type, point_type);
	{
		cmd->push_point		(pos.x,			pos.y,			0, clr, 0,0);
		cmd->push_point		(pos.x+size.x,	pos.y,			0, clr, 0,0);
		cmd->push_point		(pos.x+size.x,	pos.y+size.y,	0, clr, 0,0);
		cmd->push_point		(pos.x,			pos.y+size.y,	0, clr, 0,0);
		cmd->push_point		(pos.x,			pos.y,			0, clr, 0,0);

	}
//	cmd->set_shader();
//	cmd->set_geom();
	render_world.push_command(cmd);
#endif
}

void ui_window::tick()
{
	window_list_it it		= m_childs.begin();
	window_list_it it_e		= m_childs.end();
	for(;it!=it_e;++it)
	{
		window* w = *it;
		if(w->get_visible())
			w->tick		();
	}
}

void ui_window::set_position(float2 const& pos)				
{
	if(!m_position.similar(pos))
	{
		m_position				= pos; 
		process_event			(ev_position_changed, 0, 0);
	}
}

void ui_window::set_size(const float2& size)			
{
	if(!m_size.similar(size))
	{
		m_size					= size; 
		process_event			(ev_size_changed, 0, 0);
	}
}

bool ui_window::emit_event(enum_window_events ev, window* w, int p1, int p2)
{
	event_manager_it it			= m_event_manager.find(ev);
	if(it!=m_event_manager.end())
	{
		ui_event_handler_it h_it	=  it->second.begin();
		ui_event_handler_it h_it_e	=  it->second.end();
		for(; h_it!=h_it_e; ++h_it)
			if((*h_it)(w, p1, p2))
				return true;
	}
	return false;
}

bool ui_window::process_event(enum_window_events ev, int p1, int p2)
{
	return emit_event				(ev, this, p1, p2);
}

void ui_window::subscribe_event(enum_window_events ev, ui_event_handler handler)
{
	ui_event_handler_list& lst	= m_event_manager[ev];
	lst.push_back				(handler);
}

void ui_window::unsubscribe_event(enum_window_events ev, ui_event_handler handler)
{
	ui_event_handler_list& lst	= m_event_manager[ev];
	ui_event_handler_it h_it	= std::find(lst.begin(), lst.end(), handler);
	if(h_it!=lst.end())
		lst.erase(h_it);
}

bool ui_window::get_orphan() const
{
	return m_b_orphan;
}

void ui_window::set_orphan(bool val)
{
	m_b_orphan = val;
}
