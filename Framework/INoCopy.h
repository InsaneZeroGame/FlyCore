#pragma once

namespace Interface
{
	class INoCopy
	{
	public:
		
		INoCopy(const INoCopy&) = delete;
		INoCopy(INoCopy&&) = delete;
		void operator =(const INoCopy&) = delete;
	protected:
		INoCopy() {};
		virtual ~INoCopy() {};

	};
}