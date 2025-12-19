const std = @import("std");
const cf = @import("cf_client");

pub fn main() !void {
    var gpa: std.heap.GeneralPurposeAllocator(.{}) = .{};
    defer _ = gpa.deinit();

    const api_token = std.posix.getenv("CF_API_TOKEN") orelse {
        std.debug.print("CF_API_TOKEN environment variable is required\n", .{});
        std.process.exit(1);
    };

    const zone_id = std.posix.getenv("CF_ZONE_ID") orelse {
        std.debug.print("CF_ZONE_ID environment variable is required\n", .{});
        std.process.exit(1);
    };

    var client = cf.CFClient.init(gpa.allocator(), api_token);
    defer client.deinit();

    const metrics = try client.fetchMetrics(.{
        .zone_id = zone_id,
        .start_date = "2025-12-17T15:00:00Z",
        .end_date = "2025-12-18T15:00:00Z",
        .limit = 10,
    });
    defer gpa.allocator().free(metrics);

    std.debug.print("{f}\n", .{std.json.fmt(metrics, .{})});
}
