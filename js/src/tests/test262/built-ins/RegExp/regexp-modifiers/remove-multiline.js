// |reftest| shell-option(--enable-regexp-modifiers) skip-if(release_or_beta||!xulRuntime.shell) -- regexp-modifiers is not released yet, requires shell-options
// Copyright 2023 Ron Buckton. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
author: Ron Buckton
description: >
  multiline (`m`) modifier can be removed via `(?-m:)`.
info: |
  Runtime Semantics: CompileAtom
  The syntax-directed operation CompileAtom takes arguments direction (forward or backward) and modifiers (a Modifiers Record) and returns a Matcher.

  Atom :: `(` `?` RegularExpressionFlags `-` RegularExpressionFlags `:` Disjunction `)`
    1. Let addModifiers be the source text matched by the first RegularExpressionFlags.
    2. Let removeModifiers be the source text matched by the second RegularExpressionFlags.
    3. Let newModifiers be UpdateModifiers(modifiers, CodePointsToString(addModifiers), CodePointsToString(removeModifiers)).
    4. Return CompileSubpattern of Disjunction with arguments direction and newModifiers.

  UpdateModifiers ( modifiers, add, remove )
  The abstract operation UpdateModifiers takes arguments modifiers (a Modifiers Record), add (a String), and remove (a String) and returns a Modifiers. It performs the following steps when called:

  1. Let dotAll be modifiers.[[DotAll]].
  2. Let ignoreCase be modifiers.[[IgnoreCase]].
  3. Let multiline be modifiers.[[Multiline]].
  4. If add contains "s", set dotAll to true.
  5. If add contains "i", set ignoreCase to true.
  6. If add contains "m", set multiline to true.
  7. If remove contains "s", set dotAll to false.
  8. If remove contains "i", set ignoreCase to false.
  9. If remove contains "m", set multiline to false.
  10. Return the Modifiers Record { [[DotAll]]: dotAll, [[IgnoreCase]]: ignoreCase, [[Multiline]]: multiline }.

esid: sec-compileatom
features: [regexp-modifiers]
---*/

var re1 = /^(?-m:es$)/m;
assert(!re1.test("\nes\ns"), "$ should not match newline in modified group");
assert(re1.test("\nes"), "$ should match end of input in modified group");

var re2 = new RegExp("^(?-m:es$)", "m");
assert(!re2.test("\nes\ns"), "$ should not match newline in modified group");
assert(re2.test("\nes"), "$ should match end of input in modified group");

var re3 = /(?-m:^es)$/m;
assert(!re3.test("e\nes\n"), "^ should not match newline in modified group");
assert(re3.test("es\n"), "^ should match start of input in modified group");

var re4 = new RegExp("(?-m:^es)$", "m");
assert(!re4.test("e\nes\n"), "^ should not match newline in modified group");
assert(re4.test("es\n"), "^ should match start of input in modified group");

reportCompare(0, 0);
