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
#include <QDebug>

#include "maincontroller.h"

MenuWidget::MenuWidget(MainController *controller, QWidget *parent)
    : QWidget(parent)
    , m_controller(controller)
    , m_listView(new QListView(this))
    , m_menuModel(new QStringListModel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_listView->setModel(m_menuModel);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto *layout = new QVBoxLayout();
    layout->addWidget(m_listView);

    setLayout(layout);

    connect(m_listView, &QListView::activated, this, [this](const QModelIndex &index){
        emit activated(index.row());
    });

    connect(m_listView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous)
    {
        Q_UNUSED(previous);
        m_controller->sayTranslationTag(m_menuModel->data(current).toString());
    });
}

void MenuWidget::setItems(const QStringList &items)
{
    m_menuModel->setStringList(items);
}
