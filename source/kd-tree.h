#ifndef KD_TREE_H
#define KD_TREE_H

struct Node
{
    bool isLeaf_;
    Node *left_;
    Node *right_;
    unsigned int myDepth_;
    std::vector<int> myChildren_;
    std::array<double, 3> partitionPoint_;
    std::array<double, 3> minSearchDimensions_;
    std::array<double, 3> maxSearchDimensions_;

    Node(const std::array<double, 3> aPartitionPoint,
         const std::array<double, 3> aMins,
         const std::array<double, 3> aMaxs,
         const int aCurrentDepth) : isLeaf_(false),
                                    left_(NULL),
                                    right_(NULL),
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
inline Node *genNodex(const std::array<double, 3> aPartitionPoint,
                      const std::array<double, 3> aMins,
                      const std::array<double, 3> aMaxs,
                      const int aCurrentDepth)
{
    return new Node(aPartitionPoint, aMins, aMaxs, aCurrentDepth);
};

class KdTree
{
private:
    int maxDepth_;
    Node *treeRoot_;
    std::array<double, 3> minDimensions_;
    std::array<double, 3> maxDimensions_;

    /**
     * @brief Calculates the depth of the tree that will be generated.
     *
     * This method calculates what would be the desired depth of the tree.
     * This attempts to make the algorithm more efficient by scaling the tree depending of the estimated number of items that will fit in the bin.
     *
     * @param aEstimatedNumberOfItemFits - integer indicating the estimated number of items that will be in the bin when packing has finished.
     */
    void calculateMaxDepth(unsigned int aEstimatedNumberOfItemFits)
    {
        KdTree::maxDepth_ = ceil(sqrt(aEstimatedNumberOfItemFits / 125) + 1);
    };

