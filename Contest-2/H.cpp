/* Заданы две строки s, t и целое число k. Рассмотрим множество всех таких
 * непустых строк, которые встречаются как подстроки в s и t одновременно.
 * Найдите k-ую в лексикографическом порядке строку из этого множества.*/

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

static const std::size_t kAlphabetSize = 26;
static const std::size_t kNoState = static_cast<std::size_t>(-1);
static const long long kSafeLimit = 2e18;

struct SuffixAutomatonState {
  std::size_t len = 0;
  std::size_t link = kNoState;
  std::array<std::size_t, kAlphabetSize> next;
  bool in_first = false;
  bool in_second = false;

  SuffixAutomatonState() { next.fill(kNoState); }
};

bool IsCommon(const SuffixAutomatonState& state) {
  return state.in_first && state.in_second;
}

class GeneralizedSuffixAutomaton {
 public:
  GeneralizedSuffixAutomaton() : last_state_(0) { states_.emplace_back(); }

  void AddString(const std::string& text, bool is_second) {
    last_state_ = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
      std::size_t ch = static_cast<std::size_t>(text[i] - 'a');
      Extend(ch, is_second);
    }
  }

  std::vector<SuffixAutomatonState>& GetStates() { return states_; }

 private:
  std::vector<SuffixAutomatonState> states_;
  std::size_t last_state_;

  void MarkState(std::size_t state, bool is_second) {
    if (is_second) {
      states_[state].in_second = true;
    } else {
      states_[state].in_first = true;
    }
  }

  std::size_t CloneState(std::size_t source, std::size_t new_len) {
    std::size_t clone = states_.size();
    states_.push_back(states_[source]);
    states_[clone].len = new_len;
    return clone;
  }

  void RedirectTransitions(std::size_t walker, std::size_t char_idx,
                           std::size_t from_state, std::size_t to_state) {
    while (walker != kNoState && states_[walker].next[char_idx] == from_state) {
      states_[walker].next[char_idx] = to_state;
      walker = states_[walker].link;
    }
  }

  bool HandleExistingTransition(std::size_t char_idx, bool is_second) {
    if (states_[last_state_].next[char_idx] == kNoState) {
      return false;
    }

    std::size_t existing = states_[last_state_].next[char_idx];

    if (states_[existing].len == states_[last_state_].len + 1) {
      MarkState(existing, is_second);
      last_state_ = existing;
      return true;
    }

    std::size_t clone = CloneState(existing, states_[last_state_].len + 1);
    RedirectTransitions(last_state_, char_idx, existing, clone);
    states_[existing].link = clone;
    MarkState(clone, is_second);
    last_state_ = clone;
    return true;
  }

  std::size_t WalkUpAndLink(std::size_t walker, std::size_t char_idx,
                            std::size_t new_state) {
    while (walker != kNoState && states_[walker].next[char_idx] == kNoState) {
      states_[walker].next[char_idx] = new_state;
      walker = states_[walker].link;
    }
    return walker;
  }

  void AssignLink(std::size_t walker, std::size_t char_idx,
                  std::size_t new_state) {
    if (walker == kNoState) {
      states_[new_state].link = 0;
      return;
    }

    std::size_t existing = states_[walker].next[char_idx];

    if (states_[existing].len == states_[walker].len + 1) {
      states_[new_state].link = existing;
      return;
    }

    std::size_t clone = CloneState(existing, states_[walker].len + 1);
    RedirectTransitions(walker, char_idx, existing, clone);
    states_[existing].link = clone;
    states_[new_state].link = clone;
  }

  void Extend(std::size_t char_idx, bool is_second) {
    if (HandleExistingTransition(char_idx, is_second)) {
      return;
    }

    std::size_t new_state = states_.size();
    states_.emplace_back();
    states_[new_state].len = states_[last_state_].len + 1;
    MarkState(new_state, is_second);

    std::size_t walker = WalkUpAndLink(last_state_, char_idx, new_state);
    AssignLink(walker, char_idx, new_state);

    last_state_ = new_state;
  }
};

