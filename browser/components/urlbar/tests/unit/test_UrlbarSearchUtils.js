/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const { UrlbarSearchUtils } = ChromeUtils.importESModule(
  "resource:///modules/UrlbarSearchUtils.sys.mjs"
);
const { updateAppInfo } = ChromeUtils.importESModule(
  "resource://testing-common/AppInfo.sys.mjs"
);

let baconEngineExtension;

add_setup(async function () {
  updateAppInfo({
    name: "firefox",
  });
});

add_task(async function () {
  await UrlbarSearchUtils.init();
  // Tell the search service we are running in the US.  This also has the
  // desired side-effect of preventing our geoip lookup.
  Services.prefs.setCharPref("browser.search.region", "US");

  Services.search.restoreDefaultEngines();
  Services.search.resetToAppDefaultEngine();
});

add_task(async function search_engine_match() {
  let engine = await Services.search.getDefault();
  let domain = engine.searchUrlDomain;
  let token = domain.substr(0, 1);
  let matchedEngine = (
    await UrlbarSearchUtils.enginesForDomainPrefix(token)
  )[0];
  Assert.equal(matchedEngine, engine);
});

add_task(async function no_match() {
  Assert.equal(
    0,
    (await UrlbarSearchUtils.enginesForDomainPrefix("test")).length
  );
});

add_task(async function hide_search_engine_nomatch() {
  let engine = await Services.search.getDefault();
  let domain = engine.searchUrlDomain;
  let token = domain.substr(0, 1);
  let promiseTopic = promiseSearchTopic("engine-changed");
  await Promise.all([Services.search.removeEngine(engine), promiseTopic]);
  Assert.ok(engine.hidden);
  let matchedEngines = await UrlbarSearchUtils.enginesForDomainPrefix(token);
  Assert.ok(
    !matchedEngines.length || matchedEngines[0].searchUrlDomain != domain
  );
  engine.hidden = false;
  await TestUtils.waitForCondition(
    async () => (await UrlbarSearchUtils.enginesForDomainPrefix(token)).length
  );
  let matchedEngine2 = (
    await UrlbarSearchUtils.enginesForDomainPrefix(token)
  )[0];
  Assert.ok(matchedEngine2);
  await Services.search.setDefault(
    engine,
    Ci.nsISearchService.CHANGE_REASON_UNKNOWN
  );
});

add_task(async function onlyEnabled_option_nomatch() {
  let defaultEngine = await Services.search.getDefault();
  let domain = defaultEngine.searchUrlDomain;
  let token = domain.substr(0, 1);
  defaultEngine.hideOneOffButton = true;

  let matchedEngines = await UrlbarSearchUtils.enginesForDomainPrefix(token);
  Assert.notEqual(matchedEngines[0], defaultEngine);

  defaultEngine.hideOneOffButton = false;
  matchedEngines = await UrlbarSearchUtils.enginesForDomainPrefix(token);
  Assert.equal(matchedEngines[0].searchUrlDomain, domain);
  Assert.equal(matchedEngines[0], defaultEngine);
});

add_task(async function add_search_engine_match() {
  Assert.equal(
    0,
    (await UrlbarSearchUtils.enginesForDomainPrefix("bacon")).length
  );
  baconEngineExtension = await SearchTestUtils.installSearchExtension(
    {
      name: "bacon",
      keyword: "pork",
      search_url: "https://www.bacon.moz/",
    },
    { skipUnload: true }
  );
  let matchedEngine = (
    await UrlbarSearchUtils.enginesForDomainPrefix("bacon")
  )[0];
  Assert.ok(matchedEngine);
  Assert.equal(matchedEngine.name, "bacon");
  Assert.equal(await matchedEngine.getIconURL(), null);
  info("also type part of the public suffix");
  matchedEngine = (
    await UrlbarSearchUtils.enginesForDomainPrefix("bacon.m")
  )[0];
  Assert.ok(matchedEngine);
  Assert.equal(matchedEngine.name, "bacon");
  Assert.equal(await matchedEngine.getIconURL(), null);
});

add_task(async function match_multiple_search_engines() {
  Assert.equal(
    0,
    (await UrlbarSearchUtils.enginesForDomainPrefix("baseball")).length
  );
  await SearchTestUtils.installSearchExtension({
    name: "baseball",
    search_url: "https://www.baseball.moz/",
  });
  let matchedEngines = await UrlbarSearchUtils.enginesForDomainPrefix("ba");
  Assert.equal(
    matchedEngines.length,
    2,
    "enginesForDomainPrefix returned two engines."
  );
  Assert.equal(matchedEngines[0].name, "bacon");
  Assert.equal(matchedEngines[1].name, "baseball");
});

