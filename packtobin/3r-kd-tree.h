#include<bits/stdc++.h>
#include "constants.h"
  
const int k = constants::AllAxis.size();
  
// A structure to represent node of kd tree
struct Node
{
    int itemId;
    std::array<double,3> point; // To store k dimensional point
    Node *left, *right;

    Node* addToTree(Node* root, std::array<double,3> point, int sysid){
        std::cout << "inserting " << sysid << " " << point[0] << " " << point[1] << " " << point[2] << "  \n";
        return insertRec(root, point, 0, sysid);
    };

// Inserts a new node and returns root of modified tree
// The parameter depth is used to decide axis of comparison
    Node *insertRec(Node *root, std::array<double,3> point, unsigned int depth, unsigned int sysid){
        
        // Tree is empty?
        if (root == NULL){ 
            Node* newNode   = new Node;
            newNode->itemId = sysid;
            newNode->point  = point;
            newNode->left   = newNode->right = NULL;
            return newNode;
        };

        // Calculate current dimension (cd) of comparison
        unsigned int cd = depth % k;
        std::cout << "root: " << root->itemId << " checking in d:" << cd << " " << root->point[0] << " " << root->point[1] << " " << root->point[2] << "\n";

        // Compare the new point with root on current dimension 'cd'
        // and decide the left or right subtree
        if (point[cd] < root->point[cd]){
            root->left  = insertRec(root->left, point, depth + 1, sysid);
        } else {
            root->right = insertRec(root->right, point, depth + 1, sysid);
        };

        return root;
    };
};
  

bool arePointsSame(std::array<double,3> point1, std::array<double,3> point2)
{
    // std::cout << point1[0] << " " << point2[0] << " " << std::abs(point1[0]-point2[0]) <<" \n";
    // std::cout << point1[1] << " " << point2[1] << "\n";
    // std::cout << point1[2] << " " << point2[2] << "\n";
    // bool matches = (std::abs(point1[0]-point2[0])< 0.0001 &&
    //                 std::abs(point1[1]-point2[1])< 0.0001 &&
    //                 std::abs(point1[2]-point2[2])< 0.0001);
    // std::cout << "arePointsSame: " << matches << "\n";

    return (std::abs(point1[0]-point2[0])< 0.0001 &&
            std::abs(point1[1]-point2[1])< 0.0001 &&
            std::abs(point1[2]-point2[2])< 0.0001);;
}
  
// Searches a Point represented by "point[]" in the K D tree.
// The parameter depth is used to determine current axis.
std::vector<int> searchRec(Node* root, std::array<double,3> point, unsigned int depth, std::vector<int> passedNodes)
{
    // Base cases
    if (root == NULL){ return passedNodes; };
    // if (arePointsSame(root->point, point)){ return passedNodes; };

    unsigned int cd = depth % k;
  
    // Compare point with root with respect to cd (Current dimension)
    if (point[cd] < root->point[cd]){
        passedNodes.push_back(root->itemId);
        return searchRec(root->right, point, depth + 1, passedNodes); searchRec(root->left, point, depth + 1, passedNodes);
    };
  
    return passedNodes;
}
  

bool singlePointSearch(Node* root, std::array<double,3> point, unsigned int depth)
{
    if (root == NULL){ return false; };
    if (arePointsSame(root->point, point)){ return true; };

    unsigned int cd = depth % k;
    std::cout << "SEARCH root: " << root->itemId << " checking for d:" << cd << " " << root->point[0] << " " << root->point[1] << " " << root->point[2] << "\n";

    // Compare point with root with respect to cd (Current dimension)
    if (point[cd] < root->point[cd]){
        return singlePointSearch(root->left, point, depth + 1);
    } else {
        return singlePointSearch(root->right, point, depth + 1);
    };
};

Node* init_tree(){
    struct Node *root = NULL;  
    return root;
};

std::string searchTree(Node* root, std::array<double,3> point){
    return singlePointSearch(root, point, 0) ? "Found" : " not found";
};

std::vector<int> searchItemTreeAndReturnIntersectionCandidates(Node* root, std::array<double,3> point){
    return searchRec(root, point, 0, {});
};