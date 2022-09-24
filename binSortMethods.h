#ifndef BIN_SORT_H
#define BIN_SORT_H

struct binVolumeUtilSorter
{
  inline const bool operator()(const Bin &one, const Bin &two) const
  {
    return (one.getActVolumeUtilizationPercentage() < two.getActVolumeUtilizationPercentage());
  }
};

struct binWeightUtilSorter
{
  inline const bool operator()(const Bin &one, const Bin &two) const
  {
    return (one.getActWeightUtilizationPercentage() < two.getActWeightUtilizationPercentage());
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