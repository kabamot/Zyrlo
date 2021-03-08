/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Mar 2021
 *
 ****************************************************************************/

#include "menuwidget.h"

#include <QListView>
#include <QStringListModel>
#include <QLayout>

MenuWidget::MenuWidget(QWidget *parent)
    : QWidget(parent)
    , m_listView(new QListView(this))
    , m_menuModel(new QStringListModel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_listView->setModel(m_menuModel);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *layout = new QVBoxLayout();
    layout->addWidget(m_listView);

    setLayout(layout);

    connect(m_listView, &QListView::activated, this, &MenuWidget::onActivated);
}

void MenuWidget::setItems(const QStringList &items)
{
    m_menuModel->setStringList(items);
}

void MenuWidget::onActivated(const QModelIndex &index)
{
    if (index.data().toString().simplified() == tr("Exit")) {
        close();
    } else {
        emit activated(index.row());
    }
}
