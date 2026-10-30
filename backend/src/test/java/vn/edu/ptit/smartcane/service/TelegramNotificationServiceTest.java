package vn.edu.ptit.smartcane.service;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.time.Instant;

import org.junit.jupiter.api.Test;

import vn.edu.ptit.smartcane.model.IncidentRecord;

class TelegramNotificationServiceTest {
    @Test
    void notifiesOnlyActionableEvents() {
        assertTrue(TelegramNotificationService.shouldNotify(
                new IncidentRecord("evt-sos", "cane-01", "SOS", Instant.now())));
        assertTrue(TelegramNotificationService.shouldNotify(
                new IncidentRecord("evt-alert", "cane-01", "CANE_FALL_ALERT", Instant.now())));
        assertFalse(TelegramNotificationService.shouldNotify(
                new IncidentRecord("evt-cancel", "cane-01", "CANCEL", Instant.now())));
        assertFalse(TelegramNotificationService.shouldNotify(
                new IncidentRecord("evt-suspected", "cane-01", "CANE_FALL_SUSPECTED", Instant.now())));
    }

    @Test
    void includesMapsLinkOnlyForValidGps() {
        IncidentRecord valid = new IncidentRecord("evt-1", "cane-01", "SOS", Instant.parse("2026-10-30T10:00:00Z"));
        valid.gpsValid = true;
        valid.latitude = 10.762622;
        valid.longitude = 106.660172;

        String validMessage = TelegramNotificationService.formatMessage(valid);
        assertTrue(validMessage.contains("https://maps.google.com/?q=10.762622,106.660172"));

        IncidentRecord missing = new IncidentRecord("evt-2", "cane-01", "SOS", Instant.parse("2026-10-30T10:00:00Z"));
        missing.gpsValid = false;
        String missingMessage = TelegramNotificationService.formatMessage(missing);
        assertTrue(missingMessage.contains("GPS: unavailable"));
        assertFalse(missingMessage.contains("maps.google.com"));
    }

    @Test
    void prefersDeviceTimestampWhenPresent() {
        IncidentRecord event = new IncidentRecord("evt-3", "cane-01", "CANE_FALL_ALERT", Instant.parse("2026-10-30T10:00:00Z"));
        event.deviceTimestamp = Instant.parse("2026-10-30T09:59:00Z");
        assertTrue(TelegramNotificationService.formatMessage(event).contains("2026-10-30T09:59:00Z"));
    }
}
