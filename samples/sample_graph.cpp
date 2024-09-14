#include <glm/glm.hpp>
#include <lab>

#include <random>
#include <unordered_set>

struct Node {
  glm::vec2 position;
  float scale;
};

struct Link {
  uint16_t a, b;
  operator bool() const noexcept { return a != b; }
  bool selfloop() const noexcept { return a == b; }
};

bool operator==(const Link& l, const Link& r) noexcept {
  return (l.a == r.a && l.b == r.b) || (l.a == r.b && l.b == r.a) || (l.a == l.b && r.a == r.b);
}

// specialization of std::hash for Link objects to be able to insert Links into a set
template<>
struct std::hash<Link> {
  // Custom hash function for `Link` objects
  // The `hash(a) xor hash(b)` pattern guarantees the following:
  //  - returns the same hash value for `Link{a,b}` and `Link{b,a}` to prevent duplicate links
  //  - returns `0` hash for `Link{a, a}` (all but one self-looping link excluded automatically from set)
  //      if used in a set, a call to `set.erase(Graph::Link{0, 0})`
  //      can be used to clean the set from any potential self-looping link
  size_t operator()(const Link& lnk) const noexcept {
    return std::hash<uint16_t>{}(lnk.a) ^ std::hash<uint16_t>{}(lnk.b);
  }
};

struct Graph {
  std::vector<Node> nodes;
  std::unordered_set<Link> links;
  void randomize(size_t node_count, float connectivity);
  void add_node(glm::vec2 position, float scale);
  void add_link(uint16_t a, uint16_t b);
};

int main() {
  // randomly generate a graph
  Graph g1;
  g1.randomize(12, 0.8f);

  for (auto& n : g1.nodes) {
    std::cout << "node: " << n.position.x << ", " << n.position.y << std::endl;
  }
  for (auto& l : g1.links) {
    std::cout << "link: " << l.a << ", " << l.b << "\n";
    if (l.a == l.b) {
      std::cout << "!!!!!!!!!\n";
    }
  }
  for (auto& l : g1.links) {
    for (auto& r : g1.links) {
      if (&l != &r) {
        if (l.a == r.b && l.b == r.a) {
          std::cout << "!!!!!!!!!\n";
        }
      }
    }
  }
}

void Graph::randomize(size_t node_count, float connectivity) {
  connectivity = std::clamp(connectivity, 0.0f, 1.0f);
  size_t link_count = size_t(std::round(float(node_count * (node_count - 1) / 2) * connectivity));

  std::default_random_engine prng{std::random_device{}()};

  std::normal_distribution<float> dst_pos{0.0f, 0.2f};
  std::normal_distribution<float> dst_scl{0.02f, 0.005f};

  nodes.reserve(node_count);
  while (nodes.size() < nodes.capacity()) {
    nodes.emplace_back(glm::vec2{dst_pos(prng), dst_pos(prng)}, dst_scl(prng));
  }

  std::uniform_int_distribution<uint16_t> dst_lnk{0, uint16_t(nodes.size() - 1)};

  links.reserve(link_count + 1);
  auto zero = links.emplace(0, 0);
  while (links.size() <= link_count) {
    links.emplace(dst_lnk(prng), dst_lnk(prng));
  }
  links.erase(zero.first);
}

void Graph::add_node(glm::vec2 position, float scale) { nodes.emplace_back(position, scale); }
void Graph::add_link(uint16_t a, uint16_t b) {
  if (a != b) links.emplace(a, b);
}
