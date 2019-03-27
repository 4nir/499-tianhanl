#ifndef KEY_VALUE_STORE_CLIENT_INTERFACE
#define KEY_VALUE_STORE_CLIENT_INTERFACE

#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Defines interface for communicate with key_value_store_server_sync
class KeyValueStoreClientInterface {
 public:
  // Initializes the client. Should be called before `Put`, `Get` and
  // `DeleteKey` were called.
  virtual void Init() = 0;

  // Puts key-value pair into store, return true if succeed
  virtual bool Put(const std::string& key, const std::string& value) = 0;

  // Gets each item for key in keys. `hanle_response` will be called each time
  // `Get` receives a new reponse from stream.
  virtual bool Get(const std::vector<std::string>& keys,
                   const std::function<void(std::string)>& handle_response) = 0;

  // Deletes the key-value pair in store for the key, return true if succeed
  virtual bool DeleteKey(const std::string& key) = 0;
};
#endif