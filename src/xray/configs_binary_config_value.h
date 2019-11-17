////////////////////////////////////////////////////////////////////////////
//	Created		: 25.03.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_CONFIGS_BINARY_CONFIG_VALUE_H_INCLUDED
#define XRAY_CONFIGS_BINARY_CONFIG_VALUE_H_INCLUDED

namespace xray {
namespace configs {

class XRAY_CORE_API binary_config_value {
public:
	pcstr					id;
	pcvoid					data;				// value if type<t_embedded_types, pointer to value, or pointer to first child if table
	u32						crc;				// crc of the name
	u16						type;
	u16						count;				// children count if table, value mem_usage otherwise

public:
	typedef binary_config_value const*			const_iterator;

public:
							binary_config_value	();

			void			dump				(pcstr prefix) const;
			void			fix_up				(u32 offset);

			bool			value_exists		(pcstr key) const;

	inline	bool			empty				() const;
	inline	u32				size				() const { return u32(begin() - end()); }

	inline	const_iterator	begin				() const;
	inline	const_iterator	end					() const;

	inline binary_config_value const& operator[](u32 const index) const;
	inline binary_config_value const& operator[](int const index) const;
		   binary_config_value const& operator[](pcstr key) const;

	inline	bool			operator <			(u32 crc) const;

//assignments
	inline	operator		bool				() const;
	inline	operator		s8					() const;
	inline	operator		u8					() const;
	inline	operator		s16					() const;
	inline	operator		u16					() const;
	inline	operator		s32					() const;
	inline	operator		u32					() const;
	inline	operator		float				() const;
	inline	operator		math::float2 const&	() const;
	inline	operator		math::float3 const&	() const;
	inline	operator		math::float4 const&	() const;
	inline	operator		pcstr				() const;

private:
	template <typename T, typename P0, typename P1>
	inline	T				cast_number			() const;

	template <typename T>
	inline	T				cast_unsigned_number() const;

	template <typename T>
	inline	T				cast_signed_number	() const;
}; // class binary_config_value

} // namespace configs
} // namespace xray

#include <xray/configs_binary_config_value_inline.h>

#endif // #ifndef XRAY_CONFIGS_BINARY_CONFIG_VALUE_H_INCLUDED