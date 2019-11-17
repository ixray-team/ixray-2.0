#ifndef XRAY_RANDOMS_GENERATOR_H_INCLUDED
#define XRAY_RANDOMS_GENERATOR_H_INCLUDED

namespace xray {
namespace math {

class random32
{
public:
			random32	(u32 start_seed = 0) : m_seed(start_seed) {}

	u32		seed		()			{ return	m_seed; }
	void	seed		(u32 seed)	{ m_seed	= seed; }
	u32		random		(u32 range)	
	{
		m_seed	= 0x08088405*m_seed + 1;
		return	(u32(u64(m_seed)*u64(range) >> 32));
	}

private:
	u32					m_seed;
};
	 
} // namespace math
} // namespace xray

#endif // #ifndef XRAY_RANDOMS_GENERATOR_H_INCLUDED