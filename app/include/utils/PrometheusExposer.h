#pragma once

#include <prometheus/registry.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/exposer.h>

#include <map>
#include <string>
#include <mutex>
#include <optional>

class PrometheusExposer {
    using Labels = std::map<std::string, std::string>;
public:
    PrometheusExposer(const std::string& bind_address = "0.0.0.0:8080")
        : exposer(bind_address), registry(std::make_shared<prometheus::Registry>()) {
        exposer.RegisterCollectable(registry);
    }

    void AddCounter(const std::string& name, const std::string& help, const Labels& labels = {}) {
        std::lock_guard guard(lock);
        if (counter_families.find(name) == counter_families.end()) {
            auto& family = CreateCounterFamily(*registry, name, help);
            counter_families[name] = &family;

            family.Add(labels);
        }
    }

    template<class Value>
    void IncrementCounter(const std::string& name, Value value, const Labels& labels = {}) {
        {
            std::lock_guard guard(lock);
            if (counter_families.find(name) != counter_families.end()) {
                counter_families[name]->Add(labels).Increment(value);
                return;
            }
        }

        AddCounter(name, "", labels);

        std::lock_guard guard(lock);
        counter_families[name]->Add(labels).Increment(value);
    }

    template<class Value>
    std::optional<Value> GetCounterValue(const std::string& name, const Labels& labels = {}) {
        std::lock_guard guard(lock);
        auto it = counter_families.find(name);
        if (it == counter_families.end()) {
            return std::nullopt;
        }

        try {
            return it->second->Add(labels).Value();
        }
        catch (...) {
            return std::nullopt;
        }
    }

    void AddGauge(const std::string& name, const std::string& help, const Labels& labels = {}) {
        std::lock_guard guard(lock);
        if (gauge_families.find(name) == gauge_families.end()) {
            auto& family = CreateGaugeFamily(*registry, name, help);
            gauge_families[name] = &family;

            family.Add(labels);
        }
    }

    template<class Value>
    void SetGauge(const std::string& name, Value value, const Labels& labels = {}) {
        {
            std::lock_guard guard(lock);
            if (gauge_families.find(name) != gauge_families.end()) {
                gauge_families[name]->Add(labels).Set(value);
                return;
            }
        }
        AddGauge(name, "", labels);

        std::lock_guard guard(lock);
        gauge_families[name]->Add(labels).Set(value);
    }

    template<class Value>
    std::optional<Value> GetGaugeValue(const std::string& name, const Labels& labels = {}) {
        std::lock_guard guard(lock);
        auto it = gauge_families.find(name);
        if (it == gauge_families.end()) {
            return std::nullopt;
        }

        try {
            return it->second->Add(labels).Value();
        }
        catch (...) {
            return std::nullopt;
        }
    }

private:
    prometheus::Family<prometheus::Counter>& CreateCounterFamily(
        prometheus::Registry& registry,
        const std::string& name,
        const std::string& help) {
        return prometheus::BuildCounter()
            .Name(name)
            .Help(help)
            .Register(registry);
    }

    prometheus::Family<prometheus::Gauge>& CreateGaugeFamily(
        prometheus::Registry& registry,
        const std::string& name,
        const std::string& help) {
        return prometheus::BuildGauge()
            .Name(name)
            .Help(help)
            .Register(registry);
    }

private:
    std::shared_ptr<prometheus::Registry> registry;
    prometheus::Exposer exposer;

    std::unordered_map<std::string, prometheus::Family<prometheus::Counter>*> counter_families;
    std::unordered_map<std::string, prometheus::Family<prometheus::Gauge>*> gauge_families;

    std::mutex lock;
};
