// tests/VecodexIndexTest.cpp

#include "VecodexIndex.h"
#include "gtest/gtest.h"
#include <unordered_set>
#include "json/json.h"
#include <fstream>

const std::string basic_flat = "basic_flat.json";

bool check_meta(const std::vector<IDType> &out_meta, const std::vector<IDType> &true_meta) {
    std::vector<IDType> diff;
    std::set_symmetric_difference(out_meta.begin(), out_meta.end(), true_meta.begin(), true_meta.end(), std::back_inserter(diff));
    if (diff.size() == 0) {
        return true;
    }
    std::cerr << "True meta differs from out meta:\n";
    for (const IDType& x : diff) {
        std::cerr << x << " ";
    }
    std::cerr << "\n";
    return false;
}
void createBasicFlat(int dim) {
    Json::Value config;
    config["dim"] = dim;
    config["metric"] = "L2";
    config["M"] = 32;
    config["index"] = "Flat";
    std::ofstream out(basic_flat);
    out << config;
}
TEST(VecodexIndexTest, AddAndSearchVector) {
    // Initialize index with 2 dimensions and segment threshold of 5
    VecodexIndex index(5, IndexConfig(basic_flat));

    // Add some vectors
    std::vector<float> vector1 = {1.0f, 2.0f};
    index.addVector("vec1", vector1);

    std::vector<float> vector2 = {2.0f, 3.0f};
    index.addVector("vec2", vector2);

    std::vector<float> query = {1.5f, 2.5f};
    std::vector<std::string> results = index.search(query, 1);

    // Verify the search result

    EXPECT_TRUE(check_meta(results, {"vec1"}));
}
TEST(VecodexIndexTest, AddMultipleAndSearchTopK) {
    // Initialize index with 2 dimensions and segment threshold of 3
    VecodexIndex index(3, IndexConfig(basic_flat));

    // Add vectors
    index.addVector("vec1", {1.0f, 1.0f});
    index.addVector("vec2", {2.0f, 2.0f});
    index.addVector("vec3", {3.0f, 3.0f});

    // Add more vectors which should trigger new segment creation
    index.addVector("vec4", {4.0f, 4.0f});
    index.addVector("vec5", {5.0f, 5.0f});

    // Search for top-2 closest vectors to the query
    std::vector<float> query = {3.5f, 3.5f};
    std::vector<std::string> results = index.search(query, 2);

    // Verify that the search returns top-2 nearest vectors
    EXPECT_TRUE(check_meta(results, {"vec3", "vec4"}));
}

TEST(VecodexIndexTest, MetadataCheck) {
    // Initialize index with 2 dimensions and segment threshold of 3
    VecodexIndex index(3, IndexConfig(basic_flat));

    // Add a vector with metadata
    std::vector<float> vector1 = {1.0f, 1.0f};
    index.addVector("test1", vector1);

    // Search the vector and verify the ID
    std::vector<float> query = {1.0f, 1.0f};
    std::vector<std::string> results = index.search(query, 1);

    EXPECT_TRUE(check_meta(results, {"test1"}));

    // Additional checks for metadata if needed
    // Assuming a function is implemented to retrieve metadata based on ID
}

TEST(VecodexIndexTest, MergeSegments) {
    // Initialize index with 2 dimensions and segment threshold of 2
    VecodexIndex index(2, IndexConfig(basic_flat));

    // Add vectors to create multiple segments
    index.addVector("vec1", {1.0f, 1.0f});
    index.addVector("vec2", {1.9f, 1.9f});
    index.addVector("vec3", {3.0f, 3.0f});

    // Merge segments
    index.mergeSegments();

    // Verify that search still works correctly after merging
    std::vector<float> query = {2.5f, 2.5f};
    std::vector<std::string> results = index.search(query, 1);

    EXPECT_TRUE(check_meta(results, {"vec3"}));
}

int main(int argc, char **argv) {
    createBasicFlat(2);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