    /**
     * @brief Pre generate a fixed depth balanced kd-tree.
     *
     * This method creates a kd-tree of a certain depth.
     * The nodes in this tree each contain cartesian coordinates indicating a 3R point in a space.
     * Used for 3R space partitioning of the bin to be packed.
     *
     * @param aRoot             - root of the tree
     * @param aDepth            - the current depth of the tree
     * @param aPartitionPoint   - the 3R point that the node will be representating and splitting.
     * @param aMins             - 3R point marking the minimum border of the search area point for which this node will be responsable.
     * @param aMaxs             - 3R point marking the maximum border of the search area point for which this node will be responsable.
     * @param aRequestedDepth   - the requested maximum depth of the tree to be generated.
     */
    void generateTree(Node *aRoot,
                      const unsigned int aDepth,
                      const std::array<double, 3> aPartitionPoint,
                      const std::array<double, 3> aMins,
                      const std::array<double, 3> aMaxs,
                      const unsigned int aRequestedDepth)
    {
        const int unsigned axis = aDepth % constants::R;
        const int unsigned previousAxis = (aDepth == 0 ? aDepth : (aDepth - 1) % constants::R);

        std::array<double, 3> newPartitionPoint = aPartitionPoint;
        newPartitionPoint[previousAxis] = (aMins[previousAxis] + aMaxs[previousAxis]) / 2;

        if (aDepth > aRequestedDepth)
        {
            aRoot->Node::isLeaf_ = 1;
            aRoot->Node::partitionPoint_ = newPartitionPoint;
            return;
        };

        aRoot->Node::left_ = genNodex(newPartitionPoint, aMins, aMaxs, aDepth);
        aRoot->Node::right_ = genNodex(newPartitionPoint, aMins, aMaxs, aDepth);

        std::array<double, 3> leftMaxs = aMaxs;
        leftMaxs[axis] = newPartitionPoint[axis];
        generateTree(aRoot->Node::left_, aDepth + 1, newPartitionPoint, aMins, leftMaxs, aRequestedDepth);

        std::array<double, 3> rightMins = aMins;
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
    void addItemKeyToLeaf(Node *aRoot, const unsigned int aItemKey, const unsigned int aDepth, const std::array<double, 3> aItemMaxPosition)
    {
        if (aRoot->Node::isLeaf_)
        {
            aRoot->Node::myChildren_.push_back(aItemKey);
            return;
        };

        const unsigned int axis = aDepth % constants::R;
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
     * @brief Removes an item key from the tree.
     *
     * TODO implement proper searching, dont need to search all leaves...
     *
     * @param aRoot         - Node from where search starts.
     * @param aItemKey      - itemKey to be removed.
     */
    void removeKeyFromLeaf(Node *aRoot, const unsigned int aItemKey) const
    {
        if (aRoot == NULL)
        {
            return;
        };

        if (!aRoot->Node::myChildren_.empty())
        {
            std::vector<int>::iterator itemKeyIterator = std::find(aRoot->Node::myChildren_.begin(), aRoot->Node::myChildren_.end(), aItemKey);
            if (itemKeyIterator != aRoot->Node::myChildren_.end())
            {
                aRoot->Node::myChildren_.erase(itemKeyIterator);
            };
        }
        KdTree::removeKeyFromLeaf(aRoot->Node::left_, aItemKey);
        KdTree::removeKeyFromLeaf(aRoot->Node::right_, aItemKey);
    };

    /**
     * @brief Print tree to console..
     *
     * @param aRoot         - Node whose children will be printed.
     */
    void printTreeImp(const Node *aRoot) const
    {
        if (aRoot == NULL)
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

    /**
     * @brief Delete all nodes in the tree to free memory.
     *
     * This method deletes all nodes under the provided aRoot.
     *
     * @param aRoot - Node under which all underlying nodes will be deleted.
     */
    void deleteAllNodes(Node *node)
    {
        if (!node)
        {
            return;
        };

        KdTree::deleteAllNodes(node->Node::left_);
        KdTree::deleteAllNodes(node->Node::right_);

        delete node;
        node = nullptr;
    };

public:
    KdTree(unsigned int aEstimatedNumberOfItemFits, std::array<double, 3> aMaxDimensions) : minDimensions_(constants::START_POSITION),
                                                                                            maxDimensions_(aMaxDimensions)
    {
        KdTree::calculateMaxDepth(aEstimatedNumberOfItemFits);
        KdTree::treeRoot_ = genNodex({KdTree::maxDimensions_[constants::axis::WIDTH] / 2,
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
    const Node *getRoot() const
    {
        return KdTree::treeRoot_;
    };

    /**
     * @brief Helper method to generate a new tree.
     *
     * This method provides a simple interface to the method that generates the kd-tree.
     *
     * @param aRoot - Node from which the tree will grow.
     */
    void generateTreeHelper(Node *aRoot)
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
    void addItemKeyToLeafHelper(const int aItemKey, const std::array<double, 3> aItemMaxPosition)
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
    void getIntersectCandidates(const Node *aRoot,
                                const unsigned int aDepth,
                                const std::array<double, 3> &aStartPoint,
                                const std::array<double, 3> aMaxSearchPoint,
                                std::vector<int> &aPassedNodes) const
    {
        if (aRoot->Node::isLeaf_)
        {
            aPassedNodes.insert(aPassedNodes.end(),
                                aRoot->Node::myChildren_.begin(),
                                aRoot->Node::myChildren_.end());
            return;
        };

        const unsigned int axis = aDepth % constants::R;
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
    void printTreeImpHelper() const
    {
        KdTree::printTreeImp(KdTree::treeRoot_);
    };

    /**
     * @brief Function to delete all dynamically allocated nodes in the tree.
     *
     */
    void deleteAllNodesHelper()
    {
        KdTree::deleteAllNodes(KdTree::treeRoot_);
    }

    /**
     * @brief Removes an itemKey from the tree.
     *
     * @param aItemKey
     */
    void removeKeyFromLeafHelper(const int aItemKey)
    {
        KdTree::removeKeyFromLeaf(KdTree::treeRoot_, aItemKey);
    }
};

#endif
