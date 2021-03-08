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

class MenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MenuWidget(QWidget *parent = nullptr);
    void setItems(const QStringList &items);

signals:
    void activated(int index);

private slots:
    void onActivated(const QModelIndex &index);

private:
    QListView *m_listView;
    QStringListModel *m_menuModel;
};

