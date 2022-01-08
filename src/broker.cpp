#include "crps/broker.h"

#include <chrono>
#include <nlohmann/json.hpp>
#include <thread>

namespace crps {

Broker::Broker(std::string p_listen_address, OS* p_os, Network* p_network, Network::Protocol p_protocol, int16_t p_port)
    : m_listen_address(std::move(p_listen_address) + ":" + std::to_string(p_port)),
      m_os(p_os),
      m_network(p_network),
      m_protocol(p_protocol) {
}

bool Broker::start() {
  if (!m_network->socket(m_protocol)) {
    return false;
  }

  if (!m_network->bind(m_listen_address)) {
    return false;
  }

  if (!m_network->listen()) {
    return false;
  }

  return true;
}

void Broker::spin() {
  using namespace std::chrono_literals;
  const size_t buffer_size{64 * 1024};  // 64K is maximum UDP datagram size
  std::array<unsigned char, buffer_size> buffer{};

  // TODO(ahb)
  // Current implementation:
  //   Receive packet and directly send it to all subscribers.
  // (Likely) better implementation:
  //   Use two threads. Thread A moves incoming packets into the topic queues. Thread B processes the topic queues
  //   and sends out data to the subscribes in order of priority.
  for (;;) {
    std::string client;
    ssize_t bytes_received_signed = m_network->recvfrom(buffer.data(), buffer_size, true, &client);
    CRPS_LOGGER_DEBUG(m_os, << "received " << bytes_received_signed << " bytes from " << client << "\n");
    if (bytes_received_signed < 0) {
      m_os->logger().error() << "recvfrom() failed. This should not happen! Continuing.\n";
      continue;
    }
    auto bytes_received{static_cast<size_t>(bytes_received_signed)};

    BpType bp_type;
    BpHeader bp_header{};
    BpControlHeader bp_control_header{};
    BpDataHeader bp_data_header{};
    json control_json{};
    {
      auto logger{m_os->logger()};
      bp_type = decode_packet_buffer(buffer.data(), bytes_received, &bp_header, &bp_control_header, &bp_data_header,
                                     &control_json, &logger);
    }

    if (!verify_and_update_client_counter(client, bp_header.counter)) {
      continue;
    }

    if (bp_type == BpType::Invalid) {
      m_os->logger().error() << "decode_packet_buffer() from " << client
                             << " failed. Ignoring packet and continuing.\n";
    } else if (bp_type == BpType::Control) {
      if (!send_control_response(client, process_control_request(client, control_json))) {
        m_os->logger().error() << "Failed to send control response.\n";
        break;
      }
    } else if (bp_type == BpType::Data) {
      TopicId topic_id = bp_data_header.topic_id;
      CRPS_LOGGER_DEBUG(m_os, << "Received data packet for topic " << topic_id << " ('" << m_topics[topic_id].name
                              << "') with message type " << bp_data_header.message_type_id << " from " << client
                              << ".\n");
      if (topic_id >= m_topics.size()) {
        m_os->logger().error() << "Received data packet with invalid topic ID " << topic_id << " from " << client
                               << ". Ignoring packet and continuing.\n";
        continue;
      }
      auto& topic = m_topics[topic_id];
      if (bp_data_header.message_type_id >= m_message_types.size()) {
        m_os->logger().error() << "Received data packet with invalid message type ID " << bp_data_header.message_type_id
                               << " from " << client << ". Ignoring packet and continuing.\n";
        continue;
      }
      if (topic.message_type_id != bp_data_header.message_type_id) {
        m_os->logger().error() << "Received data packet for topic " << topic_id << " with wrong message type ID "
                               << bp_data_header.message_type_id << " from " << client
                               << ". Ignoring packet and continuing.\n";
        continue;
      }
      // auto& message_type = m_message_types[bp_data_header.message_type_id];
      for (auto subscriber_node_id : topic.subscribers) {
        auto& subscriber_node = m_nodes[subscriber_node_id];
        if (!send_data(subscriber_node.address, buffer.data(), bytes_received)) {
          m_os->logger().error() << "Failed to send message on topic " << topic_id << " ('" << topic.name
                                 << "') to node " << subscriber_node_id << " ('" << subscriber_node.name
                                 << "'). Continuing.\n";
        }
      }
    }
  }
}

bool Broker::verify_and_update_client_counter(const std::string& p_client_address, BpCounter p_counter) {
  if (m_client_counters.find(p_client_address) == m_client_counters.end()) {  // new client
    m_client_counters[p_client_address] = p_counter;
  } else if (m_client_counters[p_client_address] + 1 != p_counter) {
    m_os->logger().error() << "BpHeader::counter from " << p_client_address << " is wrong. Expected "
                           << m_client_counters[p_client_address] + 1 << " but got " << p_counter
                           << ". This indicates lost packets! Ignoring packet and continuing.\n";
    return false;
  }
  m_client_counters[p_client_address] = p_counter;
  return true;
}

json Broker::process_control_request(const std::string& p_client_address, const json& p_request) {
  json response{};
  if (p_request["scope"] == "node") {
    if (p_request["node"]["action"] == "register") {
      BpNodeId node_id{};
      if (!register_node(p_client_address, p_request["node"]["params"]["name"], &node_id)) {
        return rpc_failure_response(p_request["rpc_id"], 0, "Failed to register node");
      }
      response.clear();
      response["rpc_id"] = p_request["rpc_id"];
      response["success"] = true;
      response["node_id"] = node_id;
    } else {
      // TODO(ahb)
      m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    }
  } else if (p_request["scope"] == "message") {
    if (p_request["message"]["action"] == "register") {
      MessageTypeId message_type_id{};
      if (!register_message_type(p_request["message"]["params"]["name"], p_request["message"]["params"]["size"],
                                 &message_type_id)) {
        return rpc_failure_response(p_request["rpc_id"], 0, "Failed to register message type");
      }
      response.clear();
      response["rpc_id"] = p_request["rpc_id"];
      response["success"] = true;
      response["message_type_id"] = message_type_id;
    } else {
      // TODO(ahb)
      m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    }
  } else if (p_request["scope"] == "topic") {
    if (p_request["topic"]["action"] == "register") {
      TopicId topic_id{};
      TopicPriority actual_topic_priority{};
      if (!register_topic(p_request["topic"]["params"]["name"], p_request["topic"]["params"]["message_type_id"],
                          p_request["topic"]["params"]["priority"], &topic_id, &actual_topic_priority)) {
        return rpc_failure_response(p_request["rpc_id"], 0, "Failed to register topic");
      }
      response.clear();
      response["rpc_id"] = p_request["rpc_id"];
      response["success"] = true;
      response["topic_id"] = topic_id;
      response["topic_priority"] = actual_topic_priority;
    } else if (p_request["topic"]["action"] == "add_publisher") {
      if (!add_publisher(p_request["topic"]["params"]["node_id"], p_request["topic"]["params"]["topic_id"],
                         p_request["topic"]["params"]["topic_id"])) {
        return rpc_failure_response(p_request["rpc_id"], 0, "Failed to add publisher");
      }
      response.clear();
      response["rpc_id"] = p_request["rpc_id"];
      response["success"] = true;
    } else if (p_request["topic"]["action"] == "add_subscriber") {
      if (!add_subscriber(p_request["topic"]["params"]["node_id"], p_request["topic"]["params"]["topic_id"],
                          p_request["topic"]["params"]["topic_id"])) {
        return rpc_failure_response(p_request["rpc_id"], 0, "Failed to add subscriber");
      }
      response.clear();
      response["rpc_id"] = p_request["rpc_id"];
      response["success"] = true;
    } else {
      // TODO(ahb)
      m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    }
  } else {
    // TODO(ahb)
    m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
  }
  return response;
}

bool Broker::register_node(const std::string& p_client_address, const std::string& p_node_name, BpNodeId* p_node_id) {
  auto node_info = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const auto& ni) { return ni.name == p_node_name; });
  if (node_info == m_nodes.end()) {
    m_nodes.emplace_back(NodeInfo{p_node_name, p_client_address});
    *p_node_id = m_nodes.size() - 1;
    m_os->logger().info() << "Registered node '" << p_node_name << "' at address " << p_client_address
                          << " with node ID " << *p_node_id << ".\n";
    return true;
  }

  m_os->logger().warn() << "Cannot register already registered node '" << p_node_name << "'.\n";
  return false;
}

