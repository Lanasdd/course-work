#include "generators_helper.h"
#include <algorithm>

namespace generators {

[[maybe_unused]]
std::string GenerateWord(std::mt19937& generator, int max_length) {
  const int length{std::uniform_int_distribution<int>(1, max_length)(generator)};
  std::string word;
  word.reserve(static_cast<std::string::size_type>(length));
  for (int i{0}; i < length; ++i) {  // NOLINT(altera-id-dependent-backward-branch)
    word.push_back(static_cast<std::string::value_type&&>(
        std::uniform_int_distribution<int>(static_cast<int>('a'), static_cast<int>('z'))(generator)));
  }

  return word;
}

[[maybe_unused]]
std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length) {
  std::vector<std::string> words;
  words.reserve(static_cast<std::string::size_type>(word_count));
  for (int i = 0; i < word_count; ++i) {  // NOLINT(altera-id-dependent-backward-branch)
    words.push_back(GenerateWord(generator, max_length));
  }
  std::sort(words.begin(), words.end());
  words.erase(unique(words.begin(), words.end()), words.end());
  return words;
}

[[maybe_unused]]
std::string GenerateSearchQuery(std::mt19937& generator, const std::vector<std::string>& dictionary,
                                       int word_count, double minus_prob) {
  std::string query;
  for (int i = 0; i < word_count; ++i) {
    if (!query.empty()) {
      query.push_back(' ');
    }
    if (std::uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
      query.push_back('-');
    }
    query += dictionary[static_cast<std::vector<std::string>::size_type>(
        std::uniform_int_distribution<int>(0, static_cast<int>(dictionary.size() - 1))(generator))];
  }
  return query;
}

[[maybe_unused]]
std::vector<std::string> GenerateSearchQueries(std::mt19937& generator,
                                                                std::vector<std::string> const &dictionary,
                                              int query_count, int max_word_count) {
  std::vector<std::string> queries;
  queries.reserve(static_cast<std::vector<std::string>::size_type>(query_count));
  for (int i = 0; i < query_count; ++i) {
    queries.push_back(GenerateSearchQuery(generator, dictionary, max_word_count, 0));
  }
  return queries;
}

}  // namespace generators
