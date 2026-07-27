// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TxImportDialog.h"
#include "ui_TxImportDialog.h"

#include <QMessageBox>
#include <QApplication>
#include <QRegularExpression>

#include "utils/nodes.h"
#include "utils/Utils.h"

TxImportDialog::TxImportDialog(QWidget *parent, Wallet *wallet, Nodes *nodes)
        : WindowModalDialog(parent)
        , ui(new Ui::TxImportDialog)
        , m_wallet(wallet)
        , m_nodes(nodes)
{
    ui->setupUi(this);

    connect(ui->btn_import, &QPushButton::clicked, this, &TxImportDialog::onImport);
    connect(m_wallet, &Wallet::connectionStatusChanged, this, &TxImportDialog::updateStatus);

    ui->line_txid->setMinimumWidth(600);
    this->adjustSize();

    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    this->updateStatus(m_wallet->connectionStatus());
}

void TxImportDialog::onImport() {
    if (m_wallet->connectionStatus() == Wallet::ConnectionStatus_Disconnected) {
        m_nodes->connectToNode();
        m_wallet->setScanMempoolWhenPaused(true);
        this->updateStatus(Wallet::ConnectionStatus_Connecting);
        return;
    }

    QString txid = ui->line_txid->text().trimmed();
    if (txid.isEmpty()) return;

    static const QRegularExpression hexMatcher("^[0-9a-fA-F]{64}$");
    if (!hexMatcher.match(txid).hasMatch()) {
        Utils::showError(this, "Invalid TXID", "Transaction ID must be a 64-character hexadecimal string.");
        return;
    }

    if (m_wallet->haveTransaction(txid)) {
        Utils::showWarning(this, "Transaction already exists in wallet", "If you can't find it in your history, "
                                                                       "check if it belongs to a different account (Wallet -> Account)");
        return;
    }

    ui->btn_import->setEnabled(false);
    ui->btn_import->setText("Importing...");
    QApplication::processEvents();

    bool success = m_wallet->importTransaction(txid);

    ui->btn_import->setEnabled(true);
    ui->btn_import->setText("Import");

    if (success) {
        Utils::showInfo(this, "Success", "Transaction has been successfully imported into the wallet history.");
        this->accept();
    } else {
        Utils::showError(this, "Import Failed", "Failed to import transaction. The node might not have this transaction, or there was a network error.");
    }
}

void TxImportDialog::updateStatus(int status) {
    if (status == Wallet::ConnectionStatus_Disconnected) {
        ui->btn_import->setText("Connect");
        ui->btn_import->setEnabled(true);
    } else if (status == Wallet::ConnectionStatus_Connecting || status == Wallet::ConnectionStatus_WrongVersion) {
        ui->btn_import->setText("Connecting...");
        ui->btn_import->setEnabled(false);
    } else {
        ui->btn_import->setText("Import");
        ui->btn_import->setEnabled(true);
    }
}

TxImportDialog::~TxImportDialog() = default;
