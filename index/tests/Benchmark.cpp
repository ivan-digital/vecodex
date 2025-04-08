#include <iostream>
#include <random>
#include "Index.h"
#include "faiss.h"

using IndexHNSWType =
	vecodex::Index<baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>>;
using IndexFlatType =
	vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, std::string>>;

int main(int argc, char** argv) {
	const std::string characters =
		"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::random_device rd;
	std::mt19937_64 gen(rd());
	if (argc < 5) {
		throw std::runtime_error("Too few arguments");
		return 1;
	}
	int N = std::atoi(argv[1]);	 // amount of vectors
	int Q = std::atoi(argv[2]);	 // amount queries
	int k = std::atoi(argv[3]);	 // top-k answer
	int dim = atoi(argv[4]);	 // vector dim
	std::uniform_int_distribution<>(0, N);
	std::uniform_int_distribution<> dis(0, N);
	std::uniform_real_distribution<> dis_real(0.0, 10.0);
	IndexFlatType flat_index(dim, 100, dim, faiss::MetricType::METRIC_L2);
	IndexHNSWType hnsw_index(dim, 100, dim, 32, faiss::MetricType::METRIC_L2);

	int added = 0;
	int queried = 0;
	while (added < N) {
		int n = dis(gen) % (N - added + 1);
		int q = dis(gen) % (Q - queried + 1);
		float* add_vectors = new float[n * dim];
		std::string* ids = new std::string[n];
		float* query_vector = new float[dim];
		for (int i = 0; i < n * dim; ++i) {
			add_vectors[i] = dis_real(gen);
		}
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < 15; ++j) {
				ids[i].push_back(characters[dis(gen) % characters.size()]);
			}
		}
		for (int i = 0; i < dim; ++i) {
			query_vector[i] = dis_real(gen);
		}
		float* flat_ans = new float[k * dim];
		float* hnsw_ans = new float[k * dim];

		flat_index.add(n, ids, add_vectors);
		hnsw_index.add(n, ids, add_vectors);

		delete[] ids;
		delete[] add_vectors;
		delete[] query_vector;
		delete[] flat_ans;
		delete[] hnsw_ans;
	}
}