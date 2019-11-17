#include "pch.h"
#include "terrain_import_form.h"
#include "terrain_quad.h"
#include <FreeImage/FreeImage.h>

namespace xray{
namespace editor{


void terrain_import_form::in_constructor()
{
	m_settings	= gcnew terrain_import_settings;
	height_scale_track_bar_Scroll(this, nullptr);
}

void terrain_import_form::select_source_button_Click(System::Object^, System::EventArgs^)
{
	System::Windows::Forms::OpenFileDialog  openFileDialog;
	
	openFileDialog.FileName			= source_text_box->Text;
	openFileDialog.Filter			= "bitmap files|*.bmp;*.tga;*.dds;*.jpg|All files|*.*";
	openFileDialog.FilterIndex		= 1;

	if(openFileDialog.ShowDialog() == ::DialogResult::OK)
	{
		source_text_box->Text		= openFileDialog.FileName;

		unmanaged_string fn		(openFileDialog.FileName);
		FREE_IMAGE_FORMAT	fif = FreeImage_GetFIFFromFilename(fn.c_str());
		FIBITMAP* fibitmap		= FreeImage_Load(fif, fn.c_str());
		u32 bitmap_width		= FreeImage_GetWidth(fibitmap);
		u32 bitmap_height		= FreeImage_GetHeight(fibitmap);

		bool has_alpha			= !!FreeImage_IsTransparent(fibitmap);
		file_desc_label->Text = System::String::Format("Size : {0}x{1}. Alpha channel : {2}", bitmap_width, bitmap_height, (has_alpha)?"present":"not present");
		diffuse_rgba_radio->Enabled			= has_alpha;
		diffuse_rgb_height_a_radio->Enabled = has_alpha;
		height_scale_track_bar->Enabled		= has_alpha;

		if(!has_alpha && (diffuse_rgba_radio->Checked || diffuse_rgb_height_a_radio->Checked))
			diffuse_rgb_radio->Checked = true;

		FreeImage_Unload		(fibitmap);
	}
}

void terrain_import_form::ok_button_Click(System::Object^, System::EventArgs^)
{
	m_settings->m_source_filename	= source_text_box->Text;
	if(diffuse_rgb_radio->Checked)
		m_settings->m_options	= terrain_import_settings::options::rgb_diffuse;
	else
	if(diffuse_rgba_radio->Checked)
		m_settings->m_options	= terrain_import_settings::options::rgba_diffuse;
	else
	if(diffuse_rgb_height_a_radio->Checked)
		m_settings->m_options	= terrain_import_settings::options::rgb_diffuse_a_heightmap;
	else
	if(height_a_radio->Checked)
		m_settings->m_options	= terrain_import_settings::options::a_heightmap;

	m_settings->m_b_create_cell		= create_cell_if_empty_check->Checked;
	m_settings->m_b_stretch_image	= stretch_image_check->Checked;

	DialogResult = ::DialogResult::OK;
}

void terrain_import_form::cancel_button_Click(System::Object^, System::EventArgs^)
{
	DialogResult = ::DialogResult::Cancel;
}

void terrain_import_form::height_scale_track_bar_Scroll(System::Object^, System::EventArgs^)
{	//[1...1000] range
	float _zero		= 200.0f;
	float scale		= height_scale_track_bar->Value / _zero;

	m_settings->m_height_scale			= scale;
	height_scale_value_text_box->Text	= System::String::Format("{0:f2} (bound: +/- {1:f1}m)", m_settings->m_height_scale, 128.0f*scale);
}

} // namespace editor
} // namespace xray