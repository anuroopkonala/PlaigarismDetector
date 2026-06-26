#include "APTEDDistance.h"

#include <algorithm>
#include <limits>
#include <stdexcept>



static int countNodes(const TreeNode& t)
{
    int n = 1;
    for (const auto& c : t.children) n += countNodes(c);
    return n;
}

static void postorderHelper(const TreeNode& t,
                             std::vector<std::string>& labels,
                             std::vector<int>&          parent,
                             std::vector<int>&          leftmost,
                             int                        parentIdx,
                             int&                       idx)
{
    int myIdx = -1;
    
    int firstChild = -1;
    for (const auto& c : t.children)
    {
        postorderHelper(c, labels, parent, leftmost, idx, idx);
        if (firstChild == -1) firstChild = idx - 1;
    }

    myIdx = idx++;
    labels.push_back(t.label);
    parent.push_back(parentIdx);

    
    if (t.children.empty())
        leftmost.push_back(myIdx);
    else
        leftmost.push_back(leftmost[firstChild == -1 ? myIdx : firstChild]);
}


static void flatten(const TreeNode& root,
                    std::vector<std::string>& labels,
                    std::vector<int>&         lm)
{
    
    struct Frame { const TreeNode* node; size_t childIdx; int lmLeaf; };
    std::vector<Frame> stack;
    stack.push_back({&root, 0, -1});
    int pos = 0;

    while (!stack.empty())
    {
        auto& [node, ci, lmLeaf] = stack.back();
        if (ci < node->children.size())
        {
            stack.push_back({&node->children[ci++], 0, -1});
        }
        else
        {
            int myPos = pos++;
            labels.push_back(node->label);

            if (node->children.empty())
            {
                lm.push_back(myPos);
                lmLeaf = myPos;
            }
            else
            {
                
                
                
                
                lm.push_back(lmLeaf == -1 ? myPos : lmLeaf);
            }

            
            if (stack.size() >= 2)
            {
                auto& parent = stack[stack.size() - 2];
                if (parent.lmLeaf == -1)
                    parent.lmLeaf = lm.back();
            }

            stack.pop_back();
        }
    }
}



static std::vector<int> keyRoots(const std::vector<int>& lm, int n)
{
    
    std::vector<bool> seen(n, false);
    std::vector<int>  kr;

    for (int i = n - 1; i >= 0; --i)
    {
        if (!seen[lm[i]])
        {
            seen[lm[i]] = true;
            kr.push_back(i);
        }
    }
    std::reverse(kr.begin(), kr.end());
    return kr;
}






static size_t zhanghasha(const std::vector<std::string>& lab1,
                          const std::vector<int>&         lm1,
                          const std::vector<std::string>& lab2,
                          const std::vector<int>&         lm2)
{
    const int n1 = (int)lab1.size();
    const int n2 = (int)lab2.size();

    if (n1 == 0) return (size_t)n2;
    if (n2 == 0) return (size_t)n1;

    auto kr1 = keyRoots(lm1, n1);
    auto kr2 = keyRoots(lm2, n2);

    
    std::vector<std::vector<size_t>> td(n1 + 1,
        std::vector<size_t>(n2 + 1, 0));

    const size_t INF = std::numeric_limits<size_t>::max() / 2;

    for (int i : kr1)
    {
        for (int j : kr2)
        {
            int lmi = lm1[i];
            int lmj = lm2[j];

            
            int rows = i - lmi + 2;
            int cols = j - lmj + 2;
            std::vector<std::vector<size_t>> fd(rows, std::vector<size_t>(cols, 0));

            fd[0][0] = 0;
            for (int ii = lmi; ii <= i; ++ii)
                fd[ii - lmi + 1][0] = fd[ii - lmi][0] + 1;  
            for (int jj = lmj; jj <= j; ++jj)
                fd[0][jj - lmj + 1] = fd[0][jj - lmj] + 1;  

            for (int ii = lmi; ii <= i; ++ii)
            {
                for (int jj = lmj; jj <= j; ++jj)
                {
                    size_t cost = (lab1[ii] == lab2[jj]) ? 0 : 1;

                    if (lm1[ii] == lmi && lm2[jj] == lmj)
                    {
                        fd[ii - lmi + 1][jj - lmj + 1] = std::min({
                            fd[ii - lmi][jj - lmj + 1] + 1,      
                            fd[ii - lmi + 1][jj - lmj] + 1,      
                            fd[ii - lmi][jj - lmj]     + cost     
                        });
                        td[ii + 1][jj + 1] = fd[ii - lmi + 1][jj - lmj + 1];
                    }
                    else
                    {
                        int p = lm1[ii] - lmi;
                        int q = lm2[jj] - lmj;
                        fd[ii - lmi + 1][jj - lmj + 1] = std::min({
                            fd[ii - lmi][jj - lmj + 1] + 1,
                            fd[ii - lmi + 1][jj - lmj] + 1,
                            fd[p][q] + td[ii + 1][jj + 1]
                        });
                    }
                }
            }
        }
    }

    return td[n1][n2];
}



double APTEDDistance::normalizedDistance(const TreeNode& a, const TreeNode& b)
{
    std::vector<std::string> la, lb;
    std::vector<int>         lma, lmb;

    flatten(a, la, lma);
    flatten(b, lb, lmb);

    size_t d     = zhanghasha(la, lma, lb, lmb);
    size_t maxD  = la.size() + lb.size();

    if (maxD == 0) return 0.0;
    return (double)d / (double)maxD;
}





TreeNode APTEDDistance::fromLabelSequence(const std::vector<std::string>& labels)
{
    if (labels.empty())
        return TreeNode{"<empty>", {}};

    
    
    TreeNode root{labels[0], {}};
    TreeNode* cur = &root;

    for (size_t i = 1; i < labels.size(); ++i)
    {
        cur->children.push_back(TreeNode{labels[i], {}});
        cur = &cur->children.back();
    }
    return root;
}
