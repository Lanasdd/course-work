#include "solution.h"
#include <utility>

namespace aco {

void Solution::InitCommunities(Solution &solution, Graph::AdjacencyList const &adjacency_list) {
  for (auto const &adjacency : adjacency_list) {
    // при инициализации решения помещаем каждый узел в своё отдельное сообщество
    solution.communities[adjacency.first] = adjacency.first;
  }
}

void Solution::SetModularity(Solution &solution, double modularity) { solution.q_modularity = modularity; }

Solution::Solution(Graph::AdjacencyList const &adjacency_list) { InitCommunities(*this, adjacency_list); }

Solution::Solution(Solution const &other) { *this = other; }

Solution &Solution::operator=(Solution const &rhs) {
  if (this != &rhs) {
    communities = rhs.communities;
    q_modularity = rhs.q_modularity;
  }
  return *this;
}

Solution::Solution(Solution &&other) noexcept
    : communities{std::move(other.communities)}, q_modularity{std::exchange(other.q_modularity, 0)} {}

Solution &Solution::operator=(Solution &&rhs) noexcept {
  communities = std::move(rhs.communities);
  q_modularity = std::exchange(rhs.q_modularity, 0);
  return *this;
}
}  // namespace aco