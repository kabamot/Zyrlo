/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QMainWindow>
#include "maincontroller.h"
#include "opencv2/opencv.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLabel;
class QGridLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void start();
    void updatePreview(const cv::Mat &img);

private:
    Ui::MainWindow *ui;
    MainController m_controller;
    cv::Mat m_prevImg;
    QLabel *m_pLabelPreview;
    bool m_bSavePreviewImage = false;
protected:
    void keyPressEvent(QKeyEvent *ev);
};
