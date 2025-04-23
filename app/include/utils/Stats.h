#include <cassert>
#include <ctime>
#include <optional>
#include <chrono>


class StatsManager {
public:
    StatsManager() = default;

    StatsManager(const StatsManager&) = delete;

    StatsManager& operator=(const StatsManager&) = delete;

    StatsManager(StatsManager&&) = default;
    
    StatsManager& operator=(StatsManager&&) = default;

public:
    class ResponseStatsCollector {
        friend class StatsManager;
        
        using Clock = std::chrono::steady_clock;

    private:
        explicit ResponseStatsCollector(StatsManager& man) 
            : manager_(man), time_start_(std::chrono::steady_clock::now()) {}

        std::chrono::time_point<Clock> time_start_;
        std::optional<double> resp_time_{std::nullopt};
        bool is_error_{false};
        StatsManager& manager_;

    public:
        void SetError() {
            is_error_ = true;
        }
    
        ~ResponseStatsCollector() {
            std::chrono::duration<double> duration = std::chrono::steady_clock::now() - time_start_;
            resp_time_ = duration.count();
            manager_.AggregateStats(this);
        }
    };

private:
    void AggregateStats(ResponseStatsCollector* resp_data) {
        responses_cnt_ += 1;  
        if (resp_data->is_error_) {
            ++errors_cnt_;
            return;
        }
        if (resp_data->resp_time_.has_value()) {
            double response_time = resp_data->resp_time_.value();
            average_response_time_ = (average_response_time_ * responses_cnt_ + response_time) / (responses_cnt_ + 1);
        }
    }

public:
    ResponseStatsCollector CreateResponseData() {
        return ResponseStatsCollector(*this);
    }

private:
    double average_response_time_{0};
    size_t responses_cnt_{0};
    size_t errors_cnt_{0}; 

public:
    size_t GetErrorsCount() const {
        return errors_cnt_;
    }

    size_t GetSuccessCount() const {
        assert(responses_cnt_ > errors_cnt_);
        return responses_cnt_ - errors_cnt_;
    }

    size_t GetResponsesCount() const {
        return responses_cnt_;
    }

    double GetAverageResponseTime() const {
        return average_response_time_;
    }
};