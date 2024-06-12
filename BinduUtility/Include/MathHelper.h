#pragma once
#include <DirectXMath.h>


class MathHelper
{
public:
	
	template <typename T>
	static inline T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}
};
