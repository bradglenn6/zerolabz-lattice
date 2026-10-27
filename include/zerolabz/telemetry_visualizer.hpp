#pragma once

#include <string>
#include <sstream>
#include <atomic>
#include <chrono>

namespace zerolabz {

class LatticeTelemetryExporter {
public:
    static LatticeTelemetryExporter& instance() noexcept {
        static LatticeTelemetryExporter inst;
        return inst;
    }

    void record_vector_op(uint64_t cycles, bool pq_verified = true) noexcept {
        total_ops_.fetch_add(1, std::memory_order_relaxed);
        last_cycles_.store(cycles, std::memory_order_relaxed);
        if (pq_verified) {
            pq_attested_ops_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // Exports standard OpenMetrics / Prometheus format
    [[nodiscard]] std::string export_openmetrics() const {
        std::ostringstream ss;
        ss << "# HELP zerolabz_lattice_vector_ops_total Total vector operations processed\n";
        ss << "# TYPE zerolabz_lattice_vector_ops_total counter\n";
        ss << "zerolabz_lattice_vector_ops_total " << total_ops_.load() << "\n\n";

        ss << "# HELP zerolabz_lattice_execution_cpu_cycles Last execution latency in CPU cycles\n";
        ss << "# TYPE zerolabz_lattice_execution_cpu_cycles gauge\n";
        ss << "zerolabz_lattice_execution_cpu_cycles " << last_cycles_.load() << "\n\n";

        ss << "# HELP zerolabz_lattice_pq_attested_ops_total Total operations passing PQ-ZTA verification\n";
        ss << "# TYPE zerolabz_lattice_pq_attested_ops_total counter\n";
        ss << "zerolabz_lattice_pq_attested_ops_total " << pq_attested_ops_.load() << "\n";

        return ss.str();
    }

private:
    LatticeTelemetryExporter() = default;
    std::atomic<uint64_t> total_ops_{0};
    std::atomic<uint64_t> last_cycles_{0};
    std::atomic<uint64_t> pq_attested_ops_{0};
};

} // namespace zerolabz
