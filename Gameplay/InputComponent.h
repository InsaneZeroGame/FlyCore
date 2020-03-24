#pragma once

namespace Gameplay
{
	class InputComponent
	{
	public:
		InputComponent();

		virtual ~InputComponent();

		virtual void OnMouseWheelScroll(double x, double y) = 0;

	private:

	};

	
}