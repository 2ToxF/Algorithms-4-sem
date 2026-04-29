/* Пусть изначально словарь пуст. Обработайте q запросов следующих видов:
 * + s: добавить s в словарь. Если s уже находится в словаре, делать ничего не
 * нужно.
 * - s: удалить s из словаря. Если s нет в словаре, делать ничего не нужно.
 * - ? t: найти суммарное количество вхождений всех словарных слов (находящихся
 * в словаре на данный момент времени) в текст t.
 *
 *  Все строки s и t состоят только из букв «a», «b» и «c».
 *
 *
 * Симулирование онлайн-запросов производится следующим образом. Пусть ответ на
 * последний обработанный запрос типа «?» равен x (если запросов типа «?» ещё не
 * было, x=0). Перед обработкой очередного запроса нужно циклически сдвинуть
 * соответствующую строку (s или t) на x позиций влево. */

#include <array>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

enum class QueryType : char {
  Add = '+',
  Remove = '-',
  Count = '?',
};

class KarasManager {
 private:
  static const std::size_t kAlphabetSize = 3;

  struct AhoCorasickNode {
    std::array<std::size_t, kAlphabetSize> next_node{};
    std::size_t failure_link = 0;
    std::size_t match_count = 0;
  };

  struct AhoCorasick {
    std::vector<AhoCorasickNode> nodes;

    AhoCorasick() = default;

    explicit AhoCorasick(const std::vector<std::string>& strings) {
      BuildTrie(strings);
      BuildFailureLinks();
    }

    std::size_t CountMatches(const std::string& text) const {
      if (nodes.empty()) {
        return 0;
      }

      std::size_t total_matches = 0;
      std::size_t current_state = 0;

      for (std::size_t i = 0; i < text.size(); ++i) {
        std::size_t character_index = GetCharIndex(text[i]);
        current_state = nodes[current_state].next_node[character_index];
        total_matches += nodes[current_state].match_count;
      }

      return total_matches;
    }

   private:
    void BuildTrie(const std::vector<std::string>& strings) {
      nodes.emplace_back();

      for (std::size_t i = 0; i < strings.size(); ++i) {
        std::size_t current_state = 0;
        for (std::size_t j = 0; j < strings[i].size(); ++j) {
          std::size_t character_index = GetCharIndex(strings[i][j]);
          std::size_t next_state = nodes[current_state].next_node[character_index];

          if (next_state == 0) {
            std::size_t new_state_index = nodes.size();
            nodes[current_state].next_node[character_index] = new_state_index;
            nodes.emplace_back();
            current_state = new_state_index;
          } else {
            current_state = next_state;
          }
        }

        nodes[current_state].match_count += 1;
      }
    }

    void BuildFailureLinks() {
      std::queue<std::size_t> node_queue;
      for (std::size_t i = 0; i < kAlphabetSize; ++i) {
        std::size_t next_state = nodes[0].next_node[i];
        if (next_state != 0) {
          node_queue.push(next_state);
        }
      }

      while (!node_queue.empty()) {
        std::size_t current_state = node_queue.front();
        node_queue.pop();

        std::size_t failure_state = nodes[current_state].failure_link;
        nodes[current_state].match_count += nodes[failure_state].match_count;

        for (std::size_t i = 0; i < kAlphabetSize; ++i) {
          std::size_t next_state = nodes[current_state].next_node[i];
          std::size_t failure_next = nodes[failure_state].next_node[i];

          if (next_state != 0) {
            nodes[next_state].failure_link = failure_next;
            node_queue.push(next_state);
          } else {
            nodes[current_state].next_node[i] = failure_next;
          }
        }
      }
    }

    static std::size_t GetCharIndex(char character) {
      return static_cast<std::size_t>(character - 'a');
    }
  };

  struct AhoCorasickGroup {
    std::vector<std::string> strings;
    AhoCorasick automaton;

    AhoCorasickGroup() = default;

    explicit AhoCorasickGroup(const std::vector<std::string>& group_strings)
        : strings(group_strings), automaton(group_strings) {
    }

    void Clear() {
      strings.clear();
      automaton.nodes.clear();
    }
  };

  struct BinaryAhoCorasick {
    std::vector<AhoCorasickGroup> groups;

    void AddString(const std::string& new_string) {
      std::vector<std::string> current_strings;
      current_strings.push_back(new_string);

      MergeGroups(current_strings);

      if (!current_strings.empty()) {
        groups.emplace_back(current_strings);
      }
    }

    std::size_t CountMatches(const std::string& text) const {
      std::size_t total_matches = 0;
      for (std::size_t i = 0; i < groups.size(); ++i) {
        if (!groups[i].strings.empty()) {
          total_matches += groups[i].automaton.CountMatches(text);
        }
      }
      return total_matches;
    }

   private:
    void MergeGroups(std::vector<std::string>& current_strings) {
      for (std::size_t i = 0; i < groups.size(); ++i) {
        if (groups[i].strings.empty()) {
          groups[i] = AhoCorasickGroup(current_strings);
          current_strings.clear();
          return;
        }

        for (std::size_t j = 0; j < groups[i].strings.size(); ++j) {
          current_strings.push_back(groups[i].strings[j]);
        }
        groups[i].Clear();
      }
    }
  };

 public:
  void AddString(const std::string& text) {
    if (!active_strings_.contains(text)) {
      active_strings_.insert(text);
      added_automaton_.AddString(text);
    }
  }

  std::size_t CountMatches(const std::string& text) const {
    std::size_t added_matches = added_automaton_.CountMatches(text);
    std::size_t removed_matches = removed_automaton_.CountMatches(text);
    return added_matches - removed_matches;
  }

  void RemoveString(const std::string& text) {
    if (active_strings_.contains(text)) {
      active_strings_.erase(text);
      removed_automaton_.AddString(text);
    }
  }

 private:
  BinaryAhoCorasick added_automaton_;
  BinaryAhoCorasick removed_automaton_;
  std::unordered_set<std::string> active_strings_;
};

std::string ShiftString(const std::string& text, std::size_t shift_amount) {
  if (text.empty()) {
    return text;
  }

  std::size_t actual_shift = shift_amount % text.size();
  if (actual_shift == 0) {
    return text;
  }

  std::string result_string = text;
  for (std::size_t i = 0; i < text.size(); ++i) {
    std::size_t old_index = (i + actual_shift) % text.size();
    result_string[i] = text[old_index];
  }

  return result_string;
}

int main() {
  std::size_t query_count = 0;
  std::cin >> query_count;

  KarasManager manager;
  std::size_t last_answer = 0;

  for (std::size_t i = 0; i < query_count; ++i) {
    char query_type = '\0';
    std::string text;
    std::cin >> query_type >> text;

    std::string shifted_text = ShiftString(text, last_answer);

    switch (static_cast<QueryType>(query_type)) {
      case QueryType::Add:
        manager.AddString(shifted_text);
        break;
      case QueryType::Remove:
        manager.RemoveString(shifted_text);
        break;
      case QueryType::Count:
        last_answer = manager.CountMatches(shifted_text);
        std::cout << last_answer << '\n';
        break;
    }
  }

  return 0;
}
