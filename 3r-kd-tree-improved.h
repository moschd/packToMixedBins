#include <bits/stdc++.h>
#include "constants.h"

struct Nodex
{
    int id;
    bool isLeaf_;
    int myDepth_;
    int leafNr_;
    Nodex *left_, *right_;
    std::vector<int> myChildren_;
    std::array<double, 3> partitionPoint_, minSearchDimensions_, maxSearchDimensions_;

    void printMe()
    {
        std::cout << "NODEX " << id << "   isLeaf " << isLeaf_ << " leafNr " << leafNr_ << "     myDepth " << myDepth_ << "\n";
        std::cout << partitionPoint_[0] << " " << partitionPoint_[1] << " " << partitionPoint_[2];
    };
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
    branch->left_ = branch->right_ = NULL;
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
    KdTree(int aRequestedDepth, std::array<double, 3> aMaxDimensions)
    {
        maxDepth_ = aRequestedDepth;
        minDimensions_ = {0, 0, 0};
        maxDimensions_ = aMaxDimensions;

        treeRoot_ = genNodex({maxDimensions_[0] / 2, maxDimensions_[1] / 2, maxDimensions_[2] / 2}, minDimensions_, maxDimensions_, 0);
        generateTreeHelper(treeRoot_);
    };

    Nodex *getRoot()
    {
        return treeRoot_;
    };

    void generateTreeHelper(Nodex *aRoot)
    {
        generateTree(aRoot, aRoot->myDepth_, aRoot->partitionPoint_, aRoot->minSearchDimensions_, aRoot->maxSearchDimensions_, maxDepth_);
    };

    void printTreeImpHelper()
    {
        printTreeImp(treeRoot_, -1, "X");
    };

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
     * @param aItemKey   - The key of the item that needs to be placed in the tree.
     * @param aDepth    - The current depth of the tree.
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
     * @param aRoot      - aRoot of the tree
     * @param aDepth    - the current depth of the tree
     * @param aPartitionPoint   - the 3R point that the node will be representating and splitting.
     * @param aMins     - 3R point marking the minimum border of the search area point for which this node will be responsable.
     * @param aMaxs     - 3R point marking the maximum border of the search area point for which this node will be responsable.
     * @param aRequestedDepth    - the requested maximum depth of the tree to be generated.
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

    void printTreeImp(Nodex *aRoot, int aRootId, std::string aDirection)
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

        printTreeImp(aRoot->left_, aRoot->myDepth_, "L");
        printTreeImp(aRoot->right_, aRoot->myDepth_, "R");
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