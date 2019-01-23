#ifndef STORE
#define STORE

#include <mutex>
#include <string>
#include <unordered_map>

// Thread safe hash table based on std::unordered_map, mutators and accessors
// are guarded via mutex.
class Store {
 public:
  // returns empty string if specific keyed item is not exist
  std::string Get(std::string key);

  // returns false when the key and value cannot be put into map_
  bool Put(std::string key, std::string value);

  // return false if key is not exist
  bool Remove(std::string key);

 private:
  // stores key value pairs using default hash mechanism
  std::unordered_map<std::string, std::string> map_;
  // prevents race condition
  std::mutex map_mutex_;
};

#endif