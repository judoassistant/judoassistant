#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

#include "store_managers/store_manager.hpp"
#include "widgets/import_players_csv_dialog.hpp"
#include "widgets/import_helpers/csv_reader.hpp"

ImportPlayersCSVDialog::ImportPlayersCSVDialog(StoreManager & storeManager, CSVReader *reader, QWidget *parent)
    : QDialog(parent)
    , mStoreManager(storeManager)
    , mReader(reader)
    , mImporter(mReader)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    {
        QGroupBox *group = new QGroupBox(tr("File format"));
        QFormLayout *formLayout = new QFormLayout;

        mHeaderContent = new QCheckBox;
        mHeaderContent->setCheckState(mImporter.hasHeaderRow() ? Qt::Checked : Qt::Unchecked);

        connect(mHeaderContent, &QCheckBox::stateChanged, this, &ImportPlayersCSVDialog::setHasHeaderRow);

        mDelimiterContent = new QComboBox;
        for (char a : CSVReader::listDelimiters()) {
            std::string s; s.push_back(a);
            mDelimiterContent->addItem(QString::fromStdString(s));
            if (a == mReader->getDelimiter())
                mDelimiterContent->setCurrentText(QString::fromStdString(s));
        }

        connect(mDelimiterContent, &QComboBox::currentTextChanged, this, &ImportPlayersCSVDialog::setDelimiter);

        formLayout->addRow(tr("Header row"), mHeaderContent);
        formLayout->addRow(tr("Field Delimiter"), mDelimiterContent);
        group->setLayout(formLayout);
        mainLayout->addWidget(group);
    }

    {
        QGroupBox *group = new QGroupBox(tr("Mapping of columns"));
        QFormLayout *formLayout = new QFormLayout;

        mFirstNameContent = new QComboBox;
        mLastNameContent = new QComboBox;
        mAgeContent = new QComboBox;
        mRankContent = new QComboBox;
        mClubContent = new QComboBox;
        mWeightContent = new QComboBox;
        mCountryContent = new QComboBox;
        mSexContent = new QComboBox;

        connect(mFirstNameContent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportPlayersCSVDialog::setFirstNameColumn);
        connect(mLastNameContent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportPlayersCSVDialog::setLastNameColumn);
        connect(mAgeContent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportPlayersCSVDialog::setAgeColumn);
        connect(mRankContent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportPlayersCSVDialog::setRankColumn);
        connect(mClubContent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportPlayersCSVDialog::setClubColumn);
        connect(mWeightContent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportPlayersCSVDialog::setWeightColumn);
        connect(mCountryContent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportPlayersCSVDialog::setCountryColumn);
        connect(mSexContent, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportPlayersCSVDialog::setSexColumn);

        refillColumnBoxes();

        formLayout->addRow(tr("First name"), mFirstNameContent);
        formLayout->addRow(tr("Last name"), mLastNameContent);
        formLayout->addRow(tr("Sex"), mSexContent);
        formLayout->addRow(tr("Age"), mAgeContent);
        formLayout->addRow(tr("Weight"), mWeightContent);
        formLayout->addRow(tr("Rank"), mRankContent);
        formLayout->addRow(tr("Club"), mClubContent);
        formLayout->addRow(tr("Country"), mCountryContent);
        group->setLayout(formLayout);
        mainLayout->addWidget(group);

    }

    {
        QGroupBox *group = new QGroupBox(tr("Preview"));
        QVBoxLayout *layout = new QVBoxLayout;

        mPreviewWidget = new QTableWidget(this);
        mPreviewWidget->setMinimumWidth(800);
        mPreviewWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mPreviewWidget->horizontalHeader()->setStretchLastSection(true);
        mPreviewWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        resetPreview();
        layout->addWidget(mPreviewWidget);

        group->setLayout(layout);
        mainLayout->addWidget(group);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    mainLayout->addWidget(buttonBox);

    // mainLayout->addLayout(formLayout);
    setLayout(mainLayout);
    setWindowTitle(tr("Import players from CSV file"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ImportPlayersCSVDialog::acceptClick);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ImportPlayersCSVDialog::cancelClick);
}

