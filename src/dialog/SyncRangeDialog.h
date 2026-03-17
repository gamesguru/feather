// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SYNCRANGEDIALOG_H
#define FEATHER_SYNCRANGEDIALOG_H

#include <QDialog>
#include <QDate>

#include "libwalletqt/Wallet.h"

class QComboBox;
class QSpinBox;
class QDateEdit;
class QLabel;

class SyncRangeDialog : public QDialog
{
Q_OBJECT

public:
    explicit SyncRangeDialog(QWidget *parent, Wallet *wallet);
    ~SyncRangeDialog() override = default;

    QDate fromDate() const;
    QDate toDate() const;
    quint64 estimatedBlocks() const;
    quint64 estimatedSize() const;

private:
    void updateInfo();
    void updateFromDate();

    Wallet *m_wallet;
    QComboBox *m_presetCombo;
    QSpinBox *m_daysSpinBox;
    QDateEdit *m_fromDateEdit;
    QDateEdit *m_toDateEdit;
    QLabel *m_infoLabel;

    quint64 m_estimatedBlocks = 0;
    quint64 m_estimatedSize = 0;
};

#endif //FEATHER_SYNCRANGEDIALOG_H
