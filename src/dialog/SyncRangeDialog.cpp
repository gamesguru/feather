// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SyncRangeDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QLabel>
#include <QTabWidget>
#include <QDialogButtonBox>

#include "utils/Utils.h"
#include "utils/RestoreHeightLookup.h"

SyncRangeDialog::SyncRangeDialog(QWidget *parent, Wallet *wallet)
    : QDialog(parent)
    , m_wallet(wallet)
{
    setWindowTitle(tr("Sync Date Range"));
    setWindowIcon(QIcon(":/assets/images/appicons/64x64.png"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    auto *layout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);

    // --- Date Tab ---
    m_dateTab = new QWidget;
    auto *formLayout = new QFormLayout(m_dateTab);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    m_toDateEdit = new QDateEdit(QDate::currentDate());
    m_toDateEdit->setCalendarPopup(true);
    m_toDateEdit->setDisplayFormat("yyyy-MM-dd");

    int defaultDays = 7;

    // Preset durations dropdown
    m_presetCombo = new QComboBox;
    m_presetCombo->addItem(tr("1 day"), 1);
    m_presetCombo->addItem(tr("7 days"), 7);
    m_presetCombo->addItem(tr("30 days"), 30);
    m_presetCombo->addItem(tr("90 days"), 90);
    m_presetCombo->addItem(tr("1 year"), 365);
    m_presetCombo->addItem(tr("Custom..."), -1);
    m_presetCombo->setCurrentIndex(1); // Default to 7 days

    m_daysSpinBox = new QSpinBox;
    m_daysSpinBox->setRange(1, 3650); // 10 years
    m_daysSpinBox->setValue(defaultDays);
    m_daysSpinBox->setSuffix(tr(" days"));
    m_daysSpinBox->setVisible(false); // Hidden until "Custom..." is selected

    // Layout for preset + custom spinbox
    auto *daysLayout = new QHBoxLayout;
    daysLayout->setContentsMargins(0, 0, 0, 0);
    daysLayout->addWidget(m_presetCombo, 1);
    daysLayout->addWidget(m_daysSpinBox, 0);

    m_fromDateEdit = new QDateEdit(QDate::currentDate().addDays(-defaultDays));
    m_fromDateEdit->setCalendarPopup(true);
    m_fromDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_fromDateEdit->setToolTip(tr("Calculated from 'End date' and day span."));

    formLayout->addRow(tr("Day span:"), daysLayout);
    formLayout->addRow(tr("Start date:"), m_fromDateEdit);
    formLayout->addRow(tr("End date:"), m_toDateEdit);

    m_tabWidget->addTab(m_dateTab, tr("Date Range"));

    // --- Block Tab ---
    m_blockTab = new QWidget;
    auto *blockLayout = new QFormLayout(m_blockTab);

    quint64 currentHeight = m_wallet->blockChainHeight();
    quint64 daemonHeight = m_wallet->daemonBlockChainHeight();
    quint64 targetHeight = (daemonHeight > 0) ? daemonHeight : currentHeight;

    m_startHeightSpin = new QSpinBox;
    m_startHeightSpin->setRange(0, 9999999);
    m_startHeightSpin->setValue(std::max((quint64)0, targetHeight - 5040)); // Approx 7 days

    m_endHeightSpin = new QSpinBox;
    m_endHeightSpin->setRange(0, 9999999);
    m_endHeightSpin->setValue(targetHeight);

    blockLayout->addRow(tr("Start Height:"), m_startHeightSpin);
    blockLayout->addRow(tr("End Height:"), m_endHeightSpin);

    m_tabWidget->addTab(m_blockTab, tr("Block Height"));

    m_infoLabel = new QLabel;
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; }");

    layout->addWidget(m_tabWidget);
    layout->addWidget(m_infoLabel);

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &SyncRangeDialog::updateInfo);
    connect(m_startHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SyncRangeDialog::updateInfo);
    connect(m_endHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SyncRangeDialog::updateInfo);

    connect(m_fromDateEdit, &QDateEdit::dateChanged, this, &SyncRangeDialog::updateToDate);
    connect(m_toDateEdit, &QDateEdit::dateChanged, this, &SyncRangeDialog::updateFromDate);
    connect(m_daysSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SyncRangeDialog::updateFromDate);

    // Connect preset dropdown
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        int days = m_presetCombo->itemData(index).toInt();
        if (days == -1) {
            // Custom mode: show spinbox, keep current value
            m_daysSpinBox->setVisible(true);
        } else {
            // Preset mode: hide spinbox, set value
            m_daysSpinBox->setVisible(false);
            m_daysSpinBox->setValue(days);
        }
    });

    // Init info
    updateInfo();

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(btnBox);

    resize(320, height());
}

QDate SyncRangeDialog::fromDate() const {
    return m_fromDateEdit->date();
}

QDate SyncRangeDialog::toDate() const {
    return m_toDateEdit->date();
}

quint64 SyncRangeDialog::estimatedBlocks() const {
    return m_estimatedBlocks;
}

quint64 SyncRangeDialog::estimatedSize() const {
    return m_estimatedSize;
}

quint64 SyncRangeDialog::startHeight() const {
    return m_startHeightSpin->value();
}

quint64 SyncRangeDialog::endHeight() const {
    return m_endHeightSpin->value();
}

SyncRangeDialog::Mode SyncRangeDialog::mode() const {
    if (m_tabWidget->currentIndex() == 1) return Mode_Block;
    return Mode_Date;
}

void SyncRangeDialog::updateInfo() {
    if (this->mode() == Mode_Block) {
        quint64 start = m_startHeightSpin->value();
        quint64 end = m_endHeightSpin->value();

        m_estimatedBlocks = (end > start) ? (end - start) : 0;
        m_estimatedSize = Utils::estimateSyncDataSize(m_estimatedBlocks);
    } 
    else {
        NetworkType::Type nettype = m_wallet->nettype();
        QString filename = Utils::getRestoreHeightFilename(nettype);
        std::unique_ptr<RestoreHeightLookup> lookup(RestoreHeightLookup::fromFile(filename, nettype));
        if (!lookup || lookup->data.isEmpty()) {
            m_infoLabel->setText(tr("Unable to estimate - restore height data unavailable"));
            m_estimatedBlocks = 0;
            m_estimatedSize = 0;
            return;
        }

        QDate start = m_fromDateEdit->date();
        QDate end = m_toDateEdit->date();

        uint64_t startHeight = lookup->dateToHeight(start.startOfDay().toSecsSinceEpoch());
        uint64_t endHeight = lookup->dateToHeight(end.endOfDay().toSecsSinceEpoch());

        if (endHeight < startHeight) endHeight = startHeight;
        m_estimatedBlocks = endHeight - startHeight;
        m_estimatedSize = Utils::estimateSyncDataSize(m_estimatedBlocks);
    }

    m_infoLabel->setText(tr("Scanning ~%1 blocks\nEst. download size: %2")
                           .arg(m_estimatedBlocks)
                           .arg(Utils::formatBytes(m_estimatedSize)));
}

void SyncRangeDialog::updateFromDate() {
    m_fromDateEdit->setDate(m_toDateEdit->date().addDays(-m_daysSpinBox->value()));
    updateInfo();
}

void SyncRangeDialog::updateToDate() {
    m_toDateEdit->setDate(m_fromDateEdit->date().addDays(m_daysSpinBox->value()));
    updateInfo();
}
