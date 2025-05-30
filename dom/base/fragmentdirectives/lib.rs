/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use nsstring::{nsCString, nsString};
use rayon::prelude::*;
use thin_vec::ThinVec;
pub mod fragment_directive_impl;
mod test;

/// This struct contains the percent-decoded parts of a text directive.
/// All parts besides `start` are optional (which is indicated by an empty string).
///
/// This struct uses Gecko String types, whereas the parser internally uses Rust types.
/// Therefore, conversion functions are provided.
#[repr(C)]
pub struct TextDirective {
    prefix: nsString,
    start: nsString,
    end: nsString,
    suffix: nsString,
}

impl TextDirective {
    /// Creates a `FragmentDirectiveElement` object from a `FragmentDirectiveElementInternal` object
    /// (which uses Rust string types).
    fn from_rust_type(element: &fragment_directive_impl::TextDirective) -> Self {
        Self {
            prefix: element
                .prefix()
                .as_ref()
                .map_or_else(nsString::new, |token| nsString::from(token.value())),
            start: element
                .start()
                .as_ref()
                .map_or_else(nsString::new, |token| nsString::from(token.value())),
            end: element
                .end()
                .as_ref()
                .map_or_else(nsString::new, |token| nsString::from(token.value())),
            suffix: element
                .suffix()
                .as_ref()
                .map_or_else(nsString::new, |token| nsString::from(token.value())),
        }
    }

    /// Converts the contents of this object into Rust types.
    /// Returns `None` if the given fragment is not valid.
    /// The only invalid condition is a fragment that is missing the `start` token.
    fn to_rust_type(&self) -> Option<fragment_directive_impl::TextDirective> {
        fragment_directive_impl::TextDirective::from_parts(
            self.prefix.to_string(),
            self.start.to_string(),
            self.end.to_string(),
            self.suffix.to_string(),
        )
    }
}

/// Result of the `parse_fragment_directive()` function.
///
/// The result contains the original given URL without the fragment directive,
/// a unsanitized string version of the extracted fragment directive,
/// and an array of the parsed text directives.
#[repr(C)]
pub struct ParsedFragmentDirectiveResult {
    hash_without_fragment_directive: nsCString,
    fragment_directive: nsCString,
    text_directives: ThinVec<TextDirective>,
}

/// Parses the fragment directive from a given URL fragment.
///
/// This function writes the result data into `result`.
/// The result consists of
///   - the input url fragment without the fragment directive,
///   - the fragment directive as unparsed string,
///   - a list of the parsed and percent-decoded text directives.
///
/// Directives which are unknown will be ignored.
/// If new directive types are added in the future, they should also be considered here.
/// This function returns false if no fragment directive is found. If there is any
/// fragment directive (even if invalid), this function returns true.
#[no_mangle]
pub extern "C" fn parse_fragment_directive(
    hash: &nsCString,
    result: &mut ParsedFragmentDirectiveResult,
) -> bool {
    // sanitize inputs
    result.hash_without_fragment_directive = nsCString::new();
    result.fragment_directive = nsCString::new();
    result.text_directives.clear();

    let url_as_rust_string = hash.to_utf8();
    if let Some((stripped_hash, fragment_directive, text_directives)) =
        fragment_directive_impl::parse_fragment_directive_and_remove_it_from_hash(
            &url_as_rust_string,
        )
    {
        result
            .hash_without_fragment_directive
            .assign(&stripped_hash);
        result.fragment_directive.assign(&fragment_directive);
        result.text_directives.extend(
            text_directives
                .iter()
                .map(|text_directive| TextDirective::from_rust_type(text_directive)),
        );
        return true;
    }
    false
}

