#include "stdafx.h"
#include "Form1.h"

using xray::editor::controls::hypergraph::node;
using xray::editor::controls::hypergraph::node_style;
using xray::editor::controls::hypergraph::connection_type;
using xray::editor::controls::hypergraph::point_align;

using test_form::Form1;

System::Void Form1::button1_Click(System::Object^  sender, System::EventArgs^  e)
{
	node^ _node = gcnew node();
	node_style^ _node_style = gcnew node_style();
	//System::Drawing::StringFormat^ fmt = gcnew System::Drawing::StringFormat();
	//fmt->LineAlignment	= System::Drawing::StringAlignment::Center;
	//fmt->Alignment		= System::Drawing::StringAlignment::Center;
	//_node_style->fmt	= fmt;
	_node->assign_style(_node_style);
	_node->add_connection_point(int::typeid, "int_in");
	_node_style->get_point_style("int_in")->dir = connection_type::input;
	_node_style->get_point_style("int_in")->rect = Rectangle(0,30,10,10);
	_node_style->get_point_style("int_in")->al = point_align::left;
	_node->add_connection_point(float::typeid, "piPec");
	_node_style->get_point_style("piPec")->dir = connection_type::output;
	_node_style->get_point_style("piPec")->rect = Rectangle(109,45,10,10);
	_node_style->get_point_style("piPec")->al = point_align::right;
	hypergraph_area1->append_node(_node);

	_node = gcnew node();
	_node_style = gcnew node_style();
	//System::Drawing::StringFormat^ fmt = gcnew System::Drawing::StringFormat();
	//fmt->LineAlignment	= System::Drawing::StringAlignment::Center;
	//fmt->Alignment		= System::Drawing::StringAlignment::Center;
	//_node_style->fmt	= fmt;
	_node->assign_style(_node_style);
	_node->add_connection_point(int::typeid, "int_in");
	_node_style->get_point_style("int_in")->dir = connection_type::output;
	_node_style->get_point_style("int_in")->rect = Rectangle(109,30,10,10);
	_node_style->get_point_style("int_in")->al = point_align::right;
	_node->add_connection_point(float::typeid, "piPec");
	_node_style->get_point_style("piPec")->dir = connection_type::input;
	_node_style->get_point_style("piPec")->rect = Rectangle(0,45,10,10);
	hypergraph_area1->append_node(_node);
}

System::Void Form1::button2_Click(System::Object^  sender, System::EventArgs^  e)
{
	//propertyGrid1->BrowsableAttributes = 
	//	gcnew AttributeCollection(
	//		gcnew CategoryAttribute("Appearance")
	//	);
}

System::Void  Form1::button3_Click(System::Object^  sender, System::EventArgs^  e)
{
//	propertyGrid1->BrowsableAttributes = nullptr;
}
