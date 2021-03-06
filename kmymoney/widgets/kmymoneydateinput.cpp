/***************************************************************************
                          kmymoneydateinput.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneydateinput.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPoint>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QLabel>
#include <QKeyEvent>
#include <QEvent>
#include <QDateEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KPassivePopup>
#include <KDatePicker>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons/icons.h"

using namespace Icons;

namespace
{
const int DATE_POPUP_TIMEOUT = 1500;
const QDate INVALID_DATE = QDate(1800, 1, 1);
}

void KMyMoney::OldDateEdit::keyPressEvent(QKeyEvent* k)
{
  if ((lineEdit()->text().isEmpty() || lineEdit()->selectedText() == lineEdit()->text()) && QChar(k->key()).isDigit()) {
    // the line edit is empty which means that the date was cleared
    // or the whole text is selected and a digit character was entered
    // (the same meaning as clearing the date) - in this case set the date
    // to the current date and let the editor do the actual work
    setDate(QDate::currentDate());
    setSelectedSection(QDateTimeEdit::DaySection); // start as when focused in if the date was cleared
  }
  QDateEdit::keyPressEvent(k);
}

void KMyMoney::OldDateEdit::focusInEvent(QFocusEvent * event)
{
  QDateEdit::focusInEvent(event);
  setSelectedSection(QDateTimeEdit::DaySection);
}

bool KMyMoney::OldDateEdit::event(QEvent* e)
{
  // make sure that we keep the current date setting of a kMyMoneyDateInput object
  // across the QDateEdit::event(FocusOutEvent)
  bool rc;

  kMyMoneyDateInput* p = dynamic_cast<kMyMoneyDateInput*>(parentWidget());
  if (e->type() == QEvent::FocusOut && p) {
    QDate d = p->date();
    rc = QDateEdit::event(e);
    if (d.isValid())
      d = p->date();
    p->loadDate(d);
  } else {
    rc = QDateEdit::event(e);
  }
  return rc;
}

bool KMyMoney::OldDateEdit::focusNextPrevChild(bool next)
{
  Q_UNUSED(next)
  return true;
}

struct kMyMoneyDateInput::Private {
  QDateEdit *m_dateEdit;
  KDatePicker *m_datePicker;
  QDate m_date;
  QDate m_prevDate;
  Qt::AlignmentFlag m_qtalignment;
  QWidget *m_dateFrame;
  QPushButton *m_dateButton;
  KPassivePopup *m_datePopup;
};

kMyMoneyDateInput::kMyMoneyDateInput(QWidget *parent, Qt::AlignmentFlag flags)
    : QWidget(parent), d(new Private)
{
  d->m_qtalignment = flags;
  d->m_date = QDate::currentDate();

  QHBoxLayout *dateInputLayout = new QHBoxLayout(this);
  dateInputLayout->setSpacing(0);
  dateInputLayout->setContentsMargins(0, 0, 0, 0);
  d->m_dateEdit = new KMyMoney::OldDateEdit(d->m_date, this);
  dateInputLayout->addWidget(d->m_dateEdit, 3);
  setFocusProxy(d->m_dateEdit);
  d->m_dateEdit->installEventFilter(this); // To get d->m_dateEdit's FocusIn/Out and some KeyPress events

  // we use INVALID_DATE as a special value for multi transaction editing
  d->m_dateEdit->setMinimumDate(INVALID_DATE);
  d->m_dateEdit->setSpecialValueText(QLatin1String(" "));

  d->m_datePopup = new KPassivePopup(d->m_dateEdit);
  d->m_datePopup->setObjectName("datePopup");
  d->m_datePopup->setTimeout(DATE_POPUP_TIMEOUT);
  d->m_datePopup->setView(new QLabel(QLocale().toString(d->m_date), d->m_datePopup));

  d->m_dateFrame = new QWidget(this);
  dateInputLayout->addWidget(d->m_dateFrame);
  QVBoxLayout *dateFrameVBoxLayout = new QVBoxLayout(d->m_dateFrame);
  dateFrameVBoxLayout->setMargin(0);
  dateFrameVBoxLayout->setContentsMargins(0, 0, 0, 0);
  d->m_dateFrame->setWindowFlags(Qt::Popup);
  d->m_dateFrame->hide();

  d->m_dateEdit->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));

  d->m_datePicker = new KDatePicker(d->m_date, d->m_dateFrame);
  dateFrameVBoxLayout->addWidget(d->m_datePicker);
  // Let the date picker have a close button (Added in 3.1)
  d->m_datePicker->setCloseButton(true);

  // the next line is a try to add an icon to the button
  d->m_dateButton = new QPushButton(QIcon::fromTheme(g_Icons[Icon::ViewCalendarDay]), QString(), this);
  dateInputLayout->addWidget(d->m_dateButton);

  connect(d->m_dateButton, SIGNAL(clicked()), SLOT(toggleDatePicker()));
  connect(d->m_dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChosenRef(QDate)));
  connect(d->m_datePicker, SIGNAL(dateSelected(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(d->m_datePicker, SIGNAL(dateEntered(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(d->m_datePicker, SIGNAL(dateSelected(QDate)), d->m_dateFrame, SLOT(hide()));
}

void kMyMoneyDateInput::markAsBadDate(bool bad, const QColor& color)
{
  // the next line knows a bit about the internals of QAbstractSpinBox
  QLineEdit* le = d->m_dateEdit->findChild<QLineEdit *>(); //krazy:exclude=qclasses

  if (le) {
    QPalette palette = this->palette();
    le->setPalette(palette);
    if (bad) {
      palette.setColor(foregroundRole(), color);
      le->setPalette(palette);
    }
  }
}

void kMyMoneyDateInput::showEvent(QShowEvent* event)
{
  // don't forget the standard behaviour  ;-)
  QWidget::showEvent(event);

  // If the widget is shown, the size must be fixed a little later
  // to be appropriate. I saw this in some other places and the only
  // way to solve this problem is to postpone the setup of the size
  // to the time when the widget is on the screen.
  QTimer::singleShot(50, this, SLOT(fixSize()));
}

void kMyMoneyDateInput::fixSize()
{
  // According to a hint in the documentation of KDatePicker::sizeHint()
  // 28 pixels should be added in each direction to obtain a better
  // display of the month button. I decided, (22,14) is good
  // enough and save some space on the screen (ipwizard)
  d->m_dateFrame->setFixedSize(d->m_datePicker->sizeHint() + QSize(22, 14));
}

kMyMoneyDateInput::~kMyMoneyDateInput()
{
  delete d->m_dateFrame;
  delete d->m_datePopup;
  delete d;
}

void kMyMoneyDateInput::toggleDatePicker()
{
  int w = d->m_dateFrame->width();
  int h = d->m_dateFrame->height();

  if (d->m_dateFrame->isVisible()) {
    d->m_dateFrame->hide();
  } else {
    QPoint tmpPoint = mapToGlobal(d->m_dateButton->geometry().bottomRight());

    // usually, the datepicker widget is shown underneath the d->m_dateEdit widget
    // if it does not fit on the screen, we show it above this widget

    if (tmpPoint.y() + h > QApplication::desktop()->height()) {
      tmpPoint.setY(tmpPoint.y() - h - d->m_dateButton->height());
    }

    if ((d->m_qtalignment == Qt::AlignRight && tmpPoint.x() + w <= QApplication::desktop()->width())
        || (tmpPoint.x() - w < 0)) {
      d->m_dateFrame->setGeometry(tmpPoint.x() - width(), tmpPoint.y(), w, h);
    } else {
      tmpPoint.setX(tmpPoint.x() - w);
      d->m_dateFrame->setGeometry(tmpPoint.x(), tmpPoint.y(), w, h);
    }

    if (d->m_date.isValid() && d->m_date != INVALID_DATE) {
      d->m_datePicker->setDate(d->m_date);
    } else {
      d->m_datePicker->setDate(QDate::currentDate());
    }
    d->m_dateFrame->show();
  }
}


/** Overriding QWidget::keyPressEvent
  *
  * increments/decrements the date upon +/- or Up/Down key input
  * sets the date to current date when the 'T' key is pressed
  */
