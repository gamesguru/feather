// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SyncRangeDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDateEdit>
#include <QLabel>
#include <QDialogButtonBox>

#include "utils/Utils.h"
#include "utils/RestoreHeightLookup.h"

SyncRangeDialog::SyncRangeDialog(QWidget *parent, Wallet *wallet)
    : QDialog(parent)
    , m_wallet(wallet)
{
    setWindowTitle(tr("Rescan from Date"));
    setWindowIcon(QIcon(":/assets/images/appicons/64x64.png"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    auto *layout = new QVBoxLayout(this);    
    auto *formLayout = new QFormLayout();
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    // Default start date is 7 days ago
    QDate defaultDate = QDate::currentDate().addDays(-7);

    m_fromDateEdit = new QDateEdit(defaultDate);
    m_fromDateEdit->setCalendarPopup(true);
    m_fromDateEdit->setDisplayFormat("yyyy-MM-dd");

    formLayout->addRow(tr("Start date:"), m_fromDateEdit);

    layout->addLayout(formLayout);

    m_infoLabel = new QLabel;
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; }");

    layout->addWidget(m_infoLabel);

    connect(m_fromDateEdit, &QDateEdit::dateChanged, this, &SyncRangeDialog::updateInfo);

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

quint64 SyncRangeDialog::estimatedBlocks() const {
    return m_estimatedBlocks;
}

quint64 SyncRangeDialog::estimatedSize() const {
    return m_estimatedSize;
}

quint64 SyncRangeDialog::estimatedStartHeight() const {
    return m_estimatedStartHeight;
}

void SyncRangeDialog::updateInfo() {
    NetworkType::Type nettype = m_wallet->nettype();
    QString filename = Utils::getRestoreHeightFilename(nettype);
    std::unique_ptr<RestoreHeightLookup> lookup(RestoreHeightLookup::fromFile(filename, nettype));
    if (!lookup || lookup->data.isEmpty()) {
        m_infoLabel->setText(tr("Unable to estimate - restore height data unavailable"));
        m_estimatedBlocks = 0;
        m_estimatedSize = 0;
        m_estimatedStartHeight = 0;
        return;
    }

    QDate start = m_fromDateEdit->date();
    m_estimatedStartHeight = lookup->dateToHeight(start.startOfDay().toSecsSinceEpoch());

    uint64_t targetHeight = m_wallet->daemonBlockChainTargetHeight();
    if (targetHeight == 0) {
        targetHeight = m_wallet->blockChainHeight();
    }

    if (targetHeight < m_estimatedStartHeight) {
        targetHeight = m_estimatedStartHeight;
    }

    m_estimatedBlocks = targetHeight - m_estimatedStartHeight;
    m_estimatedSize = Utils::estimateSyncDataSize(m_estimatedBlocks);

    m_infoLabel->setText(tr("Rescanning ~%1 blocks (to tip)\nEst. download size: %2")
                           .arg(m_estimatedBlocks)
                           .arg(Utils::formatBytes(m_estimatedSize)));
}
