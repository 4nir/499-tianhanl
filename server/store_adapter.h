#ifndef STORE_ADAPTER
#define STORE_ADAPTER

#include "../store/key_value_store_client_sync.h"
#include "./dist/service_layer.pb.h"

#include <memory>
#include <string>
#include <vector>

using chirp::Chirp;
using chirp::Timestamp;
using chirp::UserInfo;

// Handles the serialization/deserialization of messags and the interaction with
// store server. In current implementation, each `Store` method will fully
// replace previous stored value.
class StoreAdapter {
 public:
  StoreAdapter() {
    store_client_ =
        std::unique_ptr<KeyValueStoreClient>(new KeyValueStoreClient);
  }
  // Initialize store_client_, should be called before all Get*/Store* methods
  // were called.
  void Init();
  // Stores serialized `UserInfo`. Returns true if succeed.
  bool StoreUserInfo(const UserInfo& user_info);
  // Gets `UserInfo` associated with the username
  UserInfo GetUserInfo(const std::string& username);
  // Stores serialized `Chirp`
  bool StoreChirp(const Chirp& chirp);
  // Gets the thread of chirps givien a starting chirp_id. The order of the
  // chirps in returned vector will be from latest reply to the root chirp.
  // Like: reply2->reply1->original_chirp
  std::vector<Chirp> GetChirpThread(const std::string& chirp_id);
  //  Gets the chirp for a given chirp_id
  Chirp GetChirp(const std::string& chirp_id);

 private:
  // client interface used to communicate with store server
  std::unique_ptr<KeyValueStoreClient> store_client_;
};

#endif