void kMyMoneyDateInput::keyPressEvent(QKeyEvent * k)
{
  QKeySequence today(i18nc("Enter todays date into date input widget", "T"));

  switch (k->key()) {
    case Qt::Key_Equal:
    case Qt::Key_Plus:
      slotDateChosen(d->m_date.addDays(1));
      k->accept();
      break;

    case Qt::Key_Minus:
      slotDateChosen(d->m_date.addDays(-1));
      k->accept();
      break;

    default:
      if (today == QKeySequence(k->key()) || k->key() == Qt::Key_T) {
        slotDateChosen(QDate::currentDate());
        k->accept();
      }
      break;
  }
  k->ignore(); // signal that the key event was not handled
}

/**
  * This function receives all events that are sent to focusWidget().
  * Some KeyPress events are intercepted and passed to keyPressEvent.
  * Otherwise they would be consumed by QDateEdit.
  */
bool kMyMoneyDateInput::eventFilter(QObject *, QEvent *e)
{
  if (e->type() == QEvent::FocusIn) {
#ifndef Q_OS_MAC
    d->m_datePopup->show(mapToGlobal(QPoint(0, height())));
#endif
    // select the date section, but we need to delay it a bit
  } else if (e->type() == QEvent::FocusOut) {
#ifndef Q_OS_MAC
    d->m_datePopup->hide();
#endif
  } else if (e->type() == QEvent::KeyPress) {
    if (QKeyEvent *k = dynamic_cast<QKeyEvent*>(e)) {
      keyPressEvent(k);
      if (k->isAccepted())
        return true; // signal that the key event was handled
    }
  }

  return false; // Don't filter the event
}

