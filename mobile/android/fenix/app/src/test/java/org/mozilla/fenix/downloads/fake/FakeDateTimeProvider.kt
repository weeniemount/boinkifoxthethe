/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.fenix.downloads.fake

import org.mozilla.fenix.downloads.listscreen.middleware.DateTimeProvider
import java.time.LocalDate
import java.time.ZoneId
import java.time.ZoneOffset

class FakeDateTimeProvider(
    private val localDate: LocalDate,
) : DateTimeProvider {

    override fun currentLocalDate(): LocalDate = localDate

    override fun currentZoneId(): ZoneId = ZoneOffset.UTC
}
