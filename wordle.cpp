#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * constraint_kind models different kinds of information provided from a guess.
 */
enum class constraint_kind { not_present, present, perfect };

/**
 * constraint captures details about information revealed from word guesses.
 */
struct constraint {
  constraint_kind kind;
  char letter;
  std::optional<std::uint8_t> position;
  std::vector<int> exclude_positions;
};

/**
 * letter_score trackes a letter and it's frequency score.
 */
struct letter_score {
  int score;
  std::string letters;
};

/**
 * letter_scores provides letter frequency counts for the English language.
 */
const std::array<letter_score, 18> letter_scores{{{12000, "e"},
                                                  {9000, "t"},
                                                  {8000, "ainos"},
                                                  {6400, "h"},
                                                  {6200, "r"},
                                                  {4400, "d"},
                                                  {4000, "l"},
                                                  {3400, "u"},
                                                  {3000, "cm"},
                                                  {2500, "f"},
                                                  {2000, "wy"},
                                                  {1700, "gp"},
                                                  {1600, "b"},
                                                  {1200, "v"},
                                                  {800, "k"},
                                                  {500, "q"},
                                                  {400, "jx"},
                                                  {200, "z"}}};

/**
 * validate checks to see if a word is valid for wordle, and then normalizes it.
 *
 * @param word the word to validate and normalize.
 * @return false if the word is invalid, true otherwise.
 */
auto validate(std::string &word) {
  while (!word.empty() && isspace(word.back())) {
    word.pop_back();
  }
  while (!word.empty() && isspace(word.front())) {
    word.erase(word.begin());
  }
  if (word.size() != 5) {
    return false;
  }
  for (char &i : word) {
    if (!std::isalpha(i)) {
      return false;
    }
    i = std::tolower(i);
  }
  return true;
}

/**
 * load_words loads the wordlist from University of Michigan.
 * @returns a vector of strings containing only lowercase alphabetical
 * characters of length 5.
 */
auto load_words() {
  std::vector<std::string> words;
  std::ifstream word_data("words-umich.txt");
  for (std::string word; std::getline(word_data, word);) {
    if (validate(word)) {
      words.emplace_back(std::move(word));
    }
  }

  return words;
}

/**
 * load_spelling_dictionary loads a spelling oriented word list.
 * @returns a vector of strings containing only lowercase alphabetical
 * characters of length 5.
 */
auto load_spelling_dictionary() {
  std::vector<std::string> words;
  std::ifstream word_data("english-dictionary.txt");
  for (std::string word; std::getline(word_data, word);) {
    if (validate(word)) {
      words.emplace_back(std::move(word));
    }
  }

  return words;
}

/**
 * load_words_2 loads a word frequency list created by tokenizing many popular
 * newspapers.
 *
 * @return a tuple of words and their frequencies. The words are a vector of
 * strings containing only lowercase alphabetical characters of length 5.
 */
auto load_words_2() {
  std::vector<std::string> words;
  std::vector<int> freqs;
  std::ifstream word_data("eng_news_2023_1M/eng_news_2023_1M-words.txt");
  for (std::string line; std::getline(word_data, line);) {
    auto word_pos = line.find('\t');
    auto freq_pos = line.find('\t', word_pos + 1);
    auto word = line.substr(word_pos + 1, freq_pos - word_pos - 1);
    auto freq = line.substr(freq_pos + 1, line.size() - freq_pos);
    // std::cout << "{" << word << "}:{" << freq << "}\n";
    if (!validate(word)) {
      continue;
    }
    // std::cout << word << ":" << freq << "\n";
    words.emplace_back(std::move(word));
    freqs.emplace_back(std::stoi(freq));
  }

  return std::make_tuple(words, freqs);
}

/**
 * score generates a words score based on the frequency value of letters in the
 * words. Duplicates are eliminated to avoid heavier weighting of edge case
 * words like 'three'.
 *
 * @param word_data the word to check.
 * @return a score based on the frequency of the letters in the word.
 */
auto score(std::string_view word_data) {
  std::string word(word_data);
  std::ranges::sort(word);
  word.erase(std::unique(word.begin(), word.end()), word.end());
  int value = 0;
  for (const auto c : word) {
    for (auto [score, letters] : letter_scores) {
      if (letters.find(c) != std::string::npos) {
        value += score;
        break;
      }
    }
  }
  return value;
}

