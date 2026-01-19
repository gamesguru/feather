// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TxImportDialog.h"
#include "ui_TxImportDialog.h"

#include <QMessageBox>

#include "utils/NetworkManager.h"
#include "utils/nodes.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>



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

    if (m_wallet->haveTransaction(txid)) {
        Utils::showWarning(this, "Transaction already exists in wallet", "If you can't find it in your history, "
                                                                       "check if it belongs to a different account (Wallet -> Account)");
        return;
    }

    // Async Import: Fetch height from daemon, then Smart Sync to it.
    ui->btn_import->setEnabled(false);
    ui->btn_import->setText("Checking...");

    QNetworkAccessManager* nam = getNetwork(); // Use global network manager
    QString url = m_nodes->connection().toURL() + "/get_transactions";
    
    QJsonObject req;
    req["txs_hashes"] = QJsonArray({txid});
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = nam->post(request, QJsonDocument(req).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, txid]() {
        reply->deleteLater();
        ui->btn_import->setEnabled(true);
        ui->btn_import->setText("Import");
        
        if (reply->error() != QNetworkReply::NoError) {
             Utils::showError(this, "Connection error", reply->errorString());
             return;
        }

        QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        QJsonObject error = json.value("error").toObject();
        if (!error.isEmpty()) {
             Utils::showError(this, "Node error", error.value("message").toString());
             return;
        }

        QJsonArray txs = json.value("txs").toArray();
        bool found = false;
        
        for (const auto &val : txs) {
            QJsonObject tx = val.toObject();
            if (tx.value("tx_hash").toString() == txid) {
                found = true;
                if (tx.value("in_pool").toBool()) {
                     Utils::showInfo(this, "Transaction is in mempool", "Feather will detect it automatically in a moment.");
                     this->accept();
                     return;
                }
                
                quint64 height = tx.value("block_height").toVariant().toULongLong();
                if (height > 0) {
                     // Check if wallet is far behind (fresh restore?)
                     quint64 currentHeight = m_wallet->blockChainHeight();

                     if (height > currentHeight + 100000) {
                          // Jump ahead to avoid full scan
                          quint64 restoreHeight = (height > 20000) ? height - 20000 : 0;
                          m_wallet->setWalletCreationHeight(restoreHeight);
                          m_wallet->rescanBlockchainAsync();
                          Utils::showInfo(this, "Optimizing Sync", "Jumped to block " + QString::number(restoreHeight) + " to find transaction.");
                     }

                     m_wallet->startSmartSync(height + 10);
                     Utils::showInfo(this, "Import started", "Scanning block " + QString::number(height) + " for transaction...");
                     this->accept();
                     return;
                }
            }
        }
        
        if (!found) {
            Utils::showError(this, "Transaction not found on node", "The connected node does not know this transaction.");
        } else {
             // Found but failed to get height? Fallback.
             Utils::showError(this, "Failed to determine block height", "Could not read block height from node response.");
        }
    });
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
