#include <bits/stdc++.h>
#include "constants.h"

struct Nodex
{
    int id;
    bool isLeaf_;
    int myDepth_;
    int leafNr_;
    Nodex *left_;
    Nodex *right_;
    std::vector<int> myChildren_;
    std::array<double, 3> partitionPoint_;
    std::array<double, 3> minSearchDimensions_;
    std::array<double, 3> maxSearchDimensions_;
};

/**
 * @brief Spawn a Nodex.
 *
 * This method creates a Nodex used as node in the kd-tree.
 * Each node represents a point in 3R.
 *
 * @param aPartitionPoint   - the 3R point that the node will be representating and splitting.
 * @param aMins             - 3R point marking the minimum border of the search area point for which this node will be responsable.
 * @param aMaxs             - 3R point marking the maximum border of the search area point for which this node will be responsable.
 * @param aCurrentDepth     - the tree depth on which this node is created.
 */
Nodex *genNodex(std::array<double, 3> aPartitionPoint, std::array<double, 3> aMins, std::array<double, 3> aMaxs, int aCurrentDepth)
{
    Nodex *branch = new Nodex;
    branch->id = 0;
    branch->isLeaf_ = 0;
    branch->myDepth_ = aCurrentDepth;
    branch->minSearchDimensions_ = aMins;
    branch->maxSearchDimensions_ = aMaxs;
    branch->left_ = NULL;
    branch->right_ = NULL;
    branch->partitionPoint_ = aPartitionPoint;
    return branch;
};

class KdTree
{
private:
    Nodex *treeRoot_;
    int maxDepth_;
    std::array<double, 3> minDimensions_;
    std::array<double, 3> maxDimensions_;
    int nrOfLeaves_;

public:
    KdTree(int aEstimatedNumberOfItemFits, std::array<double, 3> aMaxDimensions)
    {
        calculateMaxDepth(aEstimatedNumberOfItemFits);
        minDimensions_ = {0, 0, 0};
        maxDimensions_ = aMaxDimensions;

        treeRoot_ = genNodex({maxDimensions_[0] / 2, maxDimensions_[1] / 2, maxDimensions_[2] / 2}, minDimensions_, maxDimensions_, 0);
        generateTreeHelper(treeRoot_);
    };

    /**
     * @brief Return the root of the tree.
     *
     */

    Nodex *getRoot()
    {
        return treeRoot_;
    };

    /**
     * @brief Calculates the depth of the tree that will be generated.
     *
     * This method calculates what would be the desired depth of the tree.
     * This attempts to make the algorithm more efficient by scaling the tree depending of the estimated number of items that will fit in the bin.
     *
     * @param aEstimatedNumberOfItemFits - integer indicating the estimated number of items that will be in the bin when packing has finished.
     */
    void calculateMaxDepth(int aEstimatedNumberOfItemFits)
    {
        maxDepth_ = ceil(sqrt(aEstimatedNumberOfItemFits / 125) + 1);
    };

    /**
     * @brief Helper method to generate a new tree.
     *
     * This method provides a simple interface to the method that generates the kd-tree.
     *
     * @param aRoot - Node from which the tree will grow.
     */
    void generateTreeHelper(Nodex *aRoot)
    {
        generateTree(aRoot, aRoot->myDepth_, aRoot->partitionPoint_, aRoot->minSearchDimensions_, aRoot->maxSearchDimensions_, maxDepth_);
    };

    /**
     * @brief Helper method to print the tree to the console.
     *
     */
    void printTreeImpHelper()
    {
        printTreeImp(treeRoot_, "X");
    };

    /**
     * @brief Helper function to add a new itemKey to the tree.
     *
     * This method provides a simple interface to the method that adds a item to the tree.
     *
     * @param aItemKey          - The itemKey which will be added to the tree.
     * @param aItemMaxPosition  - The top right corner of the item, this is the position which will be used to search for the correct place to add the item.
     */
    void addItemKeyToLeafHelper(int aItemKey, std::array<double, 3> aItemMaxPosition)
    {
        addItemKeyToLeaf(treeRoot_, aItemKey, 0, aItemMaxPosition);
    }