/**
 * solve evaluates the word list and constraints to remove impossible words
 * from the dictionary.
 *
 * @param cs solution constraints.
 * @param words the words to evaluate.
 * @return a vector of legal words.
 */
auto solve(const std::vector<constraint> &cs,
           const std::vector<std::string> &words) {
  std::vector<std::string_view> legal_words;
  for (const auto &word : words) {
    bool fail = false;
    for (const auto &[kind, letter, position, exclude_positions] : cs) {
      switch (kind) {
      case constraint_kind::not_present:
        if (word.contains(letter)) {
          fail = true;
        }
        break;
      case constraint_kind::present:
        if (!word.contains(letter)) {
          fail = true;
        } else {
          for (const auto exclude_pos : exclude_positions) {
            if (word[exclude_pos] == letter) {
              fail = true;
            }
          }
        }
        break;
      case constraint_kind::perfect:
        if (word[*position] != letter) {
          fail = true;
        }
        break;
      }
    }

    if (!fail) {
      legal_words.emplace_back(word);
    }
  }

  return legal_words;
}

int main(int, char **) {
  std::vector<constraint> cs{{constraint_kind::not_present, 's'},
                             {constraint_kind::not_present, 't'},
                             {constraint_kind::not_present, 'n'},
                             {constraint_kind::present, 'e', {}, {0, 2, 4}},
                             {constraint_kind::perfect, 'l', 4},
                             {constraint_kind::not_present, 'w'},
                             {constraint_kind::not_present, 'h'},
                             {constraint_kind::not_present, 'o'},
                             {constraint_kind::not_present, 'q'},
                             {constraint_kind::not_present, 'u'},
                             {constraint_kind::not_present, 'a'},
                             {constraint_kind::not_present, 'i'}};

  std::cout << "loading word data\n";
  const auto &[words, freqs] = load_words_2();
  std::cout << "word list count: " << words.size() << "\n";

  const auto dictionary_words = load_spelling_dictionary();
  std::unordered_set<std::string_view> dict;
  dict.reserve(dictionary_words.size());
  for (const auto &word : dictionary_words) {
    dict.emplace(word);
  }
  std::cout << "dictionary word count: " << dictionary_words.size() << "\n";

  std::unordered_map<std::string_view, int> freq_words;
  freq_words.reserve(words.size());
  for (auto i = 0U; i < words.size(); ++i) {
    freq_words.emplace(words[i], freqs[i]);
  }

  auto legal_words = solve(cs, words);
  std::cout << "found " << legal_words.size() << " possible matches.\n";

  // Prune the legal words from the word list using an English dictionary. The
  // word list contains "words" that might not appear in an actual dictionary.
  for (auto pos = legal_words.begin(); pos != legal_words.end();) {
    if (!dict.contains(*pos)) {
      pos = legal_words.erase(pos);
    } else {
      ++pos;
    }
  }
  std::ranges::sort(legal_words);
  legal_words.erase(std::ranges::unique(legal_words).begin(),
                    legal_words.end());
  std::cout << "found " << legal_words.size() << " dictionary matches.\n";

  // Generate letter frequency scores.
  std::unordered_map<std::string_view, int> letter_freq_words;
  letter_freq_words.reserve(legal_words.size());
  for (const auto &word : legal_words) {
    letter_freq_words.emplace(word, score(word));
  }

  std::cout << "==== Sorted by letter frequency\n";
  // Sort by letter freq score
  std::ranges::sort(legal_words,
                    [&letter_freq_words](const auto &l, const auto &r) {
                      return letter_freq_words[r] < letter_freq_words[l];
                    });
  for (const auto &s : legal_words | std::ranges::views::take(15)) {
    std::cout << s << "\n";
  }

  std::cout << "==== Sorted by word frequency\n";
  // Sort by word freq score
  std::sort(legal_words.begin(), legal_words.end(),
            [&freq_words](const auto &l, const auto &r) {
              return freq_words[r] < freq_words[l];
            });
  for (const auto &s : legal_words | std::ranges::views::take(15)) {
    std::cout << s << "\n";
  }

  std::cout << "==== Best start word\n";
  // Sort all words by letter frequency and report highest score.
  const auto best_word =
      std::reduce(words.begin() + 1, words.end(), words.front(),
                  [](const auto &word1, const auto &word2) {
                    return score(word1) > score(word2) ? word1 : word2;
                  });
  std::cout << best_word << "\n";
  return 0;
}