add_task(async function test_aliased_search_engine_match() {
  Assert.equal(null, await UrlbarSearchUtils.engineForAlias("sober"));
  // Lower case
  let matchedEngine = await UrlbarSearchUtils.engineForAlias("pork");
  Assert.ok(matchedEngine);
  Assert.equal(matchedEngine.name, "bacon");
  Assert.ok(matchedEngine.aliases.includes("pork"));
  Assert.equal(await matchedEngine.getIconURL(), null);
  // Upper case
  matchedEngine = await UrlbarSearchUtils.engineForAlias("PORK");
  Assert.ok(matchedEngine);
  Assert.equal(matchedEngine.name, "bacon");
  Assert.ok(matchedEngine.aliases.includes("pork"));
  Assert.equal(await matchedEngine.getIconURL(), null);
  // Cap case
  matchedEngine = await UrlbarSearchUtils.engineForAlias("Pork");
  Assert.ok(matchedEngine);
  Assert.equal(matchedEngine.name, "bacon");
  Assert.ok(matchedEngine.aliases.includes("pork"));
  Assert.equal(await matchedEngine.getIconURL(), null);
});

add_task(async function test_aliased_search_engine_match_upper_case_alias() {
  Assert.equal(
    0,
    (await UrlbarSearchUtils.enginesForDomainPrefix("patch")).length
  );
  await SearchTestUtils.installSearchExtension({
    name: "patch",
    keyword: "PR",
    search_url: "https://www.patch.moz/",
  });
  // lower case
  let matchedEngine = await UrlbarSearchUtils.engineForAlias("pr");
  Assert.ok(matchedEngine);
  Assert.equal(matchedEngine.name, "patch");
  Assert.ok(matchedEngine.aliases.includes("PR"));
  Assert.equal(await matchedEngine.getIconURL(), null);
  // Upper case
  matchedEngine = await UrlbarSearchUtils.engineForAlias("PR");
  Assert.ok(matchedEngine);
  Assert.equal(matchedEngine.name, "patch");
  Assert.ok(matchedEngine.aliases.includes("PR"));
  Assert.equal(await matchedEngine.getIconURL(), null);
  // Cap case
  matchedEngine = await UrlbarSearchUtils.engineForAlias("Pr");
  Assert.ok(matchedEngine);
  Assert.equal(matchedEngine.name, "patch");
  Assert.ok(matchedEngine.aliases.includes("PR"));
  Assert.equal(await matchedEngine.getIconURL(), null);
});

add_task(async function remove_search_engine_nomatch() {
  let promiseTopic = promiseSearchTopic("engine-removed");
  await Promise.all([baconEngineExtension.unload(), promiseTopic]);
  Assert.equal(
    0,
    (await UrlbarSearchUtils.enginesForDomainPrefix("bacon")).length
  );
});

add_task(async function test_builtin_aliased_search_engine_match() {
  let engine = await UrlbarSearchUtils.engineForAlias("@google");
  Assert.ok(engine);
  Assert.equal(engine.name, "Google");
  let promiseTopic = promiseSearchTopic("engine-changed");
  await Promise.all([Services.search.removeEngine(engine), promiseTopic]);
  let matchedEngine = await UrlbarSearchUtils.engineForAlias("@google");
  Assert.ok(!matchedEngine);
  engine.hidden = false;
  await TestUtils.waitForCondition(() =>
    UrlbarSearchUtils.engineForAlias("@google")
  );
  engine = await UrlbarSearchUtils.engineForAlias("@google");
  Assert.ok(engine);
});

add_task(async function test_serps_are_equivalent() {
  info("Subset URL has extraneous parameters.");
  let url1 = "https://example.com/search?q=test&type=images";
  let url2 = "https://example.com/search?q=test";
  Assert.ok(!UrlbarSearchUtils.serpsAreEquivalent(url1, url2));
  info("Superset URL has extraneous parameters.");
  Assert.ok(UrlbarSearchUtils.serpsAreEquivalent(url2, url1));

  info("Same keys, different values.");
  url1 = "https://example.com/search?q=test&type=images";
  url2 = "https://example.com/search?q=test123&type=maps";
  Assert.ok(!UrlbarSearchUtils.serpsAreEquivalent(url1, url2));
  Assert.ok(!UrlbarSearchUtils.serpsAreEquivalent(url2, url1));

  info("Subset matching isn't strict (URL is subset of itself).");
  Assert.ok(UrlbarSearchUtils.serpsAreEquivalent(url1, url1));

  info("Origin and pathname are ignored.");
  url1 = "https://example.com/search?q=test";
  url2 = "https://example-1.com/maps?q=test";
  Assert.ok(UrlbarSearchUtils.serpsAreEquivalent(url1, url2));
  Assert.ok(UrlbarSearchUtils.serpsAreEquivalent(url2, url1));

  info("Params can be optionally ignored");
  url1 = "https://example.com/search?q=test&abc=123&foo=bar";
  url2 = "https://example.com/search?q=test";
  Assert.ok(!UrlbarSearchUtils.serpsAreEquivalent(url1, url2));
  Assert.ok(UrlbarSearchUtils.serpsAreEquivalent(url1, url2, ["abc", "foo"]));
});

