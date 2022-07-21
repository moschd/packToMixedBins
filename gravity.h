/*
    GRAVITY.
        Handles logic related to gravity.

    - Boxes being stacked in the air can happen since depth stacking is checked on all items before height stacking.
    - Boxes resting partly on a box.

    Prevent item floatation by returning an invalid position response when no item is under the position being evaluated.
*/

class Gravity
{
public:
    bool gravityEnabled_;
    double gravityStrengthPercentage_;

    Gravity()
    {
        gravityStrengthPercentage_ = 0.0;
    }

    Gravity(double aGravityStrengthPercentage)
    {
        gravityStrengthPercentage_ = aGravityStrengthPercentage - 0.1;
        gravityEnabled_ = (aGravityStrengthPercentage > 0.0 ? 0 : 1);
    };
};