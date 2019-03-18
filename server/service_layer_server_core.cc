#include "service_layer_server_core.h"

namespace chirpsystem {
void ServiceLayerServerCore::Init(bool dev) { store_adapter_.Init(dev); }

bool ServiceLayerServerCore::RegisterUser(const std::string& username) {
  // username cannot be empty
  if (username == "") return false;
  // username must be unique
  if (store_adapter_.KeyExists(username)) return false;
  UserInfo new_user_info;
  new_user_info.set_username(username);
  // Ownership of timestamp pointer is transferred to new_user_info
  new_user_info.set_allocated_timestamp(MakeCurrentTimestamp());
  return store_adapter_.StoreUserInfo(new_user_info);
}

ServiceLayerServerCore::ChirpStatus ServiceLayerServerCore::SendChirp(
    Chirp& chirp, const std::string& username, const std::string& text,
    const std::string& parent_id) {
  // Check if username is valid
  if (username == "" || !store_adapter_.KeyExists(username)) {
    return CHIRP_FAILED;
  }
  // If parend_id is present, check wether parent id is valid
  if (parent_id != "" && !store_adapter_.KeyExists(username)) {
    return CHIRP_FAILED;
  }
  // Prepares chirp
  Timestamp* current_timestamp = MakeCurrentTimestamp();
  chirp.set_username(username);
  chirp.set_text(text);
  chirp.set_parent_id(parent_id);
  chirp.set_id(username +
               std::to_string(current_timestamp->seconds() + std::rand()));
  chirp.set_allocated_timestamp(current_timestamp);
  bool chirp_ok = store_adapter_.StoreChirp(chirp);
  if (chirp_ok) {
    UserInfo curr_user_info = store_adapter_.GetUserInfo(username);
    curr_user_info.add_chirp_id_s(chirp.id());
    curr_user_info.set_allocated_timestamp(MakeCurrentTimestamp());
    bool user_ok = store_adapter_.StoreUserInfo(curr_user_info);
    if (user_ok) {
      return CHIRP_SUCCEED;
    } else {
      return UPDATE_USER_FAILED;
    }
  } else {
    return CHIRP_FAILED;
  }
}

bool ServiceLayerServerCore::Follow(const std::string& username,
                                    const std::string& to_follow) {
  // `username` should not be the same as `to_follow`
  if (username == to_follow) {
    return false;
  }
  UserInfo current_user_info = store_adapter_.GetUserInfo(username);
  UserInfo to_follow_user_info = store_adapter_.GetUserInfo(to_follow);
  // Can only follow registered `to_follow` for registered `username`
  if (current_user_info.username() != "" &&
      to_follow_user_info.username() != "") {
    current_user_info.add_following(to_follow);
    // update last modified time
    current_user_info.set_allocated_timestamp(MakeCurrentTimestamp());
    return store_adapter_.StoreUserInfo(current_user_info);
  } else {
    return false;
  }
}

std::vector<Chirp> ServiceLayerServerCore::Read(const std::string id) {
  // id can not be empty
  if (id == "") {
    return std::vector<Chirp>();
  }
  return store_adapter_.GetChirpThread(id);
}

// TODO(tianhanl): current implementation is not suitable to test
bool ServiceLayerServerCore::Monitor(
    const std::string& username,
    const std::function<bool(Chirp)>& handle_response, int interval,
    int time_limit) {
  // Cannot monitor for a not registered user
  if (username == "" || !store_adapter_.KeyExists(username)) {
    return false;
  }

  // Interval must be >= 0
  if (interval < 0) {
    return false;
  }

  // Starts polling
  PollUpdates(username, handle_response, interval, time_limit);
  return true;
}

Timestamp* ServiceLayerServerCore::MakeCurrentTimestamp() {
  Timestamp* timestamp = new Timestamp;
  auto current_time = system_clock::now().time_since_epoch();
  timestamp->set_seconds((duration_cast<seconds>(current_time)).count());
  timestamp->set_useconds((duration_cast<microseconds>(current_time)).count());
  return timestamp;
};

int ServiceLayerServerCore::GetCurrentTime() {
  auto mark_time = system_clock::now().time_since_epoch();
  return (duration_cast<seconds>(mark_time)).count();
}

// Current implementation uses start_time to decide from which chirps should the
// serach begin and uses seen_ids to filter duplications which helps solving the
// problem of chirps sent within one second.
std::vector<Chirp> ServiceLayerServerCore::GetFollowingChirpsAfterTime(
    const std::string& curr_username, int start_time,
    std::unordered_set<std::string>& seen_ids) {
  assert(curr_username != "");
  // Gets latest  user info incase the followings have been updated
  UserInfo curr_user_info = store_adapter_.GetUserInfo(curr_username);

  // If the users who current user is following have newer records,
  // store the new chirp to chirps
  std::vector<Chirp> chirps;

  for (const std::string& username : curr_user_info.following()) {
    UserInfo user_info = store_adapter_.GetUserInfo(username);
    // If user record has been updated after start_time
    if (user_info.timestamp().seconds() >= start_time) {
      // Adds chirp posted after mark_time and has not been seen to chirps
      for (int i = user_info.chirp_id_s_size() - 1; i >= 0; i--) {
        Chirp chirp = store_adapter_.GetChirp(user_info.chirp_id_s(i));
        const std::string& curr_id = chirp.id();
        // If the chirp is posted after mark_seconds and has not been inserted
        // into seen_ids, it is a new chirp
        if (chirp.timestamp().seconds() >= start_time &&
            seen_ids.find(curr_id) == seen_ids.end()) {
          chirps.push_back(chirp);
          seen_ids.insert(curr_id);
        } else {
          break;
        }
      }
    }
  }
  return chirps;
}

void ServiceLayerServerCore::PollUpdates(
    const std::string& username,
    const std::function<bool(Chirp)>& handle_response, const int interval,
    const int time_limit) {
  // Guard
  if (interval < 0) {
    return;
  }
  // Mark last_access_seconds, only post younger than it should be sentZ
  int start_time = GetCurrentTime();
  int last_query_seconds = start_time;
  std::unordered_set<std::string> seen_ids;

  // Uses polling to check updates.
  while (true) {
    // wait interval seconds
    std::this_thread::sleep_for(std::chrono::seconds(interval));

    std::vector<Chirp> chirps =
        GetFollowingChirpsAfterTime(username, last_query_seconds, seen_ids);

    // Now `chirps` has all chirps posted after last_query_seconds
    if (chirps.size() > 0) {
      // Chirps are sorted by creation time to ensure display order
      std::sort(chirps.begin(), chirps.end(), Older);

      // Set last_query_seconds to lastest posted chirp's seconds to avoid
      // duplication
      last_query_seconds = chirps.back().timestamp().seconds();
      for (Chirp chirp : chirps) {
        bool ok = handle_response(chirp);
        if (!ok) {
          return;
        }
      }
    }

    // Checks if time limit has been reached
    if (time_limit != -1 && GetCurrentTime() - start_time >= time_limit) {
      return;
    }
  }
}
}  // namespace chirpsystem