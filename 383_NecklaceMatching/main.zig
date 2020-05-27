const std = @import("std");

const TestData = struct {
    a: []const u8, b: []const u8, expected_result: bool
};

const TestContext = struct {
    data: []const TestData, results: []bool
};

const tests = [_]TestData{
    create_test_data("aab", "aa", false),
    create_test_data("hello", "hello", true),
    create_test_data("hello", "lohel", true),
    create_test_data("hollo", "lohel", false),
    create_test_data("glockenspiel", "spielglocken", true),
    create_test_data("glockenspiel", "glockenspien", false),
    create_test_data("keyboard", "eyboardk", true),
    create_test_data("nicole", "icolen", true),
    create_test_data("nicole", "lenico", true),
    create_test_data("nicole", "coneli", false),
    create_test_data("aabaaaaabaab", "aabaabaabaaa", true),
    create_test_data("abc", "cba", false),
    create_test_data("xxyyy", "xxxyy", false),
    create_test_data("xyxxz", "xxyxz", false),
    create_test_data("x", "x", true),
    create_test_data("x", "xx", false),
    create_test_data("x", "", false),
    create_test_data("", "", true),
    create_test_data("scott", "ttocs", false),
};

const MAX_WORKING_MEM_LENGTH = 25;

const AllocationError = error{OutOfMemory};

/// Helper function to populate the array as Zig doesn't seem to support assigning directly in array
///
fn create_test_data(a: []const u8, b: []const u8, expected_result: bool) TestData {
    return TestData{ .a = a, .b = b, .expected_result = expected_result };
}

/// Reddit Daily Challenge #383
/// https://www.reddit.com/r/dailyprogrammer/comments/ffxabb/20200309_challenge_383_easy_necklace_matching/
///
pub fn main() !void {
    //Splitting the tests across threads just to test how threading works in Zig
    var test_results: [tests.len]bool = undefined;

    const batch_size = tests.len / 2;
    var ctx1 = TestContext{ .data = tests[0..batch_size], .results = test_results[0..batch_size] };
    var ctx2 = TestContext{ .data = tests[batch_size..tests.len], .results = test_results[batch_size..tests.len] };

    const thread1 = try std.Thread.spawn(@as(TestContext, ctx1), run_test_batch);
    const thread2 = try std.Thread.spawn(@as(TestContext, ctx2), run_test_batch);

    thread1.wait();
    thread2.wait();

    const stdout = std.io.getStdOut().outStream();
    for (tests) |t, i| {
        if (test_results[i] != t.expected_result)
            try stdout.print("Failed '{}' => '{}'. Expected: {} Actual: {}\n", .{ t.a, t.b, t.expected_result, test_results[i] });
    }
    try stdout.print("Completed {} Tests\n", .{tests.len});
}

/// Runs a subset of the tests (used for multithreading)
///
fn run_test_batch(ctx: TestContext) void {
    var working_memory: [MAX_WORKING_MEM_LENGTH]u8 = undefined;
    var working_slice = working_memory[0..working_memory.len];

    for (ctx.data) |t, i| {
        const result = is_rotation(t.a, t.b, working_slice) catch false;
        ctx.results[i] = result;
    }
}

/// Simple algorithm that duplicates 'A' and searches for 'B' as a substring.
/// The substring algorithm is just a naive search and nothing fancier like KMP, etc
///
/// Zig prefers stack allocation so we need to pass in enough memory to expand 'a'
///
fn is_rotation(a: []const u8, b: []const u8, working_memory: []u8) !bool {
    if (a.len * 2 >= working_memory.len)
        return AllocationError.OutOfMemory;

    if (a.len != b.len)
        return false;

    if (a.len == 0 and b.len == 0)
        return true;

    //Duplicate 'a' e.g "hello" => "hellohello" so that 'b' is now a potential substring of 'a'
    for (a) |char, i| {
        working_memory[i] = char;
        working_memory[i + a.len] = char;
    }

    const m = b.len;
    const n = a.len * 2;

    var i: usize = 0;
    return while (i < n) : (i += 1) {
        var j: usize = 0;
        while (j < m) : (j += 1) {
            if (working_memory[i + j] != b[j])
                break;
        }

        if (j == m)
            break true;
    } else false;
}
