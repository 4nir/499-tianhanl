#include "command_client_core.h"

namespace chirpsystem {
void CommandClientCore::Run(const std::string& register_user,
                            const std::string& user, const std::string& chirp,
                            const std::string& reply, const std::string& follow,
                            const std::string& read, bool monitor, const std::string& stream) {
  service_layer_client_.Init();
  if (register_user != "") {
    bool ok = service_layer_client_.RegisterUser(register_user);
    if (!ok) {
      cout << "username: " << register_user << " cannot be registered" << endl;
    } else {
      cout << "username: " << register_user << " has been registered" << endl;
    }
    return;
  }

  // Retrives current user's username.
  // Clients must have a username to be authorized for all activities
  if (user == "") {
    cout << "a username is required for chirp, reply, follow, read, and ";
    cout << "monitor, plase provide a username using --user" << endl;
    return;
  }

  if (follow != "") {
    bool ok = service_layer_client_.Follow(user, follow);
    if (!ok) {
      cout << "username: " << follow << " cannot be followed" << endl;
    } else {
      cout << "username: " << follow << " has been followed" << endl;
    }
    return;
  }

  // use must reply with a chirp
  if (reply != "" && chirp == "") {
    cout << "--reply must be used with --chirp" << endl;
  }

  // Creates a chirp if has one.
  if (chirp != "") {
    SendChirp(user, chirp, reply);
    return;
  }

  // Read chirp thread if supplied a --read chirp_id
  if (read != "") {
    ReadChirpThread(read);
    return;
  }

  if (monitor) {
    std::cout << "Monitoring is started" << std::endl;
    // Use lambda to maintain reference to stream
    service_layer_client_.Monitor(user,
                                  [this](Chirp chirp) { PrintChirp(chirp); });
  }

  // Stream chirps --stream hashtag
  if (stream != "") {
    std::cout << "Calling stream() with hashtag: " << stream << std::endl;
    service_layer_client_.Stream(stream,
                                  [this](Chirp chirp) { PrintChirp(chirp); });
    return;
  }
}

void CommandClientCore::SendChirp(const std::string& username,
                                  const std::string& text,
                                  const std::string& parent_id) {
  const std::string& id =
      service_layer_client_.SendChirp(username, text, parent_id);
  if (id == "") {
    cout << "fail to chirp with inputted information" << endl;
  } else {
    cout << "chirp id is: " << id << endl;
  }
}

void CommandClientCore::ReadChirpThread(const std::string& id) {
  std::vector<Chirp> chirps = service_layer_client_.Read(id);
  if (chirps.size() == 0) {
    cout << "Cannot fetch chirp thread with id = " << id << endl;
    return;
  }
  for (Chirp chirp : chirps) {
    PrintChirp(chirp);
  }
}

void CommandClientCore::PrintChirp(Chirp chirp) {
  cout << "Time: " << chirp.timestamp().seconds() << endl;
  cout << "Username: " << chirp.username() << endl;
  cout << "Text: " << chirp.text() << endl;
  cout << endl;
}
}  // namespace chirpsystem