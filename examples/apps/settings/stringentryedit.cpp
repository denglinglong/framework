/* This file is part of Maliit framework
 *
 * Copyright (C) 2012 Canonical Ltd
 *
 * Contact: maliit-discuss@lists.maliit.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "stringentryedit.h"

StringEntryEdit::StringEntryEdit(const QSharedPointer<Maliit::SettingsEntry>& entry)
    : QLineEdit(entry ? entry->value().toString() : "")
    , m_entry(entry)
{
    if (entry) {
        connect (this, SIGNAL(returnPressed()),
                 this, SLOT(onReturnPressed()));
    } else {
        setDisabled(true);
    }
}

void StringEntryEdit::onReturnPressed()
{
    if (m_entry) {
        QVariant string_value(text());

        if (m_entry->isValid(string_value)) {
            m_entry->set(string_value);
        }
    }
}
