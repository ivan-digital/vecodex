// tests/VecodexIndexTest.cpp

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <random>
#include <unordered_set>
#include "IBaseIndex.h"
#include "Index.h"
#include "faiss.h"
#include "gtest/gtest.h"
#include "json/json.h"

using SegmentHNSWType =
	vecodex::Segment<baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>>;
using SegmentFLatType =
	vecodex::Segment<baseline::FaissIndex<faiss::IndexFlat, std::string>>;
using IndexHNSWType =
	vecodex::Index<baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>>;
using IndexFlatType =
	vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, std::string>>;
int erased = 0;
int inserted = 0;
template <typename Segment>
void update_callback(std::vector<size_t>&& ids,
					 std::vector<std::shared_ptr<const Segment>>&& segs) {
	erased += ids.size();
	inserted += segs.size();
}

std::vector<std::string> serialization;

template <typename Segment>
void serialize_callback(std::vector<size_t>&& ids,
						std::vector<std::shared_ptr<const Segment>>&& segs) {
	static int called = 0;
	called++;
	if (called > 2) {
		return;
	}
	for (auto&& seg : segs) {
		std::string filename = "temp_" + std::to_string(std::rand());
		seg->serialize(filename);
		serialization.push_back(filename);
	}
}

template <class IDType>
bool check_meta(const std::vector<IDType>& out_meta,
				const std::vector<IDType>& true_meta) {
	auto out_copy = out_meta;
	auto true_copy = true_meta;
	std::sort(out_copy.begin(), out_copy.end());
	std::sort(true_copy.begin(), true_copy.end());
	std::vector<IDType> diff;
	std::set_symmetric_difference(out_copy.begin(), out_copy.end(),
								  true_copy.begin(), true_copy.end(),
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
	IndexHNSWType index(2, 3, std::nullopt, 2, 2, faiss::MetricType::METRIC_L2);

	// Add some vectors
	float vectors[2][2] = {{1.0f, 2.0f}, {2.0f, 3.1f}};
	std::vector<std::string> ids = {"vec1", "vec2"};
	index.add(2, ids.data(), (float*)vectors);
	std::vector<float> query = {1.5f, 2.5f};
	std::vector<std::string> results = index.search(query, 1);

	// Verify the search result

	EXPECT_TRUE(check_meta(results, {"vec1"}));
}
TEST(VecodexIndexTest, AddMultipleAndSearchTopK) {
	// Initialize index with 2 dimensions and segment threshold of 3
	IndexFlatType index(2, 3, std::nullopt, 2, faiss::MetricType::METRIC_L2);
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
	IndexFlatType index(2, 2, std::nullopt, 2, faiss::MetricType::METRIC_L2);
	float vectors[5][2] = {
		{1.0f, 1.0f}, {1.9f, 1.9f}, {3.0f, 3.0f}, {4.0f, 4.0f}, {5.0f, 5.0f}};
	std::vector<std::string> ids = {"vec1", "vec2", "vec3", "vec4", "vec5"};
	// Add vectors to create multiple segments
	index.add(5, ids.data(), (float*)vectors);
	// Merge segments
	index.mergeSegments(index.size());
	EXPECT_EQ(index.size(), 1);

	// Verify that search still works correctly after merging
	std::vector<float> query = {2.5f, 2.5f};
	std::vector<std::string> results = index.search(query, 1);

	EXPECT_TRUE(check_meta(results, {"vec3"}));
}

TEST(VecodexIndexTest, Search) {
	IndexHNSWType index(2, 2, std::nullopt, 2, 2, faiss::MetricType::METRIC_L2);

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
	IndexHNSWType index(2, 2, std::nullopt, 2, 2, faiss::MetricType::METRIC_L2);
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

TEST(VecodexIndexTest, UpdateCallback) {
	IndexFlatType index(2, 2, update_callback<SegmentFLatType>, 2,
						faiss::MetricType::METRIC_L2);
	float vectors[5][2] = {
		{1.0f, 1.0f}, {1.9f, 1.9f}, {3.0f, 3.0f}, {4.0f, 4.0f}, {5.0f, 5.0f}};
	std::vector<std::string> ids = {"vec1", "vec2", "vec3", "vec4", "vec5"};

	index.add(ids.size(), ids.data(), (float*)vectors);
	ASSERT_EQ(inserted, 2);
	ASSERT_EQ(erased, 0);
	inserted = 0;
	index.mergeSegments(index.size());
	ASSERT_EQ(inserted, 1);
	ASSERT_EQ(erased, 3);
}

TEST(VecodexIndexTest, Serialize) {
	IndexFlatType index(2, 2, serialize_callback<SegmentFLatType>, 2,
						faiss::MetricType::METRIC_L2);
	float vectors[4][2] = {
		{1.0f, 1.0f}, {1.9f, 1.9f}, {3.0f, 3.0f}, {4.0f, 4.0f}};
	std::vector<std::string> ids = {"vec1", "vec2", "vec3", "vec4"};

	index.add(ids.size(), ids.data(), (float*)vectors);
	index.mergeSegments(index.size());

	IndexFlatType index_copy(2, 2, serialize_callback<SegmentFLatType>, 2,
							 faiss::MetricType::METRIC_L2);
	for (auto&& filename : serialization) {
		FILE* fd = std::fopen(filename.c_str(), "r");
		auto new_segment = std::make_shared<SegmentFLatType>(fd);
		index_copy.push_segment(new_segment);
		std::fclose(fd);
		std::remove(filename.c_str());
	}
	std::vector<float> q = {1.5f, 1.5f};
	auto res = index_copy.search(q, 2);
	EXPECT_TRUE(check_meta(res, {"vec1", "vec2"}));
	q = {3.5f, 3.5f};
	res = index_copy.search(q, 2);
	EXPECT_TRUE(check_meta(res, {"vec3", "vec4"}));
}

TEST(VecodexIndexTest, IBaseIndex) {
	IndexFlatType index(2, 2, std::nullopt, 2, faiss::MetricType::METRIC_L2);
	vecodex::IBaseIndex<std::string>* base_index =
		(vecodex::IBaseIndex<std::string>*)&index;

	float vectors[2][2] = {{1.0f, 2.0f}, {2.0f, 3.1f}};
	std::vector<std::string> ids = {"vec1", "vec2"};
	base_index->add(2, ids.data(), (float*)vectors);
	std::vector<float> query = {1.5f, 2.5f};
	std::vector<std::string> results = base_index->search(query, 1);

	// Verify the search result

	EXPECT_TRUE(check_meta(results, {"vec1"}));
}

TEST(VecodexIndexTest, Basic) {
	const size_t dim = 100;
	const size_t threshold = 1000;
	IndexHNSWType index_hnsw(dim, threshold, std::nullopt, dim, 2,
							 faiss::MetricType::METRIC_L2);

	IndexFlatType index_flat(dim, threshold, std::nullopt, dim,
							 faiss::MetricType::METRIC_L2);
	const size_t vec_num = 3;
	const float max_num = 10;
	const int k = 3;
	std::vector<float> vectors(vec_num * dim);
	std::vector<float> search_vec(dim);
	std::vector<std::string> ids(vec_num);
	size_t tests = 4000;
	long long sum_flat = 0;
	long long sum_hnsw = 0;
	size_t sz_hnsw = 0;
	size_t sz_flat = 0;
	for (size_t i = 0; i < tests; ++i) {
		for (size_t j = 0; j < vectors.size(); ++j) {
			vectors[j] = static_cast<float>(std::rand()) /
						 (static_cast<float>(RAND_MAX / max_num));
		}
		for (size_t j = 0; j < ids.size(); ++j) {
			ids[j] = "vec" + std::to_string(i) + "#" + std::to_string(j);
		}

		index_flat.add(ids.size(), ids.data(), vectors.data());
		index_hnsw.add(ids.size(), ids.data(), vectors.data());

		for (size_t j = 0; j < search_vec.size(); ++j) {
			search_vec[j] = static_cast<float>(std::rand()) /
							(static_cast<float>(RAND_MAX / max_num));
		}
		auto start = std::chrono::high_resolution_clock::now();
		auto flat_res = index_flat.search(search_vec, k);
		auto end = std::chrono::high_resolution_clock::now();
		sum_flat +=
			std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
				.count();

		start = std::chrono::high_resolution_clock::now();
		auto hnsw_res = index_hnsw.search(search_vec, k);
		end = std::chrono::high_resolution_clock::now();
		sum_hnsw +=
			std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
				.count();
		sz_flat += flat_res.size();
		sz_hnsw += hnsw_res.size();
		EXPECT_TRUE(sz_flat == sz_hnsw);
	}
	std::cout << std::fixed << "flat: " << static_cast<float>(sum_flat) / tests
			  << "ns, hnsw: " << static_cast<float>(sum_hnsw) / tests << "ns\n";
}
int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
