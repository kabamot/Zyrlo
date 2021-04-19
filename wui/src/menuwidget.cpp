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
#include <QLabel>
#include <QDebug>
#include <QShortcut>
#include <QtEvents>

#include "maincontroller.h"

MenuWidget::MenuWidget(const QString &name, MainController *controller, QWidget *parent)
    : QWidget(parent)
    , m_controller(controller)
    , m_listView(new QListView(this))
    , m_menuModel(new QStringListModel(this))
    , m_name(name)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_listView->setModel(m_menuModel);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto *nameLabel = new QLabel(name, this);

    auto *layout = new QVBoxLayout();
    layout->addWidget(nameLabel);
    layout->addWidget(m_listView);

    setLayout(layout);

    connect(m_listView, &QListView::activated, this, [this](const QModelIndex &index){
        emit activated(index.row(), index.data().toString());
    });

    connect(m_listView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous)
    {
        Q_UNUSED(previous);
        m_controller->sayTranslationTag(m_menuModel->data(current).toString());
    });

    // On Backspace activate last item in the menu (which has to be Exit)
    QShortcut *backShortcut = new QShortcut(Qt::Key_Backspace, this);
    connect(backShortcut, &QShortcut::activated, this, &MenuWidget::exit);
}

void MenuWidget::setItems(const QStringList &items)
{
    m_menuModel->setStringList(items);
}

void MenuWidget::removeItem(int index)
{
    m_menuModel->removeRow(index);
}

void MenuWidget::enteredToMenu()
{
    const auto menuItem = m_controller->translateTag(m_menuModel->data(m_listView->currentIndex()).toString());
    m_controller->sayText(QStringLiteral("%1. %2").arg(m_name, menuItem));
}

void MenuWidget::setItem(int nRow, const QString &item) {
    QMap<int, QVariant> mp;
    mp[0] = item;
    m_menuModel->setItemData(m_menuModel->index(nRow), mp);
    m_controller->sayText(item);
}

void MenuWidget::exit()
{
    auto lastRow = m_menuModel->rowCount() - 1;
    auto item = m_menuModel->data(m_menuModel->index(lastRow)).toString();
    emit activated(lastRow, item);
}

void MenuWidget::keyPressEvent(QKeyEvent *ev) {
    if(ev->key() == Qt::Key_Escape) {
        exit();
        return;
    }
    QWidget::keyReleaseEvent(ev);

}

void MenuWidget::keyReleaseEvent(QKeyEvent *ev) {
    QWidget::keyReleaseEvent(ev);
}
