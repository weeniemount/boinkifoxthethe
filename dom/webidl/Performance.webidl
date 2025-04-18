/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * The origin of this IDL file is
 * https://w3c.github.io/hr-time/#sec-performance
 * https://w3c.github.io/navigation-timing/#extensions-to-the-performance-interface
 * https://w3c.github.io/performance-timeline/#extensions-to-the-performance-interface
 * https://w3c.github.io/resource-timing/#sec-extensions-performance-interface
 * https://w3c.github.io/user-timing/#extensions-performance-interface
 *
 * Copyright © 2015 W3C® (MIT, ERCIM, Keio, Beihang).
 * W3C liability, trademark and document use rules apply.
 */

// DOMTimeStamp is deprecated, use EpochTimeStamp instead.
typedef unsigned long long DOMTimeStamp;
typedef unsigned long long EpochTimeStamp;
typedef double DOMHighResTimeStamp;
typedef sequence <PerformanceEntry> PerformanceEntryList;

// https://w3c.github.io/hr-time/#sec-performance
[Exposed=(Window,Worker)]
interface Performance : EventTarget {
  [DependsOn=DeviceState, Affects=Nothing]
  DOMHighResTimeStamp now();

  [Constant]
  readonly attribute DOMHighResTimeStamp timeOrigin;

  [Default] object toJSON();
};

// https://w3c.github.io/navigation-timing/#extensions-to-the-performance-interface
[Exposed=Window]
partial interface Performance {
  [Constant]
  readonly attribute PerformanceTiming timing;
  [Constant]
  readonly attribute PerformanceNavigation navigation;
};

// https://w3c.github.io/performance-timeline/#extensions-to-the-performance-interface
[Exposed=(Window,Worker)]
partial interface Performance {
  PerformanceEntryList getEntries();
  PerformanceEntryList getEntriesByType(DOMString entryType);
  PerformanceEntryList getEntriesByName(DOMString name, optional DOMString
    entryType);
};

// https://w3c.github.io/resource-timing/#sec-extensions-performance-interface
[Exposed=(Window,Worker)]
partial interface Performance {
  undefined clearResourceTimings();
  undefined setResourceTimingBufferSize(unsigned long maxSize);
  attribute EventHandler onresourcetimingbufferfull;
};

// GC microbenchmarks, pref-guarded, not for general use (bug 1125412)
[Exposed=Window]
partial interface Performance {
  [Pref="dom.enable_memory_stats"]
  readonly attribute object mozMemory;
};

// https://w3c.github.io/user-timing/#extensions-performance-interface
dictionary PerformanceMarkOptions {
  any detail;
  DOMHighResTimeStamp startTime;
};

// https://w3c.github.io/user-timing/#extensions-performance-interface
dictionary PerformanceMeasureOptions {
  any detail;
  (DOMString or DOMHighResTimeStamp) start;
  DOMHighResTimeStamp duration;
  (DOMString or DOMHighResTimeStamp) end;
};

// https://w3c.github.io/user-timing/#extensions-performance-interface
[Exposed=(Window,Worker)]
partial interface Performance {
  [Throws]
  PerformanceMark mark(DOMString markName, optional PerformanceMarkOptions markOptions = {});
  undefined clearMarks(optional DOMString markName);
  [Throws]
  PerformanceMeasure measure(DOMString measureName, optional (DOMString or PerformanceMeasureOptions) startOrMeasureOptions = {}, optional DOMString endMark);
  undefined clearMeasures(optional DOMString measureName);
};

[Exposed=Window]
partial interface Performance {
  [Pref="dom.enable_event_timing", SameObject]
  readonly attribute EventCounts eventCounts;

  [Pref="dom.performance.event_timing.enable_interactionid"]
  readonly attribute unsigned long long interactionCount;
};
