

/** MaxRectsBinPack implements the MAXRECTS data structure and different bin packing algorithms that
	use this structure. */
class MaxRectsBinPack
{
public:
	/// Instantiates a bin of size (0,0). Call Init to create a new bin.
	MaxRectsBinPack();

	/// Instantiates a bin of the given size.
	/// @param allowFlip Specifies whether the packing algorithm is allowed to rotate the input rectangles by 90 degrees to consider a better placement.
	MaxRectsBinPack(int width, int height, int aItemWidth, int aItemHeight, bool allowFlip = true);

	/// -- ! use reset to re initialize.
	/// (Re)initializes the packer to an empty bin of width x height units. Call whenever
	/// you need to restart with a new bin.
	void Init(int width, int height, int aItemWidth, int aItemHeight, bool allowFlip = true);

	void reset();

	/// Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
	/// @param rects The list of rectangles to insert. This vector will be destroyed in the process.
	/// @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
	/// @param method The rectangle placement rule to use when packing.
	void Insert(std::vector<RectSize> &rects, std::vector<Rect> &dst, HeuristicAlgorithmType method);

	/// Inserts a single rectangle into the bin, possibly rotated.
	Rect insert(HeuristicAlgorithmType method);

	/// Computes the ratio of used surface area to the total bin area.
	int Occupancy() const;

private:
	int binWidth;
	int binHeight;
	int itemWidth_;
	int itemHeight_;
	bool binAllowFlip;

	std::vector<Rect> freeRectangles;
	std::vector<Rect> newFreeRectangles;
	size_t newFreeRectanglesLastSize;
	std::vector<Rect> usedRectangles;
	std::array<std::array<int, 4>, 4> spiralPackFreeSquares_;
	std::shared_ptr<NeatPacker> neatPacker_;

	/// Computes the placement score for placing the given rectangle with the given method.
	/// @param score1 [out] The primary placement score will be outputted here.
	/// @param score2 [out] The secondary placement score will be outputted here. This is used to break ties.
	/// @return This struct identifies where the rectangle would be placed if it were placed.
	Rect ScoreRect(int width, int height, HeuristicAlgorithmType method, int &score1, int &score2) const;

	/// Places the given rectangle into the bin.
	void PlaceRect(const Rect &node);

	/// Computes the placement score for the -CP variant.
	int ContactPointScoreNode(int x, int y, int width, int height) const;

	Rect FindPositionForNewNodeBottomLeft(int width, int height, int &bestY, int &bestX) const;
	Rect FindPositionForNewNodeBestShortSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const;
	Rect FindPositionForNewNodeBestLongSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const;
	Rect FindPositionForNewNodeBestAreaFit(int width, int height, int &bestAreaFit, int &bestShortSideFit) const;
	Rect FindPositionForNewNodeContactPoint(int width, int height, int &contactScore) const;
	Rect StartNeatGroupFitPacking(int width, int height);

	void InsertNewFreeRectangle(const Rect &newFreeRect);

	void packSquare(const std::array<int, 2> aSquare, int aWidth, int aHeight);

	/// @return True if the free node was split.
	bool SplitFreeNode(const Rect &freeNode, const Rect &usedNode);

	/// Goes through the free rectangle list and removes any redundant entries.
	void PruneFreeList();
};

MaxRectsBinPack::MaxRectsBinPack(int width,
								 int height,
								 int aItemWidth,
								 int aItemHeight,
								 bool allowFlip) : binWidth(width),
												   binHeight(height),
												   itemWidth_(aItemWidth),
												   itemHeight_(aItemHeight),
												   binAllowFlip(allowFlip),
												   freeRectangles(),
												   newFreeRectangles(),
												   newFreeRectanglesLastSize(),
												   usedRectangles(),
												   spiralPackFreeSquares_()
{
	MaxRectsBinPack::Init(width, height, aItemWidth, aItemHeight, allowFlip);
}

/// @brief Reset the class attributes; start packing with a fresh bin.
void MaxRectsBinPack::reset()
{
	MaxRectsBinPack::Init(binWidth, binHeight, itemWidth_, itemHeight_, binAllowFlip);
};

