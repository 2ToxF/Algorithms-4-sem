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

#include <iostream>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

class KarasManager {
 private:
  static const std::size_t kAlphabetSize = 3;

  struct AcNode {
    std::size_t next_node[kAlphabetSize];
    std::size_t fail_node;
    std::size_t match_count;
  };

  struct AhoCorasick {
    std::vector<AcNode> nodes;
  };

  struct AcGroup {
    std::vector<std::string> strings;
    AhoCorasick automaton;
  };

  struct LogAhoCorasick {
    std::vector<AcGroup> groups;
  };

 public:
  void AddString(const std::string& text) {
    if (!active_strings_.contains(text)) {
      active_strings_.insert(text);
      AddStringToLogAc(added_automaton_, text);
    }
  }

  std::size_t CountMatches(const std::string& text) const {
    std::size_t added_matches = QueryLogAc(added_automaton_, text);
    std::size_t removed_matches = QueryLogAc(removed_automaton_, text);
    return added_matches - removed_matches;
  }

  void RemoveString(const std::string& text) {
    if (active_strings_.contains(text)) {
      active_strings_.erase(text);
      AddStringToLogAc(removed_automaton_, text);
    }
  }

 private:
  static void AddStringToLogAc(LogAhoCorasick& log_automaton,
                               const std::string& new_string) {
    std::vector<std::string> current_strings;
    current_strings.push_back(new_string);

    MergeGroups(log_automaton, current_strings);

    if (current_strings.size() != 0) {
      AcGroup new_group;
      new_group.strings = current_strings;
      new_group.automaton = BuildAc(current_strings);
      log_automaton.groups.push_back(new_group);
    }
  }

  static AhoCorasick BuildAc(const std::vector<std::string>& strings) {
    AhoCorasick automaton = BuildTrie(strings);
    BuildFailLinks(automaton);
    return automaton;
  }

  static void BuildFailLinks(AhoCorasick& automaton) {
    std::queue<std::size_t> node_queue;
    for (std::size_t i = 0; i < kAlphabetSize; ++i) {
      std::size_t next_state = automaton.nodes[0].next_node[i];
      if (next_state != 0) {
        node_queue.push(next_state);
      }
    }

    while (node_queue.size() != 0) {
      std::size_t current_state = node_queue.front();
      node_queue.pop();

      std::size_t fail_state = automaton.nodes[current_state].fail_node;
      automaton.nodes[current_state].match_count +=
          automaton.nodes[fail_state].match_count;

      for (std::size_t i = 0; i < kAlphabetSize; ++i) {
        std::size_t next_state = automaton.nodes[current_state].next_node[i];
        std::size_t fail_next = automaton.nodes[fail_state].next_node[i];

        if (next_state != 0) {
          automaton.nodes[next_state].fail_node = fail_next;
          node_queue.push(next_state);
        } else {
          automaton.nodes[current_state].next_node[i] = fail_next;
        }
      }
    }
  }

  static AhoCorasick BuildTrie(const std::vector<std::string>& strings) {
    AhoCorasick automaton;
    AcNode root_node = CreateNode();
    automaton.nodes.push_back(root_node);

    for (std::size_t i = 0; i < strings.size(); ++i) {
      std::size_t current_state = 0;
      for (std::size_t j = 0; j < strings[i].size(); ++j) {
        std::size_t character_index = GetCharIndex(strings[i][j]);
        std::size_t next_state =
            automaton.nodes[current_state].next_node[character_index];

        if (next_state == 0) {
          AcNode new_node = CreateNode();
          std::size_t new_state_index = automaton.nodes.size();
          automaton.nodes[current_state].next_node[character_index] =
              new_state_index;
          automaton.nodes.push_back(new_node);
          current_state = new_state_index;
        } else {
          current_state = next_state;
        }
      }

      automaton.nodes[current_state].match_count += 1;
    }
    return automaton;
  }

  static AcNode CreateNode() {
    AcNode new_node;
    for (std::size_t i = 0; i < kAlphabetSize; ++i) {
      new_node.next_node[i] = 0;
    }
    new_node.fail_node = 0;
    new_node.match_count = 0;
    return new_node;
  }

  static std::size_t GetCharIndex(char character) {
    if (character == 'a') {
      return 0;
    }
    if (character == 'b') {
      return 1;
    }
    return 2;
  }

  static void MergeGroups(LogAhoCorasick& log_automaton,
                          std::vector<std::string>& current_strings) {
    for (std::size_t i = 0; i < log_automaton.groups.size(); ++i) {
      if (log_automaton.groups[i].strings.size() == 0) {
        log_automaton.groups[i].strings = current_strings;
        log_automaton.groups[i].automaton = BuildAc(current_strings);
        current_strings.clear();
        return;
      }

      for (std::size_t j = 0; j < log_automaton.groups[i].strings.size(); ++j) {
        current_strings.push_back(log_automaton.groups[i].strings[j]);
      }
      log_automaton.groups[i].strings.clear();
      log_automaton.groups[i].automaton.nodes.clear();
    }
  }

  static std::size_t QueryAc(const AhoCorasick& automaton,
                             const std::string& text) {
    if (automaton.nodes.size() == 0) {
      return 0;
    }

    std::size_t total_matches = 0;
    std::size_t current_state = 0;

    for (std::size_t i = 0; i < text.size(); ++i) {
      std::size_t character_index = GetCharIndex(text[i]);
      current_state = automaton.nodes[current_state].next_node[character_index];
      total_matches += automaton.nodes[current_state].match_count;
    }

    return total_matches;
  }

  static std::size_t QueryLogAc(const LogAhoCorasick& log_automaton,
                                const std::string& text) {
    std::size_t total_matches = 0;
    for (std::size_t i = 0; i < log_automaton.groups.size(); ++i) {
      if (log_automaton.groups[i].strings.size() != 0) {
        total_matches += QueryAc(log_automaton.groups[i].automaton, text);
      }
    }
    return total_matches;
  }

  LogAhoCorasick added_automaton_;
  LogAhoCorasick removed_automaton_;
  std::unordered_set<std::string> active_strings_;
};

std::string ShiftString(const std::string& text, std::size_t shift_amount) {
  if (text.size() == 0) {
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

    if (query_type == '+') {
      manager.AddString(shifted_text);
    }
    if (query_type == '-') {
      manager.RemoveString(shifted_text);
    }
    if (query_type == '?') {
      last_answer = manager.CountMatches(shifted_text);
      std::cout << last_answer << '\n';
    }
  }

  return 0;
}
