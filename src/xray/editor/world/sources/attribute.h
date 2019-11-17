////////////////////////////////////////////////////////////////////////////
//	Created		: 11.06.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef ATTRIBUTE_H_INCLUDED
#define ATTRIBUTE_H_INCLUDED

#include "project_defines.h"
using System::AttributeTargets;

public enum class enum_connection : int {in=0, out=1, inout=2 };
[System::AttributeUsage(AttributeTargets::Property, AllowMultiple=false)]
public ref class ConnectionAttribute : public System::Attribute 
{
public:
	ConnectionAttribute	(enum_connection t):m_type(t){}
	enum_connection		m_type;
};//ConnectionAttribute


enum val{e_min_val, e_max_val, e_def_val};

[System::AttributeUsage(AttributeTargets::Property, AllowMultiple=true)]
public ref class ValueAttribute : public System::Attribute 
{
public:
	ValueAttribute(val v0, float v){
		create			(v0); 
		m_params->Add	(v);
	}
	ValueAttribute(val v0, float v1, float v2){
	   create			(v0); 
	   m_params->Add	(v1);
	   m_params->Add	(v2);
	}
	ValueAttribute(val v0, float v1, float v2, float v3){
	   create			(v0); 
	   m_params->Add	(v1);
	   m_params->Add	(v2);
	   m_params->Add	(v3);
	}
	ValueAttribute(val v0, float v1, float v2, float v3, float v4){
	   create			(v0); 
	   m_params->Add	(v1);
	   m_params->Add	(v2);
	   m_params->Add	(v3);
	   m_params->Add	(v4);
	}
	ValueAttribute(val v0, int v){
		create			(v0); 
		m_params->Add	(v);
	}
	ValueAttribute(val v0, bool v){
		create			(v0); 
		m_params->Add	(v);
	}
	ValueAttribute(val v0, System::String^ v){
		create			(v0); 
		m_params->Add	(v);
	}

	System::Collections::ArrayList^ m_params;
	val				m_type; 
private:
	void create(val v0){m_params=gcnew System::Collections::ArrayList; m_type = v0;}
}; //ValueAttribute

namespace xray {
namespace editor {

public ref struct property_link
{
	System::String^			m_src_property_name;
	System::String^			m_dst_property_name;
	object_base^			m_src_object;
	object_base^			m_dst_object;
	System::Reflection::MethodInfo^				m_src_get_method;
	System::Reflection::MethodInfo^				m_dst_set_method;
	System::Type^			m_type;

	void					refresh();
}; //property_link

public ref class link_storage
{
public:
										link_storage();
	System::Collections::ArrayList^		get_links(	object_base^ src_object, 
													System::String^ src_name, 
													object_base^ dst_object, 
													System::String^ dst_name);

	bool								has_links(	object_base^ src_object, 
													System::String^ src_name, 
													object_base^ dst_object, 
													System::String^ dst_name);

	System::Collections::ArrayList^		get_links(	);
	void								clear_all(	);

	void								create_link(object_base^ src_object, 
													System::String^ src_name, 
													object_base^ dst_object, 
													System::String^ dst_name);
	void								remove_link(object_base^ src_object, 
													System::String^ src_name, 
													object_base^ dst_object, 
													System::String^ dst_name);
	void								notify_on_changed(	object_base^ src_object, 
															System::String^ src_name);
private:
	System::Collections::ArrayList^		m_storage;
}; //link_storaga

} // namespace editor
} // namespace xray

#endif // #ifndef ATTRIBUTE_H_INCLUDED
