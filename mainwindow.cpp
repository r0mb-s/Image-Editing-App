#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QtDebug>
#include <QShortcut>
#include <QMessageBox>
#include <QThread>
#include <QMouseEvent>
#include <QPushButton>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentImage(nullptr)
    , stackOfStates()
    , scaleW(1)
    , pairOfPoints(nullptr, nullptr)
    , pairOfPointsCoordinates()
    , drept(nullptr)
    , painter(nullptr) {

    int heap_size = 10000000;
    festival_initialize(1, heap_size);

    fileMenu = menuBar()->addMenu("&File");
    toolsMenu = menuBar()->addMenu("&Tools");
    textMenu = menuBar()->addMenu("&Text");

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

void getAndValidatePoints(std::pair<QGraphicsEllipseItem *, QGraphicsEllipseItem *> pairOP, std::pair<std::pair<int, int>, std::pair<int, int>> pairCO, QGraphicsPixmapItem *currentImage, int &x1, int &y1, int &x2, int &y2) {
    x2 = pairCO.second.first;
    y2 = pairCO.second.second;
    x1 = pairCO.first.first;
    y1 = pairCO.first.second;

    int height = currentImage->pixmap().height();
    int length = currentImage->pixmap().width();

    x1 = x1 > 0 ? x1 : 0;
    x2 = x2 > 0 ? x2 : 0;
    y1 = y1 > 0 ? y1 : 0;
    y2 = y2 > 0 ? y2 : 0;
    y1 = y1 > height ? height : y1;
    y2 = y2 > height ? height : y2;
    x1 = x1 > length ? length : x1;
    x2 = x2 > length ? length : x2;

    if (x1 > x2)
        std::swap(x1, x2);
    if (y1 > y2)
        std::swap(y1, y2);
}

void MainWindow::updateImageScreen(QPixmap &image) {
    imageScene->clear();
    imageView->resetTransform();
    currentImage = imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
    imageView->scale(scaleW, scaleW);
    QString status = QString("%1x%2, %3 KB").arg(image.width()).arg(image.height()).arg(QFile(currentImagePath).size() / 1000);
    mainStatusBarLabel->setText(status);

    pairOfPoints.first = nullptr;
    pairOfPoints.second = nullptr;
    drept = nullptr;
}

void MainWindow::createActions() {
    openAction = new QAction("&Open", this);
    fileMenu->addAction(openAction);
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));

    saveAction = new QAction("&Save", this);
    fileMenu->addAction(saveAction);
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    connect(saveAction, SIGNAL(triggered(bool)), this, SLOT(saveImage()));

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
    zoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
    connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomInImage()));

    zoomOutAction = new QAction("&Zoom out", this);
    toolsMenu->addAction(zoomOutAction);
    zoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOutImage()));

    blurAction = new QAction("&Blur", this);
    toolsMenu->addAction(blurAction);
    blurAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    connect(blurAction, SIGNAL(triggered(bool)), this, SLOT(blurImage()));

    cutAction = new QAction("&Cut", this);
    toolsMenu->addAction(cutAction);
    connect(cutAction, SIGNAL(triggered(bool)), this, SLOT(cutImage()));

    selectTextAction = new QAction("&Select text", this);
    toolsMenu->addAction(selectTextAction);
    connect(selectTextAction, SIGNAL(triggered(bool)), this, SLOT(selectTextImage()));

    selectFacesAndBlurAction = new QAction("&Blur all faces", this);
    toolsMenu->addAction(selectFacesAndBlurAction);
    connect(selectFacesAndBlurAction, SIGNAL(triggered(bool)), this, SLOT(selectFacesAndBlurImage()));

    rotateLeftAction = new QAction("&Rotate left", this);
    toolsMenu->addAction(rotateLeftAction);
    connect(rotateLeftAction, SIGNAL(triggered(bool)), this, SLOT(rotateLeftImage()));

    rotateRightAction = new QAction("&Rotate right", this);
    toolsMenu->addAction(rotateRightAction);
    connect(rotateRightAction, SIGNAL(triggered(bool)), this, SLOT(rotateRightImage()));

    addTextAction = new QAction("&Add text", this);
    textMenu->addAction(addTextAction);
    connect(addTextAction, SIGNAL(triggered(bool)), this, SLOT(addTextImage()));

    removeTextAction = new QAction("&Remove text", this);
    textMenu->addAction(removeTextAction);
    connect(removeTextAction, SIGNAL(triggered(bool)), this, SLOT(removeTextImage()));
}

void MainWindow::openImage() {
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open image");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Images (*.png *.jpg *.jpeg)"));

    if (dialog.exec()) {
        currentImagePath = dialog.selectedFiles().at(0);
        QPixmap image(currentImagePath);

        updateImageScreen(image);
    }
}

