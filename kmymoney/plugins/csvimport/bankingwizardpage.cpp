/*******************************************************************************
*                                 bankingwizardpage.cpp
*                              --------------------
* begin                       : Thur Jan 01 2015
* copyright                   : (C) 2015 by Allan Anderson
* email                       : agander93@gmail.com
* copyright                   : (C) 2016 by Łukasz Wojniłowicz
* email                       : lukasz.wojnilowicz@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#include "bankingwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QStandardItemModel>
#include <QTextStream>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "csvwizard.h"
#include "csvimporter.h"

#include "ui_bankingwizardpage.h"

// ----------------------------------------------------------------------------

BankingPage::BankingPage(CSVWizard *dlg, CSVImporter *imp) :
  CSVWizardPage(dlg, imp),
  ui(new Ui::BankingPage)
{
  ui->setupUi(this);

  connect(ui->m_clear, &QAbstractButton::clicked, this, &BankingPage::clearColumns);
  connect(ui->m_radioAmount, &QAbstractButton::toggled, this, &BankingPage::amountToggled);
  connect(ui->m_radioDebitCredit, &QAbstractButton::toggled, this, &BankingPage::debitCreditToggled);
  connect(ui->m_oppositeSigns, &QAbstractButton::clicked, this, &BankingPage::oppositeSignsClicked);

  // initialize column names
  m_dlg->m_colTypeName.insert(Column::Payee,i18n("Payee"));
  m_dlg->m_colTypeName.insert(Column::Number,i18n("Number"));
  m_dlg->m_colTypeName.insert(Column::Debit,i18n("Debit"));
  m_dlg->m_colTypeName.insert(Column::Credit,i18n("Credit"));
  m_dlg->m_colTypeName.insert(Column::Date,i18n("Date"));
  m_dlg->m_colTypeName.insert(Column::Amount,i18n("Amount"));
  m_dlg->m_colTypeName.insert(Column::Category,i18n("Category"));
  m_dlg->m_colTypeName.insert(Column::Memo,i18n("Memo"));

  m_profile = dynamic_cast<BankingProfile *>(m_imp->m_profile);

  connect(ui->m_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColSelected(int)));
  connect(ui->m_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(debitColSelected(int)));
  connect(ui->m_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(creditColSelected(int)));
  connect(ui->m_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColSelected(int)));
  connect(ui->m_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(numberColSelected(int)));
  connect(ui->m_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColSelected(int)));
  connect(ui->m_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(payeeColSelected(int)));
  connect(ui->m_categoryCol, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryColSelected(int)));
}

BankingPage::~BankingPage()
{
  delete ui;
}

void BankingPage::initializePage()
{
  QHash<Column, QComboBox *> columns {{Column::Amount, ui->m_amountCol}, {Column::Debit, ui->m_debitCol},
                                           {Column::Credit, ui->m_creditCol}, {Column::Memo, ui->m_memoCol},
                                           {Column::Number, ui->m_numberCol}, {Column::Date, ui->m_dateCol},
                                           {Column::Payee,  ui->m_payeeCol},  {Column::Category, ui->m_categoryCol}};

  if (ui->m_dateCol->count() != m_imp->m_file->m_columnCount)
    m_dlg->initializeComboBoxes(columns);

  columns.remove(Column::Memo);
  for (auto it = columns.cbegin(); it != columns.cend(); ++it)
    it.value()->setCurrentIndex(m_profile->m_colTypeNum.value(it.key()));

  ui->m_oppositeSigns->setChecked(m_profile->m_oppositeSigns);

  if (m_profile->m_memoColList.count() > 0)
  {
    for (int i = 0; i < m_profile->m_memoColList.count(); ++i)
      ui->m_memoCol->setCurrentIndex(m_profile->m_memoColList.value(i));
  } else
    ui->m_memoCol->setCurrentIndex(-1);

  if (this->m_profile->m_colTypeNum.value(Column::Debit) == -1)     // If amount previously selected, set check radio_amount
    ui->m_radioAmount->setChecked(true);
  else                                     // ...else set check radio_debCred to clear amount col
    ui->m_radioDebitCredit->setChecked(true);
}

int BankingPage::nextId() const
{
  return CSVWizard::PageFormats;
}

bool BankingPage::isComplete() const
{
  return ui->m_dateCol->currentIndex() > -1 &&
         ui->m_payeeCol->currentIndex() > -1 &&
        (ui->m_amountCol->currentIndex() > -1 ||
        (ui->m_debitCol->currentIndex() > -1 &&
         ui->m_creditCol->currentIndex() > -1));
}

bool BankingPage::validateMemoComboBox()
{
  if (m_profile->m_memoColList.count() == 0)
    return true;
  for (int i = 0; i < ui->m_memoCol->count(); ++i)
  {
    QString txt = ui->m_memoCol->itemText(i);
    if (txt.contains(QLatin1Char('*')))  // check if text containing '*' belongs to valid column types
      if (m_profile->m_colNumType.value(i) != Column::Payee) {
        ui->m_memoCol->setItemText(i, QString::number(i + 1));
        m_profile->m_memoColList.removeOne(i);
        return false;
      }
  }
  return true;
}

void BankingPage::memoColSelected(int col)
{
  if (m_profile->m_colNumType.value(col) == Column::Payee ) {
    int rc = KMessageBox::Yes;
    if (isVisible())
      rc = KMessageBox::questionYesNo(m_dlg, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy the Payee data to the memo field, click 'Yes'.</center>",
                                              m_dlg->m_colTypeName.value(m_profile->m_colNumType.value(col))));
    if (rc == KMessageBox::Yes) {
      ui->m_memoCol->setItemText(col, QString::number(col + 1) + QLatin1Char('*'));
      if (!m_profile->m_memoColList.contains(col))
        m_profile->m_memoColList.append(col);
    } else {
      ui->m_memoCol->setItemText(col, QString::number(col + 1));
      m_profile->m_memoColList.removeOne(col);
    }
    //allow only separate memo field occupy combobox
    ui->m_memoCol->blockSignals(true);
    if (m_profile->m_colTypeNum.value(Column::Memo) != -1)
      ui->m_memoCol->setCurrentIndex(m_profile->m_colTypeNum.value(Column::Memo));
    else
      ui->m_memoCol->setCurrentIndex(-1);
    ui->m_memoCol->blockSignals(false);
    return;
  }

  if (m_profile->m_colTypeNum.value(Column::Memo) != -1)        // check if this memo has any column 'number' assigned...
    m_profile->m_memoColList.removeOne(col);           // ...if true remove it from memo list

  if(validateSelectedColumn(col, Column::Memo))
    if (col != - 1 && !m_profile->m_memoColList.contains(col))
      m_profile->m_memoColList.append(col);
}

void BankingPage::categoryColSelected(int col)
{
  validateSelectedColumn(col, Column::Category);
}

void BankingPage::numberColSelected(int col)
{
  validateSelectedColumn(col, Column::Number);
}

void BankingPage::payeeColSelected(int col)
{
  if (validateSelectedColumn(col, Column::Payee))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      memoColSelected(col);    // ...if true set memo field again
}

void BankingPage::dateColSelected(int col)
{
  validateSelectedColumn(col, Column::Date);
}

void BankingPage::debitColSelected(int col)
{
  validateSelectedColumn(col, Column::Debit);
}

void BankingPage::creditColSelected(int col)
{
  validateSelectedColumn(col, Column::Credit);
}

void BankingPage::amountColSelected(int col)
{
  validateSelectedColumn(col, Column::Amount);
}

void BankingPage::amountToggled(bool checked)
{
  if (checked) {
    ui->m_amountCol->setEnabled(true);  //  disable credit & debit ui choices
    ui->labelBnk_amount->setEnabled(true);
    ui->labelBnk_credits->setEnabled(false);
    ui->labelBnk_debits->setEnabled(false);

    ui->m_debitCol->setEnabled(false);
    ui->m_debitCol->setCurrentIndex(-1);
    ui->m_creditCol->setEnabled(false);
    ui->m_creditCol->setCurrentIndex(-1);
  }
}

void BankingPage::debitCreditToggled(bool checked)
{
  if (checked) {
    ui->m_debitCol->setEnabled(true);  //         if 'debit/credit' selected
    ui->labelBnk_debits->setEnabled(true);
    ui->m_creditCol->setEnabled(true);
    ui->labelBnk_credits->setEnabled(true);

    ui->m_amountCol->setEnabled(false);  //       disable 'amount' ui choices
    ui->m_amountCol->setCurrentIndex(-1);  //     as credit/debit chosen
    ui->labelBnk_amount->setEnabled(false);
  }
}

void BankingPage::oppositeSignsClicked(bool checked)
{
  m_profile->m_oppositeSigns = checked;
}

void BankingPage::clearColumns()
{
  ui->m_dateCol->setCurrentIndex(-1);
  ui->m_payeeCol->setCurrentIndex(-1);
  ui->m_memoCol->setCurrentIndex(-1);
  ui->m_numberCol->setCurrentIndex(-1);
  ui->m_amountCol->setCurrentIndex(-1);
  ui->m_debitCol->setCurrentIndex(-1);
  ui->m_creditCol->setCurrentIndex(-1);
  ui->m_categoryCol->setCurrentIndex(-1);
}

void BankingPage::resetComboBox(const Column comboBox)
{
  switch (comboBox) {
    case Column::Amount:
      ui->m_amountCol->setCurrentIndex(-1);
      break;
    case Column::Credit:
      ui->m_creditCol->setCurrentIndex(-1);
      break;
    case Column::Date:
      ui->m_dateCol->setCurrentIndex(-1);
      break;
    case Column::Debit:
      ui->m_debitCol->setCurrentIndex(-1);
      break;
    case Column::Memo:
      ui->m_memoCol->setCurrentIndex(-1);
      break;
    case Column::Number:
      ui->m_numberCol->setCurrentIndex(-1);
      break;
    case Column::Payee:
      ui->m_payeeCol->setCurrentIndex(-1);
      break;
    case Column::Category:
      ui->m_categoryCol->setCurrentIndex(-1);
      break;
    default:
      KMessageBox::sorry(m_dlg, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                    , (int)comboBox), i18n("CSV import"));
  }
}

bool BankingPage::validateSelectedColumn(const int col, const Column type)
{
  if (m_profile->m_colTypeNum.value(type) != -1)        // check if this 'type' has any column 'number' assigned...
    m_profile->m_colNumType.remove(m_profile->m_colTypeNum[type]); // ...if true remove 'type' assigned to this column 'number'

  bool ret = true;
  if (col == -1) { // user only wanted to reset his column so allow him
    m_profile->m_colTypeNum[type] = col;  // assign new column 'number' to this 'type'
  } else if (m_profile->m_colNumType.contains(col)) { // if this column 'number' has already 'type' assigned
    KMessageBox::information(m_dlg, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>",
                                     m_dlg->m_colTypeName.value(m_profile->m_colNumType.value(col))));
    resetComboBox(m_profile->m_colNumType.value(col));
    resetComboBox(type);
    ret = false;
  } else {
    m_profile->m_colTypeNum[type] = col; // assign new column 'number' to this 'type'
    m_profile->m_colNumType[col] = type; // assign new 'type' to this column 'number'
  }
  emit completeChanged();
  return ret;
}

bool BankingPage::validateCreditDebit()
{
  for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row) {
    // process credit/debit field
    if (m_profile->m_colTypeNum.value(Column::Credit) != -1 &&
        m_profile->m_colTypeNum.value(Column::Debit) != -1) {
      QString credit = m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(Column::Credit))->text();
      QString debit = m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(Column::Debit))->text();
      m_imp->processCreditDebit(credit, debit);
      if (!credit.isEmpty() && !debit.isEmpty()) {
        int ret = KMessageBox::questionYesNoCancel(m_dlg,
                                                   i18n("<center>The %1 field contains '%2'</center>"
                                                        "<center>and the %3 field contains '%4'.</center>"
                                                        "<center>Please choose which you wish to accept.</center>",
                                                        m_dlg->m_colTypeName.value(Column::Debit),
                                                        debit,
                                                        m_dlg->m_colTypeName.value(Column::Credit),
                                                        credit),
                                                   i18n("CSV invalid field values"),
                                                   KGuiItem(i18n("Accept %1", m_dlg->m_colTypeName.value(Column::Debit))),
                                                   KGuiItem(i18n("Accept %1", m_dlg->m_colTypeName.value(Column::Credit))),
                                                   KGuiItem(i18n("Cancel")));
        switch(ret) {
          case KMessageBox::Cancel:
            return false;
          case KMessageBox::Yes:
            m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(Column::Credit))->setText(QString());
            break;
          case KMessageBox::No:
            m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(Column::Debit))->setText(QString());
            break;
        }
      }
    }
  }
  return true;
}

void BankingPage::makeQIF(const MyMoneyStatement& st, const QString& outFileName)
{
  QFile oFile(outFileName);
  oFile.open(QIODevice::WriteOnly);
  QTextStream out(&oFile);

  QString buffer;
  QString strEType;

  switch (st.m_eType) {
  case MyMoneyStatement::etCreditCard:
    strEType = QStringLiteral("CCard");
  case MyMoneyStatement::etSavings:
  case MyMoneyStatement::etCheckings:
  default:
    strEType = QStringLiteral("Bank");
  }

  if (!st.m_strAccountName.isEmpty()) {
    buffer.append(QStringLiteral("!Account\n"));
    buffer.append(QChar(QLatin1Char('N')) + st.m_strAccountName + QChar(QLatin1Char('\n')));
    buffer.append(QChar(QLatin1Char('T')) + strEType + QChar(QLatin1Char('\n')));
    buffer.append(QStringLiteral("^\n"));
  }

  buffer.append(QStringLiteral("!Type:") + strEType + QChar(QLatin1Char('\n')));

  for (QList<MyMoneyStatement::Transaction>::const_iterator it = st.m_listTransactions.constBegin(); it != st.m_listTransactions.constEnd(); ++it) {
    buffer.append(QChar(QLatin1Char('D')) + it->m_datePosted.toString(QStringLiteral("MM/dd/yyyy")) + QChar(QLatin1Char('\n')));
    QString txt;
    txt.setNum(it->m_amount.toDouble(), 'f', 4);
    buffer.append(QChar(QLatin1Char('T')) + txt + QChar(QLatin1Char('\n')));
    buffer.append(QChar(QLatin1Char('P')) + it->m_strPayee + QChar(QLatin1Char('\n')));
    if (!it->m_listSplits.isEmpty())
      buffer.append(QChar(QLatin1Char('L')) + it->m_listSplits.first().m_strCategoryName + QChar(QLatin1Char('\n')));
    if (!it->m_strNumber.isEmpty())
      buffer.append(QChar(QLatin1Char('N')) + it->m_strNumber + QChar(QLatin1Char('\n')));
    if (!it->m_strMemo.isEmpty())
      buffer.append(QChar(QLatin1Char('M')) + it->m_strMemo + QChar(QLatin1Char('\n')));
    buffer.append(QStringLiteral("^\n"));
    out << buffer;// output qif file
    buffer.clear();
  }
  oFile.close();
}
