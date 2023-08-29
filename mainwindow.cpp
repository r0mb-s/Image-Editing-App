#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QtDebug>
#include <QShortcut>
#include <QMessageBox>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentImage(nullptr)
    , stackOfStates() {
    resize(800, 600);

    fileMenu = menuBar()->addMenu("&File");
    toolsMenu = menuBar()->addMenu("&Tools");

    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    setCentralWidget(imageView);

    mainStatusBar = statusBar();
    mainStatusBarLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusBarLabel);

    createActions();
}

QPixmap MainWindow::matToPixmap(cv::Mat &mat) {
    cv::Mat tmp;
    cv::cvtColor(mat, tmp, cv::COLOR_BGR2RGB);

    QImage im((const unsigned char *)(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_RGB888);

    return QPixmap::fromImage(im);
}

cv::Mat MainWindow::pixmapToMat(QPixmap &pixmap) {
    QImage tmp = pixmap.toImage().convertToFormat(QImage::Format_RGB888);

    cv::Mat mat(tmp.height(), tmp.width(), CV_8UC3);
    tmp.bytesPerLine();
    for (int i = 0; i < tmp.height(); i++)
        memcpy(mat.ptr(i), tmp.scanLine(i), tmp.bytesPerLine());

    return mat;
}

void MainWindow::updateImageScreen(QPixmap &image) {
    imageScene->clear();
    imageView->resetTransform();
    currentImage = imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
    QString status = QString("%1x%2, %3 KB").arg(image.width()).arg(image.height()).arg(QFile(currentImagePath).size() / 1000);
    mainStatusBarLabel->setText(status);
}

void MainWindow::createActions() {
    openAction = new QAction("&Open", this);
    fileMenu->addAction(openAction);
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));

    undoAction = new QAction("&Undo", this);
    fileMenu->addAction(undoAction);
    undoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    connect(undoAction, SIGNAL(triggered(bool)), this, SLOT(undoChange()));

    closeAction = new QAction("&Close", this);
    fileMenu->addAction(closeAction);
    connect(closeAction, SIGNAL(triggered(bool)), this, SLOT(closeImage()));

    exitAction = new QAction("&Exit", this);
    fileMenu->addAction(exitAction);
    exitAction->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(exit()));

    zoomInAction = new QAction("&Zoom in", this);
    toolsMenu->addAction(zoomInAction);
    zoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomInImage()));

    zoomOutAction = new QAction("&Zoom out", this);
    toolsMenu->addAction(zoomOutAction);
    zoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOutImage()));

    blurAction = new QAction("&Blur", this);
    toolsMenu->addAction(blurAction);
    blurAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    connect(blurAction, SIGNAL(triggered(bool)), this, SLOT(blurImage()));
}

void MainWindow::openImage() {
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open image");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Images (*.png *jpg)"));

    if (dialog.exec()) {
        currentImagePath = dialog.selectedFiles().at(0);
        QPixmap image(currentImagePath);

        updateImageScreen(image);

        stackOfStates.push(pixmapToMat(image));
    }
}

void MainWindow::undoChange() {
    if (stackOfStates.size() < 2 || stackOfStates.empty()) {
        QMessageBox::information(this, "Information", "No more undos!");
        return;
    }

    cv::Mat mat = stackOfStates.top();
    stackOfStates.pop();
    QPixmap image = matToPixmap(mat);

    updateImageScreen(image);
}

void MainWindow::closeImage() {
    imageScene->clear();
    mainStatusBarLabel->clear();
    currentImage = nullptr;
    currentImagePath.clear();
}

void MainWindow::zoomInImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "No image to zoom in on!");
        return;
    }
    imageView->scale(1.3, 1.3);
}

void MainWindow::zoomOutImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "No image to zoom out on!");
        return;
    }
    imageView->scale(1 / 1.3, 1 / 1.3);
}

void MainWindow::blurImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "No image to blur!");
        return;
    }

    QPixmap image = currentImage->pixmap();
    cv::Mat mat = pixmapToMat(image);
    cv::Mat matBlurred;
    cv::blur(mat, matBlurred, cv::Size(8, 8));
    image = matToPixmap(matBlurred);

    updateImageScreen(image);

    stackOfStates.push(mat);
}

MainWindow::~MainWindow() {
}
