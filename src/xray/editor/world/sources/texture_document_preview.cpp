////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "texture_document.h"
#include <xray/editor/base/managed_delegate.h>
#include <amd/compress/texture_compressor/api.h>
#include "texture_tga_to_argb_cook.h"
#include "editor_resource_manager.h"
#include "resource_editor_base.h"

using namespace System::Drawing::Imaging;
using System::Drawing::Imaging::BitmapData;

using xray::texture_compressor::TgaHeader;

namespace xray {
namespace editor {

namespace texture_document_detail
{
	union bgra {
		#pragma warning(push)
		#pragma warning(disable:4201)
		struct {
			u8 b, g, r, a;
		};
		#pragma warning(pop)
		u32 u;
	};
}

void			texture_document::create_chessboard					()
{

	const Int32	side_size = 2048;
	//make Chessboard
	s_chessboard					= gcnew Bitmap(side_size, side_size);
	// Lock the bitmap's bits.  
	System::Drawing::Rectangle rect = System::Drawing::Rectangle(0,0,side_size,side_size);
	System::Drawing::Imaging::BitmapData^ bmpData = s_chessboard->LockBits( rect, System::Drawing::Imaging::ImageLockMode::WriteOnly, s_chessboard->PixelFormat );

	// Get the address of the first line.
	IntPtr ptr = bmpData->Scan0;

	// Declare an array to hold the bytes of the bitmap.
	// This code is specific to a bitmap with 24 bits per pixels.
	int points = s_chessboard->Width * s_chessboard->Height;
	array<Int32>^rgbValues = gcnew array<Int32>(points);

	Int32 value = 0;
	Int32 cell_size_x = 10;
	Int32 cell_size_y = 8;

	for(int x=0; x<side_size; x++)
		for(int y=0; y<side_size; y++)
		{
			value = 255- (( ((x%(2*cell_size_x))-(x%cell_size_x))/cell_size_x + ((y%(2*cell_size_y))-(y%cell_size_y))/cell_size_y)%2) *52;
			rgbValues[ x+y*s_chessboard->Width + 0]	= (255<<24)+(value<<16)+(value<<8)+value;
		}

	// Copy the RGB values back to the bitmap
	System::Runtime::InteropServices::Marshal::Copy( rgbValues, 0, ptr, points );

	// Unlock the bits.
	s_chessboard->UnlockBits( bmpData );

}

void			texture_document::distroy_chessboard				(Object^ , EventArgs^ )
{
	delete				s_chessboard;
	s_chessboard_alive	= false;
	s_chessboard		= nullptr;
}


static void		draw_image_on_picture								(Single a, Single r, Single g, Single b, Image^ picture, Image^ image, Image^ background)
{
	ColorMatrix^	matrix		= gcnew ColorMatrix();
	matrix->Matrix00			= r;
	matrix->Matrix11			= g;
	matrix->Matrix22			= b;
	matrix->Matrix33			= a;

	Drawing::Graphics^	gfx		= Drawing::Graphics::FromImage(picture);
	ImageAttributes^	attrs	= gcnew ImageAttributes();
	attrs->SetColorMatrix		(matrix);
	
	gfx->DrawImage				(background, 0, 0);
	gfx->DrawImage				(image, Drawing::Rectangle(0, 0, image->Width, image->Height), 0, 0, image->Width, image->Height, Drawing::GraphicsUnit::Pixel, attrs );
}

static void		draw_image_on_picture_fast							(Single a, Single r, Single g, Single b, Bitmap^ picture, Bitmap^ image, Bitmap^ background)
{
	if(a == 1 && r+g+b > 0)
	{
		draw_image_on_picture					(a, r, g, b, picture, image, background);
		return;
	}

	System::Drawing::Rectangle	rect		= System::Drawing::Rectangle(0,0,image->Width,image->Height);
	BitmapData^					bmp_data_src	= image->LockBits( rect, System::Drawing::Imaging::ImageLockMode::ReadOnly, image->PixelFormat );
	int*						src			= (int*)(bmp_data_src->Scan0.ToPointer());
	BitmapData^					bmp_data_dst	= picture->LockBits( rect, System::Drawing::Imaging::ImageLockMode::ReadOnly, picture->PixelFormat );
	int*						dst			= (int*)(bmp_data_dst->Scan0.ToPointer());

	texture_document_detail::bgra bgra_bridge;

	int size = image->Width*image->Height;
	if(a+r+g+b == 1)
	{
		for(int i=0; i<size; i++)
		{
			bgra_bridge.u	= (*src);
			u8 lightness	= (u8)math::ceil(bgra_bridge.b*b + bgra_bridge.g*g + bgra_bridge.r*r + bgra_bridge.a*a);
			bgra_bridge.b	= lightness; 
			bgra_bridge.g	= lightness;
			bgra_bridge.r	= lightness;
			bgra_bridge.a	= 255;
			(*dst)			= bgra_bridge.u;
			++dst;
			++src;
		}
	}
	else 
	{
		if(b+g+r == 3)
		{
			for(int i=0; i<size; i++)
			{
				(*dst)		= (*src);
				++dst;
				++src;
			}
		}
		else
		{
			for(int i=0; i<size; i++)
			{
				bgra_bridge.u	= (*src);
				bgra_bridge.b	*= (u8)b; 
				bgra_bridge.g	*= (u8)g;
				bgra_bridge.r	*= (u8)r;
				bgra_bridge.a	= 255;
				(*dst)			= bgra_bridge.u;
				++dst;
				++src;
			}
		}
	}
	
	bmp_data_dst->Stride -= 500;
	picture->UnlockBits(bmp_data_dst);
	image->UnlockBits(bmp_data_src);
}

void			texture_document::tga_loaded						(xray::resources::queries_result& data, Object^ resource_path_obj)
{
	if(m_need_suppress_loading)
		return;
	String^ resource_path = safe_cast<String^>(resource_path_obj);
	//if not synk call and next query exists, then return
	if(!m_is_tga_synk_called && m_tga_request_id != data.unique_id())
		return;

	if ( data.is_failed() )
		return;

	//if previous convert query do not started, suppress them
	if(m_last_buf_struct != NULL)
	{
		if(threading::interlocked_increment(m_last_buf_struct->m_need_process_flag) == 1)
		{
			m_last_image->UnlockBits	( m_last_image_data );
			delete						m_last_image;
		}
	}
	if(m_images->Count == 0)
		delete						m_last_image;

	//Get tga buffer, lock bitmap buffer and pass them to texture_tga_to_argb cooker
	resources::pinned_ptr_const<u8>* tga_buffer	= NEW(resources::pinned_ptr_const<u8>)(data[0].get_managed_resource());
	TgaHeader* header						= (TgaHeader*)tga_buffer->c_ptr();
	m_image_has_alpha						= header->pixel_size == 32;
	m_last_image							= gcnew Bitmap(header->width, header->height);

	texture_options* options	= NULL;
	if(resource_path != nullptr && safe_cast<resource_editor_base^>(m_editor)->is_resource_loaded(resource_path))
	{
		options = static_cast<texture_options*>(safe_cast<resource_editor_base^>(m_editor)->get_loaded_resource(resource_path)->m_resource->c_ptr());
	}
	if(options != NULL)
	{
		if((options->width != m_last_image->Width || options->height != m_last_image->Height))
		{
			options->width						= m_last_image->Width;
			options->height						= m_last_image->Height;
			options->save						();
			m_editor->properties_panel->property_grid_control->Refresh();
		}
	}
	else
	{
		m_image_width = m_last_image->Width;
		m_image_height = m_last_image->Height;
		m_is_wait_for_options = true;
	}

	// Lock the bitmap's bits.  
	System::Drawing::Rectangle	rect		= System::Drawing::Rectangle(0,0,m_last_image->Width,m_last_image->Height);
	m_last_image_data						= m_last_image->LockBits( rect, System::Drawing::Imaging::ImageLockMode::ReadOnly, m_last_image->PixelFormat );
	texture_tga_argb_buffers*	buf_struct	= NEW(texture_tga_argb_buffers)(
		tga_buffer,
		m_last_image_data->Scan0.ToPointer()
	);

	m_last_buf_struct						= buf_struct;
 	const_buffer buf						((pcvoid)buf_struct, sizeof(texture_tga_argb_buffers));

	m_argb_last_request_id = editor_resource_manager::query_create_resource	(
		"",
		buf, 
		resources::texture_tga_to_argb_class,  
		gcnew managed_query_callback(this, &texture_document::image_loaded),
		g_allocator
	);

	m_images->Add(m_argb_last_request_id, m_last_image);
}

void			texture_document::image_loaded						(xray::resources::queries_result& data)
{
	if(m_need_suppress_loading)
		return;

	if(m_argb_last_request_id == data.unique_id())
	{
		m_last_image->UnlockBits( m_last_image_data );

		if(m_image_has_alpha)
		{
			m_a_checkbox->Enabled					= true;
			m_ab_checkbox->Enabled					= true;
			if(m_a_checkbox->Checked)
			{
				m_r_checkbox->Enabled					= false;
				m_g_checkbox->Enabled					= false;
				m_b_checkbox->Enabled					= false;
			}
		}
		else
		{
			m_a_checkbox->Enabled					= false;
			m_ab_checkbox->Enabled					= false;
			m_r_checkbox->Enabled					= true;
			m_g_checkbox->Enabled					= true;
			m_b_checkbox->Enabled					= true;
		}

		delete m_picture_image;
		m_picture_image							= gcnew Bitmap(m_last_image->Width, m_last_image->Height);

		if(m_a_checkbox->Enabled && m_a_checkbox->Checked)
		{
			draw_image_on_picture_fast				(1, 0, 0, 0, m_picture_image, m_last_image, s_chessboard);
		}
		else
		{
			Single			a 						= m_ab_checkbox->Checked;
			Single			r 						= m_r_checkbox->Checked;
			Single			g 						= m_g_checkbox->Checked;
			Single			b 						= m_b_checkbox->Checked;

			draw_image_on_picture_fast				((m_image_has_alpha)?a:0, r, g, b, m_picture_image, m_last_image, s_chessboard);
		}
		m_picture_box->Image					= m_picture_image;

		m_last_buf_struct						= NULL;
	}
	else
	{
		delete									m_images[data.unique_id()];
	}

	//release cooker's buffer, Dispose image and remove it from collection
	const_buffer buf						= data[0].creation_data_from_user();
	DELETE									((texture_tga_argb_buffers*)(buf.c_ptr()));
	m_images->Remove						(data.unique_id());
}

static Boolean	flag = false;
void			texture_document::rgba_check_boxes_checked_changed	(System::Object^ sender, System::EventArgs^  )
{
	if(flag)
		return;
	XRAY_UNREFERENCED_PARAMETER	(sender);
	flag = true;
	if(m_a_checkbox != sender)
		m_a_checkbox->Checked = false;
	if(m_a_checkbox->Enabled && m_a_checkbox->Checked)
	{
		m_r_checkbox->Enabled = false;
		m_g_checkbox->Enabled = false;
		m_b_checkbox->Enabled = false;
		m_ab_checkbox->Checked = false;
		draw_image_on_picture_fast				(1, 0, 0, 0, m_picture_image, m_last_image, s_chessboard);
	}
	else
	{
		m_r_checkbox->Enabled = true;
		m_g_checkbox->Enabled = true;
		m_b_checkbox->Enabled = true;

		Single			a 						= m_ab_checkbox->Checked;
		Single			r 						= m_r_checkbox->Checked;
		Single			g 						= m_g_checkbox->Checked;
		Single			b 						= m_b_checkbox->Checked;

		draw_image_on_picture_fast				((m_image_has_alpha)?a:0, r, g, b, m_picture_image, m_last_image, s_chessboard);
	}
	m_picture_box->Image		= m_picture_image;
	flag = false;
}

}//namespace editor
}//namespace xray