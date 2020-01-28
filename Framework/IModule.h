#pragma once
#include "INoCopy.h"


namespace Interface
{
	class IModule : public INoCopy
	{
	public:
		virtual void OnInit() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnDestory()  = 0;
	protected:
		IModule() {};
		virtual ~IModule() {};
	};
}