bool Broker::register_message_type(const std::string& p_message_type_name, MessageSize p_message_size,
                                   MessageTypeId* p_message_type_id) {
  auto message_type_info = std::find_if(m_message_types.begin(), m_message_types.end(),
                                        [&](const auto& mti) { return mti.name == p_message_type_name; });
  if (message_type_info == m_message_types.end()) {
    m_message_types.emplace_back(MessageTypeInfo{p_message_type_name, p_message_size});
    *p_message_type_id = m_message_types.size() - 1;
    m_os->logger().info() << "Registered new message type '" << p_message_type_name << "' of size " << p_message_size
                          << " with message ID " << *p_message_type_id << ".\n";
  } else {
    MessageTypeId message_type_id = message_type_info - m_message_types.begin();
    if (message_type_info->size != p_message_size) {
      m_os->logger().error() << "Cannot register already registered message type '" << p_message_type_name
                             << "' (ID: " << message_type_id
                             << "), but with different size (already registered size: " << message_type_info->size
                             << "; new registration size:" << p_message_size << ").\n";
      return false;
    }
    *p_message_type_id = message_type_id;
  }
  return true;
}

bool Broker::register_topic(const std::string& p_topic_name, MessageTypeId p_message_type_id,
                            TopicPriority p_topic_priority, TopicId* p_topic_id,
                            TopicPriority* p_actual_topic_priority) {
  if (p_message_type_id >= m_message_types.size()) {
    m_os->logger().error() << "Cannot register new topic '" << p_topic_name << "' of unknown message type "
                           << p_message_type_id << "\n";
  }
  auto topic = std::find_if(m_topics.begin(), m_topics.end(), [&](const auto& t) { return t.name == p_topic_name; });
  if (topic == m_topics.end()) {
    if (p_topic_priority == dontcare_topic_priority) {
      m_os->logger().error() << "Cannot register new topic '" << p_topic_name << "' with don't-care priority.\n";
      return false;
    }
    m_topics.emplace_back(Topic{p_topic_name, p_message_type_id, p_topic_priority, {}, {}});
    *p_topic_id = m_topics.size() - 1;
    *p_actual_topic_priority = m_topics.back().priority;
    m_os->logger().info() << "Registered new topic '" << p_topic_name << "' of type " << p_message_type_id << " ('"
                          << m_message_types[p_message_type_id].name << "') and priority "
                          << static_cast<int>(p_topic_priority) << " with topic ID " << *p_topic_id << ".\n";
  } else {
    TopicId topic_id = topic - m_topics.begin();
    if (topic->message_type_id != p_message_type_id) {
      m_os->logger().error() << "Cannot register already registered topic '" << p_topic_name << "' (ID: " << topic_id
                             << "), but with different message type (already registered type: "
                             << topic->message_type_id << " ('" << m_message_types[topic->message_type_id].name
                             << "'); new registration type:" << p_message_type_id << " ('"
                             << m_message_types[p_message_type_id].name << "')).\n";
      return false;
    }
    if (p_topic_priority != dontcare_topic_priority && topic->priority != p_topic_priority) {
      m_os->logger().error() << "Cannot register already registered topic '" << p_topic_name << "' (ID: " << topic_id
                             << "), but with different priorities (already registered priority: " << topic->priority
                             << "; new registration priority:" << p_topic_priority << ").\n";
      return false;
    }
    *p_topic_id = topic_id;
    *p_actual_topic_priority = topic->priority;
  }
  return true;
}

