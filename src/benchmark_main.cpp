#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "crps/linux.h"
#include "crps/node.h"
#include "demo/parse_options.h"

using json = nlohmann::json;

uint64_t now_timestamp() {
  auto now = std::chrono::steady_clock::now();
  auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto epoch_duration = now_ns.time_since_epoch();
  return epoch_duration.count();
}

class Histogram {
 public:
  explicit Histogram(size_t p_size) : m_values(p_size, 0), m_next_index(0) {
  }
  bool add(uint64_t p_value) {
    if (m_next_index >= m_values.size()) {
      return false;
    }
    m_values[m_next_index] = p_value;
    m_next_index += 1;
    return true;
  }
  json json_data() {
    json data{};
    uint64_t min_value{std::numeric_limits<uint64_t>::max()};
    uint64_t max_value{0};
    double average_value{0};
    for (unsigned i = 0; i < m_next_index; ++i) {
      auto& value = m_values[i];
      if (value < min_value) {
        min_value = value;
      }
      if (value > max_value) {
        max_value = value;
      }
      average_value += value;
    }
    average_value /= m_next_index;
    data["min"] = min_value;
    data["max"] = max_value;
    data["average"] = average_value;
    data["values_count"] = m_next_index;
    data["values"] = std::vector<uint64_t>(m_values.begin(), m_values.begin() + m_next_index);
    return data;
  }

 private:
  std::vector<uint64_t> m_values;
  unsigned m_next_index;
};

class BenchmarkNode {
 public:
  struct BenchmarkHeader {
    crps::BpNodeId generator_id;
    crps::BpNodeId reflector_id;
    uint64_t generator_timestamp;
    uint64_t counter;
  } __attribute__((packed));

  static const size_t benchmark_header_size = sizeof(BenchmarkHeader);
  static const size_t histogram_size = 1000 * 600;  // 1000 Hz for 600 s (about 5 MB)

  BenchmarkNode(std::string p_broker_ip, std::string p_node_name, std::string p_topic_name,
                std::string p_message_type_name, crps::MessageSize p_message_size, crps::TopicPriority p_topic_priority,
                std::string p_bench_node_type, unsigned p_publish_period_ms, crps::OS* p_os, crps::Network* p_network)
      : m_broker_ip(std::move(p_broker_ip)),
        m_node_name(std::move(p_node_name) + "_" + p_bench_node_type),
        m_topic_name(std::move(p_topic_name)),
        m_message_type_name(std::move(p_message_type_name)),
        m_message_size(p_message_size),
        m_topic_priority(p_topic_priority),
        m_bench_node_type(std::move(p_bench_node_type)),
        m_publish_period_ms(p_publish_period_ms),
        m_os(p_os),
        m_network(p_network) {
  }
  bool run(unsigned p_publish_count = 0);
  void print_results();

 private:
  std::string m_broker_ip;
  std::string m_node_name;
  std::string m_topic_name;
  std::string m_message_type_name;
  crps::MessageSize m_message_size;
  crps::TopicPriority m_topic_priority;
  std::string m_bench_node_type;
  unsigned m_publish_period_ms;
  crps::OS* m_os;
  crps::Network* m_network;
  std::unique_ptr<crps::Node> m_node;
  crps::Publisher* m_publisher = nullptr;
  crps::Subscriber* m_subscriber = nullptr;
  uint64_t m_counter = 0;
  std::map<crps::BpNodeId, Histogram> m_histograms;

  void ping_callback(void* p_message, crps::MessageSize p_message_size, void* p_user_data);
  void pong_callback(void* p_message, crps::MessageSize p_message_size, void* p_user_data);
  void add_latency(crps::BpNodeId p_reflector_id, uint64_t p_latency);
};

bool BenchmarkNode::run(unsigned p_publish_count) {
  if (m_message_size < benchmark_header_size) {
    m_os->logger().error() << "Message size must be at least " << benchmark_header_size << " bytes.\n";
    return false;
  }

  m_node = std::make_unique<crps::Node>(m_node_name, m_broker_ip, m_os, m_network);
  if (m_bench_node_type == "generator") {
    m_publisher =
        m_node->create_publisher(m_topic_name + "_ping", m_message_type_name, m_message_size, m_topic_priority);
    m_subscriber = m_node->create_subscriber(
        m_topic_name + "_pong", m_message_type_name, m_message_size,
        [this](auto&& p1, auto&& p2, auto&& p3) { pong_callback(p1, p2, p3); }, nullptr, m_topic_priority);
  } else if (m_bench_node_type == "reflector") {
    m_publisher =
        m_node->create_publisher(m_topic_name + "_pong", m_message_type_name, m_message_size, m_topic_priority);
    m_subscriber = m_node->create_subscriber(
        m_topic_name + "_ping", m_message_type_name, m_message_size,
        [this](auto&& p1, auto&& p2, auto&& p3) { ping_callback(p1, p2, p3); }, nullptr, m_topic_priority);
  } else {
    m_os->logger().error() << "Unknown bench_node_type '" << m_bench_node_type
                           << "' (valid options are 'generator' and 'reflector').\n";
    return false;
  }
  if (!m_node->connect()) {
    m_os->logger().error() << "Publisher failed to connect. Exiting.\n";
    return false;
  }
  m_os->logger().info() << "Node: node name: '" << m_node->name() << "; node ID: " << m_node->node_id() << ".\n";
  m_os->logger().info() << "Publisher: topic name: '" << m_publisher->topic_name()
                        << "'; topic ID: " << m_publisher->topic_id()
                        << "; topic priority: " << static_cast<int>(m_publisher->topic_priority())
                        << "; message type: " << m_publisher->message_type_name()
                        << "; message type ID: " << m_publisher->message_type_id()
                        << "; message size: " << m_publisher->message_size() << ".\n";
  m_os->logger().info() << "Subscriber: topic name: '" << m_subscriber->topic_name()
                        << "'; topic ID: " << m_subscriber->topic_id()
                        << "; topic priority: " << static_cast<int>(m_subscriber->topic_priority())
                        << "; message type: " << m_subscriber->message_type_name()
                        << "; message type ID: " << m_subscriber->message_type_id()
                        << "; message size: " << m_subscriber->message_size() << ".\n";

  if (m_bench_node_type == "generator") {
    const unsigned char padding{0x42};
    std::vector<unsigned char> buffer(m_message_size, padding);
    for (;;) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      auto* header = reinterpret_cast<BenchmarkHeader*>(buffer.data());
      header->generator_id = m_node->node_id();
      header->generator_timestamp = now_timestamp();
      header->counter = m_counter;
      m_publisher->publish(buffer.data());
      m_counter += 1;

      // TODO(ahb)
      // Should put publish() into separate thread and just have Node::spin() here.
      // However, this requires a thread safe implementation first.
      auto next_iteration_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(m_publish_period_ms);
      crps::Node::SpinOnceResult result{};
      do {  // busy waiting for now
        result = m_node->spin_once();
      } while (((next_iteration_time - std::chrono::steady_clock::now()).count() > 0) &&
               (result == crps::Node::SpinOnceResult::Success || result == crps::Node::SpinOnceResult::NoWork));

      if (p_publish_count != 0 && m_counter >= p_publish_count) {
        m_os->logger().info() << "BenchmarkNode::run() finished publishing " << p_publish_count << " times.\n";
        return true;
      }
    }
  } else {
    m_node->spin();
    m_os->logger().error() << "Node::spin returned.\n";
  }

  return true;
}

