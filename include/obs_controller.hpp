#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <functional>
#include <QObject>
#include <vector>

using websocketpp::client;
using websocketpp::config::asio_client;
using json = nlohmann::json;

class MainWindow;

class OBSController : public QObject {
    Q_OBJECT

public:
    OBSController(const std::string& host = "ws://localhost:4455", const std::string& password = "", QObject* parent = nullptr);
    ~OBSController();
    bool connect();
    void disconnect();
    bool sendCommand(const std::string& requestType, const json& requestData = {});
    void onMessage(const std::string& message);

signals:
    void statusUpdated(const std::string& message);
    void sceneListReceived(const std::vector<std::string>& scenes);

private:
    std::string computeAuthResponse(const std::string& password, const std::string& salt, const std::string& challenge);
    client<asio_client> ws_client_;
    client<asio_client>::connection_ptr connection_;
    std::string host_;
    std::string password_;
    bool authenticated_ = false;
};