void MaxRectsBinPack::Init(int width, int height, int aItemWidth, int aItemHeight, bool allowFlip)
{
	binWidth = width;
	binHeight = height;
	binAllowFlip = allowFlip;

	Rect n;
	n.x = 0;
	n.y = 0;
	n.width = width;
	n.height = height;

	usedRectangles.clear();
	freeRectangles.clear();

	freeRectangles.push_back(n);
}

Rect MaxRectsBinPack::insert(HeuristicAlgorithmType method)
{
	Rect newNode;
	// Unused in this function. We don't need to know the score after finding the position.
	int score1 = std::numeric_limits<int>::max();
	int score2 = std::numeric_limits<int>::max();
	switch (method)
	{
	case RectBestShortSideFit:
		newNode = FindPositionForNewNodeBestShortSideFit(itemWidth_, itemHeight_, score1, score2);
		break;
	case RectBottomLeftRule:
		newNode = FindPositionForNewNodeBottomLeft(itemWidth_, itemHeight_, score1, score2);
		break;
	case RectContactPointRule:
		newNode = FindPositionForNewNodeContactPoint(itemWidth_, itemHeight_, score1);
		break;
	case RectBestLongSideFit:
		newNode = FindPositionForNewNodeBestLongSideFit(itemWidth_, itemHeight_, score2, score1);
		break;
	case RectBestAreaFit:
		newNode = FindPositionForNewNodeBestAreaFit(itemWidth_, itemHeight_, score1, score2);
		break;		
	}

	if (newNode.width > 0)
	{
		PlaceRect(newNode);
	};

	return newNode;
}

void MaxRectsBinPack::Insert(std::vector<RectSize> &rects, std::vector<Rect> &dst, HeuristicAlgorithmType method)
{
	dst.clear();

	while (rects.size() > 0)
	{
		int bestScore1 = std::numeric_limits<int>::max();
		int bestScore2 = std::numeric_limits<int>::max();
		int bestRectIndex = -1;
		Rect bestNode;

		for (size_t i = 0; i < rects.size(); ++i)
		{
			int score1;
			int score2;
			Rect newNode = ScoreRect(rects[i].width, rects[i].height, method, score1, score2);

			if (score1 < bestScore1 || (score1 == bestScore1 && score2 < bestScore2))
			{
				bestScore1 = score1;
				bestScore2 = score2;
				bestNode = newNode;
				bestRectIndex = i;
			}
		}

		if (bestRectIndex == -1)
			return;

		PlaceRect(bestNode);
		dst.push_back(bestNode);
		rects[bestRectIndex] = rects.back();
		rects.pop_back();
	}
}

/**
 * @brief
 *
 * @param node - The newly created rectangle.
 */
void MaxRectsBinPack::PlaceRect(const Rect &node)
{
	for (size_t i = 0; i < freeRectangles.size();)
	{
		if (SplitFreeNode(freeRectangles[i], node))
		{
			freeRectangles[i] = freeRectangles.back();
			freeRectangles.pop_back();
		}
		else
			++i;
	}

	PruneFreeList();

	usedRectangles.push_back(node);
}

Rect MaxRectsBinPack::ScoreRect(int width, int height, HeuristicAlgorithmType method, int &score1, int &score2) const
{
	Rect newNode;
	score1 = std::numeric_limits<int>::max();
	score2 = std::numeric_limits<int>::max();
	switch (method)
	{
	case RectBestShortSideFit:
		newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2);
		break;
	case RectBottomLeftRule:
		newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2);
		break;
	case RectContactPointRule:
		newNode = FindPositionForNewNodeContactPoint(width, height, score1);
		score1 = -score1; // Reverse since we are std::minimizing, but for contact point score bigger is better.
		break;
	case RectBestLongSideFit:
		newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1);
		break;
	case RectBestAreaFit:
		newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2);
		break;
	}

	// Cannot fit the current rectangle.
	if (newNode.height == 0)
	{
		score1 = std::numeric_limits<int>::max();
		score2 = std::numeric_limits<int>::max();
	}

	return newNode;
}

