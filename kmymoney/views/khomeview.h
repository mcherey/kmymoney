/***************************************************************************
                          khomeview.h  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KHOMEVIEW_H
#define KHOMEVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

#include "mymoneyaccount.h"

#ifdef ENABLE_WEBENGINE
class QWebEngineView;
#else
class KWebView;
#endif

/**
  * Displays a 'home page' for the user.  Similar to concepts used in
  * quicken and m$-money.
  *
  * @author Michael Edwardes
  *
  * @short A view containing the home page for kmymoney.
**/
class QPrinter;
class MyMoneySchedule;
class KHomeView : public KMyMoneyViewBase
{
  Q_OBJECT
public:
  /**
    * Definition of bitmap used as argument for showAccounts().
    */
  enum paymentTypeE {
    Preferred = 1,          ///< show preferred accounts
    Payment = 2             ///< show payment accounts
  };

  explicit KHomeView(QWidget *parent = nullptr);
  ~KHomeView();

protected:
  virtual void wheelEvent(QWheelEvent *event);
  void showPayments();
  void showPaymentEntry(const MyMoneySchedule&, int cnt = 1);
  void showAccounts(paymentTypeE type, const QString& hdr);
  void showAccountEntry(const MyMoneyAccount&);
  void showFavoriteReports();
  void showForecast();
  void showNetWorthGraph();
  void showSummary();
  void showAssetsLiabilities();
  void showIncomeExpenseSummary();
  void showSchedulesSummary();
  void showBudget();
  void showCashFlowSummary();

  QString link(const QString& view, const QString& query, const QString& title = QString()) const;
  QString linkend() const;
  void loadView();

  /**
    * Overridden so we can emit the activated signal.
    *
    * @return Nothing.
    */
  void showEvent(QShowEvent* event);

public slots:

  void slotOpenUrl(const QUrl &url);
  void slotLoadView();

  /**
    * Print the current view
    */
  void slotPrintView();

signals:
  void ledgerSelected(const QString& id, const QString& transaction);
  void scheduleSelected(const QString& id);
  void reportSelected(const QString& id);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;

  /** Initializes page and sets its load status to initialized
   */
  void init();

  /**
    * Print an account and its balance and limit
    */
  void showAccountEntry(const MyMoneyAccount& acc, const MyMoneyMoney& value, const MyMoneyMoney& valueToMinBal, const bool showMinBal);

  /**
    * @param acc the investment account
    * @return the balance in the currency of the investment account
    */
  MyMoneyMoney investmentBalance(const MyMoneyAccount& acc);

  /**
   * Print text in the color set for negative numbers, if @p amount is negative
   * abd @p isNegative is true
   */
  QString showColoredAmount(const QString& amount, bool isNegative);

  /**
   * Run the forecast
   */
  void doForecast();

  /**
   * Calculate the forecast balance after a payment has been made
   */
  MyMoneyMoney forecastPaymentBalance(const MyMoneyAccount& acc, const MyMoneyMoney& payment, QDate& paymentDate);

  QPrinter *m_currentPrinter;
};

#endif
