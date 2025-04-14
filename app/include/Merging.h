#include <grpc/grpc.h>
#include <grpcpp/server_context.h>
#include "service.pb.h"
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>

using service::SearchRequest;
using service::SearchResponse;

constexpr double RFF_CONST = 60;

inline double GetScore(size_t id) {
    return 1. / (RFF_CONST + static_cast<double>(id) + 1);
}

inline SearchResponse MergeSearcherAnswers(const std::vector<SearchResponse>& responses, const size_t num_requested) {
    
    std::unordered_map<std::string_view, double> doc_to_score;

    for (size_t out = 0; out < responses.size(); ++out) {
        const auto& response = responses[out];
        for (size_t in = 0; in < response.ids().size(); ++in) {
            double score = GetScore(in);
            const auto& id = response.ids().at(in);
            if (doc_to_score.find(id) != doc_to_score.end()) {
                doc_to_score[id] += score;
            } else {
                doc_to_score[id] = score;
            }
        }
    }

    using DocScore = std::pair<std::string_view, double>; 
    std::vector<DocScore> sorted;
    for (const auto& [id, score] : doc_to_score) {
        sorted.emplace_back(std::make_pair(id, score));
    }
    std::sort(sorted.begin(), sorted.end(), [](const DocScore& first, const DocScore& second)->bool {
        return first.second > second.second;
    });

    SearchResponse final_response;
    for (size_t i = 0; i < sorted.size() && i < num_requested; ++i) {
        final_response.add_ids(std::string(std::move(sorted[i].first)));
    }
    return final_response;
}