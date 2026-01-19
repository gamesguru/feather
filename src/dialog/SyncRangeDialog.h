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
class QTabWidget;

class SyncRangeDialog : public QDialog
{
Q_OBJECT

public:
    explicit SyncRangeDialog(QWidget *parent, Wallet *wallet);
    ~SyncRangeDialog() override = default;

    enum Mode {
        Mode_Date,
        Mode_Block
    };

    QDate fromDate() const;
    QDate toDate() const;
    quint64 startHeight() const;
    quint64 endHeight() const;
    quint64 estimatedBlocks() const;
    quint64 estimatedSize() const;
    Mode mode() const;

private:
    void updateInfo();
    void updateFromDate();
    void updateToDate();

    Wallet *m_wallet;
    QTabWidget *m_tabWidget;
    QWidget *m_dateTab;
    QWidget *m_blockTab;

    // Date/Time
    QComboBox *m_presetCombo;
    QSpinBox *m_daysSpinBox;
    QDateEdit *m_fromDateEdit;
    QDateEdit *m_toDateEdit;

    // Blocks
    QSpinBox *m_startHeightSpin;
    QSpinBox *m_endHeightSpin;

    QLabel *m_infoLabel;

    quint64 m_estimatedBlocks = 0;
    quint64 m_estimatedSize = 0;
};

#endif //FEATHER_SYNCRANGEDIALOG_H
