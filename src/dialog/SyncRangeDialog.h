// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SYNCRANGEDIALOG_H
#define FEATHER_SYNCRANGEDIALOG_H

#include <QDialog>
#include <QDate>

#include "libwalletqt/Wallet.h"

class QDateEdit;
class QLabel;

class SyncRangeDialog : public QDialog
{
Q_OBJECT

public:
    explicit SyncRangeDialog(QWidget *parent, Wallet *wallet);
    ~SyncRangeDialog() override = default;

    QDate fromDate() const;
    quint64 estimatedBlocks() const;
    quint64 estimatedSize() const;
    quint64 estimatedStartHeight() const;

private:
    void updateInfo();

    Wallet *m_wallet;

    QDateEdit *m_fromDateEdit;
    QLabel *m_infoLabel;

    quint64 m_estimatedBlocks = 0;
    quint64 m_estimatedSize = 0;
    quint64 m_estimatedStartHeight = 0;
};

#endif //FEATHER_SYNCRANGEDIALOG_H