void MainWindow::saveImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nu se poate salva fara imagine!");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "/home/jana/untitled.png", tr("Images (*.png *.xpm *.jpg)"));
    if (!fileName.isEmpty()) {
        QImage im = currentImage->pixmap().toImage();
        im.save(fileName);
    }
}

void MainWindow::undoChange() {
    if (stackOfStates.empty()) {
        QMessageBox::information(this, "Information", "Nu mai exista undo-uri!");
        return;
    }

    QPixmap image = matToPixmap(stackOfStates.top());
    stackOfStates.pop();
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
        QMessageBox::information(this, "Information", "Nu exista imagine pentru a da zoom!");
        return;
    }
    imageView->scale(1.3, 1.3);
    scaleW *= 1.3;
}

void MainWindow::zoomOutImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nu exista imagine pentru a da zoom out!");
        return;
    }
    imageView->scale(1 / 1.3, 1 / 1.3);
    scaleW /= 1.3;
}

void MainWindow::blurImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nu exista imagine pentru a da blur!");
        return;
    }

    QPixmap image = currentImage->pixmap();
    cv::Mat mat = pixmapToMat(image);
    stackOfStates.push(mat.clone());
    cv::Mat matBlurred = mat.clone();

    if (pairOfPoints.first != nullptr && pairOfPoints.second != nullptr) {
        int x1, y1, x2, y2;
        getAndValidatePoints(pairOfPoints, pairOfPointsCoordinates, currentImage, x1, y1, x2, y2);
        cv::blur(mat(cv::Rect(x1, y1, x2 - x1, y2 - y1)), matBlurred(cv::Rect(x1, y1, x2 - x1, y2 - y1)), cv::Size(12, 12));
    } else {
        cv::blur(mat, matBlurred, cv::Size(12, 12));
    }

    image = matToPixmap(matBlurred);
    updateImageScreen(image);
}

void MainWindow::cutImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nu exista imagine pentru a da cut!");
        return;
    }

    if (pairOfPoints.first != nullptr && pairOfPoints.second != nullptr) {
        QPixmap image = currentImage->pixmap();
        cv::Mat mat = pixmapToMat(image);
        stackOfStates.push(mat.clone());
        cv::Mat matCropped;

        int x1, y1, x2, y2;
        getAndValidatePoints(pairOfPoints, pairOfPointsCoordinates, currentImage, x1, y1, x2, y2);
        mat(cv::Rect(x1, y1, x2 - x1, y2 - y1)).copyTo(matCropped);

        image = matToPixmap(matCropped);
        updateImageScreen(image);
    } else {
        QMessageBox::information(this, "Information", "Alegeti 2 puncte pentru a putea forma selctia de taiere!");
        return;
    }
}

void MainWindow::sayThis(char *say) {
    festival_say_text(say);
}

void MainWindow::selectTextImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nu exista imagine pentru a extrage text!");
        return;
    }

    QPixmap image = currentImage->pixmap();
    cv::Mat matF = pixmapToMat(image);

    if (pairOfPoints.first != nullptr && pairOfPoints.second != nullptr) {
        cv::Mat mat = pixmapToMat(image);

        int x1, y1, x2, y2;
        getAndValidatePoints(pairOfPoints, pairOfPointsCoordinates, currentImage, x1, y1, x2, y2);
        mat(cv::Rect(x1, y1, x2 - x1, y2 - y1)).copyTo(matF);
    }

    tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
    ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
    ocr->SetPageSegMode(tesseract::PSM_AUTO);
    ocr->SetImage(matF.data, matF.cols, matF.rows, 3, matF.step);

    char *text = ocr->GetUTF8Text();
    qDebug() << "Text din imagine: -" << text << "-";

    QDialog *diag = new QDialog;
    diag->resize(300, 30);
    QPushButton *button = new QPushButton(text, diag);
    connect(button, &QPushButton::clicked, this, [=]() {
        std::string cuv = ocr->GetUTF8Text();
        if (cuv.size() > 0)
            sayThis(ocr->GetUTF8Text());
    });
    diag->show();

    delete[] text;

    updateImageScreen(image);
}

void MainWindow::selectFacesAndBlurImage() {
    qDebug() << stackOfStates.size();
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nu exista imagine pentru a da blur la fete!");
        return;
    }

    QPixmap temp = currentImage->pixmap();
    cv::Mat im = pixmapToMat(temp);
    stackOfStates.push(im.clone());

    cv::CascadeClassifier faceCascade;
    faceCascade.load("/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_alt.xml");
    std::vector<cv::Rect> faces;
    faceCascade.detectMultiScale(im, faces, 1.1, 4, cv::CASCADE_SCALE_IMAGE, cv::Size(20, 20));

    qDebug() << faces.size();

    for (cv::Rect i : faces) {
        if (pairOfPoints.first != nullptr)
            if (pairOfPointsCoordinates.first.first > i.x && pairOfPointsCoordinates.first.first < i.x + i.width && pairOfPointsCoordinates.first.second > i.y && pairOfPointsCoordinates.first.second < i.y + i.height)
                continue;
        if (pairOfPoints.second != nullptr)
            if (pairOfPointsCoordinates.second.first > i.x && pairOfPointsCoordinates.second.first < i.x + i.width && pairOfPointsCoordinates.second.second > i.y && pairOfPointsCoordinates.second.second < i.y + i.height)
                continue;
        cv::Rect reg(i.x, i.y, i.width, i.height);
        cv::blur(im(reg), im(reg), cv::Size(51, 51));
    }

    temp = matToPixmap(im);
    updateImageScreen(temp);

    pairOfPoints.first = nullptr;
    pairOfPoints.second = nullptr;
}

