#include "core/log.hpp"
#include "ui/models/preferred_draw_systems_model.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/preferred_draw_systems_widget.hpp"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QTableView>
#include <QHeaderView>

constexpr int FIRST_ROW_WIDTH = 150;
constexpr int LAST_ROW_WIDTH = 100;

PreferredDrawSystemsWidget::PreferredDrawSystemsWidget(StoreManager &storeManager, QWidget *parent)
    : QWidget(parent)
    , mStoreManager(storeManager)
{
    QVBoxLayout *layout = new QVBoxLayout;
    auto *tableView = new QTableView;
    auto *model = new PreferredDrawSystemsModel(storeManager, tableView);

    tableView->setModel(model);
    // tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableView->setItemDelegate(new DrawSystemPreferenceDelegate(this));

    layout->addWidget(tableView);
    setLayout(layout);
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
    else if (index.column() == 1) {
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