/// Creates a percent-encoded fragment directive string from a given list of `FragmentDirectiveElement`s.
///
/// The returned string has this form:
/// `:~:text=[prefix1-,]start1[,end1][,-suffix1]&text=[prefix2-,]start2[,end2][,-suffix2]`
///
/// Invalid `FragmentDirectiveElement`s are ignored, where "invalid" means that no `start` token is provided.
///  If there are no valid `FragmentDirectiveElement`s, an empty string is returned.
#[no_mangle]
pub extern "C" fn create_fragment_directive(
    text_directives: &ThinVec<TextDirective>,
    fragment_directive: &mut nsCString,
) -> bool {
    let directives_rust = Vec::from_iter(
        text_directives
            .iter()
            .filter_map(|fragment| fragment.to_rust_type()),
    );
    if let Some(fragment_directive_rust) =
        fragment_directive_impl::create_fragment_directive_string(&directives_rust)
    {
        fragment_directive.assign(&fragment_directive_rust);
        return true;
    }

    false
}

/// Creates a percent-encoded text directive string for a single text directive.
/// The returned string has the form `text=[prefix-,]start[,end][,-suffix]`.
/// If the provided `TextDirective` is invalid (i.e. it has no `start` attribute),
/// the outparam `directive_string` is empty and the function returns false.
#[no_mangle]
pub extern "C" fn create_text_directive(
    text_directive: &TextDirective,
    directive_string: &mut nsCString,
) -> bool {
    if let Some(text_directive_rust) = text_directive.to_rust_type() {
        if let Some(text_directive_string_rust) =
            fragment_directive_impl::create_text_directive_string(&text_directive_rust)
        {
            directive_string.assign(&text_directive_string_rust);
            return true;
        }
    }
    false
}

#[repr(C)]
pub struct TextDirectiveCandidateContents<'a> {
    full_prefix_content: &'a nsString,
    full_start_content: &'a nsString,
    full_end_content: &'a nsString,
    full_suffix_content: &'a nsString,

    prefix_content: &'a nsString,
    start_content: &'a nsString,
    end_content: &'a nsString,
    suffix_content: &'a nsString,
    use_exact_matching: bool,
}

impl<'a> TextDirectiveCandidateContents<'a> {
    /// Returns true if all context terms of `candidate` are contained
    /// in the start (or end, depending on which term)
    /// of the fully expanded terms of `self`.
    fn matches_candidate(&self, candidate: &Self) -> bool {
        if !self
            .full_prefix_content
            .ends_with(&candidate.prefix_content)
        {
            return false;
        }
        if !self
            .full_suffix_content
            .starts_with(&candidate.suffix_content)
        {
            return false;
        }
        assert!(self.use_exact_matching == candidate.use_exact_matching);
        if !candidate.use_exact_matching {
            if !self
                .full_start_content
                .starts_with(&candidate.start_content)
            {
                return false;
            }
            if !self.full_end_content.ends_with(&candidate.end_content) {
                return false;
            }
        }
        true
    }
}

/// Matches all `candidates` against all `matches`, potentially in parallel.
/// Returns an array of length of `candidates`, where each element is an
/// array of all indices (in `matches`) of matches.
#[no_mangle]
pub extern "C" fn fragment_directive_filter_non_matching_candidates(
    candidates: &ThinVec<&TextDirectiveCandidateContents>,
    matches: &ThinVec<&TextDirectiveCandidateContents>,
    result: &mut ThinVec<ThinVec<usize>>,
) {
    // It's not possible (yet) to use `ThinVec` together with `rayon`.
    // Therefore it's necessary to collect into a `Vec<Vec<usize>>` first,
    // and then get a new (non-rayon) iterator to be able
    // to collect into `ThinVec<ThinVec<usize>>`.
    result.extend(
        candidates
            .par_iter()
            .map(|candidate| {
                matches
                    .iter()
                    .enumerate()
                    .filter(|(_, m)| m.matches_candidate(candidate))
                    .map(|(index, _)| index)
                    .collect::<ThinVec<_>>()
            })
            .collect::<Vec<_>>()
            .into_iter(),
    );
}