    void deleteAllNodesHelper()
    {
        deleteAllNodes(treeRoot_);
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
    void addItemKeyToLeaf(Nodex *aRoot, int aItemKey, int aDepth, std::array<double, 3> aItemMaxPosition)
    {
        if (aRoot->isLeaf_)
        {
            aRoot->myChildren_.push_back(aItemKey);
            return;
        };

        int axis = aDepth % constants::R;
        if (aItemMaxPosition[axis] < aRoot->partitionPoint_[axis])
        {
            addItemKeyToLeaf(aRoot->left_, aItemKey, aDepth + 1, aItemMaxPosition);
        }
        else
        {
            addItemKeyToLeaf(aRoot->right_, aItemKey, aDepth + 1, aItemMaxPosition);
        };
    };

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
    void getPotentialIntersectingItemKeys(Nodex *aRoot, int aDepth, std::array<double, 3> &aStartPoint, std::array<double, 3> aMaxSearchPoint, std::vector<int> &aPassedNodes)
    {
        if (aRoot == NULL)
        {
            return;
        };

        if (aRoot->isLeaf_)
        {
            aPassedNodes.insert(aPassedNodes.end(), aRoot->myChildren_.begin(), aRoot->myChildren_.end());
            return;
        };

        int axis = aDepth % constants::R;

        if (aStartPoint[axis] < aRoot->partitionPoint_[axis])
        {
            getPotentialIntersectingItemKeys(aRoot->left_, aDepth + 1, aStartPoint, aMaxSearchPoint, aPassedNodes);
        };

        if (aRoot->partitionPoint_[axis] < (aStartPoint[axis] + aMaxSearchPoint[axis]))
        {
            getPotentialIntersectingItemKeys(aRoot->right_, aDepth + 1, aStartPoint, aMaxSearchPoint, aPassedNodes);
        };
    };

    /**
     * @brief Pre generate a fixed depth balanced kd-tree.
     *
     * This method creates a kd-tree of a certain depth.
     * The nodes in this tree each contain cartesian coordinates indicating a 3R point in a space.
     * Used for 3R space partisioning of the bin to be packed.
     *
     * @param aRoot             - root of the tree
     * @param aDepth            - the current depth of the tree
     * @param aPartitionPoint   - the 3R point that the node will be representating and splitting.
     * @param aMins             - 3R point marking the minimum border of the search area point for which this node will be responsable.
     * @param aMaxs             - 3R point marking the maximum border of the search area point for which this node will be responsable.
     * @param aRequestedDepth   - the requested maximum depth of the tree to be generated.
     */
    void generateTree(Nodex *aRoot, int aDepth, std::array<double, 3> aPartitionPoint, std::array<double, 3> aMins, std::array<double, 3> aMaxs, int aRequestedDepth)
    {
        int axis = aDepth % constants::R;
        int previousAxis = (aDepth == 0 ? aDepth : (aDepth - 1) % constants::R);

        std::array<double, 3> newPartitionPoint = aPartitionPoint;
        newPartitionPoint[previousAxis] = (aMins[previousAxis] + aMaxs[previousAxis]) / 2;

        if (aDepth > aRequestedDepth)
        {
            nrOfLeaves_ += 1;
            aRoot->isLeaf_ = 1;
            aRoot->leafNr_ = nrOfLeaves_;
            aRoot->partitionPoint_ = newPartitionPoint;
            return;
        };

        aRoot->left_ = genNodex(newPartitionPoint, aMins, aMaxs, aDepth);
        aRoot->right_ = genNodex(newPartitionPoint, aMins, aMaxs, aDepth);

        std::array<double, 3> leftMaxs = aMaxs;
        leftMaxs[axis] = newPartitionPoint[axis];
        generateTree(aRoot->left_, aDepth + 1, newPartitionPoint, aMins, leftMaxs, aRequestedDepth);

        std::array<double, 3> rightMins = aMins;
        rightMins[axis] = newPartitionPoint[axis];
        generateTree(aRoot->right_, aDepth + 1, newPartitionPoint, rightMins, aMaxs, aRequestedDepth);
    }

    /**
     * @brief Print tree to console..
     *
     * @param aRoot         - Node whose children will be printed.
     * @param aDirection    - Direction from which this search came from, ie left or right node.
     */
    void printTreeImp(Nodex *aRoot, std::string aDirection)
    {
        if (aRoot == NULL)
        {
            return;
        };

        if (!aRoot->myChildren_.empty())
        {
            std::cout << "  LEAF " << aRoot->leafNr_ << " "
                      << "  " << aRoot->partitionPoint_[0] << " " << aRoot->partitionPoint_[1] << " " << aRoot->partitionPoint_[2] << "\n";
            std::cout << "      CHILDREN:\n";
            std::cout << "      ";
            for (auto d : aRoot->myChildren_)
            {
                std::cout << d << " ";
            }
            std::cout << "\n";
        }

        printTreeImp(aRoot->left_, "L");
        printTreeImp(aRoot->right_, "R");
    };

    /**
     * @brief Delete all nodes in the tree to free memory.
     *
     * This method deletes all nodes under the provided aRoot.
     *
     * @param aRoot - Node under which all underlying nodes will be deleted.
     */
    void deleteAllNodes(Nodex *node)
    {
        if (!node)
        {
            return;
        };

        deleteAllNodes(node->left_);
        deleteAllNodes(node->right_);

        delete node;
        node = nullptr;
    };
};