void ImportPlayersCSVDialog::acceptClick() {
    bool isValid = true;
    for (auto val : mIsColumnValid) {
        if (!val)
            isValid = false;
    }

    if (!isValid) {
        auto reply = QMessageBox::question(this, tr("Invalid fields"), tr("The file contains invalid fields. These will be discarded when importing. Would you like to import the players?"), QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
    }

    mImporter.import(mStoreManager);
    accept();
}

void ImportPlayersCSVDialog::cancelClick() {
    reject();
}

void ImportPlayersCSVDialog::setHasHeaderRow(int state) {
    bool checked = (state == Qt::Checked);
    mImporter.setHasHeaderRow(checked);

    if (checked) {
        mImporter.guessColumns();
        refillColumnBoxes();
    }
    resetPreview();
}

void ImportPlayersCSVDialog::setDelimiter(const QString &text) {
    char del = text.toStdString().front();
    mReader->setDelimiter(del);

    mImporter.guessColumns();
    refillColumnBoxes();
    resetPreview();
}

void ImportPlayersCSVDialog::setFirstNameColumn(int index) {
    auto oldValue = mImporter.getFirstNameColumn();
    auto newValue = (index == 0 ? std::nullopt : std::make_optional(static_cast<size_t>(index - 1)));

    mImporter.setFirstNameColumn(newValue);

    if (oldValue)
        updatePreviewColumn(*oldValue);
    if (newValue)
        updatePreviewColumn(*newValue);
}

void ImportPlayersCSVDialog::setLastNameColumn(int index) {
    auto oldValue = mImporter.getLastNameColumn();
    auto newValue = (index == 0 ? std::nullopt : std::make_optional(static_cast<size_t>(index - 1)));

    mImporter.setLastNameColumn(newValue);

    if (oldValue)
        updatePreviewColumn(*oldValue);
    if (newValue)
        updatePreviewColumn(*newValue);
}

void ImportPlayersCSVDialog::setAgeColumn(int index) {
    auto oldValue = mImporter.getAgeColumn();
    auto newValue = (index == 0 ? std::nullopt : std::make_optional(static_cast<size_t>(index - 1)));

    mImporter.setAgeColumn(newValue);

    if (oldValue)
        updatePreviewColumn(*oldValue);
    if (newValue)
        updatePreviewColumn(*newValue);
}

void ImportPlayersCSVDialog::setRankColumn(int index) {
    auto oldValue = mImporter.getRankColumn();
    auto newValue = (index == 0 ? std::nullopt : std::make_optional(static_cast<size_t>(index - 1)));

    mImporter.setRankColumn(newValue);

    if (oldValue)
        updatePreviewColumn(*oldValue);
    if (newValue)
        updatePreviewColumn(*newValue);
}

void ImportPlayersCSVDialog::setClubColumn(int index) {
    auto oldValue = mImporter.getClubColumn();
    auto newValue = (index == 0 ? std::nullopt : std::make_optional(static_cast<size_t>(index - 1)));

    mImporter.setClubColumn(newValue);

    if (oldValue)
        updatePreviewColumn(*oldValue);
    if (newValue)
        updatePreviewColumn(*newValue);
}

void ImportPlayersCSVDialog::setWeightColumn(int index) {
    auto oldValue = mImporter.getWeightColumn();
    auto newValue = (index == 0 ? std::nullopt : std::make_optional(static_cast<size_t>(index - 1)));

    mImporter.setWeightColumn(newValue);

    if (oldValue)
        updatePreviewColumn(*oldValue);
    if (newValue)
        updatePreviewColumn(*newValue);
}

void ImportPlayersCSVDialog::setCountryColumn(int index) {
    auto oldValue = mImporter.getSexColumn();
    auto newValue = (index == 0 ? std::nullopt : std::make_optional(static_cast<size_t>(index - 1)));

    mImporter.setSexColumn(newValue);

    if (oldValue)
        updatePreviewColumn(*oldValue);
    if (newValue)
        updatePreviewColumn(*newValue);
}

void ImportPlayersCSVDialog::setSexColumn(int index) {
    auto oldValue = mImporter.getSexColumn();
    auto newValue = (index == 0 ? std::nullopt : std::make_optional(static_cast<size_t>(index - 1)));

    mImporter.setSexColumn(newValue);

    if (oldValue)
        updatePreviewColumn(*oldValue);
    if (newValue)
        updatePreviewColumn(*newValue);
}

void ImportPlayersCSVDialog::refillColumnBox(QComboBox* box, std::optional<size_t> currentColumn) {
    bool state = box->blockSignals(true);
    box->clear();
    box->addItem(tr("None"));
    for (size_t i = 1; i <= mReader->columnCount(); ++i)
        box->addItem(tr("Column %1").arg(QString::number(i)));

    if (currentColumn == std::nullopt)
        box->setCurrentIndex(0);
    else
        box->setCurrentIndex(1 + *currentColumn);
    box->blockSignals(state);
}

void ImportPlayersCSVDialog::refillColumnBoxes() {
    refillColumnBox(mFirstNameContent, mImporter.getFirstNameColumn());
    refillColumnBox(mLastNameContent, mImporter.getLastNameColumn());
    refillColumnBox(mAgeContent, mImporter.getAgeColumn());
    refillColumnBox(mRankContent, mImporter.getRankColumn());
    refillColumnBox(mClubContent, mImporter.getClubColumn());
    refillColumnBox(mWeightContent, mImporter.getWeightColumn());
    refillColumnBox(mCountryContent, mImporter.getCountryColumn());
    refillColumnBox(mSexContent, mImporter.getSexColumn());
}

void ImportPlayersCSVDialog::resetPreview() {
    mPreviewWidget->clear();
    mIsColumnValid.clear();

    size_t offset = (mImporter.hasHeaderRow() ? 1 : 0);
    mPreviewWidget->setRowCount(mReader->rowCount() - offset);
    mPreviewWidget->setColumnCount(mReader->columnCount());

    for (int column = 0; column < static_cast<int>(mReader->columnCount()); ++column) {
        bool isValid = true;
        for (int row = 0; row < static_cast<int>(mReader->rowCount() - offset); ++row) {
            QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(mReader->get(row + offset, column)));
            mPreviewWidget->setItem(row, column, item);
            if (!mImporter.isValid(row + offset, column)) {
                isValid = false;
                item->setBackground(Qt::red);
            }
            else {
                item->setBackground(Qt::white);
            }
        }

        QTableWidgetItem *header = new QTableWidgetItem(QString::fromStdString(mImporter.getHeader(column)));
        if (!isValid)
            header->setBackground(Qt::red);
        else
            header->setBackground(Qt::white);

        mPreviewWidget->setHorizontalHeaderItem(column, header);
        mIsColumnValid.push_back(isValid);
    }
}

void ImportPlayersCSVDialog::updatePreviewColumn(size_t column) {
    size_t offset = (mImporter.hasHeaderRow() ? 1 : 0);
    bool isValid = true;

    for (int row = 0; row < static_cast<int>(mReader->rowCount() - offset); ++row) {
        QTableWidgetItem *item = mPreviewWidget->item(row, column);
        if (!mImporter.isValid(row+offset, column)) {
            isValid = false;
            item->setBackground(Qt::red);
        }
        else {
            item->setBackground(Qt::white);
        }
    }

    QTableWidgetItem *header = new QTableWidgetItem(QString::fromStdString(mImporter.getHeader(column)));
    if (!isValid)
        header->setBackground(Qt::red);
    else
        header->setBackground(Qt::white);
    mPreviewWidget->setHorizontalHeaderItem(column, header);
    mIsColumnValid[column] = isValid;
}

