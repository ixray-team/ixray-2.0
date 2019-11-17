////////////////////////////////////////////////////////////////////////////
//	Created 	: 18.05.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "shader_compiler.h"
#include "utility_functions.h"

#include "resource_parser.h"
//#include "file_batcher.h"

#ifdef DELETE
#undef DELETE
#endif
#include "shader_compiler_memory.h"

namespace xray {
namespace shader_compiler {



	struct DX10_Traits
	{
		typedef D3D10_SHADER_DESC						D3D_SHADER_DESC;
		typedef	D3D10_SHADER_BUFFER_DESC				D3D_SHADER_BUFFER_DESC;
		typedef	D3D10_SHADER_VARIABLE_DESC				D3D_SHADER_VARIABLE_DESC;
		typedef D3D10_SHADER_INPUT_BIND_DESC			D3D_SHADER_INPUT_BIND_DESC;
		typedef	D3D10_SHADER_TYPE_DESC					D3D_SHADER_TYPE_DESC;
		typedef D3D10_CBUFFER_TYPE						D3D_CBUFFER_TYPE;
		typedef ID3D10ShaderReflection					ID3DShaderReflection;
		typedef	ID3D10ShaderReflectionConstantBuffer	ID3DShaderReflectionConstantBuffer;
		typedef	ID3D10ShaderReflectionVariable			ID3DShaderReflectionVariable;
		typedef	ID3D10ShaderReflectionType				ID3DShaderReflectionType;
	};

