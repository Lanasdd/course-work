#include "aco_tests.h"

int main() {
#ifdef USE_ACO
  tests::TestAco();
#endif //USE_ACO
  return 0;
}