/// Computes the ratio of used surface area.
int MaxRectsBinPack::Occupancy() const
{
	uint64_t usedSurfaceArea = 0;
	for (size_t i = 0; i < usedRectangles.size(); ++i)
		usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

	return (int)usedSurfaceArea / ((uint64_t)binWidth * binHeight);
}

Rect MaxRectsBinPack::FindPositionForNewNodeBestShortSideFit(int aWidth, int aHeight, int &bestShortSideFit, int &bestLongSideFit) const
{
	Rect bestNode = {};

	bestShortSideFit = std::numeric_limits<int>::max();
	bestLongSideFit = std::numeric_limits<int>::max();

	for (size_t i = 0; i < freeRectangles.size(); ++i)
	{
		// Try to place the rectangle in upright (non-flipped) orientation.
		if (freeRectangles[i].width >= aWidth && freeRectangles[i].height >= aHeight)
		{

			int leftoverHoriz = abs(freeRectangles[i].width - aWidth);
			int leftoverVert = abs(freeRectangles[i].height - aHeight);

			/*
				shortSideFit - the shortest distance between box border and bin border.
				longSideFit - the longest distance between box border and bin border.
			*/
			int shortSideFit = std::min(leftoverHoriz, leftoverVert);
			int longSideFit = std::max(leftoverHoriz, leftoverVert);

			if (shortSideFit < bestShortSideFit || (shortSideFit == bestShortSideFit && longSideFit < bestLongSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = aWidth;
				bestNode.height = aHeight;
				bestShortSideFit = shortSideFit;
				bestLongSideFit = longSideFit;
			}
		}

		if (binAllowFlip && freeRectangles[i].width >= aHeight && freeRectangles[i].height >= aWidth)
		{
			int flippedLeftoverHoriz = abs(freeRectangles[i].width - aHeight);
			int flippedLeftoverVert = abs(freeRectangles[i].height - aWidth);
			int flippedShortSideFit = std::min(flippedLeftoverHoriz, flippedLeftoverVert);
			int flippedLongSideFit = std::max(flippedLeftoverHoriz, flippedLeftoverVert);

			if (flippedShortSideFit < bestShortSideFit || (flippedShortSideFit == bestShortSideFit && flippedLongSideFit < bestLongSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = aHeight;
				bestNode.height = aWidth;
				bestShortSideFit = flippedShortSideFit;
				bestLongSideFit = flippedLongSideFit;
			}
		}
	}
	return bestNode;
}

bool MaxRectsBinPack::SplitFreeNode(const Rect &freeNode, const Rect &usedNode)
{
	// Test if the rectangle will fit inside, if it does not, we return.
	if (usedNode.x >= freeNode.x + freeNode.width || usedNode.x + usedNode.width <= freeNode.x ||
		usedNode.y >= freeNode.y + freeNode.height || usedNode.y + usedNode.height <= freeNode.y)
	{
		return false;
	};

	// We add up to four new free rectangles to the free rectangles list below. None of these
	// four newly added free rectangles can overlap any other three, so keep a mark of them
	// to avoid testing them against each other.
	newFreeRectanglesLastSize = newFreeRectangles.size();

	if (usedNode.x < freeNode.x + freeNode.width && usedNode.x + usedNode.width > freeNode.x)
	{
		// New node at the top side of the used node.
		if (usedNode.y > freeNode.y && usedNode.y < freeNode.y + freeNode.height)
		{
			Rect newNode = freeNode;
			newNode.height = usedNode.y - newNode.y;
			InsertNewFreeRectangle(newNode);
		}

		// New node at the bottom side of the used node.
		if (usedNode.y + usedNode.height < freeNode.y + freeNode.height)
		{
			Rect newNode = freeNode;
			newNode.y = usedNode.y + usedNode.height;
			newNode.height = freeNode.y + freeNode.height - (usedNode.y + usedNode.height);
			InsertNewFreeRectangle(newNode);
		}
	}

	if (usedNode.y < freeNode.y + freeNode.height && usedNode.y + usedNode.height > freeNode.y)
	{
		// New node at the left side of the used node.
		if (usedNode.x > freeNode.x && usedNode.x < freeNode.x + freeNode.width)
		{
			Rect newNode = freeNode;
			newNode.width = usedNode.x - newNode.x;
			InsertNewFreeRectangle(newNode);
		}

		// New node at the right side of the used node.
		if (usedNode.x + usedNode.width < freeNode.x + freeNode.width)
		{
			Rect newNode = freeNode;
			newNode.x = usedNode.x + usedNode.width;
			newNode.width = freeNode.x + freeNode.width - (usedNode.x + usedNode.width);
			InsertNewFreeRectangle(newNode);
		}
	}

	return true;
}

void MaxRectsBinPack::InsertNewFreeRectangle(const Rect &newFreeRect)
{
	assert(newFreeRect.width > 0);
	assert(newFreeRect.height > 0);

	for (size_t i = 0; i < newFreeRectanglesLastSize;)
	{
		// This new free rectangle is already accounted for?
		if (IsContainedIn(newFreeRect, newFreeRectangles[i]))
			return;

		// Does this new free rectangle obsolete a previous new free rectangle?
		if (IsContainedIn(newFreeRectangles[i], newFreeRect))
		{
			// Remove i'th new free rectangle, but do so by retaining the order
			// of the older vs newest free rectangles that we may still be placing
			// in calling function SplitFreeNode().
			newFreeRectangles[i] = newFreeRectangles[--newFreeRectanglesLastSize];
			newFreeRectangles[newFreeRectanglesLastSize] = newFreeRectangles.back();
			newFreeRectangles.pop_back();
		}
		else
			++i;
	}
	newFreeRectangles.push_back(newFreeRect);
}

void MaxRectsBinPack::PruneFreeList()
{
	// Test all newly introduced free rectangles against old free rectangles.
	for (size_t i = 0; i < freeRectangles.size(); ++i)
	{
		for (size_t j = 0; j < newFreeRectangles.size();)
		{
			if (IsContainedIn(newFreeRectangles[j], freeRectangles[i]))
			{
				newFreeRectangles[j] = newFreeRectangles.back();
				newFreeRectangles.pop_back();
			}
			else
			{
				// The old free rectangles can never be contained in any of the
				// new free rectangles (the new free rectangles keep shrinking
				// in size)
				assert(!IsContainedIn(freeRectangles[i], newFreeRectangles[j]));
				++j;
			}
		}
	}

	// Merge new and old free rectangles to the group of old free rectangles.
	freeRectangles.insert(freeRectangles.end(), newFreeRectangles.begin(), newFreeRectangles.end());
	newFreeRectangles.clear();
}

// Below this line we have other 2d packing algorithms which i currently do not use.

Rect MaxRectsBinPack::FindPositionForNewNodeBottomLeft(int width, int height, int &bestY, int &bestX) const
{
	Rect bestNode = {};

	bestY = std::numeric_limits<int>::max();
	bestX = std::numeric_limits<int>::max();

	for (size_t i = 0; i < freeRectangles.size(); ++i)
	{
		// Try to place the rectangle in upright (non-flipped) orientation.
		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int topSideY = freeRectangles[i].y + height;
			if (topSideY < bestY || (topSideY == bestY && freeRectangles[i].x < bestX))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestY = topSideY;
				bestX = freeRectangles[i].x;
			}
		}
		if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
		{
			int topSideY = freeRectangles[i].y + width;
			if (topSideY < bestY || (topSideY == bestY && freeRectangles[i].x < bestX))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = height;
				bestNode.height = width;
				bestY = topSideY;
				bestX = freeRectangles[i].x;
			}
		}
	}
	return bestNode;
}
Rect MaxRectsBinPack::FindPositionForNewNodeBestLongSideFit(int width, int height,
															int &bestShortSideFit, int &bestLongSideFit) const
{
	Rect bestNode = {};

	bestShortSideFit = std::numeric_limits<int>::max();
	bestLongSideFit = std::numeric_limits<int>::max();

	for (size_t i = 0; i < freeRectangles.size(); ++i)
	{
		// Try to place the rectangle in upright (non-flipped) orientation.
		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int leftoverHoriz = abs(freeRectangles[i].width - width);
			int leftoverVert = abs(freeRectangles[i].height - height);
			int shortSideFit = std::min(leftoverHoriz, leftoverVert);
			int longSideFit = std::max(leftoverHoriz, leftoverVert);

			if (longSideFit < bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestShortSideFit = shortSideFit;
				bestLongSideFit = longSideFit;
			}
		}

		if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
		{
			int leftoverHoriz = abs(freeRectangles[i].width - height);
			int leftoverVert = abs(freeRectangles[i].height - width);
			int shortSideFit = std::min(leftoverHoriz, leftoverVert);
			int longSideFit = std::max(leftoverHoriz, leftoverVert);

			if (longSideFit < bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = height;
				bestNode.height = width;
				bestShortSideFit = shortSideFit;
				bestLongSideFit = longSideFit;
			}
		}
	}
	return bestNode;
}

