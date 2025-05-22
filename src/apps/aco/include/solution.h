#pragma once
#include <cstdint>
#include <unordered_map>

#include "defines.h"
#include "graph.h"

/**
 * \brief Solution for community detection(nodes ids to communities ids + modularity (Q))
 */

namespace aco {

struct Solution {
  //template<class Traits> friend class CommunityDetection;
  friend class CommunityDetection;

  using NodeIdType = Graph::NodeIdType;
  using CommunityIdType = Graph::IdType;
  using Communities = std::unordered_map<NodeIdType, CommunityIdType>;

  explicit Solution() noexcept(false) = default;
  explicit Solution(Graph::AdjacencyList const &adjacency_list);

  Solution(const Solution &other);
  Solution &operator=(const Solution &rhs);
  Solution(Solution &&other) noexcept ;
  Solution &operator=(Solution &&rhs) noexcept ;

  static void InitCommunities(Solution &solution, Graph::AdjacencyList const &adjacency_list);
  static void SetModularity(Solution &solution, double modularity);

 public:
  Communities communities;
  double q_modularity{0.0};
};

}