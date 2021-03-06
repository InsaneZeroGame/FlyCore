#pragma once

namespace Gameplay
{
	class InputComponent
	{
	public:
		InputComponent();

		virtual ~InputComponent();

		virtual void OnMouseWheelScroll(double x, double y) = 0;

		virtual void OnKeyPress(int key, int scancode, int action, int mods) = 0;
				
		virtual void OnMouseMove(double x, double y) = 0;
	private:

	};

	
}