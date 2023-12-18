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
#include <tesseract/baseapi.h>
#include <festival/festival.h>
#include <stack>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    void createActions();
    QPixmap matToPixmap(cv::Mat &);
    cv::Mat pixmapToMat(QPixmap &);
    void updateImageScreen(QPixmap &);
    void sayThis(char *);

private slots:
    void openImage();
    void saveImage();
    void undoChange();
    void closeImage();
    void zoomInImage();
    void zoomOutImage();
    void blurImage();
    void cutImage();
    void selectTextImage();
    void selectFacesAndBlurImage();
    void rotateLeftImage();
    void rotateRightImage();
    void addTextImage();
    void removeTextImage();

private:
    QMenu *fileMenu;
    QMenu *toolsMenu;
    QMenu *textMenu;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusBarLabel;

    QAction *openAction;
    QAction *saveAction;
    QAction *undoAction;
    QAction *exitAction;
    QAction *closeAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *blurAction;
    QAction *cutAction;
    QAction *selectTextAction;
    QAction *selectFacesAndBlurAction;
    QAction *rotateLeftAction;
    QAction *rotateRightAction;
    QAction *addTextAction;
    QAction *removeTextAction;

    QString currentImagePath;
    QGraphicsPixmapItem *currentImage;

    std::stack<cv::Mat> stackOfStates;
    double scaleW;
    std::pair<QGraphicsEllipseItem *, QGraphicsEllipseItem *> pairOfPoints;
    QGraphicsRectItem *drept;
    std::pair<std::pair<int, int>, std::pair<int, int>> pairOfPointsCoordinates;
    QPainter *painter;
};
#endif // MAINWINDOW_H
