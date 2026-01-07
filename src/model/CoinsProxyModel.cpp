// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "CoinsProxyModel.h"
#include "CoinsModel.h"
#include "libwalletqt/rows/CoinsInfo.h"
#include <QtGlobal>

CoinsProxyModel::CoinsProxyModel(QObject *parent, Coins *coins)
        : QSortFilterProxyModel(parent)
        , m_coins(coins)
        , m_searchRegExp("")
{
    m_searchRegExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    setSortRole(Qt::UserRole);
}

void CoinsProxyModel::setShowSpent(const bool showSpent) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    m_showSpent = showSpent;
    endFilterChange();
#else
    m_showSpent = showSpent;
    invalidateFilter();
#endif
}

void CoinsProxyModel::setSearchFilter(const QString &searchString) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    m_searchRegExp.setPattern(searchString);
    endFilterChange();
#else
    m_searchRegExp.setPattern(searchString);
    invalidateFilter();
#endif
}

bool CoinsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const CoinsInfo& coin = m_coins->getRow(sourceRow);

    if (!m_showSpent && coin.spent) {
        return false;
    }

    if (!m_searchRegExp.pattern().isEmpty()) {
        return coin.pubKey.contains(m_searchRegExp) || coin.address.contains(m_searchRegExp)
                || coin.hash.contains(m_searchRegExp) || coin.addressLabel.contains(m_searchRegExp)
                || coin.description.contains(m_searchRegExp);
    }

    return true;
}