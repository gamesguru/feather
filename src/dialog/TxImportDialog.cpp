// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TxImportDialog.h"
#include "ui_TxImportDialog.h"

#include <QMessageBox>

#include "utils/NetworkManager.h"
#include "utils/nodes.h"

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
        this->updateStatus(Wallet::ConnectionStatus_Connecting);
        return;
    }

    QString txid = ui->line_txid->text();

    if (m_wallet->haveTransaction(txid)) {
        Utils::showWarning(this, "Transaction already exists in wallet", "If you can't find it in your history, "
                                                                       "check if it belongs to a different account (Wallet -> Account)");
        return;
    }

    if (m_wallet->importTransaction(txid)) {
        if (!m_wallet->haveTransaction(txid)) {
            Utils::showError(this, "Unable to import transaction", "This transaction does not belong to the wallet");
            return;
        }
        Utils::showInfo(this, "Transaction imported successfully", "");
    } else {
        Utils::showError(this, "Failed to import transaction", "");
    }
    m_wallet->refreshModels();
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
