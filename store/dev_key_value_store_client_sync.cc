#include "dev_key_value_store_client_sync.h"

void DevKeyValueStoreClient::Init() { return; }

bool DevKeyValueStoreClient::Put(const std::string& key,
                                 const std::string& value) {
  return store_.Put(key, value);
}

bool DevKeyValueStoreClient::Get(
    const std::vector<std::string>& keys,
    const std::function<void(std::string)>& handle_response) {
  for (const std::string& key : keys) {
    handle_response(store_.Get(key).value_or(""));
  }
  return true;
}

bool DevKeyValueStoreClient::DeleteKey(const std::string& key) {
  return store_.Remove(key);
}