#ifndef PATH_STRING_H_INCLUDED
#define PATH_STRING_H_INCLUDED

#include <xray/fixed_string.h>

namespace xray	{
namespace fs	{

static	u32	const max_path_length			= sizeof(string_path) / sizeof(char);
typedef	fixed_string< max_path_length >		path_string;

template class XRAY_CORE_API fixed_string< max_path_length >;

} // namespace fs
} // namespace xray

#endif // FS_PATH_H_INCLUDED