Rect MaxRectsBinPack::FindPositionForNewNodeBestAreaFit(int width, int height,
														int &bestAreaFit, int &bestShortSideFit) const
{
	Rect bestNode = {};

	bestAreaFit = std::numeric_limits<int>::max();
	bestShortSideFit = std::numeric_limits<int>::max();

	for (size_t i = 0; i < freeRectangles.size(); ++i)
	{
		int areaFit = freeRectangles[i].width * freeRectangles[i].height - width * height;

		// Try to place the rectangle in upright (non-flipped) orientation.
		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int leftoverHoriz = abs(freeRectangles[i].width - width);
			int leftoverVert = abs(freeRectangles[i].height - height);
			int shortSideFit = std::min(leftoverHoriz, leftoverVert);

			if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestShortSideFit = shortSideFit;
				bestAreaFit = areaFit;
			}
		}

		if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
		{
			int leftoverHoriz = abs(freeRectangles[i].width - height);
			int leftoverVert = abs(freeRectangles[i].height - width);
			int shortSideFit = std::min(leftoverHoriz, leftoverVert);

			if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = height;
				bestNode.height = width;
				bestShortSideFit = shortSideFit;
				bestAreaFit = areaFit;
			}
		}
	}
	return bestNode;
}

// / Returns 0 if the two intervals i1 and i2 are disjoint, or the length of their overlap otherwise.
int CommonIntervalLength(int i1start, int i1end, int i2start, int i2end)
{
	if (i1end < i2start || i2end < i1start)
		return 0;
	return std::min(i1end, i2end) - std::max(i1start, i2start);
}

