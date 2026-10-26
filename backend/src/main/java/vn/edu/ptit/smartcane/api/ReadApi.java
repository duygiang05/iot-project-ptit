package vn.edu.ptit.smartcane.api;

import java.time.Duration;
import java.time.Instant;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.springframework.data.domain.PageRequest;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.server.ResponseStatusException;

import vn.edu.ptit.smartcane.model.IncidentRecord;
import vn.edu.ptit.smartcane.model.TelemetryRecord;
import vn.edu.ptit.smartcane.repo.IncidentRepository;
import vn.edu.ptit.smartcane.repo.TelemetryRepository;

@RestController
@RequestMapping("/api/devices")
@CrossOrigin(origins = {"http://localhost:5500", "http://127.0.0.1:5500"})
public class ReadApi {

    private static final int MAX_LIMIT = 500;
    private static final Duration ONLINE_THRESHOLD = Duration.ofSeconds(15);

    private final TelemetryRepository telemetry;
    private final IncidentRepository incidents;

    public ReadApi(TelemetryRepository telemetry, IncidentRepository incidents) {
        this.telemetry = telemetry;
        this.incidents = incidents;
    }

    // Return a single object (not a one-element array) or HTTP 404.
    @GetMapping("/{id}/telemetry/latest")
    public ResponseEntity<TelemetryRecord> latest(@PathVariable String id) {
        List<TelemetryRecord> rows = telemetry.findByDeviceIdOrderByReceivedAtDesc(
                id, PageRequest.of(0, 1));
        return rows.isEmpty() ? ResponseEntity.notFound().build() : ResponseEntity.ok(rows.get(0));
    }

    // Both dates use ISO 8601, e.g. 2026-10-26T00:00:00Z.
    @GetMapping("/{id}/telemetry")
    public List<TelemetryRecord> history(
            @PathVariable String id,
            @RequestParam Instant from,
            @RequestParam Instant to,
            @RequestParam(defaultValue = "100") int limit) {
        if (from.isAfter(to)) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "from must not be after to");
        }
        return telemetry.findByDeviceIdAndReceivedAtBetweenOrderByReceivedAtDesc(
                id, from, to, PageRequest.of(0, bounded(limit, MAX_LIMIT)));
    }

    @GetMapping("/{id}/incidents")
    public List<IncidentRecord> incidents(
            @PathVariable String id,
            @RequestParam(defaultValue = "20") int limit) {
        return incidents.findByDeviceIdOrderByReceivedAtDesc(
                id, PageRequest.of(0, bounded(limit, 100)));
    }

    // A telemetry-age heuristic, NOT an MQTT Last Will based online status.
    @GetMapping("/{id}/status")
    public Map<String, Object> status(@PathVariable String id) {
        List<TelemetryRecord> rows = telemetry.findByDeviceIdOrderByReceivedAtDesc(
                id, PageRequest.of(0, 1));
        Instant lastSeen = rows.isEmpty() ? null : rows.get(0).receivedAt;
        boolean online = lastSeen != null
                && lastSeen.isAfter(Instant.now().minus(ONLINE_THRESHOLD));
        Map<String, Object> response = new LinkedHashMap<>();
        response.put("device_id", id);
        response.put("online", online);
        response.put("last_seen", lastSeen);
        response.put("status_source", "telemetry_freshness");
        return response;
    }

    private int bounded(int limit, int max) {
        if (limit < 1) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "limit must be >= 1");
        }
        return Math.min(limit, max);
    }
}
