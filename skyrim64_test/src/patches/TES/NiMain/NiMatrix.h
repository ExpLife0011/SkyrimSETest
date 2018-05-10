#pragma once

#include "NiPoint.h"

class NiMatrix3
{
public:
	float m_pEntry[3][3];

	inline NiMatrix3()
	{
	}

	inline NiMatrix3(const NiMatrix3& Src)
	{
		memcpy(&m_pEntry, Src.m_pEntry, sizeof(m_pEntry));
	}

	NiMatrix3 Transpose() const
	{
		// Swap [rows, cols] with [cols, rows]. Can only be optimized with AVX.
		NiMatrix3 m;

		m.m_pEntry[0][0] = m_pEntry[0][0];
		m.m_pEntry[0][1] = m_pEntry[1][0];
		m.m_pEntry[0][2] = m_pEntry[2][0];

		m.m_pEntry[1][0] = m_pEntry[0][1];
		m.m_pEntry[1][1] = m_pEntry[1][1];
		m.m_pEntry[1][2] = m_pEntry[2][1];

		m.m_pEntry[2][0] = m_pEntry[0][2];
		m.m_pEntry[2][1] = m_pEntry[1][2];
		m.m_pEntry[2][2] = m_pEntry[2][2];

		return m;
	}

	NiPoint3 operator* (const NiPoint3& Point) const
	{
		NiPoint3 p;

		p.x = m_pEntry[0][0] * Point.x + m_pEntry[0][1] * Point.y + m_pEntry[0][2] * Point.z;
		p.y = m_pEntry[1][0] * Point.x + m_pEntry[1][1] * Point.y + m_pEntry[1][2] * Point.z;
		p.z = m_pEntry[2][0] * Point.x + m_pEntry[2][1] * Point.y + m_pEntry[2][2] * Point.z;

		return p;
	}
};
static_assert(sizeof(NiMatrix3) == 0x24);
static_assert_offset(NiMatrix3, m_pEntry, 0x0);