int MaxRectsBinPack::ContactPointScoreNode(int x, int y, int width, int height) const
{
	int score = 0;

	if (x == 0 || x + width == binWidth)
		score += height;
	if (y == 0 || y + height == binHeight)
		score += width;

	for (size_t i = 0; i < usedRectangles.size(); ++i)
	{
		if (usedRectangles[i].x == x + width || usedRectangles[i].x + usedRectangles[i].width == x)
			score += CommonIntervalLength(usedRectangles[i].y, usedRectangles[i].y + usedRectangles[i].height, y, y + height);
		if (usedRectangles[i].y == y + height || usedRectangles[i].y + usedRectangles[i].height == y)
			score += CommonIntervalLength(usedRectangles[i].x, usedRectangles[i].x + usedRectangles[i].width, x, x + width);
	}
	return score;
}

Rect MaxRectsBinPack::FindPositionForNewNodeContactPoint(int width, int height, int &bestContactScore) const
{
	Rect bestNode = {};

	bestContactScore = -1;

	for (size_t i = 0; i < freeRectangles.size(); ++i)
	{
		// Try to place the rectangle in upright (non-flipped) orientation.
		if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
		{
			int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, width, height);
			if (score > bestContactScore)
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestContactScore = score;
			}
		}
		if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
		{
			int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, height, width);
			if (score > bestContactScore)
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = height;
				bestNode.height = width;
				bestContactScore = score;
			}
		}
	}
	return bestNode;
}