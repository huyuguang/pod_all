
namespace scheme::plain {
std::mutex a_set_mutex;
std::unordered_map<void*, APtr> a_set;
std::mutex b_set_mutex;
std::unordered_map<void*, BPtr> b_set;

APtr GetAPtr(handle_t h) {
  APtr ret;
  std::scoped_lock<std::mutex> lock(a_set_mutex);
  auto it = a_set.find(h);
  if (it != a_set.end()) ret = it->second;
  return ret;
}

scheme::plain::BPtr GetBPtr(handle_t h) {
  scheme::plain::BPtr ret;
  std::scoped_lock<std::mutex> lock(b_set_mutex);
  auto it = b_set.find(h);
  if (it != b_set.end()) ret = it->second;
  return ret;
}

void AddA(A* p) {
  APtr ptr(p);
  std::scoped_lock<std::mutex> lock(a_set_mutex);
  a_set.insert(std::make_pair(p, ptr));
}

void AddB(B* p) {
  BPtr ptr(p);
  std::scoped_lock<std::mutex> lock(b_set_mutex);
  b_set.insert(std::make_pair(p, ptr));
}

bool DelA(A* p) {
  std::scoped_lock<std::mutex> lock(a_set_mutex);
  return a_set.erase(p) != 0;
}

bool DelB(B* p) {
  std::scoped_lock<std::mutex> lock(b_set_mutex);
  return b_set.erase(p) != 0;
}
}  // namespace scheme::plain