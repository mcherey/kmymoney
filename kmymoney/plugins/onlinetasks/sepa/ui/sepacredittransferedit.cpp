/*
  This file is part of KMyMoney, A Personal Finance Manager for KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sepacredittransferedit.h"
#include "ui_sepacredittransferedit.h"
#include "kguiutils.h"

#include "onlinetasks/sepa/tasks/sepaonlinetransfer.h"
#include "payeeidentifier/ibanandbic/widgets/ibanvalidator.h"
#include "payeeidentifier/payeeidentifiertyped.h"

sepaCreditTransferEdit::sepaCreditTransferEdit(QWidget *parent, QVariantList args) :
    IonlineJobEdit(parent, args),
    ui(new Ui::sepaCreditTransferEdit),
    m_onlineJob( onlineJobTyped<sepaOnlineTransfer>() ),
    m_requiredFields( new kMandatoryFieldGroup(this) ),
    m_readOnly( false )
{
    ui->setupUi(this);

    m_requiredFields->add(ui->beneficiaryIban);
    m_requiredFields->add(ui->value);
    // Other required fields are set in updateSettings()

    connect(m_requiredFields, SIGNAL(stateChanged(bool)), this, SLOT(requiredFieldsCompleted(bool)));

    connect(ui->beneficiaryName, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryNameChanged(QString)));
    connect(ui->beneficiaryIban, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryIbanChanged(QString)));
    connect(ui->beneficiaryBankCode, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryBicChanged(QString)));
    connect(ui->value, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged()));
    connect(ui->sepaReference, SIGNAL(textChanged(QString)), this, SLOT(endToEndReferenceChanged(QString)));
    connect(ui->purpose, SIGNAL(textChanged()), this, SLOT(purposeChanged()));

    connect(ui->beneficiaryName, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->beneficiaryIban, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->beneficiaryBankCode, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->value, SIGNAL(valueChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->sepaReference, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->purpose, SIGNAL(textChanged()), this, SIGNAL(onlineJobChanged()));

    // Connect signals for read only
    connect(this, SIGNAL(readOnlyChanged(bool)), ui->beneficiaryName, SLOT(setReadOnly(bool)));
    connect(this, SIGNAL(readOnlyChanged(bool)), ui->beneficiaryIban, SLOT(setReadOnly(bool)));
    connect(this, SIGNAL(readOnlyChanged(bool)), ui->beneficiaryBankCode, SLOT(setReadOnly(bool)));
    connect(this, SIGNAL(readOnlyChanged(bool)), ui->value, SLOT(setReadOnly(bool)));
    connect(this, SIGNAL(readOnlyChanged(bool)), ui->sepaReference, SLOT(setReadOnly(bool)));
    connect(this, SIGNAL(readOnlyChanged(bool)), ui->purpose, SLOT(setReadOnly(bool)));
}

sepaCreditTransferEdit::~sepaCreditTransferEdit()
{
    delete ui;
}

void sepaCreditTransferEdit::showEvent(QShowEvent* event)
{
  updateEveryStatus();
  QWidget::showEvent(event);
}

onlineJobTyped<sepaOnlineTransfer> sepaCreditTransferEdit::getOnlineJobTyped() const
{
  onlineJobTyped<sepaOnlineTransfer> sepaJob( m_onlineJob );

  sepaJob.task()->setValue( ui->value->value() );
  sepaJob.task()->setPurpose( ui->purpose->toPlainText() );
  sepaJob.task()->setEndToEndReference( ui->sepaReference->text() );

  payeeIdentifiers::ibanBic accIdent;
  accIdent.setOwnerName( ui->beneficiaryName->text() );
  accIdent.setIban( ui->beneficiaryIban->text() );
  accIdent.setBic( ui->beneficiaryBankCode->text() );
  sepaJob.task()->setBeneficiary(accIdent);

  return sepaJob;
}

void sepaCreditTransferEdit::setOnlineJob(const onlineJobTyped<sepaOnlineTransfer>& job )
{
  m_onlineJob = job;
  updateSettings();
  setReadOnly( !job.isEditable() );

  ui->purpose->setText( job.task()->purpose() );
  ui->sepaReference->setText( job.task()->endToEndReference() );
  ui->value->setValue( job.task()->value() );
  ui->beneficiaryName->setText( job.task()->beneficiaryTyped().ownerName() );
  ui->beneficiaryIban->setText( job.task()->beneficiaryTyped().paperformatIban() );
  ui->beneficiaryBankCode->setText( job.task()->beneficiaryTyped().storedBic() );
}

bool sepaCreditTransferEdit::setOnlineJob( const onlineJob& job )
{
  if( !job.isNull() && job.task()->taskName() == sepaOnlineTransfer::name() ) {
    setOnlineJob( onlineJobTyped<sepaOnlineTransfer>(job) );
    return true;
  }
  return false;
}

void sepaCreditTransferEdit::setOriginAccount(const QString& accountId)
{
  m_onlineJob.task()->setOriginAccount( accountId );
  updateSettings();
}

void sepaCreditTransferEdit::updateEveryStatus()
{
    beneficiaryNameChanged( ui->beneficiaryName->text() );
    beneficiaryIbanChanged( ui->beneficiaryIban->text() );
    beneficiaryBicChanged( ui->beneficiaryBankCode->text() );
    purposeChanged();
    valueChanged();
    endToEndReferenceChanged( ui->sepaReference->text() );
}

void sepaCreditTransferEdit::setReadOnly(const bool& readOnly)
{
  // Only set writeable if it changes something and if it is possible
  if ( readOnly != m_readOnly && (readOnly == true || getOnlineJobTyped().isEditable()) ) {
    m_readOnly = readOnly;
    emit readOnlyChanged( m_readOnly );
  }
}

void sepaCreditTransferEdit::updateSettings()
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  // Reference
  ui->sepaReference->setMaxLength( settings->endToEndReferenceLength() );
  if (settings->endToEndReferenceLength() == 0)
    ui->sepaReference->setEnabled( false );
  else
    ui->sepaReference->setEnabled( true );

  // Purpose
  ui->purpose->setAllowedChars( settings->allowedChars() );
  ui->purpose->setMaxLineLength( settings->purposeLineLength() );
  ui->purpose->setMaxLines( settings->purposeMaxLines() );
  if (settings->purposeMinLength())
    m_requiredFields->add(ui->purpose);
  else
    m_requiredFields->remove(ui->purpose);

  // Beneficiary Name
  ui->beneficiaryName->setValidator( new QRegExpValidator(QRegExp( QString("[%1]*").arg(settings->allowedChars()) ), ui->beneficiaryName) );
  ui->beneficiaryName->setMaxLength( settings->recipientNameLineLength() );

  if (settings->recipientNameMinLength() != 0)
    m_requiredFields->add(ui->beneficiaryName);
  else
    m_requiredFields->remove(ui->beneficiaryName);

  updateEveryStatus();
}

void sepaCreditTransferEdit::beneficiaryIbanChanged(const QString& iban)
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();

  QString payeeIban;
  try {
    payeeIdentifier ident = getOnlineJobTyped().task()->originAccountIdentifier();
    payeeIban = ident.data<payeeIdentifiers::ibanBic>()->electronicIban();
  } catch ( payeeIdentifier::exception& ) {
  }

  if (settings->isBicMandatory( payeeIban, iban )) {
    m_requiredFields->add( ui->beneficiaryBankCode );
    beneficiaryBicChanged( ui->beneficiaryBankCode->text() );
  } else {
    m_requiredFields->remove( ui->beneficiaryBankCode );
    beneficiaryBicChanged( ui->beneficiaryBankCode->text() );
  }
}

void sepaCreditTransferEdit::beneficiaryBicChanged(const QString& bic)
{
  if ( bic.isEmpty() && !ui->beneficiaryIban->text().isEmpty() ) {
    QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();

    const payeeIdentifiers::ibanBic payee = getOnlineJobTyped().task()->beneficiaryTyped();
    if (settings->isBicMandatory(payee.electronicIban() , ui->beneficiaryIban->text() )) {
      ui->feedbackBic->setFeedback(KMyMoneyValidationFeedback::Error, i18n("For this beneficiary's country the BIC is mandatory."));
      return;
    }
  }
  ui->feedbackBic->removeFeedback(KMyMoneyValidationFeedback::Error, i18n("For this beneficiary's country the BIC is mandatory."));
}

void sepaCreditTransferEdit::beneficiaryNameChanged( const QString& name )
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  if ( name.length() < settings->recipientNameMinLength() ) {
    ui->feedbackName->setFeedback( KMyMoneyValidationFeedback::Error, i18np("A beneficiary name is needed.", "The beneficiary name must be at least %i characters long",
        settings->recipientNameMinLength()
      ) );
  } else {
    ui->feedbackName->removeFeedback( KMyMoneyValidationFeedback::Error, i18np("A beneficiary name is needed.", "The beneficiary name must be at least %i characters long",
                                                                            settings->recipientNameMinLength()
    )  );
  }
}

void sepaCreditTransferEdit::valueChanged()
{
  if ( !ui->value->isValid() || !ui->value->value().isPositive() ) {
    ui->feedbackAmount->setFeedback( KMyMoneyValidationFeedback::Error, i18n("A positiv amount to transfer is needed.") );
    return;
  }

  const MyMoneyAccount account = getOnlineJob().responsibleMyMoneyAccount();
  const MyMoneyMoney expectedBalance = account.balance() - ui->value->value();

  if ( expectedBalance < MyMoneyMoney(  account.value("maxCreditAbsolute") ) ) {
    ui->feedbackAmount->setFeedback( KMyMoneyValidationFeedback::Warning, i18n("After this credit transfer the account's balance will be below your credit limit."));
  } else if ( expectedBalance < MyMoneyMoney( account.value("minBalanceAbsolute") )) {
    ui->feedbackAmount->setFeedback( KMyMoneyValidationFeedback::Information, i18n("After this credit transfer the account's balance will be below the minium balance."));
  } else {
    ui->feedbackAmount->removeFeedback();
  }
}

void sepaCreditTransferEdit::endToEndReferenceChanged( const QString& reference )
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  if ( settings->checkEndToEndReferenceLength( reference ) == validators::tooLong) {
    ui->feedbackReference->setFeedback( KMyMoneyValidationFeedback::Error, i18np("The end-to-end refence cannot contain more than one character.",
                                                                                 "The end-to-end refence cannot contain more than %1 characters.",
                                                                                 settings->endToEndReferenceLength()
    ) );
  } else {
    ui->feedbackReference->removeFeedback();
  }
}

void sepaCreditTransferEdit::purposeChanged()
{
  const QString purpose = ui->purpose->toPlainText();
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();

  QString message = QString();
  if (!settings->checkPurposeLineLength( purpose ))
    message = i18np("The maximal line length of %1 character per line is exceeded.", "The maximal line length of %1 characters per line is exceeded.",
                    settings->purposeLineLength())
    .append('\n');
  if (!settings->checkPurposeCharset( purpose ))
    message.append( i18n("The purpose can only contain the letters A-Z, spaces and ':?.,-()+ and /") ).append('\n');
  if ( !settings->checkPurposeMaxLines(purpose) ) {
    message.append( i18np("In the purpose only a single line is allowed.", "The purpose cannot contain more than %1 lines.",
                          settings->purposeMaxLines()) )
    .append('\n');
  } else if (settings->checkPurposeLength(purpose) == validators::tooShort) {
    message.append( i18np("A purpose is needed.", "The purpose must be at least %1 characters long.", settings->purposeMinLength()) )
    .append('\n');
  }

  // Remove the last '\n'
  message.chop(1);

  if ( !message.isEmpty() ) {
    ui->feedbackPurpose->setFeedback( KMyMoneyValidationFeedback::Error, message );
  } else {
    ui->feedbackPurpose->removeFeedback();
  }
}

QSharedPointer< const sepaOnlineTransfer::settings > sepaCreditTransferEdit::taskSettings()
{
    return getOnlineJobTyped().constTask()->getSettings();
}
