#ifndef KD_TREE_H
#define KD_TREE_H

struct Node
{
    bool isLeaf_;
    std::shared_ptr<Node> left_;
    std::shared_ptr<Node> right_;
    int myDepth_;
    std::vector<int> myChildren_;
    std::array<int, 3> partitionPoint_;
    std::array<int, 3> minSearchDimensions_;
    std::array<int, 3> maxSearchDimensions_;

    Node(const std::array<int, 3> aPartitionPoint,
         const std::array<int, 3> aMins,
         const std::array<int, 3> aMaxs,
         const int aCurrentDepth) : isLeaf_(false),
                                    left_(std::shared_ptr<Node>(nullptr)),
                                    right_(std::shared_ptr<Node>(nullptr)),
                                    myDepth_(aCurrentDepth),
                                    minSearchDimensions_(aMins),
                                    maxSearchDimensions_(aMaxs),
                                    partitionPoint_(aPartitionPoint)
    {
        Node::myChildren_.reserve(125);
    }
};

/**
 * @brief Spawn a Node.
 *
 * This method creates a Node used as node in the kd-tree.
 * Each node represents a point in 3R.
 *
 * @param aPartitionPoint   - the 3R point that the node will be representating and splitting.
 * @param aMins             - 3R point marking the minimum border of the search area point for which this node will be responsable.
 * @param aMaxs             - 3R point marking the maximum border of the search area point for which this node will be responsable.
 * @param aCurrentDepth     - the tree depth on which this node is created.
 */
inline std::shared_ptr<Node> generateNode(const std::array<int, 3> aPartitionPoint,
                                          const std::array<int, 3> aMins,
                                          const std::array<int, 3> aMaxs,
                                          const int aCurrentDepth)
{
    return std::make_shared<Node>(aPartitionPoint, aMins, aMaxs, aCurrentDepth);
};

/**
 * @brief Represents the bin in tree form.
 *
 * Contains all items currently inside the bin.
 * The items are stored by their furthest possible point in space.
 * This way, when inputting the smallest possible point of an item, we can find all items which are relevant to this particular item.
 * We can then check for intersection, gravity, stackingStyle restrictions etc...
 *
 * This allows for efficient 3D search and improves performance of the algorithm by a huge amount.
 *
 */
class KdTree
{
private:
    int maxDepth_;
    std::shared_ptr<Node> treeRoot_;
    std::array<int, 3> minDimensions_;
    std::array<int, 3> maxDimensions_;

    /**
     * @brief Calculates the depth of the tree that will be generated.
     *
     * This method calculates what would be the desired depth of the tree.
     * This attempts to make the algorithm more efficient by scaling the tree depending of the estimated number of items that will fit in the bin.
     *
     * @param aEstimatedNumberOfItemFits - integer indicating the estimated number of items that will be in the bin when packing has finished.
     */
    void calculateMaxDepth(int aEstimatedNumberOfItemFits) { KdTree::maxDepth_ = ceil(sqrt(aEstimatedNumberOfItemFits / 125) + 1); };

    /**
     * @brief Pre generate a fixed depth balanced kd-tree.
     *
     * This method creates a kd-tree of a certain depth.
     * The nodes in this tree each contain cartesian coordinates indicating a 3R point in a space.
     * Used for 3R space partitioning of the bin to be packed.
     *
     * @param aRoot             - root of the tree
     * @param aDepth            - the current depth of the tree
     * @param aPartitionPoint   - the 3R point that the node will be representing, and splitting.
     * @param aMins             - 3R point marking the minimum border of the search area for which this node will be responsable.
     * @param aMaxs             - 3R point marking the maximum border of the search area for which this node will be responsable.
     * @param aRequestedDepth   - the requested maximum depth of the tree to be generated.
     */
    void generateTree(std::shared_ptr<Node> aRoot,
                      const int aDepth,
                      const std::array<int, 3> aPartitionPoint,
                      const std::array<int, 3> aMins,
                      const std::array<int, 3> aMaxs,
                      const int aRequestedDepth)
    {
        const int axis = aDepth % constants::R;
        const int previousAxis = (aDepth == 0 ? aDepth : (aDepth - 1) % constants::R);

        std::array<int, 3> newPartitionPoint = aPartitionPoint;
        newPartitionPoint[previousAxis] = (aMins[previousAxis] + aMaxs[previousAxis]) / 2;

        if (aDepth > aRequestedDepth)
        {
            aRoot->Node::isLeaf_ = 1;
            aRoot->Node::partitionPoint_ = newPartitionPoint;
            return;
        };

        aRoot->Node::left_ = generateNode(newPartitionPoint, aMins, aMaxs, aDepth);
        aRoot->Node::right_ = generateNode(newPartitionPoint, aMins, aMaxs, aDepth);

        std::array<int, 3> leftMaxs = aMaxs;
        leftMaxs[axis] = newPartitionPoint[axis];
        generateTree(aRoot->left_, aDepth + 1, newPartitionPoint, aMins, leftMaxs, aRequestedDepth);

        std::array<int, 3> rightMins = aMins;
        rightMins[axis] = newPartitionPoint[axis];
        generateTree(aRoot->Node::right_, aDepth + 1, newPartitionPoint, rightMins, aMaxs, aRequestedDepth);
    }

