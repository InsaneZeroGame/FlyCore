#pragma once

namespace Event
{
	enum Type
	{
		WindowResize = 0x0000,
		WindowClosed = 0x0001,
	};

	enum Catagory
	{
		WindowEvent = 0x0000,
	};

	class IEvent
	{
	public:
		

	protected:
		IEvent() {};
		virtual ~IEvent() {};
	};
}