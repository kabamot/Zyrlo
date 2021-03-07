/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QMainWindow>
#include <QTextCharFormat>

#include "maincontroller.h"
#include "textposition.h"
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTextCursor;
class QGridLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *ev);

private slots:
    void start();
    void updateText(QString text);
    void highlighWord(const TextPosition &position);
    void updatePreview(const cv::Mat &img);

private:
    void setCursorAtPosition(const TextPosition &position, QTextCursor &cursor);

private:
    Ui::MainWindow *ui;
    MainController m_controller;
    TextPosition m_prevPosition;
    QTextCharFormat m_prevFormat;
    cv::Mat m_prevImg;
    bool m_bSavePreviewImage = false, m_bPreviewOn = false;
};
