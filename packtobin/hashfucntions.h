struct itemPositionHashFunction{
    size_t operator()(const std::array<double,3>& itemPos) const{
    size_t h1 = std::hash<double>()(itemPos[0]);
    size_t h2 = std::hash<double>()(itemPos[1]);
    size_t h3 = std::hash<double>()(itemPos[2]);
    return (h1 ^ (h2 << 1)) ^ (h3 << 2);
    }
};

struct itemPositionEqualsFunction{
  bool operator()( const std::array<double,3>& item1Pos, const std::array<double,3>& item2Pos ) const{
    return (item1Pos[0] == item2Pos[0] && item1Pos[1] == item2Pos[1] && item1Pos[2] == item2Pos[2]);
  }
};