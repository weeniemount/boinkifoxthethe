/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

"use strict";

/**
 * Tests if different response content types are handled correctly.
 */

const httpServer = createTestHTTPServer();
httpServer.registerContentType("html", "text/html");

const BASE_URL = `http://localhost:${httpServer.identity.primaryPort}/`;

const REDIRECT_URL = BASE_URL + "redirect.html";

// In all content previewed as HTML we ensure using proper html, head and body in order to
// prevent having them added by the <browser> when loaded as a preview.
function addBaseHtmlElements(body) {
  return `<html><head></head><body>${body}</body></html>`;
}

// This first page asserts we can redirect to another URL, even if JS happen to be executed
const FETCH_CONTENT_1 = addBaseHtmlElements(
  `Fetch 1<script>window.parent.location.href = "${REDIRECT_URL}";</script>`
);
// This second page asserts that JS is disabled
const FETCH_CONTENT_2 = addBaseHtmlElements(
  `Fetch 2<script>document.write("JS activated")</script>`
);
// This third page asserts that links and forms are disabled
const FETCH_CONTENT_3 = addBaseHtmlElements(
  `Fetch 3<a href="${REDIRECT_URL}">link</a> -- <form action="${REDIRECT_URL}"><input type="submit"></form>`
);
// This fourth page asserts responses with line breaks
const FETCH_CONTENT_4 = addBaseHtmlElements(`
  <a href="#" id="link1">link1</a>
  <a href="#" id="link2">link2</a>
`);
// This fifth page asserts that we apply inline styles
const FETCH_CONTENT_5 = addBaseHtmlElements(`
  <p style="color: red;">Hello World</p>
`);

// Use fetch in order to prevent actually running this code in the test page
const TEST_HTML = addBaseHtmlElements(`<div id="to-copy">HTML</div><script>
  fetch("${BASE_URL}fetch-1.html");
  fetch("${BASE_URL}fetch-2.html");
  fetch("${BASE_URL}fetch-3.html");
  fetch("${BASE_URL}fetch-4.html");
  fetch("${BASE_URL}fetch-5.html");
</script>`);
const TEST_URL = BASE_URL + "doc-html-preview.html";

httpServer.registerPathHandler(
  "/doc-html-preview.html",
  (request, response) => {
    response.setStatusLine(request.httpVersion, 200, "OK");
    response.write(TEST_HTML);
  }
);
httpServer.registerPathHandler("/fetch-1.html", (request, response) => {
  response.setStatusLine(request.httpVersion, 200, "OK");
  response.write(FETCH_CONTENT_1);
});
httpServer.registerPathHandler("/fetch-2.html", (request, response) => {
  response.setStatusLine(request.httpVersion, 200, "OK");
  response.write(FETCH_CONTENT_2);
});
httpServer.registerPathHandler("/fetch-3.html", (request, response) => {
  response.setStatusLine(request.httpVersion, 200, "OK");
  response.write(FETCH_CONTENT_3);
});
httpServer.registerPathHandler("/fetch-4.html", (request, response) => {
  response.setStatusLine(request.httpVersion, 200, "OK");
  response.write(FETCH_CONTENT_4);
});
httpServer.registerPathHandler("/fetch-5.html", (request, response) => {
  response.setStatusLine(request.httpVersion, 200, "OK");
  response.write(FETCH_CONTENT_5);
});
httpServer.registerPathHandler("/redirect.html", (request, response) => {
  response.setStatusLine(request.httpVersion, 200, "OK");
  response.write("Redirected!");
});

add_task(async function () {
  // Enable async events so that clicks on preview iframe's links are correctly
  // going through the parent process which is meant to cancel any mousedown.
  await pushPref("test.events.async.enabled", true);

  const { monitor } = await initNetMonitor(TEST_URL, { requestCount: 3 });
  info("Starting test... ");

  const { document, store, windowRequire } = monitor.panelWin;
  const Actions = windowRequire("devtools/client/netmonitor/src/actions/index");

  store.dispatch(Actions.batchEnable(false));

  const onNetworkEvent = waitForNetworkEvents(monitor, 3);
  await reloadBrowser();
  await onNetworkEvent;

  // The new lines are stripped when using outerHTML to retrieve HTML content of the preview iframe
  await selectIndexAndWaitForHtmlView(0, TEST_HTML);
  await selectIndexAndWaitForHtmlView(1, FETCH_CONTENT_1);
  await selectIndexAndWaitForHtmlView(2, FETCH_CONTENT_2);
  await selectIndexAndWaitForHtmlView(3, FETCH_CONTENT_3);
  await selectIndexAndWaitForHtmlView(4, FETCH_CONTENT_4);
  await selectIndexAndWaitForHtmlView(5, FETCH_CONTENT_5);

  await teardown(monitor);

  async function selectIndexAndWaitForHtmlView(index, expectedHtmlPreview) {
    info(`Select the request #${index}`);
    const onResponseContent = monitor.panelWin.api.once(
      TEST_EVENTS.RECEIVED_RESPONSE_CONTENT
    );
    store.dispatch(Actions.selectRequestByIndex(index));

    document.querySelector("#response-tab").click();

    const [browser] = await waitForDOM(
      document,
      "#response-panel .html-preview browser"
    );

    await BrowserTestUtils.browserLoaded(browser);

    info("Wait for response content to be loaded");
    await onResponseContent;

    is(
      browser.browsingContext.currentWindowGlobal.isInProcess,
      false,
      "The preview is loaded in a content process"
    );

    await SpecialPowers.spawn(
      browser.browsingContext,
      [expectedHtmlPreview],
      async function (expectedHtml) {
        is(
          content.document.documentElement.outerHTML,
          expectedHtml,
          "The text shown in the browser is incorrect for the html request."
        );
      }
    );

    if (expectedHtmlPreview == FETCH_CONTENT_5) {
      await SpecialPowers.spawn(browser.browsingContext, [], async function () {
        const p = content.document.querySelector("p");
        const computed = content.window.getComputedStyle(p);
        is(
          computed.getPropertyValue("color"),
          "rgb(255, 0, 0)",
          "The inline style was not applied"
        );
      });
    }

    // Only assert copy to clipboard on the first test page
    if (expectedHtmlPreview == TEST_HTML) {
      await waitForClipboardPromise(async function () {
        await SpecialPowers.spawn(
          browser.browsingContext,
          [],
          async function () {
            const elt = content.document.getElementById("to-copy");
            EventUtils.synthesizeMouseAtCenter(elt, { clickCount: 2 }, content);
            await new Promise(r =>
              elt.addEventListener("dblclick", r, { once: true })
            );
            EventUtils.synthesizeKey("c", { accelKey: true }, content);
          }
        );
      }, "HTML");
    }
  }
});
