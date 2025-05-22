#pragma once
#include <cstdint>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "defines.h"

/**
 * \brief Non-oriented graph implementation (based on adjacency list)
 * \todo  Добавить веса рёбер
 */

namespace aco {
class Graph {
 public:
  using IdType = int64_t;
  using SizeType = IdType;
  using NodeIdType = IdType;
  using WeightType = int64_t;

  struct Arc {
    NodeIdType to_node_id;
    WeightType weight{1};
  };

  using Arcs = std::vector<Arc>;

  struct Node {
    Arcs arcs_;
    WeightType weight_{0};

    void PushArc(NodeIdType node2, WeightType weight = 1) {
      arcs_.emplace_back(node2, weight);
      weight_ += weight;
    }
  };

  // using AdjacencyList = std::unordered_map<NodeIdType, std::vector<NodeIdType>>;
  using AdjacencyList = std::unordered_map<NodeIdType, Node>;
  using NodesIdsArray = std::vector<NodeIdType>;

  explicit Graph() noexcept(false) = default;
  /*virtual*/ ~Graph() = default;

  Graph(Graph const &other) = default;
  Graph &operator=(Graph const &rhs) = delete;
  Graph(Graph &&other) = default;
  Graph &operator=(Graph &&rhs) = delete;

  void FillNodesIdsArray();

  void ConstructGraphFromSimpleInput(std::istream &);
  void ConstructGraphFromGmlInput(std::istream &);

  static void ConstructFromFile(std::string const &, std::function<void(std::istream &)> const &);
  void ConstructGraphFromSimpleFile(std::string const &);
  void ConstructGraphFromGmlFile(std::string const &);

  const AdjacencyList &GetAdjacencyList() const noexcept { return adjacency_list_; }
  const NodesIdsArray &GetNodesIdsArray() const noexcept { return nodes_ids_; }
  SizeType NodesSize() const noexcept { return nodes_size_; }
  SizeType EdgesSize() const noexcept { return edges_size_; }
  WeightType EdgesWeight() const noexcept { return edges_weight_; }

 private:
  void PushEdge(NodeIdType node1, NodeIdType node2, WeightType weight = 1) noexcept;

  AdjacencyList adjacency_list_;
  NodesIdsArray nodes_ids_; //нужно для random_iterator для omp(разбиение циклов но потокам)
  SizeType nodes_size_{0};
  SizeType edges_size_{0};
  WeightType edges_weight_{0};
};

}  // namespace aco