void kMyMoneyDateInput::slotDateChosenRef(const QDate& date)
{
  if (date.isValid()) {
    emit dateChanged(date);
    d->m_date = date;

#ifndef Q_OS_MAC
    QLabel *lbl = static_cast<QLabel*>(d->m_datePopup->view());
    lbl->setText(QLocale().toString(date));
    lbl->adjustSize();
    if (d->m_datePopup->isVisible() || hasFocus())
      d->m_datePopup->show(mapToGlobal(QPoint(0, height()))); // Repaint
#endif
  }
}

void kMyMoneyDateInput::slotDateChosen(QDate date)
{
  if (date.isValid()) {
    // the next line implies a call to slotDateChosenRef() above
    d->m_dateEdit->setDate(date);
  } else {
    d->m_dateEdit->setDate(INVALID_DATE);
  }
}

QDate kMyMoneyDateInput::date() const
{
  QDate rc = d->m_dateEdit->date();
  if (rc == INVALID_DATE)
    rc = QDate();
  return rc;
}

void kMyMoneyDateInput::setDate(QDate date)
{
  slotDateChosen(date);
}

void kMyMoneyDateInput::loadDate(const QDate& date)
{
  d->m_date = d->m_prevDate = date;

  blockSignals(true);
  slotDateChosen(date);
  blockSignals(false);
}

void kMyMoneyDateInput::resetDate()
{
  setDate(d->m_prevDate);
}

void kMyMoneyDateInput::setMaximumDate(const QDate& max)
{
  d->m_dateEdit->setMaximumDate(max);
}

QWidget* kMyMoneyDateInput::focusWidget() const
{
  QWidget* w = d->m_dateEdit;
  while (w->focusProxy())
    w = w->focusProxy();
  return w;
}
/*
void kMyMoneyDateInput::setRange(const QDate & min, const QDate & max)
{
  d->m_dateEdit->setDateRange(min, max);
}
*/
