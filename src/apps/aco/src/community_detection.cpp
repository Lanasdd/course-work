#include "community_detection.h"
#include <omp.h>
#include <tbb/tbb.h>
#include <algorithm>
#include <cmath>
#include <execution>
#include <fstream>
#include <functional>
#include <ranges>
#include <sstream>
#include <thread>
#include "profiler.h"

namespace aco {

CommunityDetection::CommunityDetection(Graph &&graph, int64_t iterations_count, AntIdType ants_count, double alpha,
                                       double beta, double rho, double q0, uint32_t threads_count)
    : thread_count_(std::max((threads_count > 0U ? threads_count : std::thread::hardware_concurrency()), 1U)),
      graph_(std::move(graph)),
      alpha_(alpha),
      beta_(beta),
      rho_(rho),
      one_minus_rho_(1.0-rho),
      q0_(q0),
      ants_count_(ants_count),
      iterations_count_(std::max<int64_t>(iterations_count, 10LL)) {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(0);
  std::cout.tie(0);

  if (IsParallelized()) {
    omp_set_num_threads(static_cast<int32_t>(thread_count_));
  }

  if (0 == ants_count) {
    ants_count_ = std::max(thread_count_, 3U);
  }

  graph_.FillNodesIdsArray();

  initial_pheromone_ =  1 / static_cast<double>(graph_.NodesSize());
  InitPheromones();
  Solution::InitCommunities(selected_solution_, graph_.GetAdjacencyList());
  Solution::SetModularity(selected_solution_, CalculateSolutionModularity(selected_solution_));
}

void CommunityDetection::InitPheromones() { IsParallelized() ? InitPheromonesPar() : InitPheromonesSeq(); }

void CommunityDetection::InitPheromonesPar() {
  SCOPE_DURATION_MS("Pheromone initialization (parallel algorithm)");

  auto const &nodes_id_array{graph_.GetNodesIdsArray()};
  double initial_pheromone{initial_pheromone_};

  //  tbb::parallel_for(tbb::blocked_range(nodes_id_array.begin(), nodes_id_array.end()),
  //                    [&](auto const &r) {
  //                      for (auto const node : r) {
  //                        auto &p_n{pheromone_trails_[node]};
  //                        for (auto const community : nodes_id_array) {
  //                          p_n[community] = initial_pheromone;
  //                        }
  //                      }
  //                    });

//#pragma omp parallel for
  for (auto const a : nodes_id_array) {
    auto node{a};
//#pragma omp critical
    {
      //for (auto const community : nodes_id_array) {
      //        pheromone_trails_[node][community] = initial_pheromone;
      //}
      pheromone_trails_[node][node] = initial_pheromone;
    }
  }
}

void CommunityDetection::InitPheromonesSeq() {
  SCOPE_DURATION_MS("Pheromone initialization (seq algorithm)");

  auto const &a_list{graph_.GetAdjacencyList()};
  double initial_pheromone{1.0 / static_cast<double>(a_list.size())};

  for (auto const &a : a_list) {
    auto node{a.first};
    //    for (auto const &community : a_list) {
    //      pheromone_trails_[node][community.first] = initial_pheromone;
    //    }
    pheromone_trails_[node][node] = initial_pheromone;
  }
}

void CommunityDetection::RefreshPheromones(std::vector<Solution> const &solutions) {
  IsParallelized() ? RefreshPheromonesPar(solutions) : RefreshPheromonesSeq(solutions);
}

void CommunityDetection::RefreshPheromonesSeq(std::vector<Solution> const &solutions) {
  SCOPE_DURATION_MS("Refresh pheromones (seq algorithm)");

  // 1. сначала реализуем обратную связь - испаряем (девальвируем) феромон
  // по след. формуле \tau_{ij} \leftarrow (1 - \rho) \cdot \tau_{ij}
  initial_pheromone_*= one_minus_rho_;
  for (auto &community_pheromone_in_node : pheromone_trails_) {
    for (auto &community_pheromone : community_pheromone_in_node.second) {
      community_pheromone.second *= one_minus_rho_;
    }
  }

  // 2. затем на основе каждого найденного решения откладываем феромон в размере рассчитанной модулярности
  // пока без деградации предыдущего решения (плюсом, кроме прочего, здесь будет исключения умножения)
  // по след. формуле tau_{ic} \leftarrow tau_{ic} + Q(S_{k}) , здесь Q(S_{k} \gets \Delta \tau
  // дополнительно подсчитываем максимальный феромон
  double max_pheromone{0.0};
  for (auto const &[community_pheromone, delta_tau] : solutions) {
    for (auto const &[node, community] : community_pheromone) {
      max_pheromone = std::max(max_pheromone, (pheromone_trails_[node][community] += delta_tau));
    }
  }

  // 3. если так получилось, что максимальный феромон превысил заданный предел, то девальвируем весь феромон в 2 раза
  if (max_pheromone > 10.0) {
    initial_pheromone_/= 2;
    for (auto &community_pheromone_in_node : pheromone_trails_) {
      for (auto &community_pheromone : community_pheromone_in_node.second) {
        community_pheromone.second /= 2;
      }
    }
  }
}

void CommunityDetection::RefreshPheromonesPar(std::vector<Solution> const &solutions) {
  // SCOPE_DURATION_MS("Refresh pheromones (parallel algorithm)");

  // 1. сначала реализуем обратную связь - испаряем (девальвируем) феромон
  // по след. формуле \tau_{ij} \leftarrow (1 - \rho) \cdot \tau_{ij}
  // Замечание: критическая секция для инструкции обновления феромона не нужна, запись осуществляется в индивидуальную
  // ячейку таблицы, каждый поток не пересекается при записи
  initial_pheromone_*= one_minus_rho_;
  auto const &nodes_id_array{graph_.GetNodesIdsArray()};
  tbb::parallel_for(tbb::blocked_range(nodes_id_array.begin(), nodes_id_array.end()), [&](auto &r) {
    for (auto node : r) {
      for (auto &community_pheromone : pheromone_trails_.at(node)) {
        community_pheromone.second *= one_minus_rho_;
      }
    }
  });
////#pragma omp parallel for num_threads(thread_count_)
//  for (auto node : nodes_id_array) {
//    for (auto &community_pheromone : pheromone_trails_.at(node)) {
//      community_pheromone.second *= one_minus_rho_;
//    }
//  }

  // 2. затем на основе каждого найденного решения откладываем феромон в размере рассчитанной модулярности
  // пока без деградации предыдущего решения (плюсом, кроме прочего, здесь будет исключения умножения)
  // по след. формуле tau_{ic} \leftarrow tau_{ic} + Q(S_{k}) , здесь Q(S_{k} \gets \Delta \tau
  // дополнительно подсчитываем максимальный феромон
  double max_pheromone{0.0};
  tbb::parallel_for(tbb::blocked_range(solutions.begin(), solutions.end()), [&](auto &r) {
    for (auto const &[community_pheromone, delta_tau] : r) {
      for (auto const &[node, community] : community_pheromone) {
        //tbb::spin_mutex::scoped_lock lock(mtx_);
        max_pheromone = std::max(max_pheromone, (pheromone_trails_[node][community] += delta_tau));
      }
    }
  });

////#pragma omp parallel for
//  for (auto const &[community_pheromone, delta_tau] : solutions) {
////#pragma omp critical(refresh_pheromone)
//    {
//      for (auto const &[node, community] : community_pheromone) {
//        // auto& ph{pheromone_trails_[node][community]};
//        // ph = rho_* ph + delta_tau;
//        // max_pheromone = std::max(max_pheromone, ph);
//        max_pheromone = std::max(max_pheromone, (pheromone_trails_[node][community] += delta_tau));
//      }
//    }
//  }

  // 3. если так получилось, что максимальный феромон превысил заданный предел, то девальвируем весь феромон в 2 раза
  // Замечание: критическая секция для инструкции обновления феромона не нужна, запись осуществляется в индивидуальную
  // ячейку таблицы, каждый поток не пересекается при записи
  if (max_pheromone > 10.0) {
    initial_pheromone_ /= 2;
    tbb::parallel_for(tbb::blocked_range(nodes_id_array.begin(), nodes_id_array.end()), [&](auto &r) {
      for (auto node : r) {
        for (auto &community_pheromone : pheromone_trails_.at(node)) {
          community_pheromone.second /= 2;
        }
      }
    });
  }

   //#pragma omp parallel for num_threads(thread_count_)
   // for (int node : nodes_id_array) {
   //   for (auto &community_pheromone : pheromone_trails_.at(node)) {
   //     community_pheromone.second /= 2;
   //   }
   // }
}

CommunityDetection::CounterType CommunityDetection::CalcConnectionsWeightToCommunity(NodeIdType node_id,
                                                                                     CommunityIdType community_id,
                                                                                     Solution const &solution) {
  // SCOPE_DURATION_MS("Calculate connections weight to community");

  CounterType connections_weight{0};
  auto const &adjacency_list{graph_.GetAdjacencyList()};

  if (auto it{adjacency_list.find(node_id)}; it != adjacency_list.end()) {
    for (auto neighbor : it->second.arcs_) {
      if (auto it_communities{solution.communities.find(neighbor.to_node_id)};
          it_communities != solution.communities.end() && it_communities->second == community_id) {
        connections_weight += neighbor.weight;
      }
    }
  }

  return connections_weight;
}

double CommunityDetection::CalculateSolutionModularity(Solution const &solution) {
  // SCOPE_DURATION_MS("Calculate solution modularity");

  // Q = \frac{1}{2m} \sum_{v_i, v_j \in V} \left[ A_{ij} - \frac{k_i k_j}{2m} \right] \delta(c_i, c_j)
  // Расширяем модулярность на взвешенные рёбра,
  // m трактуется не как число рёбер графа, а как суммарный вес всех рёбер графа
  // k_i(k_j) трактуется не как не как степень узла i(j), а как суммарный вес всех рёбер узла i(j)
  // A_{ij} трактуется как взвешенный элемент ij матрицы смежности

  double q{0.0};
  auto const m2{2 * graph_.EdgesWeight()};
  auto const &adjacency_list{graph_.GetAdjacencyList()};
  auto const &communities{solution.communities};

  if (0 == m2) {
    return 0.0;  // в графе нет рёбер
  }

  for (auto const &[i, v_i] : adjacency_list) {
    // находим сообщество для вершины i
    if (auto it_i{communities.find(i)}; it_i != communities.end()) {
      for (auto const &[j, weight_ij] : v_i.arcs_) {  // бежим по всем связям узла (дугам)
        if (i < j) {                                  // для исключения двойного учёта по обратным дугам
          // находим сообщество для вершины j
          if (auto it_j{communities.find(j)}; it_j != communities.end()) {
            if (it_i->second == it_j->second) {  // delta(c_i, c_j) == 1 (тоже сообщество)
              // \frac{k_i k_j}{2m}
              double rand_expected{static_cast<double>(v_i.weight_ * weight_ij) / static_cast<double>(m2)};
              ////std::cout << "Aij:" << weight_ij << " rand_expected: " << rand_expected << std::endl;
              // Q += \left[ A_{ij} - \frac{k_i k_j}{2m} \right] \delta(c_i, c_j)
              q += (static_cast<double>(weight_ij) - rand_expected);
            }
          }
        }
      }
    }
  }

  /// std::cout << "Q:" << q << " P: " << q / static_cast<double>(m2) << std::endl;

  return q / static_cast<double>(m2);
}

Solution CommunityDetection::BuildSolution(AntIdType /*ant_id*/, Solution &solution,
                                           CommunityDetection::ThreadDataContext &thread_context) {
  // SCOPE_DURATION_MS("Build solution");

  auto const &adjacency_list{graph_.GetAdjacencyList()};
  //  Solution::InitCommunities(solution, adjacency_list);  // в начале помещаем каждый узел в своё отдельное сообщество
  //  Solution solution1(adjacency_list);  // в начале помещаем каждый узел в своё отдельное сообщество

#pragma omp critical
  {
    solution = selected_solution_;
  }

  // перемешиваем узлы для случайного выбора порядка
  auto nodes{graph_.GetNodesIdsArray()};
  thread_context.shuffle(nodes);

  for (NodeIdType const node : nodes) {
    // набор сообществ-кандидатов формируем из сообществ, к которым принадлежат смежные узлы и сам узел
    std::vector<CommunityIdType> possible_communities;  ////std::set<CommunityIdType> possible_communities
    if (auto it{adjacency_list.find(node)}; it != adjacency_list.end()) {
      possible_communities.reserve(it->second.arcs_.size() + 1);
      possible_communities.emplace_back(node);  // упрощаем, т.к. при иниц. node == solution.communities[node]
      for (auto const &neighbor : it->second.arcs_) {
        possible_communities.emplace_back(solution.communities[neighbor.to_node_id]);
      }
    } else {
      possible_communities.emplace_back(node);
    }

    // очистка набора сообществ-кандидатов от дублирующихся сообществ
    std::ranges::sort(possible_communities);
    possible_communities.erase(std::unique(possible_communities.begin(), possible_communities.end()),
                               possible_communities.end());

    // Делаем случайный выбор между
    // подкреплением (элитный муравей по наиболее перспективному сообществу: random_value < q0) и
    // исследованием (вероятностный выбор пути через рулетку: random_value > q0)
    if (thread_context.GetRandom() < q0_) {
      // подкрепление - элитный муравей, однозначно выбираем наиболее перспективное сообщество по правилу с учётом
      // истории исследования и эвристики

      CommunityIdType selected_community{node};  // упрощаем, т.к. при иниц. node == solution.communities[node]
      for (double selected_probability{0.0}; auto community : possible_communities) {
        double pheromone;
#pragma omp critical(pheromone_read)
        {
          pheromone = pheromone_trails_[node].try_emplace(community, initial_pheromone_).first->second;
        }

        double heuristic{
            std::max(0.1, static_cast<double>(CalcConnectionsWeightToCommunity(node, community, solution)))};
        double probability{std::pow(pheromone, alpha_) * std::pow(heuristic, beta_)};

        if (probability > selected_probability) {
          selected_probability = probability;
          selected_community = community;
        }
      }
      solution.communities[node] = selected_community;
    } else {
      // исследуем - вероятностный выбор пути через рулетку
      std::vector<CommunityIdType> communities;
      std::vector<double> probabilities;
      double total_probability{0.0};

      for (auto community : possible_communities) {
        double pheromone;
#pragma omp critical(pheromone_read)
        {
          pheromone = pheromone_trails_[node].try_emplace(community, initial_pheromone_).first->second;
        }

        double heuristic{
            std::max(0.1, static_cast<double>(CalcConnectionsWeightToCommunity(node, community, solution)))};
        double probability{std::pow(pheromone, alpha_) * std::pow(heuristic, beta_)};

        probabilities.push_back(probability);
        total_probability += probability;
        communities.push_back(community);
      }

      NormalizeProbabilities(probabilities, total_probability);  // Нормализуем вероятности выбора для сообществ
      // выбираем сообщество методом рулетки
      solution.communities[node] =
          SelectCommunityByRouletteWheel(communities, probabilities, thread_context.GetRandom());
    }
  }

  solution.q_modularity = CalculateSolutionModularity(solution);
  return solution;
}

void CommunityDetection::NormalizeProbabilities(std::vector<double> &probabilities, double total_probability) {
  if (total_probability > 0) {  // Нормализуем все вероятности по норме total_value
    for (auto &probability : probabilities) {
      probability /= total_probability;
    }
  } else {  // Всё пусто, тогда используем равномерное распределение - всё равновероятно
    for (auto &probability : probabilities) {
      probability = 1.0 / static_cast<double>(probabilities.size());
    }
  }
}

CommunityDetection::CommunityIdType CommunityDetection::CommunityDetection::SelectCommunityByRouletteWheel(
    std::vector<CommunityIdType> const &communities, std::vector<double> const &probabilities, double random_value) {
  double accumulator{0.0};
  CommunityIdType selected_community{communities[0]};

  for (size_t i{0}; i < communities.size(); i++) {
    accumulator += probabilities[i];
    if (random_value <= accumulator) {
      selected_community = communities[i];
      break;
    }
  }
  return selected_community;
}

CommunityDetection::CommunitiesMap CommunityDetection::GetCommunityMap(Solution const *p_solution) {
  // SCOPE_DURATION_MS("Convert node_to_communities map in solution to community_to_nodes map (seq algorithm)");
  if (!p_solution) p_solution = &selected_solution_;
  CommunitiesMap communities_map;
  for (auto const &[node, community] : p_solution->communities) {
    communities_map[community].emplace_back(node);
  }
  return communities_map;
}

CommunityDetection::SortedCommunitiesMap CommunityDetection::GetSortedCommunitiesMapSeq(Solution const *p_solution) {
  // SCOPE_DURATION_MS("Convert node_to_communities map in solution to community_to_nodes map (seq algorithm)");
  CommunitiesMap communities_map{GetCommunityMap(p_solution)};

  SortedCommunitiesMap sorted_communities_map;
  sorted_communities_map.reserve(communities_map.size());
  for (auto &[community, nodes] : communities_map) {
    std::ranges::sort(nodes);
    sorted_communities_map.emplace_back(community, std::move(nodes));
  }
  std::ranges::sort(sorted_communities_map,
                    [](auto const &lhs, auto const &rhs) -> bool { return lhs.second.size() > rhs.second.size(); });
  return sorted_communities_map;
}

CommunityDetection::SortedCommunitiesMap CommunityDetection::GetSortedCommunitiesMapPar(Solution const *p_solution) {
  // SCOPE_DURATION_MS("Convert node_to_communities map in solution to community_to_nodes map (parallel algorithm)");
  CommunitiesMap communities_map{GetCommunityMap(p_solution)};

  SortedCommunitiesMap sorted_communities_map;
  sorted_communities_map.reserve(communities_map.size());
  for (auto &[community, nodes] : communities_map) {
    sorted_communities_map.emplace_back(community, std::move(nodes));
  }

  tbb::parallel_sort(sorted_communities_map,
                     [](auto const &lhs, auto const &rhs) -> bool { return lhs.second.size() > rhs.second.size(); });
  tbb::parallel_for(tbb::blocked_range(sorted_communities_map.begin(), sorted_communities_map.end()),
                    [](auto& r) {
                      for (auto &[community, nodes] : r) {
                        std::ranges::sort(nodes);
                      }
                    });

  //  for (auto &[community, nodes] : communities_map) {
  //    tbb::parallel_sort(nodes);
  //    sorted_communities_map.emplace_back(community, std::move(nodes));
  //  }

  return sorted_communities_map;
}

CommunityDetection& CommunityDetection::DetectCommunitiesPar() {
  // создаём локальные контексты для потоков для уменьшения блокировок (там только генераторы случайности)
  std::vector<ThreadDataContext> thread_contexts;
  thread_contexts.reserve(thread_count_);
  for (uint32_t i{0}; i < thread_count_; ++i) {
    thread_contexts.emplace_back();
  }

  std::vector<Solution> ants(static_cast<size_t>(ants_count_));
  for (int64_t i{0}; i < iterations_count_; ++i) {
#pragma omp parallel for num_threads(thread_count_)
    for (AntIdType ant = 0; ant < ants_count_; ++ant) {
      auto &solution{ants[ant]};
      BuildSolution(ant, solution, thread_contexts[static_cast<size_t>(omp_get_thread_num())]);
#pragma omp critical
      {
        if (solution.q_modularity > selected_solution_.q_modularity) {
          selected_solution_ = solution;
        }
      }
    }

    RefreshPheromonesPar(ants);
  }
  return *this;
}

CommunityDetection& CommunityDetection::DetectCommunitiesSeq() {
  // создаём локальные контексты для потоков для уменьшения блокировок (там только генераторы случайности)
  ThreadDataContext thread_context;

  std::vector<Solution> ants(static_cast<size_t>(ants_count_));
  for (int64_t i{0}; i < iterations_count_; ++i) {
    for (AntIdType ant = 0; ant < ants_count_; ++ant) {
      auto &solution{ants[ant]};
      BuildSolution(ant, solution, thread_context);
      if (solution.q_modularity > selected_solution_.q_modularity) {
        selected_solution_ = solution;
      }
    }

    RefreshPheromonesPar(ants);
  }
  return *this;
}

CommunityDetection & CommunityDetection::DetectCommunities() { return IsParallelized() ? DetectCommunitiesPar() : DetectCommunitiesSeq(); }

std::string CommunityDetection::GetConfigSummary() {
  std::ostringstream os;
  os << "Running config summary:"
        "\n  - "
     << ants_count_
     << " ants"
        "\n  - "
     << iterations_count_
     << " iterations"
        "\n  - "
     << thread_count_
     << " threads"
        "\n  - alpha (pheromone influence): "
     << alpha_ << "\n  - beta (heuristic influence): " << beta_ << "\n  - rho (evaporation rate): " << rho_
     << "\n  - q0 (exploitation probability): " << q0_ << "\n";

  return os.str();
}

std::string CommunityDetection::BuildCommunitiesReport(SortedCommunitiesMap *p_communities) {
  const SortedCommunitiesMap& sorted_communities{p_communities ? *p_communities : GetSortedCommunitiesMapPar()};
  //auto sorted_communities{GetSortedCommunitiesMapSeq()};

  std::ostringstream ss;
  ss << "Detected " << sorted_communities.size() << " communities, modularity = " << selected_solution_.q_modularity <<":\n";

  auto adjacency_list{graph_.GetAdjacencyList()};
  for (CommunityIdType counter{0}; auto const &pair : sorted_communities) {
    ss << "Community " << pair.first << " (size: " << pair.second.size() << "): ";

    for (NodeIdType node_count{0}; auto node : pair.second) {
      if (node_count++ < 10) {
        ss << node << "(w" << adjacency_list.at(node).weight_ << ") ";
      } else {
        break;
      }
    }

    if (pair.second.size() > 10) {
      ss << "...";
    }

    ss << '\n';

    if (++counter >= 10 && sorted_communities.size() > 10) {
      ss << "... (" << (sorted_communities.size() - 10) << " more communities)\n";
      break;
    }
  }

  return ss.str();
}

bool CommunityDetection::SaveCompleteCommunitiesReportToFile(const std::string& file_name, SortedCommunitiesMap *p_communities) {
  const SortedCommunitiesMap& sorted_communities{p_communities ? *p_communities : GetSortedCommunitiesMapPar()};
  //auto sorted_communities{GetSortedCommunitiesMapSeq()};

  std::ostringstream ss;
  ss << "Detected " << sorted_communities.size() << " communities, modularity = " << selected_solution_.q_modularity <<":\n";

  auto adjacency_list{graph_.GetAdjacencyList()};
  for (auto const &pair : sorted_communities) {
    ss << "Community " << pair.first << " (size: " << pair.second.size() << "): ";

    for (auto node : pair.second) {
      ss << node << "(w" << adjacency_list.at(node).weight_ << ") ";
    }
    ss << '\n';
  }

  std::ofstream ofs(file_name);
  if (!ofs.is_open()) {
    std::cerr << "Failed to open file " << file_name << " for writing" << std::endl;
    return false;
  }

  ofs << ss.str();
  ofs.close();

  return true;
}

bool CommunityDetection::SaveGraphAndCommunitiesRToGvFile(std::string const &file_name, SortedCommunitiesMap *p_communities) {
  const SortedCommunitiesMap& sorted_communities{p_communities ? *p_communities : GetSortedCommunitiesMapPar()};
  std::ostringstream ss;

  std::mt19937 rd{std::random_device{}()};
  std::uniform_int_distribution<> generator{17, 255};

  auto gen_color = [&ss, &generator, &rd]() {
    ss << "\"#" << std::hex << generator(rd) << generator(rd) << generator(rd) << "\""<< std::dec;
  };

  ss << "graph aco_test {\n"
        "\t\tfontname=\"Helvetica,Arial,sans-serif\"\n"
        "\t\tnode [shape=doublecircle, fontsize=10, fontname=\"Helvetica,Arial,sans-serif\", color=\"cyan3\"]\n"
        "\t\tedge [fontname=\"Helvetica,Arial,sans-serif\"]\n\n";

  auto adjacency_list{graph_.GetAdjacencyList()};
  for (auto const &[community_id, nodes] : sorted_communities) {
    ss << "\t\tsubgraph cluster_" << community_id << " {\n"
       << "\t\t\t\tcluster=true;\n"
          "\t\t\t\tstyle=filled;\n"
          "\t\t\t\tcolor=";
    gen_color();
    ss << ";\n"
          "\t\t\t\tnode [style=filled,color=";
    gen_color();
    ss << "];\n"
          "\t\t\t\tlabel = \"Community #"
       << community_id << "(size: " << nodes.size() << ")\";\n";

    for (auto&node_id : nodes) {
      auto &node{adjacency_list.at(node_id)};
      for (auto &arc : node.arcs_) {
        if (node_id < arc.to_node_id) continue;
        ss << "\t\t\t\t" << node_id << " -- " << arc.to_node_id << ";\n";
      }
    }

    ss << "\t\t}\n\n";
  }
  ss << "}\n";

  std::ofstream ofs(file_name);
  if (!ofs.is_open()) {
    std::cerr << "Failed to open file " << file_name << " for writing" << std::endl;
    return false;
  }

  ofs << ss.str();
  ofs.close();

  return true;
}

}  // namespace aco