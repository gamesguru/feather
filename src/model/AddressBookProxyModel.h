// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_ADDRESSBOOKPROXYMODEL_H
#define FEATHER_ADDRESSBOOKPROXYMODEL_H

#include <QSortFilterProxyModel>

class AddressBookProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit AddressBookProxyModel(QObject* parent = nullptr);
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

public slots:
    void setSearchFilter(const QString& searchString){
        beginFilterChange();
        m_searchRegExp.setPattern(searchString);
        endFilterChange();
    }

private:
    QRegularExpression m_searchRegExp;
};

#endif //FEATHER_ADDRESSBOOKPROXYMODEL_H
