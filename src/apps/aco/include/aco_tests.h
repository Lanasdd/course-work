#pragma once
#include <string_view>


namespace tests {
using namespace std::literals;

#ifdef USE_ACO
const auto test_datasets_path{"/opt/community_detection/test_datasets/"};
const auto test_reports_path{"/opt/community_detection/test_reports/"};
const auto test_gv_path{"/opt/community_detection/test_gv/"};

void TestGroup_CommunityDetectionTxtGraph();
void TestGroup_CommunityDetectionGmlGraph();
void TestAco();

#endif //USE_ACO

}  // namespace tests

