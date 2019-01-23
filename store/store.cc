#include "store.h"

std::string Store::Get(std::string key)
{
  map_mutex_.lock();
  auto search_result = map_.find(key);
  if (search_result == map_.end())
  {
    return "";
  }
  else
  {
    return search_result->second;
  }
  map_mutex_.unlock();
}

bool Store::Put(std::string key, std::string value)
{
  map_mutex_.lock();
  map_[key] = value;
  map_mutex_.unlock();
  return true;
}

// return false if key is not exist
bool Store::Remove(std::string key)
{
  map_mutex_.lock();
  auto search_result = map_.find(key);
  bool is_succeed = search_result == map_.end();
  if (is_succeed)
  {
    map_.erase(search_result);
  }
  map_mutex_.unlock();
  return is_succeed;
}