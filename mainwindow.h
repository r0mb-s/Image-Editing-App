#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QAction>
#include <QStatusBar>
#include <QGraphicsPixmapItem>

#include <opencv2/opencv.hpp>
#include <stack>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createActions();
    QPixmap matToPixmap(cv::Mat &);
    cv::Mat pixmapToMat(QPixmap &);
    void updateImageScreen(QPixmap &);

private slots:
    void openImage();
    void undoChange();
    void closeImage();
    void zoomInImage();
    void zoomOutImage();
    void blurImage();

private:
    QMenu *fileMenu;
    QMenu *toolsMenu;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusBarLabel;

    QAction *openAction;
    QAction *undoAction;
    QAction *exitAction;
    QAction *closeAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *blurAction;

    QString currentImagePath;
    QGraphicsPixmapItem *currentImage;

    std::stack<cv::Mat> stackOfStates;
};
#endif // MAINWINDOW_H