    /**
     * @brief Find correct place to add itemKey in tree and add it.
     *
     * This method adds a itemKey to the corresponding tree leaf.
     * Each bin to be packed uses a kd-tree for space partitioning and organizing the multidimensional data (items) that it contains.
     *
     * @param aRoot             - Node from which to start searching, normally start at the aRoot of the tree.
     * @param aItemKey          - The key of the item that needs to be placed in the tree.
     * @param aDepth            - The current depth of the tree.
     * @param aItemMaxPosition  - The furthest point in space that the item reaches, ie top right corner.
     */
    void addItemKeyToLeaf(std::shared_ptr<Node> aRoot, const int aItemKey, const int aDepth, const std::array<int, 3> aItemMaxPosition)
    {
        if (aRoot->Node::isLeaf_)
        {
            aRoot->Node::myChildren_.push_back(aItemKey);
            return;
        };

        const int axis = aDepth % constants::R;
        if (aItemMaxPosition[axis] < aRoot->Node::partitionPoint_[axis])
        {
            addItemKeyToLeaf(aRoot->Node::left_,
                             aItemKey,
                             aDepth + 1,
                             aItemMaxPosition);
        }
        else
        {
            addItemKeyToLeaf(aRoot->Node::right_,
                             aItemKey,
                             aDepth + 1,
                             aItemMaxPosition);
        };
    };

    /**
     * @brief Print tree to console..
     *
     * @param aRoot         - Node whose children will be printed.
     */
    void printTreeImp(const std::shared_ptr<Node> aRoot) const
    {
        if (aRoot == nullptr)
        {
            return;
        };

        if (!aRoot->Node::myChildren_.empty())
        {
            std::cout << "LEAF "
                      << aRoot->Node::partitionPoint_[0] << " "
                      << aRoot->Node::partitionPoint_[1] << " "
                      << aRoot->Node::partitionPoint_[2] << "\n";

            std::cout << "  CHILDREN:\n";
            std::cout << "  ";
            for (auto d : aRoot->Node::myChildren_)
            {
                std::cout << d << " ";
            }
            std::cout << "\n";
        }

        KdTree::printTreeImp(aRoot->Node::left_);
        KdTree::printTreeImp(aRoot->Node::right_);
    };

public:
    KdTree(int aEstimatedNumberOfItemFits,
           std::array<int, 3> aMaxDimensions) : minDimensions_(constants::START_POSITION),
                                                maxDimensions_(aMaxDimensions),
                                                maxDepth_(8)

    {
        KdTree::calculateMaxDepth(aEstimatedNumberOfItemFits);
        KdTree::treeRoot_ = generateNode({KdTree::maxDimensions_[constants::axis::WIDTH] / 2,
                                          KdTree::maxDimensions_[constants::axis::DEPTH] / 2,
                                          KdTree::maxDimensions_[constants::axis::HEIGHT] / 2},
                                         KdTree::minDimensions_,
                                         KdTree::maxDimensions_,
                                         0);

        KdTree::generateTreeHelper(KdTree::treeRoot_);
    };

    /**
     * @brief Return the root of the tree.
     *
     */
    const std::shared_ptr<Node> &getRoot() const { return KdTree::treeRoot_; };

    /**
     * @brief Helper method to generate a new tree.
     *
     * This method provides a simple interface to the method that generates the kd-tree.
     *
     * @param aRoot - Node from which the tree will grow.
     */
    void generateTreeHelper(std::shared_ptr<Node> aRoot)
    {
        KdTree::generateTree(aRoot,
                             aRoot->Node::myDepth_,
                             aRoot->Node::partitionPoint_,
                             aRoot->Node::minSearchDimensions_,
                             aRoot->Node::maxSearchDimensions_,
                             maxDepth_);
    };

    /**
     * @brief Helper function to add a new itemKey to the tree.
     *
     * This method provides a simple interface to the method that adds a item to the tree.
     *
     * @param aItemKey          - The itemKey which will be added to the tree.
     * @param aItemMaxPosition  - The top right corner of the item, this is the position which will be used to search for the correct place to add the item.
     */
    void addItemKeyToLeafHelper(const int aItemKey, const std::array<int, 3> aItemMaxPosition)
    {
        KdTree::addItemKeyToLeaf(KdTree::treeRoot_,
                                 aItemKey,
                                 0,
                                 aItemMaxPosition);
    }

    /**
     * @brief Adds itemKeys of items which might be intersecting with the item on the provided position.
     *
     * This method adds the itemKey of items which might be intersecting with the item on the provided position to a provided vector.
     *
     * @param aRoot             - Node from which to start searching, normally start at the aRoot of the tree.
     * @param aDepth            - The current depth of the tree
     * @param aStartPoint       - The 3R point from which to start searching.
     * @param aMaxSearchPoint   - The furthest point in space that an item can be in order to still be considered a intersection candidate.
     * @param aPassedNodes      - The vector to which itemKeys will be added.
     */
    void getIntersectCandidates(const std::shared_ptr<Node> aRoot,
                                const int aDepth,
                                const std::array<int, 3> &aStartPoint,
                                const std::array<int, 3> aMaxSearchPoint,
                                std::vector<int> &aPassedNodes) const
    {
        if (aRoot->Node::isLeaf_)
        {
            aPassedNodes.insert(aPassedNodes.end(),
                                aRoot->Node::myChildren_.begin(),
                                aRoot->Node::myChildren_.end());
            return;
        };

        const int axis = aDepth % constants::R;

        if (aStartPoint[axis] < aRoot->Node::partitionPoint_[axis])
        {
            getIntersectCandidates(aRoot->Node::left_,
                                   aDepth + 1,
                                   aStartPoint,
                                   aMaxSearchPoint,
                                   aPassedNodes);
        };

        if (aRoot->Node::partitionPoint_[axis] < (aStartPoint[axis] + aMaxSearchPoint[axis]))
        {
            getIntersectCandidates(aRoot->Node::right_,
                                   aDepth + 1,
                                   aStartPoint,
                                   aMaxSearchPoint,
                                   aPassedNodes);
        };
    };

    /**
     * @brief Helper method to print the tree to the console.
     *
     */
    void printTreeImpHelper() const { KdTree::printTreeImp(KdTree::treeRoot_); };
};

#endif
