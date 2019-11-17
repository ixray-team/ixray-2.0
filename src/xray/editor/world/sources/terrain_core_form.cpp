#include "pch.h"
#include "terrain_core_form.h"
#include "terrain_object.h"
#include "tool_terrain.h"
#include "terrain_import_form.h"

using namespace System::Drawing;

namespace xray {
namespace editor {

terrain_node_size terrain_core_form::get_terrain_node_size_impl()
{
	if(!m_terrain_core)
		return terrain_node_size::tzero;
	else
		return m_terrain_core->node_size;
}

void terrain_core_form::set_terrain_node_size_impl(terrain_node_size value)
{
	if(!m_terrain_core)
		return;
	
	if(m_terrain_core->node_size == value)
		return;

	if(m_terrain_core->m_nodes.Count!=0)
	{
		MessageBox::Show(this, "Terrain not empty", "--", MessageBoxButtons::OK, MessageBoxIcon::Warning);
		return;
	}
	m_terrain_core->node_size	= value;
}

void terrain_core_form::in_constructor()
{
	current_scale						= 1;
	m_terrain_core						= nullptr;
	propertyGrid->BrowsableAttributes	= gcnew AttributeCollection( gcnew CategoryAttribute("terrain") );
	propertyGrid->SelectedObject		= nullptr;
	m_first_node						= Point(-5, 5);
	m_selected_node_lt.x				= 0;
	m_selected_node_lt.z				= 0;
	m_selected_node_rb					= m_selected_node_lt;
}

void terrain_core_form::terrain_core_form_Activated(System::Object^, System::EventArgs^)
{
	m_terrain_core			= m_tool->get_terrain_core();
	background_image		= "global_map.bmp";
	background_image_size	= Drawing::Size(2000, 2000);
	propertyGrid->SelectedObject = this;
}

void terrain_core_form::terrain_core_form_Deactivate(System::Object^, System::EventArgs^)
{
	m_terrain_core	= nullptr; // invalidate
}

void terrain_core_form::terrain_core_form_FormClosing(System::Object^, System::Windows::Forms::FormClosingEventArgs^  e)
{
	e->Cancel	= true;
	Hide		();
}

terrain_node_key	capture_start_node;

void get_drawing_rectangle(Drawing::Rectangle% result, Point first, terrain_node_key lt, terrain_node_key rb, int nsz)
{
	result.X		= (lt.x - first.X) * nsz	- 5;
	result.Y		= (first.Y - lt.z) * nsz	- 5;
	result.Width	= (rb.x-lt.x+1) * nsz		+ 10;
	result.Height	= (lt.z-rb.z+1) * nsz		+ 10;
}

void terrain_core_form::global_map_panel_MouseMove(System::Object^, System::Windows::Forms::MouseEventArgs^)
{
	terrain_node_key key	= get_node_key_mouse();
	label1->Text			= System::String::Format("{0} : {1}", key.x, key.z);

	if(m_mouse_rect_capture)
	{
		terrain_node_key prev_lt	= m_selected_node_lt;
		terrain_node_key prev_rb	= m_selected_node_rb;

		m_selected_node_lt.x		= Math::Min(capture_start_node.x, key.x);
		m_selected_node_lt.z		= Math::Max(capture_start_node.z, key.z);

		m_selected_node_rb.x		= Math::Max(capture_start_node.x, key.x);
		m_selected_node_rb.z		= Math::Min(capture_start_node.z, key.z);

		if(!prev_lt.eq(m_selected_node_lt) || !prev_rb.eq(m_selected_node_rb))
		{
			int nsz					= (int)node_size / current_scale;
			Drawing::Rectangle		r;
			get_drawing_rectangle	(r, m_first_node, prev_lt, prev_rb, nsz);

			Drawing::Rectangle		r2;
			get_drawing_rectangle	(r2, m_first_node, m_selected_node_lt, m_selected_node_rb, nsz);

			global_map_panel->Invalidate(r);
			global_map_panel->Invalidate(r2);
		}
	}else
	{
		//m_selected_node_lt		= key;
		//m_selected_node_rb		= key;
	}
}

void terrain_core_form::global_map_panel_MouseLeave(System::Object^, System::EventArgs^)
{

}

void terrain_core_form::global_map_panel_MouseDown(System::Object^, System::Windows::Forms::MouseEventArgs^ e)
{
	terrain_node_key key	= get_node_key_mouse();
	if(e->Button==System::Windows::Forms::MouseButtons::Left)
	{
		m_mouse_rect_capture	= true;
		capture_start_node		= key;
	
		if(!key.eq(m_selected_node_lt) || !key.eq(m_selected_node_rb))
		{
			int nsz					= (int)node_size / current_scale;
			Drawing::Rectangle		r;
			get_drawing_rectangle	(r, m_first_node, m_selected_node_lt, m_selected_node_rb, nsz);

			m_selected_node_lt		= key;
			m_selected_node_rb		= key;

			Drawing::Rectangle		r2;
			get_drawing_rectangle	(r2, m_first_node, m_selected_node_lt, m_selected_node_rb, nsz);

			global_map_panel->Invalidate(r);
			global_map_panel->Invalidate(r2);
		}
	}
}

void terrain_core_form::global_map_panel_MouseUp(System::Object^, System::Windows::Forms::MouseEventArgs^ e)
{
	if(e->Button==System::Windows::Forms::MouseButtons::Left)
		m_mouse_rect_capture	= false;
}

void terrain_core_form::global_map_menu_Opening(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e)
{
	if(!m_terrain_core)
	{
		e->Cancel				= true;
		return;
	}

	System::Windows::Forms::ContextMenuStrip^ menu	= safe_cast<System::Windows::Forms::ContextMenuStrip^>(sender);
	menu->Items->Clear		();

	bool b_add				= false;
	bool b_clear			= false;
	for(int ix = m_selected_node_lt.x; ix<=m_selected_node_rb.x; ++ix)
		for(int iz = m_selected_node_rb.z; iz<=m_selected_node_lt.z; ++iz)
		{
			terrain_node_key key(ix, iz);
			if(m_terrain_core->m_nodes.ContainsKey(key))
				b_clear		= true;

			if(!m_terrain_core->m_nodes.ContainsKey(key))
				b_add		= true;
		}

	System::Windows::Forms::ToolStripItem^ item;

	if(b_clear)
	{
		if(m_selected_node_lt.eq(m_selected_node_rb))
			item				= menu->Items->Add("Clear node");
		else
			item				= menu->Items->Add("Clear nodes");

		item->Click			+= gcnew System::EventHandler(this, &terrain_core_form::clear_node);
	}
	
	if(b_add)
	{
		if(m_selected_node_lt.eq(m_selected_node_rb))
			item				= menu->Items->Add("Add node");
		else
			item				= menu->Items->Add("Add nodes");

		item->Click			+= gcnew System::EventHandler(this, &terrain_core_form::add_node);
	}
}

terrain_node_key terrain_core_form::get_node_key_mouse()
{
	terrain_node_key result;
	int	nsz			= (int)node_size;
	if(nsz==0)		
		return		result;
	
	nsz								/= current_scale;

	Drawing::Size area_sz	= global_map_panel->Size;
	Point pt_local			= global_map_panel->PointToClient(MousePosition);
	
	int xx			= (pt_local.X)/nsz;
	int zz			= (pt_local.Y)/nsz;

	result.x		= xx+m_first_node.X;
	result.z		= m_first_node.Y-zz;
	return			result;
}


void terrain_core_form::global_map_panel_Paint(System::Object^, System::Windows::Forms::PaintEventArgs^ e)
{
	int	nsz		= (int)node_size;
	if(nsz==0)
		return;
	
	nsz								/= current_scale;

	Drawing::Graphics^ g			= e->Graphics;
	Drawing::Pen^ grid_pen			= System::Drawing::Pens::Red;
	Drawing::Brush^ font_brush		= System::Drawing::Brushes::White;
	Drawing::Brush^ node_brush		= System::Drawing::Brushes::MediumBlue;
	Drawing::Pen^ selected_nodes_pen = gcnew System::Drawing::Pen(Color::Black);
	selected_nodes_pen->Width		= 3.0f;
	Drawing::Font^ font				= Font;

	// draw grid
	Drawing::Point	pt_from, pt_to;

	Drawing::Size	area_sz			= global_map_panel->Size;

	// vert liles
	int num				= 0;
	for(int ix=0; ix<area_sz.Width; ix+= nsz, num++)
	{
		pt_from		= Point(ix, 0);
		pt_to		= Point(ix, area_sz.Height);
		g->DrawLine	(grid_pen, pt_from, pt_to);

		g->DrawString(System::String::Format("{0}", num+m_first_node.X),
						font,font_brush,
						pt_from); // impl thru graphics->MeasureString

	}

	// horz liles
	num				= m_first_node.Y;
	for(int iy=0; iy<area_sz.Height; iy+= nsz, num--)
	{
		pt_from		= Point(0, iy);
		pt_to		= Point(area_sz.Width, iy);
		g->DrawLine (grid_pen, pt_from, pt_to);
		
		g->DrawString(System::String::Format("{0}", num),
						font,font_brush,
						pt_from); // impl thru graphics->MeasureString
	}

	// filled nodes
	Drawing::Rectangle	r;
	r.Width			= nsz;
	r.Height		= nsz;
	int max_x		= m_first_node.X + area_sz.Width/nsz;
	int max_y		= m_first_node.Y - area_sz.Height/nsz;

	for(int ix=m_first_node.X; ix<=max_x; ++ix)
		for(int iy=m_first_node.Y; iy>=max_y; --iy)
		{
			terrain_node_key key(ix, iy);
			if(m_terrain_core->m_nodes.ContainsKey(key))
			{
				r.X				= (ix-m_first_node.X)*nsz;
				r.Y				= (m_first_node.Y-iy)*nsz;
				g->FillRectangle(node_brush, r);
			}
		}
	// draw selection region
	Drawing::Rectangle		selected_rect;
	selected_rect.X			= (m_selected_node_lt.x - m_first_node.X) * nsz;
	selected_rect.Y			= (m_first_node.Y - m_selected_node_lt.z) * nsz;
	selected_rect.Width		= (m_selected_node_rb.x-m_selected_node_lt.x+1) * nsz;
	selected_rect.Height	= (m_selected_node_lt.z-m_selected_node_rb.z+1) * nsz;
	g->DrawRectangle		(selected_nodes_pen, selected_rect);
}

void terrain_core_form::add_node(System::Object^, System::EventArgs^)
{
	for(int ix = m_selected_node_lt.x; ix<=m_selected_node_rb.x; ++ix)
		for(int iz = m_selected_node_rb.z; iz<=m_selected_node_lt.z; ++iz)
		{
			terrain_node_key key(ix, iz);
			if(!m_terrain_core->m_nodes.ContainsKey(key))
				m_terrain_core->create_node		(key, 0);
		}
	global_map_panel->Invalidate	();
}

void terrain_core_form::clear_node(System::Object^, System::EventArgs^)
{
	for(int ix = m_selected_node_lt.x; ix<=m_selected_node_rb.x; ++ix)
		for(int iz = m_selected_node_rb.z; iz<=m_selected_node_lt.z; ++iz)
		{
			terrain_node_key key(ix, iz);
			if(m_terrain_core->m_nodes.ContainsKey(key))
				m_terrain_core->clear_node		(key);
		}
	global_map_panel->Invalidate	();
}

void terrain_core_form::button_move_left_Click(System::Object^, System::EventArgs^)
{
	m_first_node.X					-=1;
	global_map_panel->Invalidate	();
}

void terrain_core_form::button_move_right_Click(System::Object^, System::EventArgs^)
{
	m_first_node.X					+=1;
	global_map_panel->Invalidate	();
}

void terrain_core_form::button_move_up_Click(System::Object^, System::EventArgs^)
{
	m_first_node.Y					+=1;
	global_map_panel->Invalidate	();
}

void terrain_core_form::button_move_down_Click(System::Object^, System::EventArgs^)
{
	m_first_node.Y					-=1;
	global_map_panel->Invalidate	();
}

void terrain_core_form::set_current_view_scale_impl(int value)
{
	if(value<1)
		value = 1;

	m_current_view_scale			= value;
	global_map_panel->Invalidate	();
}

void terrain_core_form::import_button_Click(System::Object^, System::EventArgs^)
{
	terrain_import_form		import_form;

	if(import_form.ShowDialog() == ::DialogResult::OK)
	{
		terrain_import_settings^ settings	= import_form.m_settings;
		m_terrain_core->import				(settings, m_selected_node_lt, m_selected_node_rb);
	}
}

} // namespace editor
} // namespace xray