bool Broker::add_publisher(BpNodeId p_node_id, TopicId p_topic_id, MessageTypeId p_message_type_id) {
  if (!validate_topic_parameters(p_node_id, p_topic_id, p_message_type_id)) {
    m_os->logger().error() << "Failed adding publisher.\n";
    return false;
  }

  m_topics[p_topic_id].publishers.push_back(p_node_id);
  m_os->logger().info() << "Added publisher from node " << p_node_id << " ('" << m_nodes[p_node_id].name
                        << "') to topic " << p_topic_id << " ('" << m_topics[p_topic_id].name << "')\n";
  return true;
}

bool Broker::add_subscriber(BpNodeId p_node_id, TopicId p_topic_id, MessageTypeId p_message_type_id) {
  if (!validate_topic_parameters(p_node_id, p_topic_id, p_message_type_id)) {
    m_os->logger().error() << "Failed adding subscriber.\n";
    return false;
  }

  m_topics[p_topic_id].subscribers.push_back(p_node_id);
  m_os->logger().info() << "Added subscriber from node " << p_node_id << " ('" << m_nodes[p_node_id].name
                        << "') to topic " << p_topic_id << " ('" << m_topics[p_topic_id].name << "')\n";
  return true;
}

bool Broker::send_control_response(const std::string& p_address, const json& p_response) {
  std::vector<unsigned char> buffer;
  bp_control_json_to_packet_buffer(p_response, m_client_counters[p_address], &buffer);
  CRPS_LOGGER_DEBUG(m_os, << "send_control_response(" << p_address << ", " << p_response.dump() << ") " << buffer.size()
                          << " bytes\n");
  return m_network->sendto(p_address, buffer.data(), buffer.size());
}

