/***************************************************************************
                          kcategoriesview.cpp  -  description
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcategoriesview.h"
#include "kmymoneyaccountsviewbase_p.h"

#include "accountsviewproxymodel.h"
#include "kmymoneyglobalsettings.h"
#include "icons.h"

using namespace Icons;

namespace Ui {
  class KCategoriesView;
}

class KCategoriesViewPrivate : public KMyMoneyAccountsViewBasePrivate
{
    Q_DECLARE_PUBLIC(KCategoriesView)

public:

    KCategoriesViewPrivate(KCategoriesView *qq) :
        q_ptr(qq),
        ui(new Ui::KCategoriesView),
        m_haveUnusedCategories(false)
    {
    }

    ~KCategoriesViewPrivate()
    {
    }

    void init()
    {
        Q_Q(KCategoriesView);
      ui->setupUi(q);
      m_accountTree = &ui->m_accountTree;

      // setup icons for collapse and expand button
      ui->m_collapseButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListCollapse]));
      ui->m_expandButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListExpand]));

      m_proxyModel = ui->m_accountTree->init(View::Categories);

      q->connect(m_proxyModel, &AccountsProxyModel::unusedIncomeExpenseAccountHidden, q, &KCategoriesView::slotUnusedIncomeExpenseAccountHidden);
      q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterFixedString);

    }

    KCategoriesView       *q_ptr;
    Ui::KCategoriesView   *ui;
    bool                  m_haveUnusedCategories;
};

