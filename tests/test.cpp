



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
#include "RollingHash.h"
#include "Winnower.h"
#include "InvertedIndex.h"



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



static void testRollingHash()
{
    std::cout << "\n[RollingHash — true O(N) Rabin-Karp]\n";
    RollingHash rh(3);

    CHECK(rh.compute({"a","b"}).empty());

    auto h2 = rh.compute({"x","y","z"});
    CHECK(h2.size() == 1);

    CHECK(h2[0] == rh.compute({"x","y","z"})[0]);

    auto h3 = rh.compute({"a","b","c"});
    CHECK(h2[0] != h3[0]);

    CHECK(rh.compute({"a","b","c","d","e"}).size() == 3);

    RollingHash rh5(5);
    auto big = rh5.compute({"int","VAR1","=","NUM",";","return","VAR1",";"});
    CHECK(big.size() == 4);

    for (auto h : big) CHECK(h != 0);
}

static void testWinnower()
{
    std::cout << "\n[Winnower — O(N) sliding-window minimum]\n";
    Winnower w(3);

    CHECK(w.select({1,2}).empty());

    auto fp1 = w.select({5,5,5,5,5});
    CHECK(!fp1.empty());
    for (size_t i = 1; i < fp1.size(); ++i)
        CHECK(fp1[i].tokenIndex != fp1[i-1].tokenIndex);

    std::vector<uint64_t> h10 = {9,2,8,1,7,3,6,4,5,0};
    auto fp2 = w.select(h10);
    
    CHECK(!fp2.empty());

    uint64_t minH = *std::min_element(h10.begin(), h10.end());
    bool found = false;
    for (auto& fp : fp2) if (fp.hash == minH) { found = true; break; }
    CHECK(found);

    for (auto& fp : fp2) CHECK(fp.tokenIndex < h10.size());
}

static void testInvertedIndex()
{
    std::cout << "\n[InvertedIndex]\n";

    {
        InvertedIndex idx;
        idx.insert("a.cpp", {{100,0},{200,1}});
        idx.insert("b.cpp", {{300,0},{400,1}});
        CHECK(idx.candidates().empty());
    }

    {
        InvertedIndex idx;
        idx.insert("x.cpp", {{111,0},{222,1}});
        idx.insert("y.cpp", {{111,0},{333,1}});
        auto c = idx.candidates();
        CHECK(c.size() == 1);
        CHECK(c[0].sharedHashes == 1);
    }

    {
        InvertedIndex idx;
        idx.insert("p.cpp", {{10,0},{20,1}});
        idx.insert("q.cpp", {{10,0},{20,1}});
        auto c = idx.candidates();
        CHECK(c.size() == 1);
        CHECK(c[0].sharedHashes == 2);
    }

    {
        InvertedIndex idx;
        idx.insert("f1.cpp", {{1,0},{2,1}});
        idx.insert("f2.cpp", {{1,0},{3,1}});  
        idx.insert("f3.cpp", {{2,0},{3,1}});  
        auto c = idx.candidates();
        CHECK(c.size() == 3);  
    }

    {
        InvertedIndex idx;
        idx.insert("a.cpp", {{1,0},{2,1},{3,2}});
        idx.insert("b.cpp", {{1,0},{2,1}});       
        idx.insert("c.cpp", {{1,0},{2,1},{3,2}}); 
        auto c = idx.candidates();
        CHECK(!c.empty() && c[0].sharedHashes >= c.back().sharedHashes);
    }
}

static void testRollingProperty()
{
    std::cout << "\n[RollingHash — rolling consistency check]\n";

    RollingHash rh(4);
    std::vector<std::string> tokens = {"int","VAR1","=","NUM",";","return","VAR1",";","}"};
    auto hashes = rh.compute(tokens);

    for (size_t i = 0; i + 4 <= tokens.size(); ++i)
    {
        std::vector<std::string> sub(tokens.begin() + i, tokens.begin() + i + 4);
        auto subH = rh.compute(sub);
        CHECK(subH.size() == 1 && subH[0] == hashes[i]);
    }
}

int main()
{
    testRollingHash();
    testWinnower();
    testInvertedIndex();
    testRollingProperty();
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