void MainWindow::rotateLeftImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nu exista imagine pentru a o roti!");
        return;
    }

    QPixmap image = currentImage->pixmap();
    cv::Mat mat = pixmapToMat(image);
    stackOfStates.push(mat.clone());

    cv::rotate(mat, mat, cv::ROTATE_90_COUNTERCLOCKWISE);

    image = matToPixmap(mat);
    updateImageScreen(image);
}

void MainWindow::rotateRightImage() {
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nu exista imagine pentru a o roti!");
        return;
    }

    QPixmap image = currentImage->pixmap();
    cv::Mat mat = pixmapToMat(image);
    stackOfStates.push(mat.clone());

    cv::rotate(mat, mat, cv::ROTATE_90_CLOCKWISE);

    image = matToPixmap(mat);
    updateImageScreen(image);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (currentImage == nullptr) {
        openImage();
        return;
    }
    if (pairOfPoints.first != nullptr && pairOfPoints.second != nullptr) {
        pairOfPoints.first = nullptr;
        pairOfPoints.second = nullptr;
    }
    if (pairOfPoints.first == nullptr || pairOfPoints.second == nullptr) {
        QPointF pt = imageView->mapToScene(event->pos());
        pt.setX(pt.x() - 1 / scaleW);
        pt.setY(pt.y() - 19 / scaleW);

        qDebug() << pt << " - " << scaleW;

        if (pairOfPoints.first == nullptr) {
            if (drept) {
                imageScene->removeItem(drept);
                drept = nullptr;
            }
            pairOfPoints.first = imageScene->addEllipse(pt.x(), pt.y(), 5 / scaleW, 5 / scaleW, QPen(), QBrush(Qt::SolidPattern));
            pairOfPointsCoordinates.first.first = pt.x();
            pairOfPointsCoordinates.first.second = pt.y();
        } else {
            pairOfPoints.second = imageScene->addEllipse(pt.x(), pt.y(), 5 / scaleW, 5 / scaleW, QPen(), QBrush(Qt::VerPattern));
            pairOfPointsCoordinates.second.first = pt.x();
            pairOfPointsCoordinates.second.second = pt.y();

            int x1, y1, x2, y2;
            getAndValidatePoints(pairOfPoints, pairOfPointsCoordinates, currentImage, x1, y1, x2, y2);

            drept = imageScene->addRect(x1, y1, x2 - x1, y2 - y1, QPen(Qt::black, 5 / scaleW, Qt::DashDotLine));
            imageScene->removeItem(pairOfPoints.first);
            imageScene->removeItem(pairOfPoints.second);
        }
    }
}

void MainWindow::addTextImage() {
    bool ok;
    QString text = QInputDialog::getText(0, "Text", "", QLineEdit::Normal, "", &ok);
    int fontSize = QInputDialog::getInt(0, "Marimea scrisului", 0);
    QString culoare = QInputDialog::getItem(0, "Culoare text", "", { "negru", "rosu", "galben", "albastru", "alb" });
    QColor col;
    if (culoare.compare("negru") == 0)
        col.setRgb(0, 0, 0);
    if (culoare.compare("rosu") == 0)
        col.setRgb(255, 0, 0);
    if (culoare.compare("galben") == 0)
        col.setRgb(255, 255, 204);
    if (culoare.compare("albastru") == 0)
        col.setRgb(0, 0, 255);
    if (culoare.compare("alb") == 0)
        col.setRgb(255, 255, 255);

    if (ok && !text.isEmpty()) {
        QPixmap image = currentImage->pixmap();
        cv::Mat mat = pixmapToMat(image);
        stackOfStates.push(mat.clone());

        painter = new QPainter(&image);
        painter->setPen(QPen(col));
        painter->setFont(QFont("Times", fontSize, QFont::Bold));

        int x1, y1, x2, y2;
        getAndValidatePoints(pairOfPoints, pairOfPointsCoordinates, currentImage, x1, y1, x2, y2);
        painter->drawText(x1, y1, x2 - x1, y2 - y1, Qt::AlignCenter, text);

        painter->end();

        updateImageScreen(image);
    }
}

void MainWindow::removeTextImage() {
    return;
}

MainWindow::~MainWindow() {
}
