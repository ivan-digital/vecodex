#include <iostream>
#include <random>
#include <chrono>
#include <fstream>
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
	IndexFlatType flat_index(dim, 20, dim, faiss::MetricType::METRIC_L2);
	IndexHNSWType hnsw_index(dim, 20, dim, 32, faiss::MetricType::METRIC_L2);


	int added = 0;
	int queried = 0;
	std::ofstream flat_stat("flat_stat.csv");
	std::ofstream hnsw_stat("hnsw_stat.csv");
	flat_stat << "stat\n";
	hnsw_stat << "stat\n";
	while (added < N) {
		int n = dis(gen) % (N - added + 1);
		int q = dis(gen) % (Q - queried + 1);
		float* add_vectors = new float[n * dim];
		std::string* ids = new std::string[n];
		for (int i = 0; i < n * dim; ++i) {
			add_vectors[i] = dis_real(gen);
		}
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < 15; ++j) {
				ids[i].push_back(characters[dis(gen) % characters.size()]);
			}
		}

		auto start = std::chrono::high_resolution_clock::now();
		flat_index.add(n, ids, add_vectors);
		auto finish = std::chrono::high_resolution_clock::now();
		flat_stat << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << "\n";
		start = std::chrono::high_resolution_clock::now();
		hnsw_index.add(n, ids, add_vectors);
		finish = std::chrono::high_resolution_clock::now();
		hnsw_stat << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << "\n";
		added++;
		delete[] ids;
		delete[] add_vectors;
	}
	flat_stat.close();
	hnsw_stat.close();
	flat_stat.open("flat_srch.csv");
	hnsw_stat.open("hnsw_srch.csv");
	flat_stat << "stat\n";
	hnsw_stat << "stat\n";
	while (queried < Q) {
		queried++;
		int q = dis(gen) % (Q - queried + 1);
		std::vector<float> query_vector(dim);
		for (int i = 0; i < dim; ++i) {
			query_vector[i] = dis_real(gen);
		}

		auto start = std::chrono::high_resolution_clock::now();
		auto ans_flat = flat_index.search(query_vector, k);
		auto finish = std::chrono::high_resolution_clock::now();
		flat_stat << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << "\n";

		start = std::chrono::high_resolution_clock::now();
		auto ans_hnsw = hnsw_index.search(query_vector, k);
		finish = std::chrono::high_resolution_clock::now();
		hnsw_stat << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << "\n";
	}
}