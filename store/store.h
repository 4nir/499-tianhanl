#ifndef STORE
#define STORE

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

// Thread safe hash table based on std::unordered_map, mutators and accessors
// are guarded via mutex.
class Store {
 public:
  // returns `std::nullopt` if specific keyed item is not exist
  std::optional<std::string> Get(const std::string &key);

  // returns false when the key and value cannot be put into map_
  bool Put(const std::string &key, const std::string &value);

  // return false if key is not exist
  bool Remove(const std::string &key);

 private:
  // stores key value pairs using default hash mechanism
  std::unordered_map<std::string, std::string> map_;
  // prevents race condition
  std::mutex map_mutex_;
};

#endif