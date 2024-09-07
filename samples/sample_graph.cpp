#include <lab>

#include <glm/glm.hpp>

#include <random>
#include <unordered_set>

struct Graph {
  struct Node {
    glm::vec2 position;
    float scale;
  };

  struct Link {
    uint16_t a, b;
    bool operator==(const Link& c) const {
      // Link(a, b) == Link(b, a) -> true
      return (a == c.a && b == c.b) | (a == c.b && b == c.a);
    }
  };

  void generate(size_t node_count, float connectivity = 0.5f);

  std::vector<Node> nodes;
  std::vector<Link> links;
};

int main() {
  Graph g1;
  g1.generate(12, 0.8f);
  for (auto& n : g1.nodes) {
    std::cout << "node: " << n.position.x << ", " << n.position.y << std::endl;
  }
  for (auto& l : g1.links) {
    std::cout << "link: " << l.a << ", " << l.b << "\n";

    if (l.a == l.b) {
      std::cout << "!!!!!!!!!\n";
    }
  }
}

// injects hash function for Graph::Link objects into std, so that unordered_set works
template<>
struct std::hash<Graph::Link> {
  size_t operator()(const Graph::Link& lnk) const noexcept {
    // the hash(a) xor hash(b) pattern guarantees the following:
    //  - returns 0 if a == b (all self referencing links can be excluded by checking hash != 0)
    //  - returns the same hash for Link{a,b} as for Link{b,a} to prevent
    return std::hash<uint16_t>{}(lnk.a) ^ std::hash<uint16_t>{}(lnk.b);
  }
};

void Graph::generate(size_t node_count, float connectivity) {
  connectivity = std::clamp(connectivity, 0.0f, 1.0f);
  size_t link_count = size_t(std::round(float(node_count * (node_count - 1) / 2) * connectivity));

  std::default_random_engine prng{std::random_device{}()};

  std::normal_distribution<float> dst_pos{0.0f, 0.2f};
  std::normal_distribution<float> dst_scl{0.02f, 0.005f};

  nodes.clear();
  links.clear();

  nodes.reserve(node_count);
  while (nodes.size() < nodes.capacity()) {
    nodes.emplace_back(glm::vec2{dst_pos(prng), dst_pos(prng)}, dst_scl(prng));
  }

  std::uniform_int_distribution<uint16_t> dst_lnk{0, uint16_t(nodes.size() - 1)};
  std::unordered_set<Link> unique_links;

  links.reserve(link_count);
  while (links.size() < links.capacity()) {
    auto link = unique_links.emplace(dst_lnk(prng), dst_lnk(prng));
    if (std::hash<Link>{}(*link.first) != 0 && link.second) links.emplace_back(link.first->a, link.first->b);
  }

  assert(std::hash<Link>{}({1, 2}) == std::hash<Link>{}({2, 1}));
  std::cout << std::hash<Link>{}({1, 2}) << ", " << std::hash<Link>{}({2, 1}) << std::endl;

  return;
}
