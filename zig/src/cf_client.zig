const std = @import("std");
const log = std.log.scoped(.cf_client);

pub const CFStatusMetric = struct {
    status: u16,
    count: u64,
};

const CFResponse = struct {
    data: struct {
        viewer: struct {
            zones: []struct {
                httpRequestsAdaptiveGroups: []struct {
                    count: u64,
                    dimensions: struct {
                        edgeResponseStatus: u16,
                    },
                },
            },
        },
    },
};

pub const CFFetchOptions = struct {
    zone_id: []const u8,
    start_date: []const u8,
    end_date: []const u8,
    limit: u32,
};

pub const CFClient = struct {
    allocator: std.mem.Allocator,
    http_client: std.http.Client,
    api_token: []const u8,

    const base_url = "https://api.cloudflare.com/client/v4/graphql";

    pub fn init(allocator: std.mem.Allocator, api_token: []const u8) CFClient {
        return .{
            .allocator = allocator,
            .http_client = .{ .allocator = allocator },
            .api_token = api_token,
        };
    }

    pub fn deinit(self: *CFClient) void {
        self.http_client.deinit();
    }

    pub fn fetchMetrics(self: *CFClient, opts: CFFetchOptions) ![]CFStatusMetric {
        // Build authorization header
        var auth_buf: [256]u8 = undefined;
        const auth_header = std.fmt.bufPrint(&auth_buf, "Bearer {s}", .{self.api_token}) catch return error.BufferTooSmall;

        // Build GraphQL query
        var payload_buf: [1024]u8 = undefined;
        const payload = std.fmt.bufPrint(&payload_buf,
            \\{{"query": "{{ viewer {{ zones(filter: {{zoneTag: \"{s}\"}}) {{ httpRequestsAdaptiveGroups(limit: {d}, filter: {{datetime_geq: \"{s}\", datetime_leq: \"{s}\"}}, orderBy: [count_DESC]) {{ count dimensions {{ edgeResponseStatus }} }}}}}}}}"}}
        , .{ opts.zone_id, opts.limit, opts.start_date, opts.end_date }) catch return error.BufferTooSmall;

        log.debug("Sending HTTP POST request to [{s}]", .{base_url});

        var response_buf: std.io.Writer.Allocating = .init(self.allocator);
        defer response_buf.deinit();

        const result = try self.http_client.fetch(.{
            .method = .POST,
            .location = .{ .url = base_url },
            .extra_headers = &.{
                .{ .name = "User-Agent", .value = "cf-stats-exporter/1.0" },
                .{ .name = "Content-Type", .value = "application/json" },
                .{ .name = "Authorization", .value = auth_header },
            },
            .payload = payload,
            .response_writer = &response_buf.writer,
        });

        if (result.status != .ok) {
            log.err("HTTP request failed with status {}", .{result.status});
            return error.HttpError;
        }

        const body = response_buf.written();
        log.debug("Response body: {s}", .{body});

        // Parse response
        const parsed = try std.json.parseFromSlice(
            CFResponse,
            self.allocator,
            body,
            .{ .ignore_unknown_fields = true },
        );
        defer parsed.deinit();

        // Convert to StatusMetric array
        const raw_groups = parsed.value.data.viewer.zones[0].httpRequestsAdaptiveGroups;
        var metrics: std.ArrayList(CFStatusMetric) = .empty;
        try metrics.ensureTotalCapacity(self.allocator, raw_groups.len);
        for (raw_groups) |g| {
            metrics.appendAssumeCapacity(.{
                .status = g.dimensions.edgeResponseStatus,
                .count = g.count,
            });
        }

        return try metrics.toOwnedSlice(self.allocator);
    }
};
