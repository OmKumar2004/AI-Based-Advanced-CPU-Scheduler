#include "mainwindow.h"
#include <QVBoxLayout>
#include <QSpinBox>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QStackedWidget>
#include <QHeaderView>

// MainWindow Constructor
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), currentFileIndex(0) {
    setupBasePage();
    setupSimulationPage();

    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(basePage);
    stackedWidget->addWidget(simulationPage);

    setCentralWidget(stackedWidget);

    // Set main window size
    resize(900, 720); // Start with 800px wide and 600px high

    // Start with the base page
    stackedWidget->setCurrentWidget(basePage);
}

// Setup for Base Page
void MainWindow::setupBasePage() {
    basePage = new QWidget(this);

    // Widgets
    processCountInput = new QSpinBox(basePage);
    processCountInput->setRange(1, 100);
    processCountInput->setValue(4);

    processTable = new QTableWidget(basePage);
    processTable->setColumnCount(7);
    processTable->setHorizontalHeaderLabels({"ID", "Burst", "Wait", "Priority", "CPU%", "Memory%", "Completed"});

    // Set column widths
    processTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    for (int i = 0; i < processTable->columnCount(); ++i) {
        processTable->horizontalHeader()->setMinimumSectionSize(120);
    }
    processTable->horizontalHeader()->setStretchLastSection(true); // Last column snaps to the window

    simulateButton = new QPushButton("Simulate", basePage);

    // Layout
    auto* layout = new QVBoxLayout(basePage);
    layout->addWidget(new QLabel("Enter the number of processes:", basePage));
    layout->addWidget(processCountInput);
    layout->addWidget(processTable);
    layout->addWidget(simulateButton);

    connect(processCountInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateTableRows);
    connect(simulateButton, &QPushButton::clicked, this, &MainWindow::simulate);

    updateTableRows(processCountInput->value());
}

// Setup for Simulation Page
void MainWindow::setupSimulationPage() {
    simulationPage = new QWidget(this);

    // Widgets
    processExecutionLabel = new QLabel("Will now execute Process ID: -", simulationPage);
    processStatesTable = new QTableWidget(simulationPage);
    qTable = new QTableWidget(simulationPage);
    nextButton = new QPushButton("Next", simulationPage);

    // Configure Process States Table
    processStatesTable->setColumnCount(7);
    processStatesTable->setHorizontalHeaderLabels({"ID", "Burst", "Wait", "Priority", "CPU%", "Memory%", "Completed"});
    processStatesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    for (int i = 0; i < processStatesTable->columnCount(); ++i) {
        processStatesTable->horizontalHeader()->setMinimumSectionSize(120);
    }
    processStatesTable->horizontalHeader()->setStretchLastSection(true);

    // Configure Q-Table
    qTable->setColumnCount(6);
    qTable->setHorizontalHeaderLabels({"Process ID", "Priority", "CPU Utilization", "Memory Usage", "Waiting Time", "Burst Time"});
    qTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    for (int i = 0; i < qTable->columnCount(); ++i) {
        qTable->horizontalHeader()->setMinimumSectionSize(120);
    }
    qTable->horizontalHeader()->setStretchLastSection(true);

    // Layout
    auto* layout = new QVBoxLayout(simulationPage);
    layout->addWidget(processExecutionLabel);
    layout->addWidget(new QLabel("Current Process States:", simulationPage));
    layout->addWidget(processStatesTable);
    layout->addWidget(new QLabel("Q-Table:", simulationPage));
    layout->addWidget(qTable);
    layout->addWidget(nextButton);

    connect(nextButton, &QPushButton::clicked, this, &MainWindow::loadNextFile);
}

// Update Process Table Rows
void MainWindow::updateTableRows(int rowCount) {
    processTable->setRowCount(rowCount);
    for (int i = 0; i < rowCount; ++i) {
        if (!processTable->item(i, 0)) {
            processTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        }
    }
}

// Simulate Button Clicked
void MainWindow::simulate() {
    QStringList processData;
    for (int i = 0; i < processTable->rowCount(); ++i) {
        QStringList row;
        for (int j = 0; j < processTable->columnCount(); ++j) {
            auto* item = processTable->item(i, j);
            row << (item ? item->text() : "");
        }
        processData << row.join(",");
    }

    QString inputFile = "input_data.txt";
    QFile file(inputFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << processData.join("\n");
        file.close();
    }

    QProcess::execute("./simulator", {inputFile});

    // Switch to simulation page
    stackedWidget->setCurrentWidget(simulationPage);
    loadNextFile();
}

// Load Next File
void MainWindow::loadNextFile() {
    QString fileName = QString("output_iteration_%1.txt").arg(currentFileIndex++);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        processExecutionLabel->setText("All processes executed!");
        nextButton->setEnabled(false);
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Parse and display content
    parseAndDisplayFile(content);
    if (!QFile::remove(fileName)) {
        qWarning("Failed to delete the file: %s", qPrintable(fileName));
    }
}

// Parse and Display File Content
void MainWindow::parseAndDisplayFile(const QString& content) {
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);

    // Parse Process States
    int processStatesStart = lines.indexOf("Current Process States:") + 2;
    int qTableStart = lines.indexOf("Q-Table:");

    QStringList processStateLines = lines.mid(processStatesStart, qTableStart - processStatesStart);
    processStatesTable->setRowCount(processStateLines.size());
    processStatesTable->setColumnCount(7); // As per your table structure
    processStatesTable->setHorizontalHeaderLabels({"ID", "Burst", "Wait", "Priority", "CPU%", "Memory%", "Completed"});

    for (int i = 0; i < processStateLines.size(); ++i) {
        QStringList columns = processStateLines[i].split('\t');
        for (int j = 0; j < columns.size(); ++j) {
            processStatesTable->setItem(i, j, new QTableWidgetItem(columns[j].trimmed()));
        }
    }

    // Parse Q-Table
    QStringList qTableLines = lines.mid(qTableStart+2);
    qTable->setRowCount(qTableLines.size()-1);
    qTable->setColumnCount(6); // Adjust as needed Process ID
    qTable->setHorizontalHeaderLabels({"Process ID", "Priority", "CPU Utilization", "Memory Usage", "Waiting Time", "Burst Time"});

    for (int i = 0; i < qTableLines.size()-1; ++i) {
        QStringList columns = qTableLines[i].split('\t');
        for (int j = 0; j < columns.size(); ++j) {
            qTable->setItem(i, j, new QTableWidgetItem(columns[j].trimmed()));
        }
    }

    // Update Execution Label
    QString executionLine = lines.last();
    processExecutionLabel->setText(executionLine);
}
