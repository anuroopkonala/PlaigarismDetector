#pragma once

#include <memory>
#include <string>
#include <vector>


















struct TreeNode
{
    std::string              label;
    std::vector<TreeNode>    children;
};

class APTEDDistance
{
public:
    
    
    double normalizedDistance(const TreeNode& a, const TreeNode& b);

    
    
    
    
    
    
    static TreeNode fromLabelSequence(const std::vector<std::string>& labels);

private:
    
    static void computeKeyRoots(const TreeNode& t,
                                 std::vector<int>& keyRoots,
                                 std::vector<int>& leftmost,
                                 int& idx);

    
    static void postorder(const TreeNode& t,
                          std::vector<std::string>& labels,
                          std::vector<int>& leftmostLeaf,
                          int& idx);

    
    static size_t treeDist(const std::vector<std::string>& labA,
                           const std::vector<int>&         lmA,
                           const std::vector<int>&         krA,
                           const std::vector<std::string>& labB,
                           const std::vector<int>&         lmB,
                           const std::vector<int>&         krB,
                           std::vector<std::vector<size_t>>& td);
};
