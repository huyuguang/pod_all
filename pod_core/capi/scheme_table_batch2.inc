
namespace scheme::table::batch2 {
std::mutex session_set_mutex;
std::unordered_map<void*, SessionPtr> session_set;
std::mutex client_set_mutex;
std::unordered_map<void*, ClientPtr> client_set;

void AddSession(Session* p) {
  SessionPtr ptr(p);
  std::scoped_lock<std::mutex> lock(session_set_mutex);
  session_set.insert(std::make_pair(p, ptr));
}

SessionPtr GetSessionPtr(handle_t h) {
  SessionPtr ret;
  std::scoped_lock<std::mutex> lock(session_set_mutex);
  auto it = session_set.find(h);
  if (it != session_set.end()) ret = it->second;
  return ret;
}

bool DelSession(Session* p) {
  std::scoped_lock<std::mutex> lock(session_set_mutex);
  return session_set.erase(p) != 0;
}

void AddClient(Client* p) {
  ClientPtr ptr(p);
  std::scoped_lock<std::mutex> lock(client_set_mutex);
  client_set.insert(std::make_pair(p, ptr));
}

ClientPtr GetClientPtr(handle_t h) {
  ClientPtr ret;
  std::scoped_lock<std::mutex> lock(client_set_mutex);
  auto it = client_set.find(h);
  if (it != client_set.end()) ret = it->second;
  return ret;
}

bool DelClient(Client* p) {
  std::scoped_lock<std::mutex> lock(client_set_mutex);
  return client_set.erase(p) != 0;
}
}  // namespace scheme::table::batch2