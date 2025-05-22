#pragma once
#include <random>  // mt19937
#include <string>
#include <vector>

namespace generators {

[[maybe_unused]] std::string GenerateWord(std::mt19937 &generator, int max_length);

[[maybe_unused]] std::vector<std::string> GenerateDictionary(std::mt19937 &generator, int word_count, int max_length);

[[maybe_unused]] std::string GenerateSearchQuery(std::mt19937 &generator, std::vector<std::string> const &dictionary,
                                                 int word_count, double minus_prob);

[[maybe_unused]] std::vector<std::string> GenerateSearchQueries(std::mt19937 &generator,
                                                                std::vector<std::string> const &dictionary,
                                                                int query_count, int max_word_count);

template <class Type>
[[maybe_unused]] Type GenerateValue(std::mt19937 &generator, Type min_value, Type max_value) {
  return std::uniform_int_distribution<Type>(min_value, max_value)(generator);
}

template <class Type>
[[maybe_unused]] std::vector<Type> GenerateValues(std::mt19937 &generator, std::size_t count, Type min_value,
                                                  Type max_value) {
  std::vector<Type> values;
  values.reserve(count);
  for (; 0 < count; count--) {  // NOLINT(altera-id-dependent-backward-branch)
    values.push_back(GenerateValue(generator, min_value, max_value));
  }
  sort(values.begin(), values.end());
  values.erase(unique(values.begin(), values.end()), values.end());
  return values;
}

}  // namespace generators
