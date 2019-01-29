#include "store_adapter.h"

void StoreAdapter::Init() { store_client_->Init(); }

bool StoreAdapter::StoreUserInfo(const UserInfo& user_info) {
  std::string serialized_string = "";
  user_info.SerializeToString(&serialized_string);
  return store_client_->Put(user_info.username(), serialized_string);
}

UserInfo StoreAdapter::GetUserInfo(const std::string& username) {
  // Get serialized UserInfo for the username and store in `serialized_string`
  std::vector<std::string> keys = {username};
  std::string serialized_string = "";
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });
  // Deserialize UserInfo
  UserInfo user_info;
  user_info.ParseFromString(serialized_string);
  return user_info;
}

bool StoreAdapter::StoreChirp(const Chirp& chirp) {
  std::string serialized_string = "";
  chirp.SerializeToString(&serialized_string);
  return store_client_->Put(chirp.id(), serialized_string);
}

Chirp StoreAdapter::GetChirp(const std::string& chirp_id) {
  // Get serialized Chirp for the chirp_id and store in `serialized_string`
  std::vector<std::string> keys = {chirp_id};
  std::string serialized_string = "";
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });
  // Deserialize Chirp
  Chirp chirp;
  chirp.ParseFromString(serialized_string);
  return chirp;
}

// TODO(tianhanl): Improve implementation to better utilize stream
std::vector<Chirp> StoreAdapter::GetChirpThread(const std::string& chirp_id) {
  Chirp curr_chirp = GetChirp(chirp_id);
  std::vector<Chirp> chirps = {curr_chirp};
  // Recursively gets parent chirp until reachs root chirp
  while (curr_chirp.parent_id() != "") {
    curr_chirp = GetChirp(chirp_id);
    chirps.push_back(curr_chirp);
  }
  return chirps;
}
