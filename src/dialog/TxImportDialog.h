// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_TXIMPORTDIALOG_H
#define FEATHER_TXIMPORTDIALOG_H

#include <QDialog>

#include "components.h"
#include "utils/daemonrpc.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class TxImportDialog;
}

class Nodes;

class TxImportDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit TxImportDialog(QWidget *parent, Wallet *wallet, Nodes *nodes);
    ~TxImportDialog() override;

private slots:
    void onImport();

private:
    void updateStatus(int status);

    QScopedPointer<Ui::TxImportDialog> ui;
    Wallet *m_wallet;
    Nodes *m_nodes;
};


#endif //FEATHER_TXIMPORTDIALOG_H
