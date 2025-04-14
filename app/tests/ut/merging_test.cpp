#include "gtest/gtest.h"
#include "service.pb.h"
#include "Merging.h"
#include <string>
#include <vector>

using service::SearchRequest;
using service::SearchResponse;

namespace service {

bool operator==(const SearchResponse& first, const SearchResponse& second) {
    if (first.ids().size() != second.ids().size()) {
        return false;
    }

    auto it_first = first.ids().begin();
    auto it_second = second.ids().begin();
    while (it_first != first.ids().end()) {
        if (*it_first != *it_second) {
            return false;
        }
        ++it_first;
        ++it_second;
    }

    return true;
}

} // namespace service

TEST(vecodex_coordinator_merge, zero_response) {
    SearchResponse fake;
    SearchResponse fake2;
    auto merged = MergeSearcherAnswers(std::vector{fake}, 47);
    EXPECT_EQ(merged, fake);
}

TEST(vecodex_coordinator_merge, one_response) {
    SearchResponse fake;
    size_t k = 50;
    for (size_t i = 0; i < k; ++i) {
        fake.mutable_ids()->Add(std::to_string(i));
    }

    auto merged = MergeSearcherAnswers(std::vector{fake}, k);
    EXPECT_EQ(merged, fake);

    merged = MergeSearcherAnswers(std::vector{fake}, k * 2);
    EXPECT_EQ(merged, fake);

    merged = MergeSearcherAnswers(std::vector{fake}, k / 2);
    for (size_t i = 0; i < k / 2; ++i) {
        EXPECT_EQ(merged.ids().at(i), fake.ids().at(i));
    }
}

TEST(vecodex_coordinator_merge, few_response) {
    size_t responses = 3;
    size_t k = 5;
    auto fakes = std::vector<SearchResponse>(responses);
    for (size_t resp = 0; resp < responses; ++resp) {
        for (size_t i = 0; i < k; ++i) {
            size_t fake_id = (resp + i) % k;
            fakes[resp].mutable_ids()->Add(std::to_string(fake_id));
        }
    }

    auto merged = MergeSearcherAnswers(fakes, k);
    auto right_order = std::vector<std::string> {"2", "1", "3", "0", "4"};
    SearchResponse right_answer;
    right_answer.mutable_ids()->Assign(right_order.begin(), right_order.end());

    EXPECT_EQ(merged, right_answer);
}