const std = @import("std");

const TestData = struct {
    year_a: u64, year_b: u64, expected_result: u64
};

/// Helper function to populate the array as Zig doesn't seem to support assigning directly in array
///
fn createTestData(a: u64, b: u64, expected_result: u64) TestData {
    return TestData{ .year_a = a, .year_b = b, .expected_result = expected_result };
}

/// Reddit Daily Challenge #376
/// https://www.reddit.com/r/dailyprogrammer/comments/b0nuoh/20190313_challenge_376_intermediate_the_revised/
///
/// Given two positive year numbers (with the second one greater than or equal to the first),
/// find out how many leap days (Feb 29ths) appear between Jan 1 of the first year, and Jan 1 of the second year in the Revised Julian Calendar.
/// This is equivalent to asking how many leap years there are in the interval between the two years, including the first but excluding the second.
///
pub fn main() !void {
    const tests = [_]TestData{
        createTestData(2016, 2017, 1),
        createTestData(2019, 2020, 0),
        createTestData(1900, 1901, 0),
        createTestData(2000, 2001, 1),
        createTestData(2800, 2801, 0),
        createTestData(123456, 123456, 0),
        createTestData(1234, 5678, 1077),
        createTestData(123456, 7891011, 1881475),
        createTestData(123456789101112, 1314151617181920, 288412747246240),
    };

    const stdout = std.io.getStdOut().outStream();
    for (tests) |t| {
        const leap_count = leaps(t.year_a, t.year_b);
        if (leap_count != t.expected_result)
            try stdout.print("Failed '{}' => '{}'. Expected: {} Actual: {}\n", .{ t.year_a, t.year_b, t.expected_result, leap_count });
    }
    try stdout.print("Completed {} Tests\n", .{tests.len});
}

/// The Revised Julian Calendar is a calendar system very similar to the familiar Gregorian Calendar, but slightly more accurate in terms of average year length. The Revised Julian Calendar has a leap day on Feb 29th of leap years as follows:
///  Years that are evenly divisible by 4 are leap years.
///  Exception: Years that are evenly divisible by 100 are not leap years.
///  Exception to the exception: Years for which the remainder when divided by 900 is either 200 or 600 are leap years.
/// For instance, 2000 is an exception to the exception: the remainder when dividing 2000 by 900 is 200. So 2000 is a leap year in the Revised Julian Calendar
///
fn leaps(year_a: u64, year_b: u64) u64 {
    return totalLeaps(year_b) - totalLeaps(year_a);
}

/// Calculate the number of leap years to the given year starting at zero
///
inline fn totalLeaps(year: u64) u64 {
    const y = year - 1;
    return y / 4 - y / 100 + (y - 200) / 900 + (y - 600) / 900;
}
