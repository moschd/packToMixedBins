#ifndef RECT_H
#define RECT_H

struct RectRow
{
	int axis_;
	int nrOfItems_;
	int width_;
	int height_;
	int stackingSpaceConsumption_;
	int singularItemRowConsumption_;
	bool valid_;
};

struct RectSize
{
	int width;
	int height;
};

struct Rect
{
	int x;
	int y;
	int width;
	int height;
	int corner;
};

bool IsContainedIn(const Rect &a, const Rect &b)
{
	return a.x >= b.x && a.y >= b.y && a.x + a.width <= b.x + b.width && a.y + a.height <= b.y + b.height;
}

#endif