	struct DX11_Traits
	{
		typedef D3D11_SHADER_DESC						D3D_SHADER_DESC;
		typedef	D3D11_SHADER_BUFFER_DESC				D3D_SHADER_BUFFER_DESC;
		typedef	D3D11_SHADER_VARIABLE_DESC				D3D_SHADER_VARIABLE_DESC;
		typedef D3D11_SHADER_INPUT_BIND_DESC			D3D_SHADER_INPUT_BIND_DESC;
		typedef	D3D11_SHADER_TYPE_DESC					D3D_SHADER_TYPE_DESC;
		typedef D3D11_CBUFFER_TYPE						D3D_CBUFFER_TYPE;
		typedef ID3D11ShaderReflection					ID3DShaderReflection;
		typedef	ID3D11ShaderReflectionConstantBuffer	ID3DShaderReflectionConstantBuffer;
		typedef	ID3D11ShaderReflectionVariable			ID3DShaderReflectionVariable;
		typedef	ID3D11ShaderReflectionType				ID3DShaderReflectionType;
	};


class includer :
	public ID3DInclude,
	public ID3DXInclude,
	private boost::noncopyable 
{
public:
	inline includer( const std::vector<define_set>&  defines_set, char const* const shader_file_name, char const* const full_path ) :
		m_defines_set		( defines_set ),
		m_full_path			( full_path ),
		m_shader_file_name	( shader_file_name )
	{
		char* file_buffer	= 0;
		u32 file_size		= 0;
		read_file(m_shader_file_name,file_buffer,file_size);

		for ( std::vector<define_set>::const_iterator it = m_defines_set.begin(); it!=m_defines_set.end(); ++it)
			if ( find_simple_text_sensitive(file_buffer,it->define->get_name().c_str(),false)!=-1)
			{
				bool found = false;
				for ( u32 i=0; i<m_found_defines.size(); i++)
				{
					if (m_found_defines[i]==it->define->get_name())
					{
						found = true;
						break;
					}
				}
				if ( !found)
					m_found_defines.push_back(it->define->get_name());
			}
	}

private:
	HRESULT open_internal	( LPCSTR short_file_name, LPCVOID *ppData, UINT *pBytes )
	{
		string_path			file_name;
		strcpy_s			( file_name, m_full_path );
		strcat_s			( file_name, short_file_name );

		char* file_buffer	= 0;
		u32 file_size		= 0;
		read_file(file_name,file_buffer,file_size);
		*ppData				= file_buffer;
		*pBytes				= file_size;
		
		if ( file_buffer==0 || file_size==0 )
			return S_OK;
		
		for ( std::vector<define_set>::const_iterator it = m_defines_set.begin(); it!=m_defines_set.end(); ++it)
			if ( find_simple_text_sensitive((const char*)*ppData,it->define->get_name().c_str(),false)!=-1)
			{
				bool found = false;
				for ( u32 i=0; i<m_found_defines.size(); i++)
				{
					if (m_found_defines[i]==it->define->get_name())
					{
						found = true;
						break;
					}
				}
				if (!found)
					m_found_defines.push_back(it->define->get_name());
			}

		bool found = false;
		for ( std::vector<std::string>::const_iterator it = m_includes.begin(); it!=m_includes.end(); ++it)
		{
			if ( *it==file_name )
			{
				found = true;
				break;
			}
		}
		if (!found)
			m_includes.push_back(file_name);

		std::sort(m_includes.begin(),m_includes.end());
		return				S_OK;
	}

	HRESULT __stdcall	Open	(D3DXINCLUDE_TYPE IncludeType, LPCSTR short_file_name, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
	{
		(void)IncludeType;
		(void)pParentData;

		return				open_internal( short_file_name, ppData, pBytes );
	}

	HRESULT __stdcall	Open	(D3D10_INCLUDE_TYPE IncludeType, LPCSTR short_file_name, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
	{
		(void)IncludeType;
		(void)pParentData;

		return				open_internal( short_file_name, ppData, pBytes );
	}

	HRESULT __stdcall	Close	(LPCVOID	pData)
	{
		(void)pData;
		return				S_OK;
	}

public:
	const std::vector<std::string>& get_found_defines	( ) const { return m_found_defines; }
	const std::vector<std::string>& get_includes		( ) const { return m_includes;		}

private:
	std::vector<std::string>			m_includes;
	std::vector<std::string>			m_found_defines;
	const std::vector<define_set>&		m_defines_set;
	char const* const					m_full_path;
	char const* const					m_shader_file_name;
}; // class includer 



bool compiler::compile( bool legacy, u32& out_shader_size, u32& out_instruction_number, std::vector<std::string>& out_errors, std::vector<std::string>& out_warnings )
{
	out_shader_size = 0;
	
	std::string command_line = GetCommandLine();
	
	s32 sm_pos = command_line.find("-sm");
	std::string sm_text;
	
	std::string first_digit = "3";
	std::string second_digit = "0";
	
	if (sm_pos!=-1)
	{
		first_digit = command_line[sm_pos+3];
		second_digit = command_line[sm_pos+4];
	}

	std::string shader_model = first_digit + "_" + second_digit;

	if (first_digit=="3")
		legacy = strstr(command_line.c_str(),"-dx10")!=0;

	std::string file_extension;
	get_file_extension(m_file_name.c_str(),file_extension);

	std::string shader_version_id = file_extension + "_" + shader_model;

	std::string			file_path;// = "./";
	get_file_path(m_file_name.c_str(),file_path);

	
	includer inc(m_defines_set,m_file_name.c_str(),file_path.c_str());

	std::vector<std::string> found_defines = inc.get_found_defines();

	for (std::vector<define_set>::iterator def_it = m_defines_set.begin(); def_it!=m_defines_set.end(); ++def_it)
	{
		bool found = false;
		for (std::vector<std::string>::const_iterator it = found_defines.begin(); it!=found_defines.end(); ++it)
			if (def_it->define->get_name()==*it)
			{
				found = true;
				def_it->usable = true;
				break;
			}
		if (!found)
			def_it->usable = false;
	}
	
	D3D_SHADER_MACRO macros[260];
	memset(&macros[0],0,sizeof(macros));
	
	u32 macros_index = 0;
	for (std::vector<define_set>::const_iterator it = m_defines_set.begin(); it!=m_defines_set.end(); ++it)
	{
		defination* define = it->define;
		define_value* def_value = define->get_value(it->index_iter);

		if (!define || !def_value || def_value->is_undefined())
			continue;

		macros[macros_index].Name = define->get_name().c_str();
		macros[macros_index].Definition = define->get_value(it->index_iter)->get_value().c_str();
		macros_index++;
	}
	const char* errors_buffer = 0;
	u32 errors_buffer_size = 0;

	bool is_legacy  = legacy;
	bool is_dx10	= strstr(GetCommandLine(),"-dx10") != 0;
	bool is_dx11	= strstr(GetCommandLine(),"-dx11") != 0;
	
	//if shader model < 4.0, TODO: more resource parsing
	if (is_legacy)
	{
		unsigned long compiler_options = D3DXSHADER_DEBUG | D3DXSHADER_PACKMATRIX_ROWMAJOR | D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
		
		ID3DXBuffer* shader_buffer = 0;
		ID3DXBuffer* error_buffer = 0;
		
		if ( !strstr(command_line.c_str(),"-debug_mode") )
			compiler_options&=~D3DXSHADER_DEBUG;
		
		LPD3DXCONSTANTTABLE	constants = 0;

		D3DXCompileShader
		(
			m_buffer,
			m_buffer_size,
			(D3DXMACRO*)macros,
			&inc,
			m_main_function_id.c_str(),
			shader_version_id.c_str(),
			compiler_options,
			&shader_buffer,
			&error_buffer,
			&constants
		);

		if (error_buffer)
		{
			errors_buffer = (pstr)error_buffer->GetBufferPointer();
			errors_buffer_size = error_buffer->GetBufferSize();
		}
		
		if (constants && shader_buffer)
		{
			out_shader_size = shader_buffer->GetBufferSize();
			resource_parser::process_dx9(shader_buffer->GetBufferPointer(),constants->GetBufferPointer());
		}
	}
	else if (is_dx10 || is_dx11)
	{
		u32 compiler_options = D3D10_SHADER_PACK_MATRIX_ROW_MAJOR | D3D10_SHADER_DEBUG;
		
		ID3DBlob* shader_buffer	= 0;
		ID3DBlob* error_buffer	= 0;
		
		if ( !strstr(GetCommandLine(),"-debug_mode") )
		{
			compiler_options&=~D3D10_SHADER_DEBUG;
		}
		
		D3DCompile
		(
			m_buffer,
			m_buffer_size,
			m_file_name.c_str(),
			macros,
			&inc,
			m_main_function_id.c_str(),
			shader_version_id.c_str(),
			compiler_options,
			0,
			&shader_buffer,
			&error_buffer
		);
		if (error_buffer)
		{
			errors_buffer = (pstr)error_buffer->GetBufferPointer();
			errors_buffer_size = error_buffer->GetBufferSize();
		}

		if (shader_buffer)
		{
			out_shader_size = shader_buffer->GetBufferSize();

			if (is_dx10)
				resource_parser::process_dx10_dx11<DX10_Traits,true>(shader_buffer->GetBufferPointer(),shader_buffer->GetBufferSize(),out_instruction_number);
			else if (is_dx11)
				resource_parser::process_dx10_dx11<DX11_Traits,false>(shader_buffer->GetBufferPointer(),shader_buffer->GetBufferSize(),out_instruction_number);
		}
	}
	

	found_defines = inc.get_found_defines();
	
	for (std::vector<define_set>::iterator def_it = m_defines_set.begin(); def_it!=m_defines_set.end(); ++def_it)
	{
		bool found = false;
		for (std::vector<std::string>::const_iterator it = found_defines.begin(); it!=found_defines.end(); ++it)
		{
			if (def_it->define->get_name()==*it)
			{
				found = true;
				def_it->usable = true;
				break;
			}
		}
		if (!found)
			def_it->usable = false;
	}

	if (strstr(GetCommandLine(), "-show_includes"))
	{
		std::vector<std::string> includes = inc.get_includes();

		if (includes.size())
			printf("\n\n      INCLUDES\n");
		
		for (std::vector<std::string>::const_iterator it = includes.begin(); it!=includes.end(); ++it)
			printf("\n         %s",it->c_str());
		
		printf("\n");
	}
	if (errors_buffer)
	{
		std::string errors_text;
		std::vector<std::string> lines;

		errors_text.assign(errors_buffer,errors_buffer + errors_buffer_size);
		u32 curr_pos = 0;
		for (u32 i=0; i<errors_text.size(); i++)
		{
			if (errors_text[i]=='\n')
			{
				std::string line;
				for (u32 s=curr_pos; s<=i; s++)
					line+=errors_text[s];
				curr_pos = i+1;
				lines.push_back(line);
			}
		}
		for (u32 i=0; i<lines.size(); i++)
		{
			if (lines[i].find("error")!=-1)
			{
				out_errors.push_back(lines[i]);
			}
			else if (lines[i].find("warning")!=-1)
			{
				out_warnings.push_back(lines[i]);
			}
		}
	}

	return out_errors.size()==0;
}


} // namespace shader_compiler
} // namespace xray