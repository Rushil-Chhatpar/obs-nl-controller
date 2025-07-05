#pragma once
#include <QMainWindow>
#include <QComboBox>
#include "obs_controller.hpp"
#include "nlp_parser.hpp"

class QLineEdit;
class QPushButton;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onCommandEntered();
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onSwitchSceneButtonClicked();
    void updateStatus(const std::string& message);

private:
    void fetchSceneList();
    OBSController controller_;
    NLPParser parser_;
    QLineEdit* commandInput_;
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QComboBox* sceneComboBox_;
    QPushButton* switchSceneButton_;
    QLabel* statusLabel_;
};