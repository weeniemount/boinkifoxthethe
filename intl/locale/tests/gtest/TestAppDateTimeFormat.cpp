#include "gtest/gtest.h"
#include "mozilla/gtest/MozAssertions.h"
#include "mozilla/intl/AppDateTimeFormat.h"
#include "mozilla/intl/DateTimeFormat.h"

namespace mozilla::intl {
using Style = DateTimeFormat::Style;
using StyleBag = DateTimeFormat::StyleBag;
using ComponentsBag = DateTimeFormat::ComponentsBag;

static DateTimeFormat::StyleBag ToStyleBag(Maybe<DateTimeFormat::Style> date,
                                           Maybe<DateTimeFormat::Style> time) {
  DateTimeFormat::StyleBag style;
  style.date = date;
  style.time = time;
  return style;
}

TEST(AppDateTimeFormat, FormatPRExplodedTime)
{
  PRTime prTime = 0;
  PRExplodedTime prExplodedTime;
  PR_ExplodeTime(prTime, PR_GMTParameters, &prExplodedTime);

  AppDateTimeFormat::sLocale = new nsCString("en-US");
  AppDateTimeFormat::DeleteCache();
  StyleBag style = ToStyleBag(Some(Style::Long), Some(Style::Long));

  nsAutoString formattedTime;
  nsresult rv =
      AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"January") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1970") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"12:00:00 AM") != kNotFound ||
              formattedTime.Find(u"12:00:00\u202FAM") != kNotFound ||
              formattedTime.Find(u"00:00:00") != kNotFound);

  prExplodedTime = {0, 0, 19, 0, 1, 0, 1970, 4, 0, {(19 * 60), 0}};

  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);

  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"January") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1970") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"12:19:00 AM") != kNotFound ||
              formattedTime.Find(u"12:19:00\u202FAM") != kNotFound ||
              formattedTime.Find(u"00:19:00") != kNotFound);

  prExplodedTime = {0, 0,    0, 7, 1,
                    0, 1970, 4, 0, {(6 * 60 * 60), (1 * 60 * 60)}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"January") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1970") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"7:00:00 AM") != kNotFound ||
              formattedTime.Find(u"7:00:00\u202FAM") != kNotFound ||
              formattedTime.Find(u"07:00:00") != kNotFound);

  prExplodedTime = {
      0, 0,    29, 11, 1,
      0, 1970, 4,  0,  {(10 * 60 * 60) + (29 * 60), (1 * 60 * 60)}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"January") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1970") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"11:29:00 AM") != kNotFound ||
              formattedTime.Find(u"11:29:00\u202FAM") != kNotFound ||
              formattedTime.Find(u"11:29:00") != kNotFound);

  prExplodedTime = {0, 0, 37, 23, 31, 11, 1969, 3, 364, {-(23 * 60), 0}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"December") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"31") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1969") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"11:37:00 PM") != kNotFound ||
              formattedTime.Find(u"11:37:00\u202FPM") != kNotFound ||
              formattedTime.Find(u"23:37:00") != kNotFound);

  prExplodedTime = {0, 0, 0, 17, 31, 11, 1969, 3, 364, {-(7 * 60 * 60), 0}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"December") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"31") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1969") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"5:00:00 PM") != kNotFound ||
              formattedTime.Find(u"5:00:00\u202FPM") != kNotFound ||
              formattedTime.Find(u"17:00:00") != kNotFound);

  prExplodedTime = {
      0,  0,    47, 14,  31,
      11, 1969, 3,  364, {-((10 * 60 * 60) + (13 * 60)), (1 * 60 * 60)}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"December") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"31") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1969") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"2:47:00 PM") != kNotFound ||
              formattedTime.Find(u"2:47:00\u202FPM") != kNotFound ||
              formattedTime.Find(u"14:47:00") != kNotFound);

  ComponentsBag components{};
  components.weekday = mozilla::Some(DateTimeFormat::Text::Short);
  components.timeZoneName = mozilla::Some(DateTimeFormat::TimeZoneName::Short);
  // From above: Wed, 31 Dec 1969 14:47:00 -09:13
  rv = AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"Wed") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"-09:13") != kNotFound ||
              formattedTime.Find(u"-9:13") != kNotFound);
}

