// tests/VecodexIndexTest.cpp

#include <iostream>
#include <unordered_set>
#include "Index.h"
#include "faiss.h"
#include "gtest/gtest.h"
#include "json/json.h"
template <class IDType>
bool check_meta(const std::vector<IDType>& out_meta,
				const std::vector<IDType>& true_meta) {
	std::vector<IDType> diff;
	std::set_symmetric_difference(out_meta.begin(), out_meta.end(),
								  true_meta.begin(), true_meta.end(),
								  std::back_inserter(diff));
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

TEST(VecodexIndexTest, AddAndSearchVector) {

	// Initialize index with 2 dimensions and segment threshold of 5
	vecodex::IndexConfig<baseline::FaissIndex<faiss::IndexFlat, std::string>>
		config(baseline::IndexFlatAdd<std::string>,
			   baseline::IndexFlatSearch<std::string>,
			   baseline::IndexFlatMerge<std::string>);
	vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, std::string>,
				   std::string, int, faiss::MetricType>
		index(2, 3, config, {2, faiss::MetricType::METRIC_L2});

	// Add some vectors
	float vectors[2][2] = {{1.0f, 2.0f}, {2.0f, 3.0f}};
	std::vector<std::string> ids = {"vec1", "vec2"};
	index.add(2, ids.data(), (float*)vectors);
	std::vector<float> query = {1.5f, 2.5f};
	std::vector<std::string> results = index.search(query, 1);

	// Verify the search result

	EXPECT_TRUE(check_meta(results, {"vec1"}));
}
TEST(VecodexIndexTest, AddMultipleAndSearchTopK) {
	// Initialize index with 2 dimensions and segment threshold of 3
	vecodex::IndexConfig<baseline::FaissIndex<faiss::IndexFlat, std::string>>
		config(baseline::IndexFlatAdd<std::string>,
			   baseline::IndexFlatSearch<std::string>,
			   baseline::IndexFlatMerge<std::string>);
	vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, std::string>,
				   std::string, int, faiss::MetricType>
		index(2, 3, config, {2, faiss::MetricType::METRIC_L2});
	float vectors[5][2] = {
		{1.0f, 1.0f}, {2.0f, 2.0f}, {3.0f, 3.0f}, {4.0f, 4.0f}, {5.0f, 5.0f}};
	std::vector<std::string> ids = {"vec1", "vec2", "vec3", "vec4", "vec5"};
	// Add vectors
	index.add(5, ids.data(), (float*)vectors);

	// Search for top-2 closest vectors to the query
	std::vector<float> query = {3.5f, 3.5f};
	std::vector<std::string> results = index.search(query, 2);

	// Verify that the search returns top-2 nearest vectors
	EXPECT_TRUE(check_meta(results, {"vec3", "vec4"}));
}

TEST(VecodexIndexTest, MergeSegments) {
	// Initialize index with 2 dimensions and segment threshold of 2
	vecodex::IndexConfig<baseline::FaissIndex<faiss::IndexFlat, std::string>>
		config(baseline::IndexFlatAdd<std::string>,
			   baseline::IndexFlatSearch<std::string>,
			   baseline::IndexFlatMerge<std::string>);
	vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, std::string>,
				   std::string, int, faiss::MetricType>
		index(2, 2, config, {2, faiss::MetricType::METRIC_L2});
	float vectors[5][2] = {
		{1.0f, 1.0f}, {1.9f, 1.9f}, {3.0f, 3.0f}, {4.0f, 4.0f}, {5.0f, 5.0f}};
	std::vector<std::string> ids = {"vec1", "vec2", "vec3"};
	// Add vectors to create multiple segments
	index.add(3, ids.data(), (float*)vectors);
	// Merge segments
	EXPECT_TRUE(index.mergeSegments().size() == 1);

	// Verify that search still works correctly after merging
	std::vector<float> query = {2.5f, 2.5f};
	std::vector<std::string> results = index.search(query, 1);

	EXPECT_TRUE(check_meta(results, {"vec3"}));
}

TEST(VecodexIndexTest, HNSWSearch) {
	vecodex::IndexConfig<
		baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>>
		config(baseline::IndexHNSWAdd<std::string>,
			   baseline::IndexHNSWSearch<std::string>,
			   baseline::IndexHNSWMerge<std::string>);
	vecodex::Index<baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>,
				   std::string, int, int, faiss::MetricType>
		index(2, 2, config, {2, 2, faiss::MetricType::METRIC_L2});

	std::vector<float> vectors(4 * 2);	// n * dim
	vectors[0] = vectors[1] = 1.0f;
	vectors[2] = vectors[3] = 2.0f;
	vectors[4] = vectors[5] = 0.1f;
	vectors[6] = vectors[7] = 6.0f;
	std::vector<std::string> ids = {"vec1", "vec2", "vec0", "vec6"};
	index.add(4, ids.data(), vectors.data());
	std::vector<std::string> results = index.search({3.0f, 3.0f}, 2);
	EXPECT_TRUE(check_meta(results, {"vec2", "vec1"}));
}

TEST(VecodexIndexTest, Delete) {
	vecodex::IndexConfig<
		baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>>
		config(baseline::IndexHNSWAdd<std::string>,
			   baseline::IndexHNSWSearch<std::string>,
			   baseline::IndexHNSWMerge<std::string>,
			   baseline::IndexHNSWDelete<std::string>);
	vecodex::Index<baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>,
				   std::string, int, int, faiss::MetricType>
		index(2, 2, config, {2, 2, faiss::MetricType::METRIC_L2});
	std::vector<float> vectors(4 * 2);	// n * dim
	vectors[0] = vectors[1] = 1.0f;
	vectors[2] = vectors[3] = 2.0f;
	vectors[4] = vectors[5] = 0.1f;
	vectors[6] = vectors[7] = 6.0f;
	std::vector<std::string> ids = {"vec1", "vec2", "vec0", "vec6"};
	index.add(4, ids.data(), vectors.data());
	ids = {"vec1", "vec2"};
	index.erase(2, ids.data());
	std::vector<std::string> results = index.search({3.0f, 3.0f}, 3);
	EXPECT_TRUE(check_meta(results, {"vec0", "vec6"}));
	ids[0] = "vec0";
	index.erase(1, ids.data());
	results = index.search({3.0f, 3.0f}, 2);
	EXPECT_TRUE(check_meta(results, {"vec6"}));
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
