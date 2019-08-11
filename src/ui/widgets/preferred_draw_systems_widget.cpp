#include "core/actions/add_draw_system_preference_row.hpp"
#include "core/actions/erase_draw_system_preference_row.hpp"
#include "ui/models/preferred_draw_systems_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/preferred_draw_systems_widget.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>

PreferredDrawSystemsWidget::PreferredDrawSystemsWidget(StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    QVBoxLayout *layout = new QVBoxLayout;
    mTableView = new QTableView;
    auto *model = new PreferredDrawSystemsModel(storeManager, mTableView);

    mTableView->setModel(model);
    // mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableView->horizontalHeader()->setStretchLastSection(true);
    mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    mTableView->setItemDelegate(new DrawSystemPreferenceDelegate(this));
    mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    mTableView->setColumnWidth(0, 150);
    connect(mTableView, &QTableView::customContextMenuRequested, this, &PreferredDrawSystemsWidget::showContextMenu);

    layout->addWidget(mTableView);
    setLayout(layout);
    setMinimumWidth(300);
}

std::unordered_set<std::size_t> PreferredDrawSystemsWidget::selectedRows() {
    std::unordered_set<std::size_t> rows;
    auto selection = mTableView->selectionModel()->selection();

    for (auto index : selection.indexes())
        rows.insert(index.row());

    return rows;
}

void PreferredDrawSystemsWidget::showContextMenu(const QPoint &pos) {
    auto rows = selectedRows();

    if (rows.empty())
        return;

    QMenu menu;
    {
        QAction *action = menu.addAction(tr("Insert row below"));
        connect(action, &QAction::triggered, this, &PreferredDrawSystemsWidget::insertRowBelow);
    }
    {
        QAction *action = menu.addAction(tr("Erase row"));
        if (rows.size() != 1 || *(rows.begin()) == 0)
            action->setEnabled(false);
        connect(action, &QAction::triggered, this, &PreferredDrawSystemsWidget::eraseRow);
    }

    menu.exec(mTableView->mapToGlobal(pos), 0);
}

DrawSystemPreferenceDelegate::DrawSystemPreferenceDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QWidget* DrawSystemPreferenceDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QWidget *editor;
    if (index.column() == 0) {
        QSpinBox *spinBox = new QSpinBox(parent);
        spinBox->setFrame(false);
        spinBox->setMinimum(1);

        editor = spinBox;
    }
    else { // (index.column() == 1) {
        QComboBox *comboBox = new QComboBox(parent);
        for (const auto &drawSystem : DrawSystem::getDrawSystems())
            comboBox->addItem(QString::fromStdString(drawSystem->getName()), QVariant::fromValue(drawSystem->getIdentifier()));
        comboBox->setFrame(false);
        connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, &DrawSystemPreferenceDelegate::commitComboBox);
        editor = comboBox;
    }

    return editor;
}

void DrawSystemPreferenceDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if (index.column() == 0) {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        int value = index.model()->data(index, Qt::EditRole).toInt();
        spinBox->setValue(value);
    }
    else if (index.column() == 1) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        auto identifier = index.model()->data(index, Qt::EditRole).value<DrawSystemIdentifier>();
        comboBox->setCurrentIndex(static_cast<int>(DrawSystem::getDrawSystemIndex(identifier)));
    }
}

void DrawSystemPreferenceDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if (index.column() == 0) {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        int value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }
    else if (index.column() == 1) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        QVariant value = comboBox->currentData();
        model->setData(index, value, Qt::EditRole);
    }
}

void DrawSystemPreferenceDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}

void DrawSystemPreferenceDelegate::commitComboBox() {
    QComboBox *editor = static_cast<QComboBox*>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void PreferredDrawSystemsWidget::insertRowBelow() {
    auto rows = selectedRows();

    if (rows.empty())
        return;

    std::size_t bottomRow = 0;
    for (std::size_t val : rows)
        bottomRow = std::max(val,bottomRow);

    mStoreManager.dispatch(std::make_unique<AddDrawSystemPreferenceRow>(bottomRow+1));
}

void PreferredDrawSystemsWidget::eraseRow() {
    auto rows = selectedRows();

    if (rows.size() != 1)
        return;

    std::size_t row = *(rows.begin());

    if (row == 0)
        return;

    mStoreManager.dispatch(std::make_unique<EraseDrawSystemPreferenceRow>(row));
}

