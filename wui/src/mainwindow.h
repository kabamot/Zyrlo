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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTextCursor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void start();
    void updateText(QString text);
    void highlighWord(const TextPosition &position);

private:
    void setCursorAtPosition(const TextPosition &position, QTextCursor &cursor);

private:
    Ui::MainWindow *ui;
    MainController m_controller;
    TextPosition m_prevPosition;
    QTextCharFormat m_prevFormat;
};
