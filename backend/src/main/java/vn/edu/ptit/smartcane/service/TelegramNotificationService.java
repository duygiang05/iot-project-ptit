package vn.edu.ptit.smartcane.service;

import java.net.URI;
import java.net.URLEncoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.time.Instant;
import java.time.format.DateTimeFormatter;
import java.util.Locale;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;
import org.springframework.scheduling.annotation.Async;
import org.springframework.transaction.event.TransactionPhase;
import org.springframework.transaction.event.TransactionalEventListener;

import vn.edu.ptit.smartcane.model.IncidentRecord;
import vn.edu.ptit.smartcane.realtime.RealtimeSaved;

/** Sends Telegram alerts after an incident has committed to the database. */
@Component
public class TelegramNotificationService {
    private final String botToken;
    private final String chatId;
    private final HttpClient http = HttpClient.newBuilder()
            .connectTimeout(Duration.ofSeconds(5))
            .build();

    public TelegramNotificationService(
            @Value("${smartcane.telegram.bot-token:}") String botToken,
            @Value("${smartcane.telegram.chat-id:}") String chatId) {
        this.botToken = botToken;
        this.chatId = chatId;
    }

    @TransactionalEventListener(phase = TransactionPhase.AFTER_COMMIT)
    @Async("telegramExecutor")
    public void onSaved(RealtimeSaved saved) {
        if (!"incident".equals(saved.type()) || !(saved.data() instanceof IncidentRecord incident)) return;
        if (!shouldNotify(incident)) return;
        if (botToken.isBlank() || chatId.isBlank()) {
            System.out.println("[TELEGRAM] not configured; skipped event_id=" + incident.eventId);
            return;
        }
        send(incident);
    }

    static boolean shouldNotify(IncidentRecord event) {
        return "SOS".equals(event.type) || "CANE_FALL_ALERT".equals(event.type);
    }

    static String formatMessage(IncidentRecord event) {
        String title = "SOS".equals(event.type) ? "SOS ALERT" : "SUSPECTED CANE FALL ALERT";
        Instant occurredAt = event.deviceTimestamp != null ? event.deviceTimestamp : event.receivedAt;
        StringBuilder message = new StringBuilder()
                .append(title)
                .append("\nDevice: ").append(event.deviceId)
                .append("\nEvent: ").append(event.eventId)
                .append("\nTime (UTC): ").append(DateTimeFormatter.ISO_INSTANT.format(occurredAt));

        if (Boolean.TRUE.equals(event.gpsValid) && event.latitude != null && event.longitude != null) {
            message.append(String.format(Locale.ROOT,
                    "\nMap: https://maps.google.com/?q=%.6f,%.6f",
                    event.latitude, event.longitude));
        } else {
            message.append("\nGPS: unavailable");
        }
        return message.toString();
    }

    private void send(IncidentRecord event) {
        final Instant started = Instant.now();
        final String body = "chat_id=" + encode(chatId) + "&text=" + encode(formatMessage(event));
        final HttpRequest request = HttpRequest.newBuilder()
                .uri(URI.create("https://api.telegram.org/bot" + botToken + "/sendMessage"))
                .timeout(Duration.ofSeconds(10))
                .header("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8")
                .POST(HttpRequest.BodyPublishers.ofString(body))
                .build();
        try {
            HttpResponse<String> response = http.send(request, HttpResponse.BodyHandlers.ofString());
            long latencyMs = Duration.between(event.receivedAt, Instant.now()).toMillis();
            if (response.statusCode() >= 200 && response.statusCode() < 300 &&
                    response.body() != null && response.body().contains("\"ok\":true")) {
                System.out.printf("[TELEGRAM] sent event_id=%s end_to_end_ms=%d http_ms=%d%n",
                        event.eventId, latencyMs, Duration.between(started, Instant.now()).toMillis());
            } else {
                System.err.printf("[TELEGRAM] rejected event_id=%s status=%d%n",
                        event.eventId, response.statusCode());
            }
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
            System.err.println("[TELEGRAM] interrupted event_id=" + event.eventId);
        } catch (Exception ex) {
            // Never print request URI or exception details: the URI contains the bot token.
            System.err.printf("[TELEGRAM] failed event_id=%s error=%s%n",
                    event.eventId, ex.getClass().getSimpleName());
        }
    }

    private static String encode(String value) {
        return URLEncoder.encode(value, StandardCharsets.UTF_8);
    }
}