std::vector<std::size_t> SortStatesByLen(
    const std::vector<SuffixAutomatonState>& states) {
  std::size_t num_states = states.size();
  std::size_t max_len = 0;
  for (std::size_t i = 0; i < num_states; ++i) {
    max_len = std::max(max_len, states[i].len);
  }

  std::vector<std::size_t> counter(max_len + 1, 0);
  for (std::size_t i = 0; i < num_states; ++i) {
    ++counter[states[i].len];
  }
  for (std::size_t i = 1; i <= max_len; ++i) {
    counter[i] += counter[i - 1];
  }

  std::vector<std::size_t> order(num_states);
  for (std::size_t i = num_states; i > 0; --i) {
    order[--counter[states[i - 1].len]] = i - 1;
  }

  return order;
}

void PropagateMembership(std::vector<SuffixAutomatonState>& states) {
  std::vector<std::size_t> order = SortStatesByLen(states);

  for (std::size_t i = states.size(); i > 0; --i) {
    std::size_t st = order[i - 1];
    if (states[st].link == kNoState) {
      continue;
    }
    std::size_t lk = states[st].link;
    if (states[st].in_first) {
      states[lk].in_first = true;
    }
    if (states[st].in_second) {
      states[lk].in_second = true;
    }
  }
}

void ClampToSafeLimit(long long& value) { value = std::min(value, kSafeLimit); }

void AccumulateChildren(const std::vector<SuffixAutomatonState>& states,
                        std::size_t state, std::vector<long long>& reachable) {
  for (std::size_t ch = 0; ch < kAlphabetSize; ++ch) {
    std::size_t next = states[state].next[ch];
    if (next != kNoState) {
      reachable[state] += reachable[next];
      ClampToSafeLimit(reachable[state]);
    }
  }
}

std::vector<long long> CountReachableStrings(
    const std::vector<SuffixAutomatonState>& states) {
  std::vector<std::size_t> order = SortStatesByLen(states);
  std::size_t num_states = states.size();
  std::vector<long long> reachable(num_states, 0);

  for (std::size_t i = num_states; i > 0; --i) {
    std::size_t st = order[i - 1];
    if (IsCommon(states[st]) && st != 0) {
      reachable[st] = 1;
    }
    AccumulateChildren(states, st, reachable);
  }

  return reachable;
}

std::string TraceKthString(const std::vector<SuffixAutomatonState>& states,
                           const std::vector<long long>& reachable,
                           long long target_k) {
  std::string result;
  std::size_t cur_state = 0;
  long long remaining = target_k;

  while (remaining > 0) {
    for (std::size_t ch = 0; ch < kAlphabetSize; ++ch) {
      std::size_t next = states[cur_state].next[ch];
      if (next == kNoState) {
        continue;
      }
      if (reachable[next] < remaining) {
        remaining -= reachable[next];
        continue;
      }
      result += static_cast<char>('a' + ch);
      cur_state = next;
      if (IsCommon(states[cur_state])) {
        --remaining;
      }
      break;
    }
  }

  return result;
}

GeneralizedSuffixAutomaton BuildGeneralizedSuffixAutomaton(
    const std::string& str_first, const std::string& str_second) {
  GeneralizedSuffixAutomaton suffix_automation;
  suffix_automation.AddString(str_first, false);
  suffix_automation.AddString(str_second, true);
  return suffix_automation;
}

std::string FindKthCommonSubstring(const std::string& str_first,
                                   const std::string& str_second,
                                   long long target_k) {
  GeneralizedSuffixAutomaton suffix_automation =
      BuildGeneralizedSuffixAutomaton(str_first, str_second);

  std::vector<SuffixAutomatonState>& states = suffix_automation.GetStates();
  PropagateMembership(states);

  std::vector<long long> reachable = CountReachableStrings(states);

  if (reachable[0] < target_k) {
    return "-1";
  }

  return TraceKthString(states, reachable, target_k);
}

int main() {
  std::string str_first;
  std::string str_second;
  long long target_k = 0;
  std::cin >> str_first >> str_second >> target_k;

  std::cout << FindKthCommonSubstring(str_first, str_second, target_k) << '\n';
  return 0;
}
