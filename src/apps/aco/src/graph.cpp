#include "graph.h"
#include <cmath>
#include <fstream>
#include <functional>
#include <sstream>
#include <tbb/tbb.h>

namespace aco {

namespace {
inline void ClearLine(std::string &line) {
  line.erase(0, line.find_first_not_of("\t "));
  line.erase(line.find_last_not_of("\t ") + 1);
}
}  // namespace

void Graph::ConstructGraphFromSimpleInput(std::istream &stream) {
  WeightType weight{1};
  for (std::string line; std::getline(stream, line);) {
    ClearLine(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::istringstream ss{std::move(line)};

    if (NodeIdType node1, node2; ss >> node1 >> node2) {
      ss >> weight;
      PushEdge(node1, node2, std::max<WeightType>(weight, 1));
    }
  }
}

void Graph::ConstructGraphFromGmlInput(std::istream &stream) {
  bool graph_flag{false};
  bool node_flag{false};
  bool edge_flag{false};
  NodeIdType node1{-1};
  NodeIdType node2{-1};

  for (std::string line, token; std::getline(stream, line);) {
    ClearLine(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::istringstream ss(std::move(line));
    ss >> token;

    if (token == "graph") {
      graph_flag = true;
    } else if (token == "node") {
      node_flag = true;
      edge_flag = false;
    } else if (token == "edge") {
      node_flag = false;
      edge_flag = true;
    } else if (token == "[") {
      continue;
    } else if (token == "]") {
      if (node_flag) {
        node_flag = false;
      } else if (edge_flag) {
        if (node1 != -1 && node2 != -1) {
          PushEdge(node1, node2);
          node1 = -1;
          node2 = -1;
        }
        edge_flag = false;
      } else if (graph_flag) {
        // graph_flag = false;
        break;
      }
    } else if (edge_flag && token == "source") {
      ss >> node1;
    } else if (edge_flag && token == "target") {
      ss >> node2;
    }
  }
}

void Graph::ConstructFromFile(std::string const &file_name, std::function<void(std::istream &)> const &f) {
  std::ifstream file_stream(file_name);
  if (!file_stream.is_open()) {
    std::cerr << "Error. Failed to open file " << file_name << '\n';
  } else {
    f(file_stream);
    file_stream.close();
  }
}

void Graph::ConstructGraphFromSimpleFile(std::string const &file_name) {
  ConstructFromFile(file_name, [this](std::istream &stream) { ConstructGraphFromSimpleInput(stream); });
}

void Graph::ConstructGraphFromGmlFile(std::string const &file_name) {
  ConstructFromFile(file_name, [this](std::istream &stream) { ConstructGraphFromGmlInput(stream); });
}

void Graph::PushEdge(NodeIdType node1, NodeIdType node2, WeightType weight) noexcept {
  if (node1 != node2) {  // не петля
    if (node1 >= nodes_size_) {
      nodes_size_ = node1 + 1;
    }
    if (node2 >= nodes_size_) {
      nodes_size_ = node2 + 1;
    }

    adjacency_list_[node1].PushArc(node2, weight);
    adjacency_list_[node2].PushArc(node1, weight);

    ++edges_size_;
    edges_weight_ += weight;
  }
}

void Graph::FillNodesIdsArray() {
  if (0 == nodes_size_) {
    return;
  }

  nodes_ids_.clear();
  nodes_ids_.reserve(static_cast<NodesIdsArray::size_type>(nodes_size_));

  for (auto const &e : adjacency_list_) {
    nodes_ids_.emplace_back(e.first);
  }
}

}  // namespace aco
