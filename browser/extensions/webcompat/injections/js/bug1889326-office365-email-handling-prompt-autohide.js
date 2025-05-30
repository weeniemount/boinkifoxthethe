/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

/**
 * Bug 1889326 - Office 365 email handling prompt autohide
 *
 * This site patch prevents the notification bar on Office 365
 * apps from popping up on each page-load, offering to handle
 * email with Outlook.
 */

/* globals exportFunction */

const warning =
  "Office 365 Outlook email handling prompt has been hidden. See https://bugzilla.mozilla.org/show_bug.cgi?id=1889326 for details.";

const localStorageKey = "mailProtocolHandlerAlreadyOffered";

const proto = Object.getPrototypeOf(navigator).wrappedJSObject;
const { registerProtocolHandler } = proto;
const { localStorage } = window.wrappedJSObject;

proto.registerProtocolHandler = exportFunction(function (scheme, url, title) {
  if (localStorage.getItem(localStorageKey)) {
    console.info(warning);
    return undefined;
  }
  registerProtocolHandler.call(this, scheme, url, title);
  localStorage.setItem(localStorageKey, true);
  return undefined;
}, window);
