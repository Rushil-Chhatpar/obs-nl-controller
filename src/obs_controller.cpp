#include "obs_controller.hpp"
#include <iostream>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <vector>
#include <iomanip>
#include <sstream>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

OBSController::OBSController(const std::string& host, const std::string& password)
    : host_(host), password_(password) {
    ws_client_.clear_access_channels(websocketpp::log::alevel::all);
    ws_client_.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect);
    ws_client_.init_asio();
    ws_client_.set_open_handler([this](websocketpp::connection_hdl hdl) {
        connection_ = ws_client_.get_con_from_hdl(hdl);
    });
    ws_client_.set_message_handler([this](websocketpp::connection_hdl, client<asio_client>::message_ptr msg) {
        onMessage(msg->get_payload());
    });
}

OBSController::~OBSController() {
    disconnect();
}

bool OBSController::connect() {
    websocketpp::lib::error_code ec;
    connection_ = ws_client_.get_connection(host_, ec);
    if (ec) {
        std::cerr << "Connection failed: " << ec.message() << std::endl;
        return false;
    }
    ws_client_.connect(connection_);
    ws_client_.run();
    return true;
}

void OBSController::disconnect() {
    if (connection_) {
        ws_client_.close(connection_, websocketpp::close::status::normal, "");
    }
    ws_client_.stop();
}

std::string OBSController::computeAuthResponse(const std::string& password, const std::string& salt, const std::string& challenge) {
    // Step 1: Concatenate password and salt, then SHA256
    std::string pass_salt = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(pass_salt.c_str()), pass_salt.length(), hash);

    // Step 2: Convert SHA256 hash to base64
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    BIO_write(bio, hash, SHA256_DIGEST_LENGTH);
    BIO_flush(bio);

    char* base64_data;
    long base64_len = BIO_get_mem_data(bio, &base64_data);
    std::string base64_pass_salt(base64_data, base64_len);
    BIO_free_all(bio);

    // Step 3: Concatenate base64 hash with challenge, then SHA256 again
    std::string auth_challenge = base64_pass_salt + challenge;
    SHA256(reinterpret_cast<const unsigned char*>(auth_challenge.c_str()), auth_challenge.length(), hash);

    // Step 4: Convert final hash to base64
    bio = BIO_new(BIO_s_mem());
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    BIO_write(bio, hash, SHA256_DIGEST_LENGTH);
    BIO_flush(bio);

    base64_len = BIO_get_mem_data(bio, &base64_data);
    std::string auth_response(base64_data, base64_len);
    BIO_free_all(bio);

    return auth_response;
}

bool OBSController::sendCommand(const std::string& requestType, const json& requestData) {
    if (!authenticated_ || !connection_) {
        std::cerr << "Not connected or authenticated" << std::endl;
        return false;
    }
    json request = {
        {"op", 6}, // Request opcode for WebSocket 5.x
        {"d", {
            {"requestType", requestType},
            {"requestId", requestType + "_id"},
            {"requestData", requestData}
        }}
    };
    try {
        connection_->send(request.dump());
        std::cout << "Sent command: " << requestType << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error sending command: " << e.what() << std::endl;
        return false;
    }
    return true;
}

void OBSController::onMessage(const std::string& message) {
    json response = json::parse(message, nullptr, false);
    if (response.is_discarded()) {
        std::cerr << "Invalid JSON received" << std::endl;
        return;
    }
    std::cout << "Received: " << message << std::endl;

    if (response.contains("op") && response["op"] == 0) { // Hello message
        if (!password_.empty() && response["d"].contains("authentication")) {
            std::string salt = response["d"]["authentication"]["salt"];
            std::string challenge = response["d"]["authentication"]["challenge"];
            std::string auth_response = computeAuthResponse(password_, salt, challenge);
            json identify = {
                {"op", 1}, // Identify opcode
                {"d", {
                    {"rpcVersion", 1},
                    {"authentication", auth_response}
                }}
            };
            try {
                connection_->send(identify.dump());
            } catch (const std::exception& e) {
                std::cerr << "Error sending Identify: " << e.what() << std::endl;
            }
        } else {
            json identify = {
                {"op", 1}, // Identify opcode
                {"d", {{"rpcVersion", 1}}}
            };
            try {
                connection_->send(identify.dump());
            } catch (const std::exception& e) {
                std::cerr << "Error sending Identify: " << e.what() << std::endl;
            }
        }
    } else if (response.contains("op") && response["op"] == 2) { // Identified message
        authenticated_ = response["d"]["negotiatedRpcVersion"] == 1;
        std::cout << "Authentication " << (authenticated_ ? "successful" : "failed") << std::endl;
    } else if (response.contains("op") && response["op"] == 7) { // RequestResponse
        std::cout << "Command response: " << response["d"]["responseData"].dump() << std::endl;
    }
}