add_task(async function test_get_root_domain_from_engine() {
  let extension = await SearchTestUtils.installSearchExtension(
    {
      name: "TestEngine2",
      search_url: "https://example.com/",
    },
    { skipUnload: true }
  );
  let engine = Services.search.getEngineByName("TestEngine2");
  Assert.equal(UrlbarSearchUtils.getRootDomainFromEngine(engine), "example");
  await extension.unload();

  extension = await SearchTestUtils.installSearchExtension(
    {
      name: "TestEngine",
      search_url: "https://www.subdomain.othersubdomain.example.com",
    },
    { skipUnload: true }
  );
  engine = Services.search.getEngineByName("TestEngine");
  Assert.equal(UrlbarSearchUtils.getRootDomainFromEngine(engine), "example");
  await extension.unload();

  // We let engines with URL ending in .test through even though its not a valid
  // TLD.
  extension = await SearchTestUtils.installSearchExtension(
    {
      name: "TestMalformed",
      search_url: "https://mochi.test/",
      search_url_get_params: "search={searchTerms}",
    },
    { skipUnload: true }
  );
  engine = Services.search.getEngineByName("TestMalformed");
  Assert.equal(UrlbarSearchUtils.getRootDomainFromEngine(engine), "mochi");
  await extension.unload();

  // We return the domain for engines with a malformed URL.
  extension = await SearchTestUtils.installSearchExtension(
    {
      name: "TestMalformed",
      search_url: "https://subdomain.foobar/",
      search_url_get_params: "search={searchTerms}",
    },
    { skipUnload: true }
  );
  engine = Services.search.getEngineByName("TestMalformed");
  Assert.equal(
    UrlbarSearchUtils.getRootDomainFromEngine(engine),
    "subdomain.foobar"
  );
  await extension.unload();
});

add_task(async function matchAllDomainLevels() {
  let baseHostname = "matchalldomainlevels";
  Assert.equal(
    (await UrlbarSearchUtils.enginesForDomainPrefix(baseHostname)).length,
    0,
    `Sanity check: No engines initially match ${baseHostname}`
  );

  // Install engines with the following domains. When we match engines below,
  // perfectly matching domains should come before partially matching domains.
  let baseDomain = `${baseHostname}.com`;
  let perfectDomains = [baseDomain, `www.${baseDomain}`];
  let partialDomains = [`foo.${baseDomain}`, `foo.bar.${baseDomain}`];

  // Install engines with partially matching domains first so that the test
  // isn't incidentally passing because engines are installed in the order it
  // ultimately expects them in. Wait for each engine to finish installing
  // before starting the next one to avoid intermittent out-of-order failures.
  let extensions = [];
  for (let list of [partialDomains, perfectDomains]) {
    for (let domain of list) {
      let ext = await SearchTestUtils.installSearchExtension(
        {
          name: domain,
          search_url: `https://${domain}/`,
        },
        { skipUnload: true }
      );
      extensions.push(ext);
    }
  }

  // Perfect matches come before partial matches.
  let expectedDomains = [...perfectDomains, ...partialDomains];

  // Do searches for the following strings. Each should match all the engines
  // installed above.
  let searchStrings = [baseHostname, baseHostname + "."];
  for (let searchString of searchStrings) {
    info(`Searching for "${searchString}"`);
    let engines = await UrlbarSearchUtils.enginesForDomainPrefix(searchString, {
      matchAllDomainLevels: true,
    });
    // Domain names are saved in engine names.
    let actualDomains = engines.map(e => e.name);
    Assert.deepEqual(
      actualDomains,
      expectedDomains,
      "Expected matching engine names/domains in the expected order"
    );
  }

  await Promise.all(extensions.map(e => e.unload()));
});

function promiseSearchTopic(expectedVerb) {
  return new Promise(resolve => {
    Services.obs.addObserver(function observe(subject, topic, verb) {
      info("browser-search-engine-modified: " + verb);
      if (verb == expectedVerb) {
        Services.obs.removeObserver(observe, "browser-search-engine-modified");
        resolve();
      }
    }, "browser-search-engine-modified");
  });
}
