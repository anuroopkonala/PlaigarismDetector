



#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "APTEDDistance.h"
#include "Normalizer.h"
#include "SimilarityEngine.h"
#include "ThreadPool.h"
#include "Token.h"



static int passed = 0, failed = 0;

#define CHECK(cond) \
    do { \
        if (cond) { std::cout << "  PASS  " #cond "\n"; ++passed; } \
        else      { std::cout << "  FAIL  " #cond " (line " << __LINE__ << ")\n"; ++failed; } \
    } while(0)

#define CHECK_NEAR(a, b, eps) \
    CHECK(std::abs((a)-(b)) < (eps))







static void testSimilarityEngine()
{
    std::cout << "\n[SimilarityEngine — TF-IDF cosine + LCS]\n";

    
    {
        SimilarityEngine e;
        std::vector<std::string> d = {"int","VAR1","=","NUM",";","return","VAR1",";"};
        e.addDocument("a.cpp", d);
        e.addDocument("b.cpp", d);
        CHECK_NEAR(e.cosineSimilarity("a.cpp","b.cpp"), 1.0, 1e-9);
    }

    
    {
        SimilarityEngine e;
        e.addDocument("x.cpp", {"for","while","if"});
        e.addDocument("y.cpp", {"class","struct","enum"});
        CHECK_NEAR(e.cosineSimilarity("x.cpp","y.cpp"), 0.0, 1e-9);
    }

    
    {
        SimilarityEngine e;
        e.addDocument("p.cpp", {"int","float","double","char"});
        e.addDocument("q.cpp", {"int","float","void","bool"});
        double c = e.cosineSimilarity("p.cpp","q.cpp");
        CHECK(c > 0.0 && c < 1.0);
    }

    // LCS: identical → 1
    {
        SimilarityEngine e;
        std::vector<std::string> d = {"for","VAR1","=","NUM"};
        e.addDocument("a.cpp", d);
        e.addDocument("b.cpp", d);
        CHECK_NEAR(e.lcsSimilarity("a.cpp","b.cpp"), 1.0, 1e-9);
    }

    // LCS: disjoint → 0
    {
        SimilarityEngine e;
        e.addDocument("a.cpp", {"A","B","C"});
        e.addDocument("b.cpp", {"X","Y","Z"});
        CHECK_NEAR(e.lcsSimilarity("a.cpp","b.cpp"), 0.0, 1e-9);
    }

    // LCS: known subsequence
    {
        SimilarityEngine e;
        e.addDocument("a.cpp", {"1","2","3","4","5"});
        e.addDocument("b.cpp", {"1","3","5"});   // LCS = {1,3,5}, len=3, max=5
        double lcs = e.lcsSimilarity("a.cpp","b.cpp");
        CHECK_NEAR(lcs, 3.0/5.0, 1e-9);
    }
}



static void testNormalizer()
{
    std::cout << "\n[Normalizer]\n";
    Normalizer norm;

    Token kw  {TokenType::Keyword,    "int",    1, 1};
    Token id1 {TokenType::Identifier, "myVar",  1, 5};
    Token id2 {TokenType::Identifier, "myVar",  2, 5};
    Token lit {TokenType::Literal,    "42",     3, 1};
    Token cmt {TokenType::Comment,    "//hi",   4, 1};

    auto result = norm.normalize({kw, id1, id2, lit, cmt});

    CHECK(result.size() == 4);              
    CHECK(result[0].value == "int");        
    CHECK(result[1].value == "VAR");  
    CHECK(result[2].value == "VAR");  
    CHECK(result[3].value == "NUM");        

    
    Token id3 {TokenType::Identifier, "otherVar", 5, 1};
    auto result2 = norm.normalize({id1, id3});
    CHECK(result2[0].value == "VAR");
    CHECK(result2[1].value == "VAR");
}



static void testAPTEDDistance()
{
    std::cout << "\n[APTEDDistance — Zhang-Shasha tree edit distance]\n";
    APTEDDistance apted;

    
    TreeNode t1{"root", {{"a",{}},{"b",{}}}};
    TreeNode t2{"root", {{"a",{}},{"b",{}}}};
    CHECK_NEAR(apted.normalizedDistance(t1, t2), 0.0, 1e-9);

    
    TreeNode t3{"X", {{"Y",{}},{"Z",{}}}};
    double d1 = apted.normalizedDistance(t1, t3);
    CHECK(d1 > 0.0 && d1 <= 1.0);

    
    TreeNode s1{"leaf",{}}, s2{"leaf",{}};
    CHECK_NEAR(apted.normalizedDistance(s1, s2), 0.0, 1e-9);

    
    TreeNode s3{"other",{}};
    double d2 = apted.normalizedDistance(s1, s3);
    CHECK(d2 > 0.0 && d2 <= 1.0);

    
    TreeNode empty{"<empty>",{}};
    double d3 = apted.normalizedDistance(empty, t1);
    CHECK(d3 > 0.0);

    
    auto tree = APTEDDistance::fromLabelSequence({"root"});
    CHECK(tree.label == "root");
    CHECK(tree.children.empty());

    
    auto tree2 = APTEDDistance::fromLabelSequence({"FunctionDecl","ForStmt","ReturnStmt"});
    CHECK(tree2.label == "FunctionDecl");
    CHECK(!tree2.children.empty());
}



static void testThreadPool()
{
    std::cout << "\n[ThreadPool]\n";

    
    {
        ThreadPool pool(4);
        std::vector<std::future<int>> futs;
        for (int i = 0; i < 20; ++i)
            futs.push_back(pool.enqueue([i]{ return i * i; }));

        int sum = 0;
        for (auto& f : futs) sum += f.get();
        
        CHECK(sum == 2470);
    }

    
    {
        ThreadPool pool(1);
        std::vector<std::future<std::string>> futs;
        for (int i = 0; i < 5; ++i)
            futs.push_back(pool.enqueue([i]{ return std::to_string(i); }));
        std::string combined;
        for (auto& f : futs) combined += f.get();
        CHECK(combined == "01234");
    }

    
    {
        ThreadPool pool(2);
        auto fut = pool.enqueue([]() -> int {
            throw std::runtime_error("test error");
            return 0;
        });
        bool caught = false;
        try { fut.get(); }
        catch (const std::exception&) { caught = true; }
        CHECK(caught);
    }
}



int main()
{
    testSimilarityEngine();
    testNormalizer();
    testAPTEDDistance();
    testThreadPool();

    std::cout << "\n══════════════════════════════════════\n";
    std::cout << "Results: " << passed << " passed";
    if (failed)
        std::cout << ", " << failed << " FAILED";
    std::cout << "\n══════════════════════════════════════\n";

    return (failed == 0) ? 0 : 1;
}
