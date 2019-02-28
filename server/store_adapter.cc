#include "store_adapter.h"

#include <iterator>

void StoreAdapter::Init(bool dev) {
  if (dev) {
    store_client_ =
        std::unique_ptr<DevKeyValueStoreClient>(new DevKeyValueStoreClient);
  } else {
    store_client_ =
        std::unique_ptr<KeyValueStoreClient>(new KeyValueStoreClient);
  }
  store_client_->Init();
}

bool StoreAdapter::StoreUserInfo(const UserInfo& user_info) {
  // username cannot be empty
  if (user_info.username() == "") return false;
  std::string serialized_string;
  user_info.SerializeToString(&serialized_string);
  return store_client_->Put(user_info.username(), serialized_string);
}

UserInfo StoreAdapter::GetUserInfo(const std::string& username) {
  UserInfo user_info;
  // username must be not empty
  if (username == "") {
    return user_info;
  }
  // Get serialized UserInfo for the username and store in `serialized_string`
  std::vector<std::string> keys = {username};
  std::string serialized_string;
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });
  // Deserialize UserInfo
  user_info.ParseFromString(serialized_string);
  return user_info;
}

bool StoreAdapter::StoreChirp(const Chirp& chirp) {
  // chirp id cannot be empty
  if (chirp.id() == "") return false;

  // If chirp is a reply, store it as reply to its parent
  if (chirp.parent_id() != "") {
    StoreReply(chirp.id(), chirp.parent_id());
  }

  std::string serialized_string;
  chirp.SerializeToString(&serialized_string);
  return store_client_->Put(chirp.id(), serialized_string);
}

Chirp StoreAdapter::GetChirp(const std::string& chirp_id) {
  Chirp chirp;
  // chirp_id must be not empty
  if (chirp_id == "") {
    return chirp;
  }
  // Get serialized Chirp for the chirp_id and store in `serialized_string`
  std::vector<std::string> keys = {chirp_id};
  std::string serialized_string;
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });
  // Deserialize Chirp
  chirp.ParseFromString(serialized_string);
  return chirp;
}

bool StoreAdapter::KeyExists(const std::string& key) {
  std::string result = "";
  std::vector<std::string> keys = {key};
  store_client_->Get(keys, [&result](std::string value) { result = value; });
  return result != "";
}

std::vector<Chirp> StoreAdapter::GetChirpThread(const std::string& chirp_id) {
  std::vector<Chirp> chirps;
  std::vector<std::string> keys = GetThreadKeys(chirp_id);
  store_client_->Get(keys, [&chirps](std::string value) {
    // If value is empty means that the chirp with this id is not existing.
    if (value == "") return;
    Chirp chirp;
    chirp.ParseFromString(value);
    chirps.push_back(chirp);
  });
  return chirps;
}

std::vector<std::string> StoreAdapter::GetReplyIds(const std::string& curr_id) {
  std::vector<std::string> ids;

  // Get ReplyRecord for curr_id
  const std::string& key = "reply-" + curr_id;
  std::vector<std::string> keys = {key};
  std::string serialized_string;
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });

  // Store each reply_id to ids if store has a record for curr_id
  if (serialized_string != "") {
    ReplyRecord reply_record;
    reply_record.ParseFromString(serialized_string);
    for (std::string id : reply_record.reply_ids()) {
      ids.push_back(id);
    }
  }

  return ids;
}

bool StoreAdapter::StoreReply(const std::string& curr_id,
                              const std::string& parent_id) {
  // Use `reply-` as a prefix to indicate it is key for `ReplyRecord`
  const std::string& key = "reply-" + parent_id;

  // Get previous `ReplyRecord` for this key
  std::vector<std::string> keys = {key};
  std::string serialized_string;
  store_client_->Get(keys, [&serialized_string](std::string value) {
    serialized_string = value;
  });

  // Create a empty ReplyRecord
  ReplyRecord reply_record;
  // If previous record exists, restores its data
  if (serialized_string != "") {
    reply_record.ParseFromString(serialized_string);
  }
  reply_record.set_id(parent_id);
  reply_record.add_reply_ids(curr_id);

  // Store updated ReplyRecord
  reply_record.SerializeToString(&serialized_string);
  return store_client_->Put(key, serialized_string);
}

std::vector<std::string> StoreAdapter::GetThreadKeys(
    const std::string& chirp_id) {
  std::vector<std::string> keys = {chirp_id};
  int curr_pos = 0;
  // Using queue to do pre-order traversal
  while (curr_pos < keys.size()) {
    std::vector<std::string> curr_keys = GetReplyIds(keys[curr_pos]);
    keys.insert(keys.end(), std::make_move_iterator(curr_keys.begin()),
                std::make_move_iterator(curr_keys.end()));
    curr_pos += 1;
  }
  return keys;
}