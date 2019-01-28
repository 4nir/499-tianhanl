#include "store_adapter.h"

void StoreAdapter::Init() { store_client_->init(); }

bool StoreAdapter::StoreUserInfo(const UserInfo& user_info) {
  std::string serialized_string = "";
  user_info.SerializeToString(&serialized_string);
  return store_client_->Put(user_info.username(), serialized_string);
}

UserInfo StoreAdapter::GetUserInfo(const std::string& username) {
  std::vector<std::string> keys = {username};
  std::string serialized_string = "";
  UserInfo user_info;
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });
  user_info.ParseFromString(serialized_string);
  return user_info;
}

bool StoreAdapter::StoreChirp(const Chirp& chirp) {
  std::string serialized_string = "";
  chirp.SerializeToString(&serialized_string);
  return store_client_->Put(chirp.id(), serialized_string);
}

Chirp StoreAdapter::GetChirp(const std::string& chirp_id) {
  std::vector<std::string> keys = {chirp_id};
  std::string serialized_string = "";
  Chirp chirp;
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });
  chirp.ParseFromString(serialized_string);
  return chirp;
}

// TODO: Improve implementation to better utilize stream
std::vector<Chirp> StoreAdapter::GetChirpThread(const std::string& chirp_id) {
  Chirp curr_chirp = GetChirp(chirp_id);
  std::vector<Chirp> chirps = {curr_chirp};
  while (curr_chirp.parent_id() != "") {
    curr_chirp = GetChirp(chirp_id);
    chirps.push_back(curr_chirp);
  }
  return chirps;
}
