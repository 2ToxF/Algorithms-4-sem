/*  Даны строки s и t. Постройте сжатое суффиксное дерево, которое содержит все
    суффиксы строки s и строки t. Найдите такое дерево, которое содержит
    минимальное количество вершин.

    Формат ввода
    В первой строке записана строка s (1≤∣s∣≤10^5), последний символ строки
    равен ‘`$`’, остальные символы строки — маленькие латинские буквы. Во второй
    строке записана строка t (1≤∣t∣≤10^5), последний символ строки равен ‘`#`’,
    остальные символы строки — маленькие латинские буквы.

    Формат вывода
    Пронумеруйте вершины дерева от 0 до n−1 в порядке обхода в глубину, обходя
    поддеревья в порядке лексикографической сортировки исходящих из вершины
    ребер. Используйте ASCII-коды символов для опре- деления их порядка. В
    первой строке выведите целое число n — количество вершин дерева. В следующих
    n−1 строках выведите описание вершин дерева, кроме корня, в порядке
    увеличения их номеров. Описание вершины дерева v состоит из четырех целых
    чисел: p, w, lf, rg, где p (0≤p<n, p≠v) — номер родителя текущей вершины, w
    (0≤w≤1) — номер строки для определения подстроки на ребре. Если w=0, то на
    ребре, ведущем из p в v, написана подстрока s[lf…rg−1] (0≤lf<rg≤∣s∣). Если
    w=1, то на ребре, ведущем из p в v, написана подстрока t[lf…rg−1]
    (0≤lf<rg≤∣t∣). */

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

static const std::size_t kInvalidNode = static_cast<std::size_t>(-1);
static const std::size_t kInfEnd = static_cast<std::size_t>(-1);

struct TreeNode {
  std::size_t start;
  std::size_t end;
  std::size_t suffix_link;
  std::map<char, std::size_t> children;

  TreeNode(std::size_t st, std::size_t en)
      : start(st), end(en), suffix_link(kInvalidNode) {}

  std::size_t EdgeLength(std::size_t cur_end) const {
    std::size_t real_end = (end == kInfEnd) ? cur_end : end;
    return real_end - start;
  }
};

class SuffixTree {
 public:
  explicit SuffixTree(const std::string& text)
      : text_(text),
        remainder_(0),
        active_node_(0),
        active_edge_(0),
        active_length_(0),
        cur_end_(0) {
    nodes_.emplace_back(0, 0);
    for (std::size_t i = 0; i < text_.size(); ++i) {
      ExtendTree(i);
    }
    FinalizeBuild();
  }

  const std::vector<TreeNode>& GetNodes() const { return nodes_; }

 private:
  std::string text_;
  std::vector<TreeNode> nodes_;
  std::size_t remainder_;
  std::size_t active_node_;
  std::size_t active_edge_;
  std::size_t active_length_;
  std::size_t cur_end_;

  std::size_t AddNode(std::size_t st, std::size_t en) {
    nodes_.emplace_back(st, en);
    return nodes_.size() - 1;
  }

  void LinkInternal(std::size_t& last_internal, std::size_t node) {
    if (last_internal != kInvalidNode) {
      nodes_[last_internal].suffix_link = node;
    }
    last_internal = node;
  }

  void MoveActivePoint(std::size_t phase) {
    if (active_node_ == 0 && active_length_ != 0) {
      --active_length_;
      active_edge_ = phase - remainder_ + 1;
    } else if (nodes_[active_node_].suffix_link != kInvalidNode) {
      active_node_ = nodes_[active_node_].suffix_link;
    } else {
      active_node_ = 0;
    }
  }

  void AddLeafFromActive(std::size_t phase, std::size_t& last_internal) {
    char edge_char = text_[active_edge_];
    std::size_t leaf = AddNode(phase, kInfEnd);
    nodes_[active_node_].children[edge_char] = leaf;
    LinkInternal(last_internal, active_node_);
    last_internal = kInvalidNode;
  }

  bool WalkDownIfNeeded(std::size_t next) {
    std::size_t edge_len = nodes_[next].EdgeLength(cur_end_);
    if (active_length_ < edge_len) {
      return false;
    }
    active_edge_ += edge_len;
    active_length_ -= edge_len;
    active_node_ = next;
    return true;
  }