TEST(AppDateTimeFormat, DateFormatSelectors)
{
  PRTime prTime = 0;
  PRExplodedTime prExplodedTime;
  PR_ExplodeTime(prTime, PR_GMTParameters, &prExplodedTime);

  AppDateTimeFormat::sLocale = new nsCString("en-US");
  AppDateTimeFormat::DeleteCache();

  nsAutoString formattedTime;

  {
    ComponentsBag components{};
    components.year = Some(DateTimeFormat::Numeric::Numeric);
    components.month = Some(DateTimeFormat::Month::TwoDigit);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("01/1970", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
  {
    ComponentsBag components{};
    components.year = Some(DateTimeFormat::Numeric::Numeric);
    components.month = Some(DateTimeFormat::Month::Long);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("January 1970", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
  {
    ComponentsBag components{};
    components.month = Some(DateTimeFormat::Month::Long);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("January", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
  {
    ComponentsBag components{};
    components.weekday = Some(DateTimeFormat::Text::Short);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("Thu", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
}

TEST(AppDateTimeFormat, FormatPRExplodedTimeForeign)
{
  PRTime prTime = 0;
  PRExplodedTime prExplodedTime;
  PR_ExplodeTime(prTime, PR_GMTParameters, &prExplodedTime);

  AppDateTimeFormat::sLocale = new nsCString("de-DE");
  AppDateTimeFormat::DeleteCache();
  StyleBag style = ToStyleBag(Some(Style::Long), Some(Style::Long));

  nsAutoString formattedTime;
  nsresult rv =
      AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"1.") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"Januar") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1970") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"12:00:00 AM") != kNotFound ||
              formattedTime.Find(u"12:00:00\u202FAM") != kNotFound ||
              formattedTime.Find(u"00:00:00") != kNotFound);

  prExplodedTime = {0, 0, 19, 0, 1, 0, 1970, 4, 0, {(19 * 60), 0}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"1.") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"Januar") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1970") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"12:19:00 AM") != kNotFound ||
              formattedTime.Find(u"12:19:00\u202FAM") != kNotFound ||
              formattedTime.Find(u"00:19:00") != kNotFound);

  prExplodedTime = {0, 0,    0, 7, 1,
                    0, 1970, 4, 0, {(6 * 60 * 60), (1 * 60 * 60)}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"1.") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"Januar") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1970") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"7:00:00 AM") != kNotFound ||
              formattedTime.Find(u"7:00:00\u202FAM") != kNotFound ||
              formattedTime.Find(u"07:00:00") != kNotFound);

  prExplodedTime = {
      0, 0,    29, 11, 1,
      0, 1970, 4,  0,  {(10 * 60 * 60) + (29 * 60), (1 * 60 * 60)}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"1.") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"Januar") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1970") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"11:29:00 AM") != kNotFound ||
              formattedTime.Find(u"11:29:00\u202FAM") != kNotFound ||
              formattedTime.Find(u"11:29:00") != kNotFound);

  prExplodedTime = {0, 0, 37, 23, 31, 11, 1969, 3, 364, {-(23 * 60), 0}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"31.") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"Dezember") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1969") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"11:37:00 PM") != kNotFound ||
              formattedTime.Find(u"11:37:00\u202FPM") != kNotFound ||
              formattedTime.Find(u"23:37:00") != kNotFound);

  prExplodedTime = {0, 0, 0, 17, 31, 11, 1969, 3, 364, {-(7 * 60 * 60), 0}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"31.") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"Dezember") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1969") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"5:00:00 PM") != kNotFound ||
              formattedTime.Find(u"5:00:00\u202FPM") != kNotFound ||
              formattedTime.Find(u"17:00:00") != kNotFound);

  prExplodedTime = {
      0,  0,    47, 14,  31,
      11, 1969, 3,  364, {-((10 * 60 * 60) + (13 * 60)), (1 * 60 * 60)}};
  rv = AppDateTimeFormat::Format(style, &prExplodedTime, formattedTime);
  ASSERT_NS_SUCCEEDED(rv);
  ASSERT_TRUE(formattedTime.Find(u"31.") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"Dezember") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"1969") != kNotFound);
  ASSERT_TRUE(formattedTime.Find(u"2:47:00 PM") != kNotFound ||
              formattedTime.Find(u"2:47:00\u202FPM") != kNotFound ||
              formattedTime.Find(u"14:47:00") != kNotFound);
}

TEST(AppDateTimeFormat, DateFormatSelectorsForeign)
{
  PRTime prTime = 0;
  PRExplodedTime prExplodedTime;
  PR_ExplodeTime(prTime, PR_GMTParameters, &prExplodedTime);

  AppDateTimeFormat::sLocale = new nsCString("de-DE");
  AppDateTimeFormat::DeleteCache();

  nsAutoString formattedTime;
  {
    ComponentsBag components{};
    components.year = Some(DateTimeFormat::Numeric::Numeric);
    components.month = Some(DateTimeFormat::Month::TwoDigit);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("01.1970", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
  {
    ComponentsBag components{};
    components.year = Some(DateTimeFormat::Numeric::Numeric);
    components.month = Some(DateTimeFormat::Month::Long);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("Januar 1970", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
  {
    ComponentsBag components{};
    components.weekday = Some(DateTimeFormat::Text::Short);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("Do", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
  {
    ComponentsBag components{};
    components.weekday = Some(DateTimeFormat::Text::Long);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("Donnerstag", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
  {
    ComponentsBag components{};
    components.month = Some(DateTimeFormat::Month::Long);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("Januar", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
  {
    ComponentsBag components{};
    components.weekday = Some(DateTimeFormat::Text::Short);

    nsresult rv =
        AppDateTimeFormat::Format(components, &prExplodedTime, formattedTime);
    ASSERT_NS_SUCCEEDED(rv);
    ASSERT_STREQ("Do", NS_ConvertUTF16toUTF8(formattedTime).get());
  }
}

}  // namespace mozilla::intl
