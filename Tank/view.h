#include <SFMl/Graphics.hpp>

using namespace sf;

View view;

View getCoordinateView(float x, float y)
{
	float tempX = x; float tempY=y;
	if (x < 304) tempX = 304;
	if (y < 224) tempY = 224;
	if (x > 688) tempX = 688;
	if (y > 768) tempY = 768;

	view.setCenter(tempX + 16, tempY + 16);
	return view;
}
