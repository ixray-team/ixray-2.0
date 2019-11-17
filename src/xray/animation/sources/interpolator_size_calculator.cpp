////////////////////////////////////////////////////////////////////////////
//	Created		: 29.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "interpolator_size_calculator.h"
#include "instant_interpolator.h"
#include "linear_interpolator.h"
#include "fermi_dirac_interpolator.h"

using xray::animation::interpolator_size_calculator;
using xray::animation::instant_interpolator;
using xray::animation::linear_interpolator;
using xray::animation::fermi_dirac_interpolator;

bool interpolator_size_calculator::visit	( instant_interpolator& interpolator )
{
	XRAY_UNREFERENCED_PARAMETER	( interpolator );
	m_size						+= sizeof( instant_interpolator );
	return						true;
}

bool interpolator_size_calculator::visit	( instant_interpolator const& interpolator ) const
{
	XRAY_UNREFERENCED_PARAMETER	( interpolator );
	m_size						+= sizeof( instant_interpolator );
	return						true;
}

bool interpolator_size_calculator::visit	( linear_interpolator const& interpolator ) const
{
	XRAY_UNREFERENCED_PARAMETER	( interpolator );
	m_size						+= sizeof( linear_interpolator );
	return						true;
}

bool interpolator_size_calculator::visit	( fermi_dirac_interpolator const& interpolator ) const
{
	XRAY_UNREFERENCED_PARAMETER	( interpolator );
	m_size						+= sizeof( fermi_dirac_interpolator );
	return						true;
}