  bool TryMatchOrSplit(std::size_t phase, std::size_t next,
                       std::size_t& last_internal) {
    if (text_[nodes_[next].start + active_length_] == text_[phase]) {
      ++active_length_;
      LinkInternal(last_internal, active_node_);
      return true;
    }

    char edge_char = text_[active_edge_];
    std::size_t split =
        AddNode(nodes_[next].start, nodes_[next].start + active_length_);
    nodes_[active_node_].children[edge_char] = split;

    std::size_t leaf = AddNode(phase, kInfEnd);
    nodes_[split].children[text_[phase]] = leaf;

    nodes_[next].start += active_length_;
    nodes_[split].children[text_[nodes_[next].start]] = next;

    LinkInternal(last_internal, split);
    last_internal = split;
    return false;
  }

  void ExtendTree(std::size_t phase) {
    cur_end_ = phase + 1;
    ++remainder_;
    std::size_t last_internal = kInvalidNode;

    while (remainder_ != 0) {
      if (active_length_ == 0) {
        active_edge_ = phase;
      }

      char edge_char = text_[active_edge_];
      if (!nodes_[active_node_].children.contains(edge_char)) {
        AddLeafFromActive(phase, last_internal);
        --remainder_;
        MoveActivePoint(phase);
        continue;
      }

      std::size_t next = nodes_[active_node_].children[edge_char];

      if (WalkDownIfNeeded(next)) {
        continue;
      }

      if (TryMatchOrSplit(phase, next, last_internal)) {
        break;
      }

      --remainder_;
      MoveActivePoint(phase);
    }
  }

  void FinalizeBuild() {
    for (std::size_t i = 0; i < nodes_.size(); ++i) {
      if (nodes_[i].end == kInfEnd) {
        nodes_[i].end = text_.size();
      }
    }
  }
};

struct EdgeLabel {
  std::size_t which_string;
  std::size_t left;
  std::size_t right;
};

EdgeLabel TranslateEdge(std::size_t start, std::size_t end,
                        std::size_t len_first) {
  if (start < len_first) {
    return {0, start, std::min(end, len_first)};
  }
  return {1, start - len_first, end - len_first};
}

struct OutputVertexDescription {
  std::size_t parent;
  std::size_t which_string;
  std::size_t left;
  std::size_t right;
};

struct StackFrame {
  std::size_t tree_node;
  std::size_t parent_out_id;
};

std::vector<OutputVertexDescription> CollectEntries(
    const std::vector<TreeNode>& nodes, std::size_t len_first) {
  std::vector<OutputVertexDescription> entries;
  std::vector<StackFrame> stk;
  stk.push_back({0, 0});
  std::size_t next_id = 0;

  while (!stk.empty()) {
    StackFrame frame = stk.back();
    stk.pop_back();

    std::size_t my_id = next_id;
    ++next_id;

    if (frame.tree_node != 0) {
      EdgeLabel label = TranslateEdge(nodes[frame.tree_node].start,
                                      nodes[frame.tree_node].end, len_first);
      entries.push_back(
          {frame.parent_out_id, label.which_string, label.left, label.right});
    }

    const std::map<char, std::size_t>& kids = nodes[frame.tree_node].children;

    std::vector<std::pair<char, std::size_t>> rev_kids(kids.rbegin(),
                                                       kids.rend());
    for (const auto& [ch, child] : rev_kids) {
      stk.push_back({child, my_id});
    }
  }

  return entries;
}

SuffixTree BuildTree(const std::string& str_first,
                     const std::string& str_second) {
  std::string concat = str_first + str_second;
  return SuffixTree(concat);
}

void PrintTree(const SuffixTree& tree, std::size_t len_first) {
  std::vector<OutputVertexDescription> entries =
      CollectEntries(tree.GetNodes(), len_first);

  std::size_t vertex_count = entries.size() + 1;
  std::cout << vertex_count << '\n';

  for (std::size_t i = 0; i < entries.size(); ++i) {
    std::cout << entries[i].parent << ' ' << entries[i].which_string << ' '
              << entries[i].left << ' ' << entries[i].right << '\n';
  }
}

int main() {
  std::string str_first;
  std::string str_second;
  std::getline(std::cin, str_first);
  std::getline(std::cin, str_second);

  SuffixTree tree = BuildTree(str_first, str_second);
  PrintTree(tree, str_first.size());
  return 0;
}
