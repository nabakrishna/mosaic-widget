// Unit tests for mosaic::widgets::activity_datetime -- the small typed
// grammar Special Activity uses to turn "tomorrow 6:00 pm" into a real
// absolute timestamp a reminder can be scheduled from (see
// ActivityDateTime.h's class comment for why this exists instead of a
// calendar picker). Pure string/time logic, no rendering, no D2D -- the
// second natural candidate for headless testing alongside LayoutEngine.

#include <gtest/gtest.h>
#include <ctime>
#include "widgets/ActivityDateTime.h"

using mosaic::widgets::activity_datetime::Parse;
using mosaic::widgets::activity_datetime::FormatForDisplay;
using mosaic::widgets::activity_datetime::FormatForEditing;

namespace {

std::tm LocalTimeOf(int64_t epochSeconds) {
    std::time_t t = static_cast<std::time_t>(epochSeconds);
    std::tm out{};
    localtime_s(&out, &t);
    return out;
}

} // namespace

// --- Parse: rejection of malformed input -----------------------------------

TEST(ActivityDateTimeParse, EmptyStringIsRejected) {
    EXPECT_FALSE(Parse(L"").has_value());
}

TEST(ActivityDateTimeParse, WhitespaceOnlyIsRejected) {
    EXPECT_FALSE(Parse(L"   \t\r\n  ").has_value());
}

TEST(ActivityDateTimeParse, GarbageTextIsRejected) {
    EXPECT_FALSE(Parse(L"whenever, idk").has_value());
}

TEST(ActivityDateTimeParse, TodayWithoutATimeIsRejected) {
    // "today" alone has no time-of-day component -- Parse requires a
    // schedulable HH:MM, not just a bare date keyword.
    EXPECT_FALSE(Parse(L"today").has_value());
}

TEST(ActivityDateTimeParse, WordBoundaryPreventsFalseKeywordMatch) {
    // "todayxyz" must not be treated as "today" with trailing garbage --
    // StartsWithWord requires the keyword to end at a word boundary.
    EXPECT_FALSE(Parse(L"todayxyz 6:00 pm").has_value());
}

TEST(ActivityDateTimeParse, MissingColonInTimeIsRejected) {
    EXPECT_FALSE(Parse(L"today 600 pm").has_value());
}

TEST(ActivityDateTimeParse, MinuteAbove59IsRejected) {
    EXPECT_FALSE(Parse(L"today 6:75").has_value());
}

TEST(ActivityDateTimeParse, TwelveHourClockRejectsHourAboveTwelve) {
    // "13:00 pm" is nonsensical in 12-hour notation.
    EXPECT_FALSE(Parse(L"today 13:00 pm").has_value());
}

TEST(ActivityDateTimeParse, TwelveHourClockRejectsHourZero) {
    EXPECT_FALSE(Parse(L"today 0:00 am").has_value());
}

TEST(ActivityDateTimeParse, TwentyFourHourClockRejectsHourAbove23) {
    EXPECT_FALSE(Parse(L"today 24:00").has_value());
}

TEST(ActivityDateTimeParse, UnrecognizedMeridiemTextIsRejected) {
    EXPECT_FALSE(Parse(L"today 6:00 xm").has_value());
}

TEST(ActivityDateTimeParse, IsoDateWithInvalidMonthIsRejected) {
    EXPECT_FALSE(Parse(L"2026-13-01 6:00 pm").has_value());
}

TEST(ActivityDateTimeParse, IsoDateWithInvalidDayIsRejected) {
    EXPECT_FALSE(Parse(L"2026-01-32 6:00 pm").has_value());
}

TEST(ActivityDateTimeParse, IsoDateMissingDashesIsRejected) {
    EXPECT_FALSE(Parse(L"20260918 6:00 pm").has_value());
}

// --- Parse: accepted grammar round-trips to the right wall-clock time ------

TEST(ActivityDateTimeParse, TodayWithPmTimeProducesCorrectHour) {
    auto result = Parse(L"today 6:00 pm");
    ASSERT_TRUE(result.has_value());
    std::tm parsed = LocalTimeOf(*result);
    EXPECT_EQ(parsed.tm_hour, 18);
    EXPECT_EQ(parsed.tm_min, 0);

    std::time_t now = std::time(nullptr);
    std::tm today{};
    localtime_s(&today, &now);
    EXPECT_EQ(parsed.tm_year, today.tm_year);
    EXPECT_EQ(parsed.tm_mon, today.tm_mon);
    EXPECT_EQ(parsed.tm_mday, today.tm_mday);
}

TEST(ActivityDateTimeParse, NoonPmIsTwelveHundredHours) {
    auto result = Parse(L"today 12:00 pm");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(LocalTimeOf(*result).tm_hour, 12);
}

TEST(ActivityDateTimeParse, MidnightAmIsZeroHundredHours) {
    auto result = Parse(L"today 12:00 am");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(LocalTimeOf(*result).tm_hour, 0);
}

