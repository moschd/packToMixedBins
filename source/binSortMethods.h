#ifndef BIN_SORT_H
#define BIN_SORT_H

struct binVolumeUtilSorter
{
  inline const bool operator()(const Bin &one, const Bin &two) const
  {
    return (one.getActVolumeUtilPercentage() < two.getActVolumeUtilPercentage());
  }
};

struct binWeightUtilSorter
{
  inline const bool operator()(const Bin &one, const Bin &two) const
  {
    return (one.getActWeightUtilPercentage() < two.getActWeightUtilPercentage());
  }
};

struct binCreationSorter
{
  inline const bool operator()(const Bin &one, const Bin &two) const
  {
    return (one.id_ < two.id_);
  }
};
#endif