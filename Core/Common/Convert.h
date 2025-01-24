///////////////////////////////////////////////////////////////////////////////////////////
// Filename:      Convert.h
// Description:   utility class for converting between types and formats
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

class Convert
{
	using UINT = unsigned int;
	using BYTE = unsigned char;

public:
	inline static UINT ArgbToAbgr(const UINT argb)
	{
		const BYTE A = (argb >> 24) & 0xFF;
		const BYTE R = (argb >> 16) & 0xFF;
		const BYTE G = (argb >>  8) & 0xFF;
		const BYTE B = (argb >>  0) & 0xFF;

		return (A << 24) | (B << 16) | (G << 8) | (R << 0);
	}
};
