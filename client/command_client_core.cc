#include "./command_client_core.h"

void CommandClientCore::Run(const std::string& register_user,
                            const std::string& user, const std::string& chirp,
                            const std::string& reply, const std::string& follow,
                            const std::string& read, bool monitor) {
  service_layer_client_.Init();

  // Retrives current user's username.
  // Clients must have a username to be authorized for all activities
  std::string username = "";
  if (register_user != "") {
    username = register_user;
    // register_user this username
    bool ok = service_layer_client_.RegisterUser(username);
    if (!ok) {
      cout << "username: " << username << " cannot be registered" << endl;
      return;
    }
  } else if (user != "") {
    username = user;
  } else {
    cout << "A username is required to chirp. Please use --register or "
            "--user to"
         << " provide a username." << endl;
    return;
  }

  // use must reply with a chirp
  if (reply != "" && chirp == "") {
    cout << "--reply must be used with --chirp" << endl;
  }

  // Creates a chirp if has one.
  if (chirp != "") {
    SendChirp(username, chirp, reply);
  }

  // Read chirp thread if supplied a --read chirp_id
  if (read != "") {
    ReadChirpThread(read);
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
  for (Chirp chirp : chirps) {
    cout << "Time: " << chirp.timestamp().seconds() << endl;
    cout << "Username: " << chirp.username() << endl;
    cout << "Text: " << chirp.text() << endl;
    cout << endl;
  }
}