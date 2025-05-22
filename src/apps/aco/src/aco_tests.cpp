#include "aco_tests.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "assert_helpers.h"
#include "community_detection.h"
#include "graph.h"
#include "log_duration.h"
#include "omp.h"
#include "tests_helper.h"

namespace tests {

#ifdef USE_ACO

template <typename F>
inline static void TestCommunityDetectionFromFile(F f, const std::string &test_dataset_file, int64_t iterations_count,
                                                  aco::CommunityDetection::AntIdType ants_count, double alpha,
                                                  double beta, double rho, double q0, uint32_t threads_count,
                                                  bool make_gv_flag = true) {
  std::cerr << "Конфигурация: iterations " << iterations_count << ", ants count " << ants_count << ", alpha " << alpha
            << ", beta " << beta << ", rho " << rho << ", q0 " << q0 << ", threads count " << threads_count
            << std::endl;

  aco::Graph graph;
  f(graph, test_datasets_path + test_dataset_file);

  auto const nodes_size{graph.NodesSize()};

  std::cerr << "Граф загружен, конфигурация " << nodes_size << " узлов, " << graph.EdgesSize()
            << " рёбер, общий вес рёбер " << graph.EdgesWeight() << std::endl;

  aco::CommunityDetection cd(std::move(graph), iterations_count, ants_count, alpha, beta, rho, q0, threads_count);
  std::cerr << cd.GetConfigSummary();

  {
    LOG_DURATION("Определение сообществ");
    cd.DetectCommunities();
  }

  auto communities{cd.GetSortedCommunitiesMapPar()};
  std::cerr << cd.BuildCommunitiesReport(&communities) << std::endl;

  std::ostringstream os;
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  os << "." << std::put_time(&tm, "%d.%m.%Y %H-%M-%S");

  {
    LOG_DURATION("Cохранение в файле представления сообществ в графе");
    std::string report_file_name{test_reports_path + test_dataset_file + os.str() + ".report"};
    if (cd.SaveCompleteCommunitiesReportToFile(report_file_name, &communities)) {
      std::cerr << "Информация с определёнными сообществами сохранена в файле "s.append(report_file_name) << std::endl;
    }
  }

  if (make_gv_flag && nodes_size < 512) {
    LOG_DURATION("Генерация графического представления сообществ в графе");
    std::string gv_file_name{test_gv_path + test_dataset_file + os.str() + ".gv"};
    if (cd.SaveGraphAndCommunitiesRToGvFile(gv_file_name, &communities)) {
      std::cerr << "Dot файле в формате Graphviz сохранён в  "s.append(gv_file_name) << std::endl;
      if (0 == std::system(("dot \""s + gv_file_name + "\" -Tsvg -o \""s + gv_file_name + ".svg\""s).c_str())) {
        std::cerr << "Графическое представление сообществ в графе сохранёно в  "s.append(gv_file_name).append(".svg")
                  << std::endl;
      }
    }
  }
}

inline static void TestCommunityDetectionFromTxtFile(std::string const &test_dataset_file, int64_t iterations_count,
                                                     aco::CommunityDetection::AntIdType ants_count, double alpha,
                                                     double beta, double rho, double q0, uint32_t threads_count) {
  tests::TestCommunityDetectionFromFile(
      [](aco::Graph &graph, std::string const &file_name) {
        LOG_DURATION("Загружаем граф из текстового файла " + file_name);
        graph.ConstructGraphFromSimpleFile(file_name);
      },
      test_dataset_file, iterations_count, ants_count, alpha, beta, rho, q0, threads_count);
}

inline static void TestCommunityDetectionFromGmlFile(std::string const &test_dataset_file, int64_t iterations_count,
                                                     aco::CommunityDetection::AntIdType ants_count, double alpha,
                                                     double beta, double rho, double q0, uint32_t threads_count) {
  tests::TestCommunityDetectionFromFile(
      [](aco::Graph &graph, std::string const &file_name) {
        LOG_DURATION("Загружаем граф из файла формата Graph Modelling Language " + file_name);
        graph.ConstructGraphFromGmlFile(file_name);
      },
      test_dataset_file, iterations_count, ants_count, alpha, beta, rho, q0, threads_count);
}

void TestGroup_CommunityDetectionTxtGraph() {
  std::string const file{"karate.txt"};
  constexpr int64_t kIterationsCount{10};
  constexpr aco::CommunityDetection::AntIdType ants_count{1000};
  constexpr double kAlpha{1.0};
  constexpr double kBeta{2.0};
  constexpr double kQ0{0.9};
  uint32_t const threads_count{0};

  double kRho{0.1};
  RUN_TEST_WITH_PARAMS_AND_DURATION_LOG_EX_SUF(
      TestCommunityDetectionFromTxtFile, _Rho01,
      std::tuple(file, kIterationsCount, ants_count, kAlpha, kBeta, kRho, kQ0, threads_count));
  kRho = 0.5;
  RUN_TEST_WITH_PARAMS_AND_DURATION_LOG_EX_SUF(
      TestCommunityDetectionFromTxtFile, _Rho05,
      std::tuple(file, kIterationsCount, ants_count, kAlpha, kBeta, kRho, kQ0, threads_count));
  kRho = 0.9;
  RUN_TEST_WITH_PARAMS_AND_DURATION_LOG_EX_SUF(
      TestCommunityDetectionFromTxtFile, _Rho09,
      std::tuple(file, kIterationsCount, ants_count, kAlpha, kBeta, kRho, kQ0, threads_count));
}

void TestGroup_CommunityDetectionGmlGraph() {
  std::string const file{"celegans_metabolic.gml"};
  constexpr int64_t kIterationsCount{3};
  constexpr aco::CommunityDetection::AntIdType ants_count{30};
  constexpr double kAlpha{1.0};
  constexpr double kBeta{1.0};
  constexpr double kQ0{0.8};
  uint32_t const threads_count{0};

  double kRho{0.1};
  RUN_TEST_WITH_PARAMS_AND_DURATION_LOG_EX_SUF(
      TestCommunityDetectionFromGmlFile, _Rho01,
      std::tuple(file, kIterationsCount, ants_count, kAlpha, kBeta, kRho, kQ0, threads_count));
  kRho = 0.5;
  RUN_TEST_WITH_PARAMS_AND_DURATION_LOG_EX_SUF(
      TestCommunityDetectionFromGmlFile, _Rho05,
      std::tuple(file, kIterationsCount, ants_count, kAlpha, kBeta, kRho, kQ0, threads_count));
  kRho = 0.9;
  RUN_TEST_WITH_PARAMS_AND_DURATION_LOG_EX_SUF(
      TestCommunityDetectionFromGmlFile, _Rho09,
      std::tuple(file, kIterationsCount, ants_count, kAlpha, kBeta, kRho, kQ0, threads_count));
}

void TestAco() {
  std::cerr << "\n[start aco tests]\n";

  TestGroup_CommunityDetectionTxtGraph();
  TestGroup_CommunityDetectionGmlGraph();

  std::cout.flush();
  std::cerr << "[finish aco tests]" << std::endl;
}

#endif  // USE_ACO

}  // namespace tests
