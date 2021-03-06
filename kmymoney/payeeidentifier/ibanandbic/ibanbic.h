/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PAYEEIDENTIFIER_IBANBIC_H
#define PAYEEIDENTIFIER_IBANBIC_H

#include <QtCore/QString>
#include <QtCore/QChar>

#include "payeeidentifier/payeeidentifierdata.h"
#include "mymoneyunittestable.h"
#include "iban_bic_identifier_export.h"

class ibanBicData;

namespace payeeIdentifiers
{
/**
 * @brief Plugin to handle IBANs and BICs
 *
 * Can store a pair of an International Bank Account Number (ISO 13616) and Business Identifier Code (ISO 9362).
 *
 */
class IBAN_BIC_IDENTIFIER_EXPORT ibanBic : public payeeIdentifierData
{
  KMM_MYMONEY_UNIT_TESTABLE
public:
  PAYEEIDENTIFIER_IID(ibanBic, "org.kmymoney.payeeIdentifier.ibanbic");

  enum bicAllocationStatus {
    bicAllocated = 0,
    bicNotAllocated,
    bicAllocationUncertain
  };

  ibanBic();
  ibanBic(const ibanBic& other);

  ibanBic* clone() const;
  ibanBic* createFromXml(const QDomElement& element) const;
  void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
   * @brief Set an owner name for this account
   */
  void setOwnerName(const QString& ownerName) {
    m_ownerName  = ownerName;
  }
  QString ownerName() const {
    return m_ownerName;
  }

  /**
   * @brief Set a IBAN
   *
   * The IBAN can contain spaces and other special chars.
   */
  void setIban(const QString& iban);

  /** @copydoc m_iban
   * Use this method if you know that iban is in electronic format already. No futher checks are done.
   */
  void setElectronicIban(const QString& iban) {
    Q_ASSERT(iban == ibanToElectronic(iban));
    m_iban = iban;
  }

  /** @copydoc m_iban */
  QString electronicIban() const {
    return m_iban;
  }

  /**
   * @brief Returns iban in human readable format
   * @see toPaperformatIban()
   */
  QString paperformatIban(const QString& separator = QLatin1String(" ")) const;

  /**
   * @brief Set Business Identifier Code
   *
   * Call without parameter or QString() to remove bic
   *
   * @param bic will be normalized
   */
  void setBic(const QString& bic = QString());

  /**
   * @brief Business Identifier Code
   * According to ISO 9362
   *
   * The returned bic is normalized:
   * A tailing XXX is omitted, all characters are uppercase.
   */
  QString storedBic() const {
    return m_bic;
  }

  /**
   * @copydoc storedBic()
   *
   * Return a stored BIC (if there is any) or try to use the iban to get a BIC.
   */
  QString bic() const;

  /**
   * @brief Business Identifier Code with tailing XXX
   *
   * Like @a bic() but always 11 characters long (if bic is invalid, it can have another length).
   */
  QString fullBic() const;

  /**
   * @copydoc fullBic()
   *
   * This method will not try to use the iban to get a bic.
   */
  QString fullStoredBic() const;

  /**
   * @brief Lookup institutions name
   *
   * Uses any available information to return an institutionName
   */
  QString institutionName() const {
    return institutionNameByBic(bic());
  }

  virtual bool operator==(const payeeIdentifierData& other) const;
  bool operator==(const ibanBic& other) const;
  virtual bool isValid() const;

  /**
   * @brief Extends a bic to 11 characters
   *
   * Also all characters are made upper case.
   */
  static QString bicToFullFormat(QString bic);

  /**
   * @brief Converts an iban to canonical format for machines
   *
   * Will remove all white spaces.
   */
  static QString ibanToElectronic(const QString& iban);

  /**
   * @brief Converts an iban to human readable format
   *
   * Grouped in four letters strings separated by a white space.
   *
   * @param iban an iban, not needed to be canonical, valid or completed
   * @param separator Overwrite the default separator (e.g. a smaller space)
   */
  static QString ibanToPaperformat(const QString& iban, const QString& seperator = QLatin1String(" "));

  /**
   * @brief Extract Basic Bank Account Number
   *
   * Returns the Basic Bank Account Number (BBAN) from the IBAN.
   * The BBAN is the IBAN without country code and the two digit checksum.
   */
  static QString bban(const QString& iban);

  static int ibanLengthByCountry(const QString& countryCode);

  static QString institutionNameByBic(const QString& bic);

  static QString bicByIban(const QString& iban);

  static QString localBankCodeByIban(const QString& iban);

  /**
   * @brief Chech if IBAN is valid
   */
  bool isIbanValid() const;

  /**
   * @brief Check if IBAN can be valid
   *
   * This method also checks if the given country code is valid.
   *
   * If also local aware checks are done (e.g. character set and length of BBAN).
   *
   * @todo Implement local aware checks
   */
  static bool isIbanValid(const QString& iban);


  /**
   * @brief Check if this BIC is assigned to an bank
   *
   * This method does not check the given BIC but looks up in the database directly.
   * So it might be useful if time consumption is important but isBicAllocated() should
   * be your first option.
   *
   * @param bic BIC to test in canonical format (always 11 characters long, all characters uppercase)
   * @see isBicAllocated()
   */
  static bicAllocationStatus isCanonicalBicAllocated(const QString& bic);

  /** @brief Check if this BIC is assigned to an bank
   *
   * @param bic BIC to test.
   */
  static bicAllocationStatus isBicAllocated(const QString& bic);

  /**
   * @brief Check the checksum
   *
   * Test if the ISO 7064 mod 97-10 checksum of the iban is correct.
   *
   * @param iban An IBAN in electronic format (important!)
   */
  static bool validateIbanChecksum(const QString& iban);

  static const int ibanMaxLength;

  QString storagePluginIid() const {
    return QLatin1String("org.kmymoney.payeeIdentifier.ibanbic.sqlStoragePlugin");
  }
  bool sqlSave(QSqlDatabase databaseConnection, const QString& objectId) const;
  bool sqlModify(QSqlDatabase databaseConnection, const QString& objectId) const;
  bool sqlRemove(QSqlDatabase databaseConnection, const QString& objectId) const;

private:
  /**
   * @brief Business Identifier Code
   * According to ISO 9362
   *
   * A trailing XXX must be ommitted. All characters must be upper case.
   */
  QString m_bic;

  /**
   * @brief  International Bank Account Number
   * According to ISO 13616-1:2007 Part 1
   * in normalized (electronic) format (no spaces etc.)
   */
  QString m_iban;

  QString m_ownerName;

  static ::ibanBicData* getIbanBicData();
  static ::ibanBicData* m_ibanBicData;

  bool writeQuery(QSqlQuery& query, const QString& id) const;
  payeeIdentifierData* createFromSqlDatabase(QSqlDatabase db, const QString& identId) const;

  static QString canonizeBic(const QString& bic);
};

} // namespace payeeIdentifiers

#endif // PAYEEIDENTIFIER_IBANBIC_H

