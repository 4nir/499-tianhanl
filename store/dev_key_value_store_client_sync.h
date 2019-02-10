#ifndef DEV_KEY_VALUE_STORE_CLIENT_SYNC
#define DEV_KEY_VALUE_STORE_CLIENT_SYNC
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "./key_value_store_client_interface.h"
#include "store.h"

// Client interface to communicate with key_value_store_server_sync.
class DevKeyValueStoreClient : public KeyValueStoreClientInterface {
 public:
  // Initializes the client store_. Should be called before `Put`, `Get` and
  // `DeleteKey` were called.
  void Init() override;

  // Puts key-value pair into store, return true if succeed
  bool Put(const std::string& key, const std::string& value) override;

  // Gets each item for key in keys. `hanle_response` will be called each time
  // `Get` receives a new reponse from stream.
  bool Get(const std::vector<std::string>& keys,
           std::function<void(std::string)> handle_response) override;

  // Deletes the key-value pair in store for the key, return true if succeed
  bool DeleteKey(const std::string& key) override;

 private:
  //  Store instance used to store key value pairs
  Store store_;
};
#endif