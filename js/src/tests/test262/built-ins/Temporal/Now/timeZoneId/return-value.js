// |reftest| shell-option(--enable-temporal) skip-if(!this.hasOwnProperty('Temporal')||!xulRuntime.shell) -- Temporal is not enabled unconditionally, requires shell-options
// Copyright (C) 2021 the V8 project authors. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.now.timezoneid
description: Temporal.Now.timeZoneId returns a string
info: |
  1. Return DefaultTimeZone().
features: [Temporal]
---*/

assert.sameValue(
  typeof Temporal.Now.timeZoneId(),
  "string",
  "Temporal.Now.timeZoneId() returns a string"
);

reportCompare(0, 0);