bool Broker::send_data(const std::string& p_address, const unsigned char* p_buffer, size_t p_packet_size) {
  CRPS_LOGGER_DEBUG(m_os, << "send_data(" << p_address << ") " << p_packet_size << " bytes\n");
  return m_network->sendto(p_address, p_buffer, p_packet_size);
}

json Broker::rpc_failure_response(int rpc_id, int p_error_code, const std::string& p_error_message) {
  json response{};
  response["rpc_id"] = rpc_id;
  response["success"] = false;
  response["error_code"] = p_error_code;
  response["error_message"] = p_error_message;
  m_os->logger().warn() << "RPC failure: " << response << "\n";
  return response;
}

bool Broker::validate_topic_parameters(BpNodeId p_node_id, TopicId p_topic_id, MessageTypeId p_message_type_id) {
  if (p_node_id >= m_nodes.size()) {
    m_os->logger().error() << "Cannot add pub/sub with unknown node ID " << p_node_id << ".\n";
    return false;
  }
  if (p_topic_id >= m_topics.size()) {
    m_os->logger().error() << "Cannot add pub/sub with unknown topic ID " << p_topic_id << ".\n";
    return false;
  }
  if (p_message_type_id >= m_message_types.size()) {
    m_os->logger().error() << "Cannot add pub/sub with unknown message type ID " << p_message_type_id << ".\n";
    return false;
  }
  auto& topic = m_topics[p_topic_id];
  auto& message_type = m_message_types[topic.message_type_id];
  if (topic.message_type_id != p_message_type_id) {
    m_os->logger().error() << "Cannot add pub/sub to topic " << p_topic_id << " ('" << topic.name
                           << "') with wrong message type (actual topic message type: " << topic.message_type_id
                           << " ('" << message_type.name << "'); requested type: " << p_message_type_id << " ('"
                           << m_message_types[p_message_type_id].name << "'))\n";
    return false;
  }
  return true;
}

}  // namespace crps