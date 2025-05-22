#pragma once
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <tbb/tbb.h>
#include "defines.h"
#include "graph.h"
#include "solution.h"

/**
 * \brief aco implementation
 */

namespace aco {

// class CDDefaultTraits {
//   static double PheromoneInfluence(CommunityDetection cd, Graph::IdType ant_id, Graph::IdType iteration_id) {
//     return alpha_start;
//   };
//
//   static double PheromoneInfluence(double alpha_start, double alpha_stop, Graph::IdType ant_id,
//                                    Graph::IdType ants_count, Graph::IdType iteration_id,
//                                    Graph::IdType max_iteration_id) {
//     return alpha_start;
//   };
// };
//
// template <class Traits>
class CommunityDetection {
 public:
  using NodeIdType = Graph::NodeIdType;
  using CommunityIdType = Solution::CommunityIdType;
  using AntIdType = Graph::IdType;
  using CounterType = Graph::IdType;
  using PheromoneTrailsType = double;
  using PheromoneTrails = std::unordered_map<NodeIdType, std::unordered_map<CommunityIdType, PheromoneTrailsType>>;
  using CommunitiesMap = std::unordered_map<CommunityIdType, std::vector<NodeIdType>>;
  using SortedCommunitiesMap = std::vector<std::pair<CommunityIdType, std::vector<NodeIdType>>>;

  class ThreadDataContext {
   private:
    std::mt19937_64 rd_{std::random_device{}()};
    std::uniform_real_distribution<PheromoneTrailsType> generator_{0.0, 1.0};

   public:
    explicit ThreadDataContext() = default;

    PheromoneTrailsType GetRandom() { return generator_(rd_); }

    template <class T>
    void shuffle(std::vector<T> &array) {
      std::ranges::shuffle(array, rd_);
    }
  };

  explicit CommunityDetection(Graph &&graph, int64_t iterations_count, AntIdType ants_count, double alpha, double beta,
                              double rho, double q0, uint32_t threads_count = 0);
  /*virtual*/ ~CommunityDetection() = default;

  CommunityDetection(CommunityDetection const &other) = default;
  CommunityDetection &operator=(CommunityDetection const &rhs) = delete;
  CommunityDetection(CommunityDetection &&other) noexcept = default;
  CommunityDetection &operator=(CommunityDetection &&rhs) = delete;

  bool IsParallelized() const { return thread_count_ > 1; }  // использование распаралеливания

  CommunityDetection &DetectCommunities();

  CommunitiesMap GetCommunityMap(Solution const *p_solution = nullptr);
  SortedCommunitiesMap GetSortedCommunitiesMapPar(Solution const *p_solution = nullptr);
  SortedCommunitiesMap GetSortedCommunitiesMapSeq(Solution const *p_solution = nullptr);

  std::string BuildCommunitiesReport(SortedCommunitiesMap *p_communities = nullptr);
  bool SaveCompleteCommunitiesReportToFile(std::string const &file_name, SortedCommunitiesMap *p_communities = nullptr);
  bool SaveGraphAndCommunitiesRToGvFile(std::string const &file_name, SortedCommunitiesMap *p_communities = nullptr);
  std::string GetConfigSummary();

 private:
  CommunityDetection &DetectCommunitiesPar();
  CommunityDetection &DetectCommunitiesSeq();

  void InitPheromones();
  void InitPheromonesPar();
  void InitPheromonesSeq();

  void RefreshPheromones(std::vector<Solution> const &solutions);
  void RefreshPheromonesPar(std::vector<Solution> const &solutions);
  void RefreshPheromonesSeq(std::vector<Solution> const &solutions);

  double CalculateSolutionModularity(Solution const &solution);

  CounterType CalcConnectionsWeightToCommunity(NodeIdType node_id, CommunityIdType community_id,
                                               Solution const &solution);

  Solution BuildSolution(AntIdType ant_id, Solution &solution, CommunityDetection::ThreadDataContext &thread_context);

  static void NormalizeProbabilities(std::vector<double> &probabilities, double total_value);

  static CommunityIdType SelectCommunityByRouletteWheel(std::vector<CommunityIdType> const &communities,
                                                        std::vector<double> const &probabilities, double random_value);

  uint32_t thread_count_;
  Solution selected_solution_;
  Graph graph_;
  double alpha_{1};
  double beta_{2};
  double rho_{0.2};
  double one_minus_rho_{0.8};
  double q0_{0.9};

  AntIdType ants_count_;
  int64_t iterations_count_;

  PheromoneTrails pheromone_trails_;
  double initial_pheromone_{0};

  static tbb::spin_mutex mtx_;
};
}  // namespace aco
