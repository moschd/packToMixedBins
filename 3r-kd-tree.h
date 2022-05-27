#include<bits/stdc++.h>
#include "constants.h"
  
const int k = constants::AllAxis.size();
unsigned int searchhits = 0;


struct Node {
    int itemSysId_;
    std::array<double,3> endPoint_, partitionPoint_, minSearchDimensions_, maxSearchDimensions_; 
    std::vector<int> myChildren_;
    Node *left_, *right_;

    Node* addToTree(Node* root, int sysid, std::array<double,3> maxDims, std::array<double,3> endPoint){
        return insertNodeInTree(root, sysid, 0, endPoint, {maxDims[0]/2, maxDims[1]/2, maxDims[2]/2}, {0.0,0.0,0.0}, maxDims);
    };

    Node* insertNodeInTree(Node *root, unsigned int rItemSysId, unsigned int rDepth, 
    std::array<double,3> rEndPoint, std::array<double,3> rPartitionPoint, std::array<double,3> rMins, std::array<double,3> rMaxs){

        /* Inserts a new node and returns root of modified tree */

        unsigned int axis = rDepth % k;
        unsigned int previousAxis = (rDepth-1) % k;

        std::array<double,3> newPartitionPoint = (root == NULL ? rPartitionPoint : root->partitionPoint_);
        newPartitionPoint[previousAxis] = (rMins[previousAxis] + rMaxs[previousAxis])/2;

        if (root == NULL){ 
            Node* newNode   = new Node;
            newNode->itemSysId_ = rItemSysId;
            newNode->endPoint_  = rEndPoint;
            newNode->partitionPoint_ = newPartitionPoint;
            newNode->minSearchDimensions_ =  rMins;
            newNode->maxSearchDimensions_ =  rMaxs;
            newNode->left_ = newNode->right_ = NULL;
            return newNode;
        };

        root->minSearchDimensions_ = rMins;
        root->maxSearchDimensions_ = rMaxs;
        
        root->myChildren_.push_back(rItemSysId);
        if (rEndPoint[axis] < root->partitionPoint_[axis]){
            rMaxs[axis] = root->partitionPoint_[axis];
            root->left_  = insertNodeInTree(root->left_, rItemSysId, rDepth + 1, rEndPoint, newPartitionPoint, rMins, rMaxs);
        } else {
            rMins[axis] = root->partitionPoint_[axis];
            root->right_ = insertNodeInTree(root->right_, rItemSysId, rDepth + 1, rEndPoint, newPartitionPoint, rMins, rMaxs);
        };
        return root;
    };
 
};
  

bool arePointsSame(std::array<double,3> point1, std::array<double,3> point2){
    return (std::abs(point1[0]-point2[0])< 0.0001 &&
            std::abs(point1[1]-point2[1])< 0.0001 &&
            std::abs(point1[2]-point2[2])< 0.0001);
};

void endpointsInKdTree(Node* root, int depth, std::array<double,3>& startPoint, std::array<double,3> maxSearchPoint, std::vector<int>& passedNodes){
    if (root == NULL){ return; };

    unsigned int cd = depth % k;

    if(root->left_ == NULL && root->right_ == NULL){
        if(startPoint[0] < root->endPoint_[0] && startPoint[1] < root->endPoint_[1] && startPoint[2] < root->endPoint_[2]){
            passedNodes.push_back(root->itemSysId_);
            return;          
        };
    };

    if(startPoint[0] < root->partitionPoint_[0] && startPoint[1] < root->partitionPoint_[1] && startPoint[2] < root->partitionPoint_[2]){
        passedNodes.insert(passedNodes.end(), root->myChildren_.begin(), root->myChildren_.end()); 
        passedNodes.push_back(root->itemSysId_);
        return;
    };

    if(startPoint[cd] < root->partitionPoint_[cd]){
        endpointsInKdTree(root->left_, depth+1, startPoint, maxSearchPoint, passedNodes);
    };

    if(root->partitionPoint_[cd] < (startPoint[cd]+maxSearchPoint[cd])){
        endpointsInKdTree(root->right_, depth+1, startPoint, maxSearchPoint, passedNodes);
    };
};


bool singlePointSearch(Node* root, std::array<double,3> endPoint, unsigned int depth){
    if (root == NULL){ return false; };
    if (arePointsSame(root->endPoint_, endPoint)){ return true; };

    unsigned int cd = depth % k;

    if (endPoint[cd] < root->endPoint_[cd]){
        return singlePointSearch(root->left_, endPoint, depth + 1);
    } else {
        return singlePointSearch(root->right_, endPoint, depth + 1);
    };
};

Node* init_tree(){
    struct Node *root = NULL;  
    return root;
};

void printTree(Node* root,int p, std::string W){
    if(root == NULL){ return; };

    printTree(root->left_, root->itemSysId_, "L");
    printTree(root->right_, root->itemSysId_, "R");
};