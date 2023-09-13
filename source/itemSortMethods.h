#ifndef ITEM_SORT_H
#define ITEM_SORT_H

struct consKeyAndVolumeSorter
{
    inline const bool operator()(const std::shared_ptr<Item> one, const std::shared_ptr<Item> two) const
    {
        return one->Item::itemConsolidationKey_ < two->Item::itemConsolidationKey_ ||
               (one->Item::itemConsolidationKey_ == two->Item::itemConsolidationKey_ &&
                one->Item::volume_ > two->Item::volume_);
    }
};

struct consKeyAndWeightSorter
{
    inline const bool operator()(const std::shared_ptr<Item> one, const std::shared_ptr<Item> two) const
    {
        return one->Item::itemConsolidationKey_ < two->Item::itemConsolidationKey_ ||
               (one->Item::itemConsolidationKey_ == two->Item::itemConsolidationKey_ &&
                one->Item::weight_ > two->Item::weight_);
    }
};

// struct consKeySorter
// {
//     inline const bool operator()(const std::shared_ptr<Item> one, const std::shared_ptr<Item> two) const
//     {
//         return one->Item::itemConsolidationKey_ < two->Item::itemConsolidationKey_;
//     }
// };

// struct weightSorter
// {
//     inline const bool operator()(const std::shared_ptr<Item> one, const std::shared_ptr<Item> two) const
//     {
//         return one->Item::weight_ < two->Item::weight_;
//     }
// };

struct volumeSorter
{
    inline const bool operator()(const std::shared_ptr<Item> one, const std::shared_ptr<Item> two) const
    {
        return one->Item::volume_ > two->Item::volume_;
    }
};

#endif