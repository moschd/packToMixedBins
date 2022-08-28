#ifndef MISCFUNCTIONS_H
#define MISCFUNCTIONS_H

struct consKeyAndVolumeSorter
{
  inline bool operator()(const Item &one, const Item &two)
  {
    return one.Item::itemConsolidationKey_ < two.Item::itemConsolidationKey_ ||
           (one.Item::itemConsolidationKey_ == two.Item::itemConsolidationKey_ &&
            one.Item::maxVolume_ > two.Item::maxVolume_);
  }
};

struct consKeyAndWeightSorter
{
  inline bool operator()(const Item &one, const Item &two)
  {
    return one.Item::itemConsolidationKey_ < two.Item::itemConsolidationKey_ ||
           (one.Item::itemConsolidationKey_ == two.Item::itemConsolidationKey_ &&
            one.Item::weight_ > two.Item::weight_);
  }
};

#endif