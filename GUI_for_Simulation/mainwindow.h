#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private:
    QStackedWidget* stackedWidget;
    QWidget* basePage;
    QWidget* simulationPage;

    // Base Page
    QSpinBox* processCountInput;
    QTableWidget* processTable;
    QPushButton* simulateButton;

    // Simulation Page
    QLabel* processExecutionLabel;
    QTableWidget* processStatesTable;
    QTableWidget* qTable;
    QPushButton* nextButton;

    int currentFileIndex;

    void setupBasePage();
    void setupSimulationPage();
    void updateTableRows(int rowCount);
    void simulate();
    void loadNextFile();
    void parseAndDisplayFile(const QString& content);
};

#endif // MAINWINDOW_H
