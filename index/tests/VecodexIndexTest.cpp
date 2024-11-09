// tests/VecodexIndexTest.cpp

#include "VecodexIndex.h"
#include "gtest/gtest.h"

TEST(VecodexIndexTest, AddAndSearchVector) {
    // Initialize index with 2 dimensions and segment threshold of 5
    VecodexIndex index(2, 5);

    // Add some vectors
    std::vector<float> vector1 = {1.0f, 2.0f};
    std::unordered_map<std::string, std::string> metadata1 = {{"name", "vector1"}};
    index.addVector("vec1", vector1, metadata1);

    std::vector<float> vector2 = {2.0f, 3.0f};
    std::unordered_map<std::string, std::string> metadata2 = {{"name", "vector2"}};
    index.addVector("vec2", vector2, metadata2);

    std::vector<float> query = {1.5f, 2.5f};
    std::vector<std::string> results = index.search(query, 1);

    // Verify the search result
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "vec1");
}

TEST(VecodexIndexTest, AddMultipleAndSearchTopK) {
    // Initialize index with 2 dimensions and segment threshold of 3
    VecodexIndex index(2, 3);

    // Add vectors
    index.addVector("vec1", {1.0f, 1.0f}, {{"name", "vector1"}});
    index.addVector("vec2", {2.0f, 2.0f}, {{"name", "vector2"}});
    index.addVector("vec3", {3.0f, 3.0f}, {{"name", "vector3"}});

    // Add more vectors which should trigger new segment creation
    index.addVector("vec4", {4.0f, 4.0f}, {{"name", "vector4"}});
    index.addVector("vec5", {5.0f, 5.0f}, {{"name", "vector5"}});

    // Search for top-2 closest vectors to the query
    std::vector<float> query = {3.5f, 3.5f};
    std::vector<std::string> results = index.search(query, 2);

    // Verify that the search returns top-2 nearest vectors
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], "vec3");
    EXPECT_EQ(results[1], "vec4");
}

TEST(VecodexIndexTest, MetadataCheck) {
    // Initialize index with 2 dimensions and segment threshold of 3
    VecodexIndex index(2, 3);

    // Add a vector with metadata
    std::vector<float> vector1 = {1.0f, 1.0f};
    std::unordered_map<std::string, std::string> metadata1 = {{"type", "test"}, {"category", "A"}};
    index.addVector("test1", vector1, metadata1);

    // Search the vector and verify the ID
    std::vector<float> query = {1.0f, 1.0f};
    std::vector<std::string> results = index.search(query, 1);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "test1");

    // Additional checks for metadata if needed
    // Assuming a function is implemented to retrieve metadata based on ID
}

TEST(VecodexIndexTest, MergeSegments) {
    // Initialize index with 2 dimensions and segment threshold of 2
    VecodexIndex index(2, 2);

    // Add vectors to create multiple segments
    index.addVector("vec1", {1.0f, 1.0f}, {{"name", "vector1"}});
    index.addVector("vec2", {2.0f, 2.0f}, {{"name", "vector2"}});
    index.addVector("vec3", {3.0f, 3.0f}, {{"name", "vector3"}});

    // Merge segments
    index.mergeSegments();

    // Verify that search still works correctly after merging
    std::vector<float> query = {2.5f, 2.5f};
    std::vector<std::string> results = index.search(query, 1);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "vec3");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

