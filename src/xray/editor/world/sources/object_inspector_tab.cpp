////////////////////////////////////////////////////////////////////////////
//	Created		: 20.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_inspector_tab.h"
#include "property_holder.h"
#include "object_base.h"
#include "tool_base.h"

using xray::editor::object_base;
using xray::editor::object_inspector_tab;
using xray::editor::controls::property_grid;
using xray::editor::object_list;

void object_inspector_tab::in_constructor()
{
	Width						= 100;
	Height						= 100;
	m_property_grid_host		= gcnew property_grid_host( );
	m_property_grid_host->Width	= 100;
	m_property_grid_host->Height= 100;
	m_property_grid_host->Dock	= System::Windows::Forms::DockStyle::Fill;

	SuspendLayout			( );
	Controls->Add			( m_property_grid_host );
	ResumeLayout			( false );
}

void object_inspector_tab::show_properties(object_list^ objects)
{
	if(objects)
	{
		cli::array<System::Object^>^ props	= gcnew cli::array<System::Object^> (objects->Count);

		int idx = 0;
		for each(object_base^ o in objects)
		{
			lib_item^ li					= o->owner()->get_library_item(o->get_lib_name(), false);
			if(li)
			{
				::property_holder* ph		= dynamic_cast<::property_holder*>(o->get_property_holder());
				li->m_property_restrictor->apply_to(ph);
			}

			xray::editor::property_holder* ph	= o->get_property_holder();
			::property_holder*		p			= dynamic_cast< ::property_holder* >( ph );
			props[idx++]						= p->container();
		}

		m_property_grid_host->selected_objects		= props;
	}else
		m_property_grid_host->selected_object		= nullptr;
}