TEST(ActivityDateTimeParse, TwentyFourHourTimeWithNoMeridiemIsAccepted) {
    auto result = Parse(L"today 18:30");
    ASSERT_TRUE(result.has_value());
    std::tm parsed = LocalTimeOf(*result);
    EXPECT_EQ(parsed.tm_hour, 18);
    EXPECT_EQ(parsed.tm_min, 30);
}

TEST(ActivityDateTimeParse, TomorrowAdvancesTheDateByOneDay) {
    auto todayResult = Parse(L"today 6:00 pm");
    auto tomorrowResult = Parse(L"tomorrow 6:00 pm");
    ASSERT_TRUE(todayResult.has_value());
    ASSERT_TRUE(tomorrowResult.has_value());

    // Exactly 24 hours apart -- mktime normalizes any month/year rollover
    // (e.g. running this test on the last day of the month), so a raw
    // epoch-seconds delta is the correct, DST-day-length caveat aside,
    // way to check this rather than comparing tm_mday fields directly.
    EXPECT_EQ(*tomorrowResult - *todayResult, 24 * 60 * 60);
}

TEST(ActivityDateTimeParse, IsoDateIsParsedExactly) {
    auto result = Parse(L"2026-09-18 11:59 pm");
    ASSERT_TRUE(result.has_value());
    std::tm parsed = LocalTimeOf(*result);
    EXPECT_EQ(parsed.tm_year + 1900, 2026);
    EXPECT_EQ(parsed.tm_mon + 1, 9);
    EXPECT_EQ(parsed.tm_mday, 18);
    EXPECT_EQ(parsed.tm_hour, 23);
    EXPECT_EQ(parsed.tm_min, 59);
}

TEST(ActivityDateTimeParse, ParsingIsCaseInsensitive) {
    auto lower = Parse(L"today 6:00 pm");
    auto upper = Parse(L"TODAY 6:00 PM");
    auto mixed = Parse(L"ToDaY 6:00 Pm");
    ASSERT_TRUE(lower.has_value());
    ASSERT_TRUE(upper.has_value());
    ASSERT_TRUE(mixed.has_value());
    EXPECT_EQ(*lower, *upper);
    EXPECT_EQ(*lower, *mixed);
}

TEST(ActivityDateTimeParse, ExtraWhitespaceIsTolerated) {
    auto tight = Parse(L"today 6:00 pm");
    auto loose = Parse(L"  today   6:00 pm  ");
    ASSERT_TRUE(tight.has_value());
    ASSERT_TRUE(loose.has_value());
    EXPECT_EQ(*tight, *loose);
}

// --- FormatForDisplay / FormatForEditing round-tripping --------------------

TEST(ActivityDateTimeFormat, TodayDisplaysWithTodayLabel) {
    auto parsed = Parse(L"today 6:00 pm");
    ASSERT_TRUE(parsed.has_value());
    std::wstring display = FormatForDisplay(*parsed);
    EXPECT_EQ(display, L"Today, 6:00 PM");
}

TEST(ActivityDateTimeFormat, TomorrowDisplaysWithTomorrowLabel) {
    auto parsed = Parse(L"tomorrow 6:00 pm");
    ASSERT_TRUE(parsed.has_value());
    std::wstring display = FormatForDisplay(*parsed);
    EXPECT_EQ(display, L"Tomorrow, 6:00 PM");
}

TEST(ActivityDateTimeFormat, FixedPastDateDisplaysWithMonthAndDay) {
    auto parsed = Parse(L"2020-01-15 11:59 pm");
    ASSERT_TRUE(parsed.has_value());
    std::wstring display = FormatForDisplay(*parsed);
    EXPECT_EQ(display, L"15 Jan, 11:59 PM");
}

TEST(ActivityDateTimeFormat, EditingFormatOfTodayStartsWithTodayKeyword) {
    auto parsed = Parse(L"today 6:00 pm");
    ASSERT_TRUE(parsed.has_value());
    std::wstring editing = FormatForEditing(*parsed);
    EXPECT_EQ(editing, L"today 6:00 pm");
}

TEST(ActivityDateTimeFormat, EditingFormatRoundTripsThroughParse) {
    // FormatForEditing's whole purpose is producing a string Parse
    // accepts unchanged (see ActivityDateTime.h) -- this is the actual
    // contract to test, not just a fixed string comparison.
    auto original = Parse(L"2020-01-15 11:59 pm");
    ASSERT_TRUE(original.has_value());

    std::wstring editing = FormatForEditing(*original);
    auto roundTripped = Parse(editing);

    ASSERT_TRUE(roundTripped.has_value());
    EXPECT_EQ(*original, *roundTripped);
}

TEST(ActivityDateTimeFormat, FixedPastDateEditingFormatUsesIsoGrammar) {
    auto parsed = Parse(L"2020-01-15 11:59 pm");
    ASSERT_TRUE(parsed.has_value());
    std::wstring editing = FormatForEditing(*parsed);
    EXPECT_EQ(editing, L"2020-01-15 11:59 pm");
}