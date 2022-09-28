#ifndef MISCFUNCTIONS_H
#define MISCFUNCTIONS_H

struct consKeyAndVolumeSorter
{
  inline const bool operator()(const Item &one, const Item &two) const
  {
    return one.Item::itemConsolidationKey_ < two.Item::itemConsolidationKey_ ||
           (one.Item::itemConsolidationKey_ == two.Item::itemConsolidationKey_ &&
            one.Item::maxVolume_ > two.Item::maxVolume_);
  }
};

struct consKeyAndWeightSorter
{
  inline const bool operator()(const Item &one, const Item &two) const
  {
    return one.Item::itemConsolidationKey_ < two.Item::itemConsolidationKey_ ||
           (one.Item::itemConsolidationKey_ == two.Item::itemConsolidationKey_ &&
            one.Item::weight_ > two.Item::weight_);
  }
};

#endif