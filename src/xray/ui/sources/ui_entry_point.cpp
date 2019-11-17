#include "pch.h"
#include "ui_world.h"
#include <xray/ui/api.h>

static xray::uninitialized_reference<ui_world>	s_world;

xray::ui::allocator_type* xray::ui::g_allocator = 0;

xray::ui::world* xray::ui::create_world(engine& engine, input::world& input, render::world& render)
{
	XRAY_CONSTRUCT_REFERENCE	( s_world, ui_world)	(engine, input, render);
	return						(&*s_world);
}

void xray::ui::destroy_world	(xray::ui::world*& world)
{
	R_ASSERT					(&*s_world == world);
	XRAY_DESTROY_REFERENCE		( s_world );
	world						= 0;
}

void xray::ui::memory_allocator	(allocator_type& allocator)
{
	ASSERT						(!g_allocator);
	g_allocator					= &allocator;
}