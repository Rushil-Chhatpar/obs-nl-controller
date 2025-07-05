#include "mainwindow.hpp"
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QTimer>
#include <QMetaType>
#include <thread>
#include <vector>
#include <string>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), controller_("ws://localhost:4455", "x826ciLsU5FJucvP", this), parser_(controller_) {
    // Register std::string and std::vector<std::string> for Qt signal/slot
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    commandInput_ = new QLineEdit(this);
    commandInput_->setPlaceholderText("Enter command (e.g., start recording, switch to scene Main)");
    layout->addWidget(commandInput_);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    startButton_ = new QPushButton("Start Recording", this);
    stopButton_ = new QPushButton("Stop Recording", this);
    buttonLayout->addWidget(startButton_);
    buttonLayout->addWidget(stopButton_);
    layout->addLayout(buttonLayout);

    QHBoxLayout* sceneLayout = new QHBoxLayout();
    sceneComboBox_ = new QComboBox(this);
    switchSceneButton_ = new QPushButton("Switch Scene", this);
    sceneLayout->addWidget(sceneComboBox_);
    sceneLayout->addWidget(switchSceneButton_);
    layout->addLayout(sceneLayout);

    statusLabel_ = new QLabel("Disconnected", this);
    layout->addWidget(statusLabel_);

    connect(commandInput_, &QLineEdit::returnPressed, this, &MainWindow::onCommandEntered);
    connect(startButton_, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);
    connect(stopButton_, &QPushButton::clicked, this, &MainWindow::onStopButtonClicked);
    connect(switchSceneButton_, &QPushButton::clicked, this, &MainWindow::onSwitchSceneButtonClicked);
    connect(&controller_, &OBSController::statusUpdated, this, &MainWindow::updateStatus);
    connect(&controller_, &OBSController::sceneListReceived, this, [this](const std::vector<std::string>& scenes) {
        sceneComboBox_->clear();
        for (const auto& scene : scenes) {
            sceneComboBox_->addItem(QString::fromStdString(scene));
        }
    });

    // Start WebSocket in a separate thread
    std::thread ws_thread([this]() {
        if (!controller_.connect()) {
            updateStatus("Failed to connect to OBS WebSocket");
        }
    });
    ws_thread.detach();

    // Fetch scene list after authentication
    QTimer::singleShot(2000, this, &MainWindow::fetchSceneList);
}

MainWindow::~MainWindow() {
    controller_.disconnect();
}

void MainWindow::onCommandEntered() {
    QString command = commandInput_->text();
    parser_.processCommand(command.toStdString());
    commandInput_->clear();
}

void MainWindow::onStartButtonClicked() {
    parser_.processCommand("start recording");
}

void MainWindow::onStopButtonClicked() {
    parser_.processCommand("stop recording");
}

void MainWindow::onSwitchSceneButtonClicked() {
    QString scene = sceneComboBox_->currentText();
    if (!scene.isEmpty()) {
        parser_.processCommand("switch to scene " + scene.toStdString());
    } else {
        statusLabel_->setText("No scene selected");
    }
}

void MainWindow::fetchSceneList() {
    controller_.sendCommand("GetSceneList", {});
}

void MainWindow::updateStatus(const std::string& message) {
    statusLabel_->setText(QString::fromStdString(message));
}