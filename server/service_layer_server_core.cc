#include "./service_layer_server_core.h"

void ServiceLayerServerCore::Init(bool dev) { store_adapter_.Init(dev); }

bool ServiceLayerServerCore::RegisterUser(const std::string& username) {
  if (username == "") return false;
  UserInfo new_user_info;
  new_user_info.set_username(username);
  // Ownership of timestamp pointer is transferred to new_user_info
  new_user_info.set_allocated_timestamp(MakeCurrentTimestamp());
  return store_adapter_.StoreUserInfo(new_user_info);
}

ServiceLayerServerCore::ChirpStatus ServiceLayerServerCore::SendChirp(
    Chirp& chirp, const std::string& username, const std::string& text,
    const std::string& parent_id) {
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
  UserInfo current_user_info = store_adapter_.GetUserInfo(username);
  current_user_info.add_following(to_follow);
  // update last modified time
  current_user_info.set_allocated_timestamp(MakeCurrentTimestamp());
  return store_adapter_.StoreUserInfo(current_user_info);
}

std::vector<Chirp> ServiceLayerServerCore::Read(const std::string id) {
  return store_adapter_.GetChirpThread(id);
}

bool ServiceLayerServerCore::Monitor(
    const std::string& username,
    const std::function<bool(Chirp)>& handle_response, int time_limit) {
  UserInfo curr_user_info = store_adapter_.GetUserInfo(username);
  // Starts monitoring
  std::thread monitoring(
      [&curr_user_info, handle_response, time_limit, this]() {
        std::vector<Chirp> chirps;
        // Mark last_access_seconds, only post younger than it should be sent
        int start_time = GetCurrentTime();
        int last_access_seconds = start_time;
        // Uses polling to check updates.
        while (true) {
          std::vector<Chirp> chirps;
          // Checks if time limit has been reached
          if (time_limit != -1 && GetCurrentTime() - start_time >= time_limit) {
            return;
          }
          // Store curent access time
          int mark_seconds = last_access_seconds;
          // wait 2 seconds
          std::this_thread::sleep_for(std::chrono::seconds(2));
          // If the users who current user is following has newer record,
          // store the new chirp to chiprs
          for (const std::string& username : curr_user_info.following()) {
            UserInfo user_info = store_adapter_.GetUserInfo(username);
            // If user record has been updated after mark_time
            if (user_info.timestamp().seconds() >= mark_seconds) {
              // Adds chirp posted after mark_time to chirps
              for (int i = user_info.chirp_id_s_size() - 1; i >= 0; i--) {
                Chirp chirp = store_adapter_.GetChirp(user_info.chirp_id_s(i));
                // If the chirp is posted after mark_seconds, it is a new
                // chirp
                if (chirp.timestamp().seconds() >= mark_seconds) {
                  last_access_seconds = std::max(
                      last_access_seconds, int(chirp.timestamp().seconds()));
                  chirps.push_back(chirp);
                } else {
                  break;
                }
              }
            }
          }
          // Now `chirps` has all chirps posted after time of mark
          if (chirps.size() > 0) {
            // Increase last_access_seconds to avoid duplication
            last_access_seconds += 1;
            std::sort(chirps.begin(), chirps.end(), Older);
            for (Chirp chirp : chirps) {
              bool ok = handle_response(chirp);
              if (!ok) {
                return;
              }
            }
          }
        }
      });
  // Wait for monitoring to end
  monitoring.join();
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