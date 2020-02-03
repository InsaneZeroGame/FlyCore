#pragma once
#include "INoCopy.h"
#include <cstdint>

namespace Interface
{
	class RenderObject : public INoCopy
	{
	public:
		virtual ~RenderObject() {};

	protected:
		RenderObject() {};
	};
}