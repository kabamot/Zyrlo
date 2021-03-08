/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Mar 2021
 *
 ****************************************************************************/

#pragma once

#include <QWidget>
#include <QString>
#include <QVector>

class QListView;
class QStringListModel;
class QAction;
class MainController;

class MenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MenuWidget(const QString &name, MainController *controller, QWidget *parent = nullptr);
    void setItems(const QStringList &items);
    void enteredToMenu();

signals:
    void activated(int index);

private:
    MainController   *m_controller;
    QListView        *m_listView;
    QStringListModel *m_menuModel;
    QString           m_name;
};

