#include "store_adapter.h"

void StoreAdapter::Init() { store_client_->Init(); }

bool StoreAdapter::StoreUserInfo(const UserInfo& user_info) {
  std::string serialized_string;
  user_info.SerializeToString(&serialized_string);
  return store_client_->Put(user_info.username(), serialized_string);
}

UserInfo StoreAdapter::GetUserInfo(const std::string& username) {
  // Get serialized UserInfo for the username and store in `serialized_string`
  std::vector<std::string> keys = {username};
  std::string serialized_string;
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });
  // Deserialize UserInfo
  UserInfo user_info;
  user_info.ParseFromString(serialized_string);
  return user_info;
}

bool StoreAdapter::StoreChirp(const Chirp& chirp) {
  // If chirp is a reply, store it as reply to its parent
  if (chirp.parent_id() != "") {
    std::lock_guard<std::mutex> guard(map_mutex_);
    reply_map_[chirp.parent_id()] = chirp.id();
  }
  std::string serialized_string;
  chirp.SerializeToString(&serialized_string);
  return store_client_->Put(chirp.id(), serialized_string);
}

Chirp StoreAdapter::GetChirp(const std::string& chirp_id) {
  // Get serialized Chirp for the chirp_id and store in `serialized_string`
  std::vector<std::string> keys = {chirp_id};
  std::string serialized_string;
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });
  // Deserialize Chirp
  Chirp chirp;
  chirp.ParseFromString(serialized_string);
  return chirp;
}

std::vector<Chirp> StoreAdapter::GetChirpThread(const std::string& chirp_id) {
  std::vector<Chirp> chirps;
  std::vector<std::string> keys = {chirp_id};

  // Find IDs of replies to `chirp_id`, and push the IDs to keys
  {
    std::lock_guard<std::mutex> guard(map_mutex_);
    auto search_result = reply_map_.find(chirp_id);
    while (search_result != reply_map_.end()) {
      keys.push_back(search_result->second);
      search_result = reply_map_.find(search_result->second);
    }
  }
  store_client_->Get(keys, [&chirps](std::string value) {
    Chirp chirp;
    chirp.ParseFromString(value);
    chirps.push_back(chirp);
  });
  return chirps;
}
