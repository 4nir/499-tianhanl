#ifndef STORE_ADAPTER
#define STORE_ADAPTER

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../store/dev_key_value_store_client_sync.h"
#include "../store/key_value_store_client_sync.h"
#include "dist/service_layer.pb.h"

using chirp::Chirp;
using chirp::ReplyRecord;
using chirp::Timestamp;
using chirp::UserInfo;

namespace chirpsystem {
// Handles the serialization/deserialization of messags and the interaction with
// store server. In current implementation, each `Store` method will fully
// replace previous stored value.
// TODO: Change mthod return to make it easier to find what is the cause of
// errors
class StoreAdapter {
 public:
  // Initialize store_client_, should be called before all Get*/Store* methods
  // were called.
  // If `dev` is true, Init will use DevKeyValueStoreClient for store_client_,
  // which does not reply a running store_server instance.
  // Else Init will use KeyValueStoreClient for store_client_.
  void Init(bool dev = false);

  // Stores serialized `UserInfo`. Returns true if succeed.
  // `user_info` must have a username
  bool StoreUserInfo(const UserInfo& user_info);

  // Gets `UserInfo` associated with the username, return `UserInfo` has emtpy
  // username if username is not registerd.
  UserInfo GetUserInfo(const std::string& username);

  // Stores serialized `Chirp`, return false indicates given chirp cannot be
  // stored
  // `chirp` must have a id
  bool StoreChirp(const Chirp& chirp);

  // Gets the thread of chirps givien a starting chirp_id. The order of the
  // chirps in returned vector will be from curr chirp to all of its replies
  // Like: curr_chirp->reply1->reply2
  std::vector<Chirp> GetChirpThread(const std::string& chirp_id);

  //  Gets the chirp for a given chirp_id
  Chirp GetChirp(const std::string& chirp_id);

  // Checks is a key already existed in the store, return true if it is existed
  bool KeyExists(const std::string& key);

 private:
  // Gets the IDs of replies to `curr_id` chirp
  std::vector<std::string> GetReplyIds(const std::string& curr_id);

  // Stores `curr_id` chirp as a reply to `parent_id` chirp
  bool StoreReply(const std::string& curr_id, const std::string& parent_id);

  // Get all keys for the chirp thread starting from chirp_id, result is in
  // pre-order sequence
  // EX: A -> B
  //       -> C
  // returns [A, B, C]
  std::vector<std::string> GetThreadKeys(const std::string& chirp_id);

  // client interface used to communicate with store server
  std::unique_ptr<KeyValueStoreClientInterface> store_client_;
};
}  // namespace chirpsystem

#endif