void BenchmarkNode::ping_callback(void* p_message, crps::MessageSize p_message_size, void* /* UNUSED */) {
  if (m_message_size != p_message_size) {
    m_os->logger().error() << "ping(): Received message with wrong size (expected: " << m_message_size
                           << "; got: " << p_message_size << ").\n";
    return;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* header = reinterpret_cast<BenchmarkHeader*>(p_message);
  CRPS_LOGGER_DEBUG(m_os, << "ping(): generator_id=" << header->generator_id << " generator_timestamp="
                          << header->generator_timestamp << " counter=" << header->counter << "\n");
  header->reflector_id = m_node->node_id();
  m_publisher->publish(p_message);
}

void BenchmarkNode::pong_callback(void* p_message, crps::MessageSize p_message_size, void* /* UNUSED */) {
  if (m_message_size != p_message_size) {
    m_os->logger().error() << "pong(): Received message with wrong size (expected: " << m_message_size
                           << "; got: " << p_message_size << ").\n";
    return;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* header = reinterpret_cast<BenchmarkHeader*>(p_message);
  uint64_t round_trip_latency = now_timestamp() - header->generator_timestamp;
  CRPS_LOGGER_DEBUG(m_os, << "pong(): generator_id=" << header->generator_id << " reflector_id=" << header->reflector_id
                          << " generator_timestamp=" << header->generator_timestamp << " counter=" << header->counter
                          << "; round trip latency: " << round_trip_latency << " ns.\n");
  add_latency(header->reflector_id, round_trip_latency);
}

void BenchmarkNode::add_latency(crps::BpNodeId p_reflector_id, uint64_t p_latency) {
  if (m_histograms.find(p_reflector_id) == m_histograms.end()) {
    m_histograms.emplace(p_reflector_id, Histogram{histogram_size});
  }
  if (!m_histograms.at(p_reflector_id).add(p_latency)) {
    m_os->logger().error() << "Failed to add latency value for reflector node ID " << p_reflector_id << "\n";
  }
}

void BenchmarkNode::print_results() {
  json result = json::array();
  for (auto& histogram : m_histograms) {
    json histogram_result;
    auto histogram_data = histogram.second.json_data();
    histogram_result["generator_id"] = m_node->node_id();
    histogram_result["reflector_id"] = histogram.first;
    histogram_result["data"] = histogram_data;
    result.emplace_back(histogram_result);
  }
  std::cout << result.dump() << "\n";
}

int main(int argc, char* argv[]) {  // NOLINT(bugprone-exception-escape)
  auto options = parse_options(argc, argv);
  std::string broker_ip{option_or_default(options, "broker_ip", "127.0.0.1")};
  std::string node_name{option_or_default(options, "node_name", "bench_node")};
  std::string topic_name{option_or_default(options, "topic_name", "bench_topic")};
  std::string message_type_name{option_or_default(options, "message_type_name", "bench_type")};
  crps::MessageSize message_size{static_cast<crps::MessageSize>(
      std::stoul(option_or_default(options, "message_size", std::to_string(BenchmarkNode::benchmark_header_size))))};
  crps::TopicPriority topic_priority{
      static_cast<crps::TopicPriority>(std::stoi(option_or_default(options, "topic_priority", "1")))};
  unsigned publish_period_ms{static_cast<unsigned>(std::stoul(option_or_default(options, "publish_period_ms", "500")))};
  unsigned publish_count{static_cast<unsigned>(std::stoul(option_or_default(options, "publish_count", "0")))};
  // valid options are "generator" and "reflector"
  std::string bench_node_type{option_or_default(options, "bench_node_type", "generator")};

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto benchmark_node{std::make_unique<BenchmarkNode>(broker_ip, node_name, topic_name, message_type_name, message_size,
                                                      topic_priority, bench_node_type, publish_period_ms, os.get(),
                                                      network.get())};

  if (!benchmark_node->run(publish_count)) {
    return 1;
  }

  benchmark_node->print_results();

  return 0;
}