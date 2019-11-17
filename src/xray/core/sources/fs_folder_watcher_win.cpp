/////////////////////////////////////////////////////////////////////////////////
//	Created 	: 01.02.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
/////////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "fs_folder_watcher_win.h"
#include <xray/fs_utils.h>
#include <xray/fs_path.h>

namespace xray {
namespace fs {

folder_watcher::folder_watcher (pcstr folder_name, file_change_callback const & callback, threading::event * receive_changes_event) 
							:	m_path					(folder_name),
								m_callback				(callback),
								m_current_buffer		(m_buffer[0]),
								m_next_buffer			(m_buffer[1]),
								m_receive_changes_event	(receive_changes_event)
{
	verify_path_is_portable						(m_path.c_str());
	m_handle								=	CreateFile (convert_to_native(m_path.c_str()).c_str(),
															FILE_LIST_DIRECTORY,
															FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
															0,
															OPEN_EXISTING,
															FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
															0);

	R_ASSERT									(m_handle != INVALID_HANDLE_VALUE);
	m_overlapped.hEvent						=	this;
	subscribe_to_receive_changes				(swap_buffers_false);
}

folder_watcher::~folder_watcher								( )
{
	m_overlapped.hEvent						=	NULL;
	R_ASSERT									(m_handle != INVALID_HANDLE_VALUE);
	CloseHandle									(m_handle);
}

void   folder_watcher::subscribe_to_receive_changes (swap_buffers_bool const swap)
{
	if ( swap == swap_buffers_true )
		std::swap								(m_current_buffer, m_next_buffer);

	R_ASSERT									(m_receive_changes_event);
	m_overlapped.hEvent						=	m_receive_changes_event->get_handle();

	BOOL const result						=	ReadDirectoryChangesW ( m_handle,
																	m_current_buffer,
																	sizeof(m_buffer[0]),
																	TRUE,
																		FILE_NOTIFY_CHANGE_FILE_NAME |
																		FILE_NOTIFY_CHANGE_DIR_NAME |
																		FILE_NOTIFY_CHANGE_SIZE | 
																		FILE_NOTIFY_CHANGE_LAST_WRITE | 
																		FILE_NOTIFY_CHANGE_CREATION,
																	NULL,
																	& m_overlapped,
																	NULL );
	R_ASSERT_U									(result);
}

void   folder_watcher::try_receive_changes ()
{
	DWORD bytes_transfered					=	0;
	BOOL const overlapped_successfull		=	GetOverlappedResult(m_handle, & m_overlapped, &bytes_transfered, false);

	if ( !overlapped_successfull )
	{
		u32 const last_error				=	GetLastError();
		if ( last_error == ERROR_IO_INCOMPLETE )
			return;

		if ( last_error == ERROR_OPERATION_ABORTED )
			return;

		FATAL									("Async operation failed. ErrorCode: %d", last_error);
	}

	if ( !bytes_transfered )
	{
		subscribe_to_receive_changes			(swap_buffers_false);
		return;
	}

	subscribe_to_receive_changes				(swap_buffers_true);
	receive_changes								();
}

static inline
FILE_NOTIFY_INFORMATION *   next_file_notify_information (FILE_NOTIFY_INFORMATION * const info)
{
	if ( !info->NextEntryOffset )
		return									0;
	return										(FILE_NOTIFY_INFORMATION*)(((u8*)info) + info->NextEntryOffset);
}

static 
bool   get_file_name_from_file_notify_information (FILE_NOTIFY_INFORMATION * const info, buffer_string * out_file_name)
{
	CURE_ASSERT_CMP								(info->FileNameLength, <, MAX_PATH, return false);
	size_t	size_in_bytes					=	((info->FileNameLength + 1) * 2);
	pstr	result							=	(pstr)ALLOCA(size_in_bytes);
	WCHAR	wc_file_name[MAX_PATH + 1]		=	{ 0 };
	memory::copy								(wc_file_name, 
												 array_size(wc_file_name), 
												 info->FileName, 
												 std::min( DWORD(MAX_PATH * sizeof(WCHAR)), info->FileNameLength));
	wc_file_name[info->FileNameLength]		=	0;

	size_t converted_characters				=	0;
	errno_t	const error						=	wcstombs_s	(&converted_characters, result, size_in_bytes, wc_file_name, size_in_bytes);
	R_ASSERT_U									(!error);

	* out_file_name							=	result;
	return										true;
}

void   folder_watcher::receive_changes ()
{
//	u32 start_file_changes_size	= m_file_changes.size();
	FILE_NOTIFY_INFORMATION * current		=	(FILE_NOTIFY_INFORMATION *)m_next_buffer;

	file_change_info	info;
	info.type								=	file_change_info::change_type_unset;

	while ( current ) 
	{
		if ( info.type != file_change_info::change_type_unset )
		{
			info.type						=	file_change_info::change_type_unset;
			info.file_path					=	"";
			info.renamed_old_file_path		=	"";
			info.renamed_new_file_path		=	"";
		}

		path_string								relative_file_path;
		if ( !get_file_name_from_file_notify_information(current, & relative_file_path) 
			  || relative_file_path.find(".svn") != relative_file_path.npos )
		{
			current							=	next_file_notify_information(current);
			continue;
		}

		relative_file_path					=	convert_to_portable(relative_file_path.c_str());
		info.file_path						=	m_path;
		info.file_path						+=	'/';
		info.file_path						+=	relative_file_path;

		switch ( current->Action ) 
		{
			case FILE_ACTION_ADDED :			info.type	=	file_change_info::change_type_added;	break;
			case FILE_ACTION_REMOVED :			info.type	=	file_change_info::change_type_removed;	break;
			case FILE_ACTION_MODIFIED :			info.type	=	file_change_info::change_type_modified;	break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				info.renamed_old_file_path	=	m_path;
				info.renamed_old_file_path	+=	'/';
				info.renamed_old_file_path	+=	relative_file_path;
				if ( info.renamed_new_file_path.length() )
					info.type				=	file_change_info::change_type_renamed;
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:	
				info.renamed_new_file_path	=	m_path;
				info.renamed_new_file_path	+=	'/';
				info.renamed_new_file_path	+=	relative_file_path;
				if ( info.renamed_old_file_path.length() )
					info.type				=	file_change_info::change_type_renamed;
				break;
		};

		if ( info.type != file_change_info::change_type_unset )
		{
			if ( info.type == file_change_info::change_type_renamed )
				info.file_path				=	"";

			m_callback							(info);
			info.type						=	file_change_info::change_type_unset;
		}

		current								=	next_file_notify_information(current);
	}

	R_ASSERT									(info.type == file_change_info::change_type_unset);
}

} // namespace fs
} // namespace xray
