/// Specifies the different heuristic rules that will be iterated over when deciding where to place a new rectangle.
enum HeuristicAlgorithmType
{
    RectBestShortSideFit, ///< -BSSF: Positions the rectangle against the short side of a free rectangle into which it fits the best.
    RectBestLongSideFit,  ///< -BLSF: Positions the rectangle against the long side of a free rectangle into which it fits the best.
    RectBestAreaFit,      ///< -BAF: Positions the rectangle into the smallest free rect into which it fits.
    RectBottomLeftRule,   ///< -BL: Does the Tetris placement.
    RectContactPointRule, ///< -CP: Chooses the placement where the rectangle touches other rects as much as possible.
    SpiralFitPacker,      ///< -Spiral packing pattern.
    NeatGroupFitPacker,   ///< -Neat fitting packing pattern.
    Last
};