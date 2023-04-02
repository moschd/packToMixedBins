#ifndef SPIRAL_PACKER_H
#define SPIRAL_PACKER_H

/**
 * @brief Algorithm to pack items in a spiral pattern.
 *
 */
class SpiralPacker
{
private:
    int binWidth_;
    int binHeight_;
    int itemWidth_;
    int itemHeight_;
    bool allowRotation_;
    std::array<std::array<int, 4>, 4> spiralPackFreeSquares_;

    void initialize()
    {
        if (allowRotation_ && itemWidth_ > itemHeight_)
        {
            std::swap(itemWidth_, itemHeight_);
        }

        std::array<int, 4> squareOne = {0, 0, binWidth_ - itemHeight_, itemHeight_};
        std::array<int, 4> squareTwo = {0, itemHeight_, itemHeight_, binHeight_};
        std::array<int, 4> squareThree = {itemHeight_, binHeight_ - itemHeight_, binWidth_, binHeight_};
        std::array<int, 4> squareFour = {binWidth_ - itemHeight_, 0, binWidth_, binHeight_ - itemHeight_};

        SpiralPacker::spiralPackFreeSquares_ = {squareOne, squareTwo, squareThree, squareFour};
    }

public:
    ///		   Construct 4 squares, with start xy, end xy.
    ///		   These 4 squares will then be packed one-by-one, forming a spiral packing pattern.
    /// 	   Incase the width is greater than the height, the dimensions are swapped.
    ///	       The longest dimension should be the height, this is the attribute that prevents item intersection.
    SpiralPacker(int aBinWidth,
                 int aBinHeight,
                 int aItemWidth,
                 int aItemHeight,
                 bool aAllowRotation) : binWidth_(aBinWidth),
                                        binHeight_(aBinHeight),
                                        itemWidth_(aItemWidth),
                                        itemHeight_(aItemHeight),
                                        allowRotation_(aAllowRotation)

    {
        SpiralPacker::initialize();
    }

    /**
     * @brief Pack the rectangle in a fitting square.
     * The following values are stored inside the arrays, representing the open space inside a square.
     * spiralPackFreeSquares_[square][0] - x start
     * spiralPackFreeSquares_[square][1] - y Start
     * spiralPackFreeSquares_[square][2] - x End
     * spiralPackFreeSquares_[square][3] - y End
     *
     * @return Rect
     */
    Rect insert()
    {
        Rect newRect = {};

        // spiralPacking is not safe due to bin/item dimensions.
        if ((itemHeight_ * 2) < binWidth_)
        {
            return newRect;
        }

        int square = 0;
        while (newRect.width == 0 && square < 4)
        {
            newRect.x = SpiralPacker::spiralPackFreeSquares_[square][0];
            newRect.y = SpiralPacker::spiralPackFreeSquares_[square][1];

            if (!(square & 1))
            {
                if (SpiralPacker::spiralPackFreeSquares_[square][0] + itemWidth_ < SpiralPacker::spiralPackFreeSquares_[square][2])
                {
                    newRect.width = itemWidth_;
                    newRect.height = itemHeight_;
                    SpiralPacker::spiralPackFreeSquares_[square][0] += itemWidth_;
                    break;
                }
            }
            else
            {
                if (SpiralPacker::spiralPackFreeSquares_[square][1] + itemWidth_ < SpiralPacker::spiralPackFreeSquares_[square][3])
                {
                    // Square 3 is special since we need to subtract from the end position instead of adding to the start.
                    // This causes the items to be connected instead of placing the item at the end of the bin.
                    if (square == 3)
                    {

                        // Get the distance left over from square 0, subtract it to make the items connect with the last item from square 0.
                        const int spaceLeftOverAtRightSideOfSquareOne = (SpiralPacker::spiralPackFreeSquares_[0][2] - SpiralPacker::spiralPackFreeSquares_[0][0]);
                        newRect.x = (SpiralPacker::spiralPackFreeSquares_[square][2] - itemHeight_) - spaceLeftOverAtRightSideOfSquareOne;

                        // Get the distance left over after packing the max amount of items, and add it to y coordinate.
                        // This causes the item to be connected to the last item from the top right square.
                        newRect.y += (SpiralPacker::spiralPackFreeSquares_[2][1] % itemWidth_);
                    }

                    newRect.width = itemHeight_;
                    newRect.height = itemWidth_;
                    SpiralPacker::spiralPackFreeSquares_[square][1] += itemWidth_;

                    break;
                }
            }
            square += 1;
        };
        return newRect;
    };

    /// Reset the algorithm.
    void reset()
    {
        SpiralPacker::initialize();
    };
};
#endif