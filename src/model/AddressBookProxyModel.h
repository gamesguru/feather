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
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        beginFilterChange();
        m_searchRegExp.setPattern(searchString);
        endFilterChange();
#else
        m_searchRegExp.setPattern(searchString);
        invalidateFilter();
#endif
    }

private:
    QRegularExpression m_searchRegExp;
};

#endif //FEATHER_ADDRESSBOOKPROXYMODEL_H
