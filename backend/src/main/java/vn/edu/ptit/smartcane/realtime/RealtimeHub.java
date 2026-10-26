package vn.edu.ptit.smartcane.realtime;

import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import org.springframework.data.domain.PageRequest;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;
import org.springframework.web.servlet.mvc.method.annotation.SseEmitter;

import vn.edu.ptit.smartcane.model.TelemetryRecord;
import vn.edu.ptit.smartcane.repo.TelemetryRepository;

@Component
public class RealtimeHub {
    private static final Duration ONLINE_THRESHOLD = Duration.ofSeconds(15);
    private final Map<String, CopyOnWriteArrayList<SseEmitter>> clients = new ConcurrentHashMap<>();
    private final Map<String, Boolean> knownOnline = new ConcurrentHashMap<>();
    private final TelemetryRepository telemetry;

    public RealtimeHub(TelemetryRepository telemetry) {
        this.telemetry = telemetry;
    }

    public SseEmitter subscribe(String deviceId) {
        SseEmitter emitter = new SseEmitter(30L * 60 * 1000); // 30 minutes, client automatically reconnects
        clients.computeIfAbsent(deviceId, ignored -> new CopyOnWriteArrayList<>()).add(emitter);
        emitter.onCompletion(() -> remove(deviceId, emitter));
        emitter.onTimeout(() -> remove(deviceId, emitter));
        emitter.onError(error -> remove(deviceId, emitter));
        try {
            emitter.send(SseEmitter.event().name("connected")
                    .data(Map.of("device_id", deviceId, "connected", true)).reconnectTime(3000));
            // Initial state allows subscribers to render even when the device is offline.
            send(emitter, "status", status(deviceId));
        } catch (Exception e) {
            remove(deviceId, emitter);
            emitter.complete();
        }
        return emitter;
    }

    public void broadcast(String deviceId, String type, Object payload) {
        List<SseEmitter> subscribers = clients.get(deviceId);
        if (subscribers == null) return;
        for (SseEmitter emitter : subscribers) send(emitter, type, payload);
    }

    private void send(SseEmitter emitter, String name, Object payload) {
        try {
            synchronized (emitter) { emitter.send(SseEmitter.event().name(name).data(payload)); }
        } catch (IOException | IllegalStateException ex) {
            clients.forEach((deviceId, subscribers) -> subscribers.remove(emitter));
            try { emitter.complete(); } catch (Exception ignored) { }
        }
    }

    private void remove(String deviceId, SseEmitter emitter) {
        List<SseEmitter> subscribers = clients.get(deviceId);
        if (subscribers != null) {
            subscribers.remove(emitter);
            if (subscribers.isEmpty()) { clients.remove(deviceId, subscribers); knownOnline.remove(deviceId); }
        }
    }

    private Map<String, Object> status(String deviceId) {
        List<TelemetryRecord> recent = telemetry.findByDeviceIdOrderByReceivedAtDesc(
                deviceId, PageRequest.of(0, 1));
        Instant lastSeen = recent.isEmpty() ? null : recent.get(0).receivedAt;
        boolean online = lastSeen != null && lastSeen.isAfter(Instant.now().minus(ONLINE_THRESHOLD));
        java.util.Map<String, Object> data = new java.util.LinkedHashMap<>();
        data.put("device_id", deviceId);
        data.put("online", online);
        data.put("last_seen", lastSeen);
        data.put("status_source", "telemetry_freshness");
        return data;
    }

    @Scheduled(fixedDelay = 5000)
    public void pushStatusAndHeartbeat() {
        // Only query for devices with active SSE subscribers.
        Set<String> devices = Set.copyOf(clients.keySet());
        for (String deviceId : devices) {
            Map<String, Object> data = status(deviceId);
            boolean online = Boolean.TRUE.equals(data.get("online"));
            Boolean previous = knownOnline.put(deviceId, online);
            if (previous == null || previous != online) broadcast(deviceId, "status", data);
            broadcast(deviceId, "heartbeat", Map.of("time", Instant.now().toString()));
        }
    }
}
