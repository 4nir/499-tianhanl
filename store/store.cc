#include "store.h"

std::string Store::Get(std::string key) {
  std::lock_guard<std::mutex> guard(map_mutex_);
  auto search_result = map_.find(key);
  if (search_result == map_.end()) {
    return "";
  } else {
    return search_result->second;
  }
}

bool Store::Put(std::string key, std::string value) {
  std::lock_guard<std::mutex> guard(map_mutex_);
  map_[key] = value;
  return true;
}

// return false if key is not exist
bool Store::Remove(std::string key) {
  std::lock_guard<std::mutex> guard(map_mutex_);
  auto search_result = map_.find(key);
  bool is_succeed = search_result == map_.end();
  if (is_succeed) {
    map_.erase(search_result);
  }
  return is_succeed;
}