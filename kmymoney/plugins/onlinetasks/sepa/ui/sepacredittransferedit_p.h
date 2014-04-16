/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
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

#ifndef SEPACREDITTRANSFEREDIT_P_H
#define SEPACREDITTRANSFEREDIT_P_H

#include <QtGui/QValidator>

/**
  * @brief Validator for IBAN
  * A check if this code actualy exists is not done
  */
class ibanValidator : public QValidator
{
  Q_OBJECT
public:
  explicit ibanValidator(QObject* parent = 0)
    : QValidator(parent) {}
  State validate(QString &string, int &pos) const;
};

#endif // SEPACREDITTRANSFEREDIT_P_H
