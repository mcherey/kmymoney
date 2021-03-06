/***************************************************************************
                          kconfirmmanualenterdlg.cpp
                             -------------------
    begin                : Mon Apr  9 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kconfirmmanualenterdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QRadioButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTextEdit>
#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoneyutils.h"
#include "mymoneytransaction.h"
#include "ui_kconfirmmanualenterdlgdecl.h"

struct KConfirmManualEnterDlg::Private {
  Ui::KConfirmManualEnterDlgDecl ui;
};

KConfirmManualEnterDlg::KConfirmManualEnterDlg(const MyMoneySchedule& schedule, QWidget* parent) :
    QDialog(parent), d(new Private)
{
  d->ui.setupUi(this);
  d->ui.buttonGroup1->setId(d->ui.m_discardRadio, 0);
  d->ui.buttonGroup1->setId(d->ui.m_onceRadio, 1);
  d->ui.buttonGroup1->setId(d->ui.m_setRadio, 2);

  d->ui.m_onceRadio->setChecked(true);

  if (schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
    d->ui.m_setRadio->setEnabled(false);
    d->ui.m_discardRadio->setEnabled(false);
  }
}

KConfirmManualEnterDlg::~KConfirmManualEnterDlg()
{
  delete d;
}

void KConfirmManualEnterDlg::loadTransactions(const MyMoneyTransaction& to, const MyMoneyTransaction& tn)
{
  QString messageDetail("<qt>");
  MyMoneyFile* file = MyMoneyFile::instance();
  int noItemsChanged = 0;

  try {
    if (to.splits().isEmpty())
      throw MYMONEYEXCEPTION(i18n("Transaction %1 has no splits", to.id()));
    if (tn.splits().isEmpty())
      throw MYMONEYEXCEPTION(i18n("Transaction %1 has no splits", tn.id()));

    QString po, pn;
    if (!to.splits().front().payeeId().isEmpty())
      po = file->payee(to.splits().front().payeeId()).name();
    if (!tn.splits().front().payeeId().isEmpty())
      pn = file->payee(tn.splits().front().payeeId()).name();

    if (po != pn) {
      noItemsChanged++;
      messageDetail += i18n("<p>Payee changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", po, pn);
    }

    if (to.splits().front().accountId() != tn.splits().front().accountId()) {
      noItemsChanged++;
      messageDetail += i18n("<p>Account changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>"
                            , file->account(to.splits().front().accountId()).name()
                            , file->account(tn.splits().front().accountId()).name());
    }

    if (file->isTransfer(to) && file->isTransfer(tn)) {
      if (to.splits()[1].accountId() != tn.splits()[1].accountId()) {
        noItemsChanged++;
        messageDetail += i18n("<p>Transfer account changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>"
                              , file->account(to.splits()[1].accountId()).name()
                              , file->account(tn.splits()[1].accountId()).name());
      }
    } else {
      QString co, cn;
      switch (to.splitCount()) {
        default:
          co = i18nc("Split transaction (category replacement)", "Split transaction");
          break;
        case 2:
          co = file->accountToCategory(to.splits()[1].accountId());
        case 1:
          break;
      }

      switch (tn.splitCount()) {
        default:
          cn = i18nc("Split transaction (category replacement)", "Split transaction");
          break;
        case 2:
          cn = file->accountToCategory(tn.splits()[1].accountId());
        case 1:
          break;
      }
      if (co != cn) {
        noItemsChanged++;
        messageDetail += i18n("<p>Category changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", co, cn);
      }
    }

    QString mo, mn;
    mo = to.splits().front().memo();
    mn = tn.splits().front().memo();
    if (mo.isEmpty())
      mo = QString("<i>") + i18nc("Empty memo", "empty") + QString("</i>");
    if (mn.isEmpty())
      mn = QString("<i>") + i18nc("Empty memo", "empty") + QString("</i>");
    if (mo != mn) {
      noItemsChanged++;
      messageDetail += i18n("<p>Memo changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", mo, mn);
    }

    QString no, nn;
    no = to.splits().front().number();
    nn = tn.splits().front().number();
    if (no.isEmpty())
      no = QString("<i>") + i18nc("No number", "empty") + QString("</i>");
    if (nn.isEmpty())
      nn = QString("<i>") + i18nc("No number", "empty") + QString("</i>");
    if (no != nn) {
      noItemsChanged++;
      messageDetail += i18n("<p>Number changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", no, nn);
    }

    const MyMoneySecurity& sec = MyMoneyFile::instance()->security(to.commodity());
    MyMoneyMoney ao, an;
    ao = to.splits().front().value();
    an = tn.splits().front().value();
    if (ao != an) {
      noItemsChanged++;
      messageDetail += i18n("<p>Amount changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", ao.formatMoney(sec.smallestAccountFraction()), an.formatMoney(sec.smallestAccountFraction()));
    }

    MyMoneySplit::reconcileFlagE fo, fn;
    fo = to.splits().front().reconcileFlag();
    fn = tn.splits().front().reconcileFlag();
    if (fo != fn) {
      noItemsChanged++;
      messageDetail += i18n("<p>Reconciliation flag changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>",    KMyMoneyUtils::reconcileStateToString(fo, true), KMyMoneyUtils::reconcileStateToString(fn, true));
    }
  } catch (const MyMoneyException &e) {
    KMessageBox::error(this, i18n("Fatal error in determining data: %1", e.what()));
  }

  messageDetail += "</qt>";
  d->ui.m_details->setText(messageDetail);
  return;
}

KConfirmManualEnterDlg::Action KConfirmManualEnterDlg::action() const
{
  if (d->ui.m_discardRadio->isChecked())
    return UseOriginal;
  if (d->ui.m_setRadio->isChecked())
    return ModifyAlways;
  return ModifyOnce;
}
