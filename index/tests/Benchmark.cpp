#include <iostream>
#include <random>
#include <chrono>
#include <fstream>
#include <cmath>
#include "Index.h"
#include "faiss.h"
#include <faiss/MetricType.h>

using IndexHNSWType =
	vecodex::Index<baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>>;
using IndexFlatType =
	vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, std::string>>;
int counter_callback = 0;
void update_callback(
	std::vector<size_t>&& ids,
	std::vector<std::shared_ptr<vecodex::ISegment<std::string>>>&& segs) {
	counter_callback++;
}
double dist(const std::vector<double>& a) {
	double ans = 0;
	for (double x : a) {
		ans += x * x;
	}
	return std::sqrt(ans);
}


int main(int argc, char** argv) {
	const std::string characters =
		"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::random_device rd;
	std::mt19937_64 gen(rd());
	bool enable_merge = false;
	if (argc < 5) {
		throw std::runtime_error("Too few arguments");
		return 1;
	}
	int N = std::atoi(argv[1]);	 // amount of vectors
	int Q = std::atoi(argv[2]);	 // amount queries
	int k = std::atoi(argv[3]);	 // top-k answer
	int dim = atoi(argv[4]);	 // vector dim
	if (argc > 5 && atoi(argv[5]) == 1) {
		enable_merge = true;
	}
	std::uniform_int_distribution<>(0, N);
	std::uniform_int_distribution<> dis(0, N);
	std::uniform_real_distribution<> dis_real(0.0, 10.0);
	IndexFlatType flat_index(dim, 100, enable_merge, dim, faiss::MetricType::METRIC_L2);
	IndexHNSWType hnsw_index(dim, 100, enable_merge, dim, 2, faiss::MetricType::METRIC_L2);
	hnsw_index.setUpdateCallback(update_callback);

	int batch = 20;

	int added = 0;
	int queried = 0;
	std::ofstream flat_stat("flat_stat_" + std::to_string(enable_merge) + ".csv");
	std::ofstream hnsw_stat("hnsw_stat_" + std::to_string(enable_merge) + ".csv");
	flat_stat << "stat,n\n";
	hnsw_stat << "stat,n\n";
	std::unordered_map<std::string, std::vector<double> > data;
	while (added < N) {
		int n = batch;
		added += n;
		std::cout << added << "/" << N << "\n";
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
			std::vector<double> v(add_vectors + i * dim, add_vectors + (i + 1) * dim);
			data[ids[i]] = v;
		}

		auto start = std::chrono::high_resolution_clock::now();
		flat_index.add(n, ids, add_vectors);
		auto finish = std::chrono::high_resolution_clock::now();
		flat_stat << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << "," << added << "\n";
		start = std::chrono::high_resolution_clock::now();
		hnsw_index.add(n, ids, add_vectors);
		finish = std::chrono::high_resolution_clock::now();
		hnsw_stat << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << "," << added << "\n";
		delete[] ids;
		delete[] add_vectors;
	}
	flat_stat.close();
	hnsw_stat.close();
	flat_stat.open("flat_srch_" + std::to_string(enable_merge) + ".csv");
	hnsw_stat.open("hnsw_srch_" + std::to_string(enable_merge) + ".csv");
	std::ofstream hnsw_recall("hnsw_recall_" + std::to_string(enable_merge) + ".csv");
	std::ofstream flat_recall("flat_recall_" + std::to_string(enable_merge) + ".csv");
	flat_stat << "stat\n";
	hnsw_stat << "stat\n";

	hnsw_recall << "min,max\n";
	flat_recall << "min,max\n";
	while (queried < Q) {
		queried++;
		std::cout << queried << "/" << Q << "\n";
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

		std::sort(ans_flat.begin(), ans_flat.end(), [&](const std::string& a, const std::string& b) {
			return dist(data[a]) < dist(data[b]);
		});

		std::sort(ans_hnsw.begin(), ans_hnsw.end(), [&](const std::string& a, const std::string& b) {
			return dist(data[a]) < dist(data[b]);
		});

		hnsw_recall << dist(data[ans_hnsw[0]]) << "," << dist(data[ans_hnsw.back()]) << "\n";
		flat_recall << dist(data[ans_flat[0]]) << "," << dist(data[ans_flat.back()]) << "\n";
	}
	std::cout << "counter: " << counter_callback << "\n";
}
