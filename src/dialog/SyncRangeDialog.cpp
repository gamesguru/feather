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
    auto *formLayout = new QFormLayout;
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

    m_infoLabel = new QLabel;
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; }");

    formLayout->addRow(tr("Day span:"), daysLayout);
    formLayout->addRow(tr("Start date:"), m_fromDateEdit);
    formLayout->addRow(tr("End date:"), m_toDateEdit);

    layout->addLayout(formLayout);
    layout->addWidget(m_infoLabel);

    connect(m_fromDateEdit, &QDateEdit::dateChanged, this, &SyncRangeDialog::updateInfo);
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

void SyncRangeDialog::updateInfo() {
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    // Modern Qt (Ubuntu 22.04+)
    uint64_t startHeight = lookup->dateToHeight(start.startOfDay().toSecsSinceEpoch());
    uint64_t endHeight = lookup->dateToHeight(end.endOfDay().toSecsSinceEpoch());
#else
    // Legacy Qt 5.12 (Ubuntu 20.04)
    // Manually construct the DateTime for 00:00:00 and 23:59:59
    uint64_t startHeight = lookup->dateToHeight(QDateTime(start, QTime(0, 0, 0)).toSecsSinceEpoch());
    uint64_t endHeight = lookup->dateToHeight(QDateTime(end, QTime(23, 59, 59)).toSecsSinceEpoch());
#endif

    if (endHeight < startHeight) endHeight = startHeight;
    m_estimatedBlocks = endHeight - startHeight;
    m_estimatedSize = Utils::estimateSyncDataSize(m_estimatedBlocks);

    m_infoLabel->setText(tr("Scanning ~%1 blocks\nEst. download size: %2")
                           .arg(m_estimatedBlocks)
                           .arg(Utils::formatBytes(m_estimatedSize)));
}

void SyncRangeDialog::updateFromDate() {
    m_fromDateEdit->setDate(m_toDateEdit->date().addDays(-m_daysSpinBox->value()));
    updateInfo();
}
