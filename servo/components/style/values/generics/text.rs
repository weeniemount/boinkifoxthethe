/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

//! Generic types for text properties.

use crate::Zero;
use std::fmt::{self, Write};
use style_traits::{CssWriter, ToCss};

/// A generic value that is either a number or `auto`.
#[derive(
    Animate,
    Clone,
    ComputeSquaredDistance,
    Copy,
    Debug,
    MallocSizeOf,
    Parse,
    PartialEq,
    SpecifiedValueInfo,
    ToAnimatedValue,
    ToAnimatedZero,
    ToComputedValue,
    ToCss,
    ToResolvedValue,
    ToShmem,
)]
#[repr(C, u8)]
pub enum NumberOrAuto<N> {
    /// `auto`
    Auto,
    /// `<number>`
    Number(N),
}

/// A generic value for the `hyphenate-limit-chars` property.
#[derive(
    Animate,
    Clone,
    ComputeSquaredDistance,
    Debug,
    MallocSizeOf,
    PartialEq,
    SpecifiedValueInfo,
    ToAnimatedValue,
    ToAnimatedZero,
    ToComputedValue,
    ToResolvedValue,
    ToShmem,
)]
#[repr(C)]
pub struct GenericHyphenateLimitChars<Integer> {
    /// Required minimum number of characters in a hyphenated word.
    pub total_word_length: NumberOrAuto<Integer>,
    /// Required minumum number of characters before the hyphen.
    pub pre_hyphen_length: NumberOrAuto<Integer>,
    /// Required minumum number of characters after the hyphen.
    pub post_hyphen_length: NumberOrAuto<Integer>,
}

impl<Integer: ToCss + PartialEq> ToCss for GenericHyphenateLimitChars<Integer> {
    fn to_css<W>(&self, dest: &mut CssWriter<W>) -> fmt::Result
    where
        W: Write,
    {
        self.total_word_length.to_css(dest)?;

        if self.pre_hyphen_length != NumberOrAuto::Auto ||
           self.post_hyphen_length != self.pre_hyphen_length {
            dest.write_char(' ')?;
            self.pre_hyphen_length.to_css(dest)?;
            if self.post_hyphen_length != self.pre_hyphen_length {
                dest.write_char(' ')?;
                self.post_hyphen_length.to_css(dest)?;
            }
        }

        Ok(())
    }
}

/// A generic value for the `initial-letter` property.
#[derive(
    Clone,
    Copy,
    Debug,
    MallocSizeOf,
    PartialEq,
    SpecifiedValueInfo,
    ToComputedValue,
    ToResolvedValue,
    ToShmem,
)]
#[repr(C)]
pub struct GenericInitialLetter<Number, Integer> {
    /// The size, >=1, or 0 if `normal`.
    pub size: Number,
    /// The sink, >=1, if specified, 0 otherwise.
    pub sink: Integer,
}

pub use self::GenericInitialLetter as InitialLetter;
impl<N: Zero, I: Zero> InitialLetter<N, I> {
    /// Returns `normal`.
    #[inline]
    pub fn normal() -> Self {
        InitialLetter {
            size: N::zero(),
            sink: I::zero(),
        }
    }
}

impl<N: ToCss + Zero, I: ToCss + Zero> ToCss for InitialLetter<N, I> {
    fn to_css<W>(&self, dest: &mut CssWriter<W>) -> fmt::Result
    where
        W: Write,
    {
        if self.size.is_zero() {
            return dest.write_str("normal");
        }
        self.size.to_css(dest)?;
        if !self.sink.is_zero() {
            dest.write_char(' ')?;
            self.sink.to_css(dest)?;
        }
        Ok(())
    }
}

/// Implements type for text-decoration-thickness
/// which takes the grammar of auto | from-font | <length> | <percentage>
///
/// https://drafts.csswg.org/css-text-decor-4/
#[repr(C, u8)]
#[cfg_attr(feature = "servo", derive(Deserialize, Serialize))]
#[derive(
    Animate,
    Clone,
    Copy,
    ComputeSquaredDistance,
    Debug,
    Eq,
    MallocSizeOf,
    Parse,
    PartialEq,
    SpecifiedValueInfo,
    ToAnimatedValue,
    ToAnimatedZero,
    ToComputedValue,
    ToCss,
    ToResolvedValue,
    ToShmem,
)]
#[allow(missing_docs)]
pub enum GenericTextDecorationLength<L> {
    LengthPercentage(L),
    Auto,
    FromFont,
}

/// Implements type for text-indent
/// which takes the grammar of [<length-percentage>] && hanging? && each-line?
///
/// https://drafts.csswg.org/css-text/#propdef-text-indent
#[repr(C)]
#[derive(
    Animate,
    Clone,
    ComputeSquaredDistance,
    Debug,
    Eq,
    MallocSizeOf,
    PartialEq,
    SpecifiedValueInfo,
    ToAnimatedValue,
    ToAnimatedZero,
    ToComputedValue,
    ToCss,
    ToResolvedValue,
    ToShmem,
)]
pub struct GenericTextIndent<LengthPercentage> {
    /// The amount of indent to be applied to the inline-start of the first line.
    pub length: LengthPercentage,
    /// Apply indent to non-first lines instead of first.
    #[animation(constant)]
    #[css(represents_keyword)]
    pub hanging: bool,
    /// Apply to each line after a hard break, not only first in block.
    #[animation(constant)]
    #[css(represents_keyword)]
    pub each_line: bool,
}

impl<LengthPercentage: Zero> GenericTextIndent<LengthPercentage> {
    /// Return the initial zero value.
    pub fn zero() -> Self {
        Self {
            length: LengthPercentage::zero(),
            hanging: false,
            each_line: false,
        }
    }
}
