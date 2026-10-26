package vn.edu.ptit.smartcane.service;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.ArgumentCaptor;
import vn.edu.ptit.smartcane.model.IncidentRecord;
import vn.edu.ptit.smartcane.model.TelemetryRecord;
import vn.edu.ptit.smartcane.repo.IncidentRepository;
import vn.edu.ptit.smartcane.repo.TelemetryRepository;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.*;

class MessageServiceTest {
  TelemetryRepository telemetry;
  IncidentRepository incidents;
  MessageService service;

  @BeforeEach void setup() {
    telemetry = mock(TelemetryRepository.class);
    incidents = mock(IncidentRepository.class);
    service = new MessageService(
        telemetry,
        incidents,
        new ObjectMapper(),
        event -> {}
    );
  }

  @Test void validTelemetryIsPersisted() {
    service.process("smartcane/cane-01/telemetry", """
        {"device_id":"cane-01","distance_cm":43.5,"gps_valid":false,
         "water_detected":false,"light_level":1200,"led_on":true,"battery_voltage":null}
        """);
    var capture = ArgumentCaptor.forClass(TelemetryRecord.class);
    verify(telemetry).save(capture.capture());
    assertEquals("cane-01", capture.getValue().deviceId);
    assertEquals(43.5, capture.getValue().distanceCm);
    assertNull(capture.getValue().batteryVoltage);
  }

  @Test void wrongDeviceTopicRejected() {
    assertThrows(IllegalArgumentException.class, () ->
        service.process("smartcane/other/telemetry", "{\"device_id\":\"cane-01\"}"));
    verifyNoInteractions(telemetry);
  }

  @Test void malformedJsonRejected() {
    assertThrows(IllegalArgumentException.class, () ->
        service.process("smartcane/cane-01/telemetry", "{no-json"));
    verifyNoInteractions(telemetry);
  }

  @Test void badGpsCannotBeStored() {
    assertThrows(IllegalArgumentException.class, () ->
        service.process("smartcane/cane-01/telemetry", "{\"device_id\":\"cane-01\",\"gps_valid\":true}"));
    verifyNoInteractions(telemetry);
  }

  @Test void validSosIsStored() {
    service.process("smartcane/cane-01/events", """
        {"event_id":"cane-01-1","device_id":"cane-01","type":"SOS","gps_valid":false}
        """);
    var capture = ArgumentCaptor.forClass(IncidentRecord.class);
    verify(incidents).save(capture.capture());
    assertEquals("SOS", capture.getValue().type);
    assertEquals("cane-01-1", capture.getValue().eventId);
  }

  @Test void duplicatedEventIsIgnored() {
    when(incidents.existsByEventId("evt-1")).thenReturn(true);
    service.process("smartcane/cane-01/events", """
        {"device_id":"cane-01","event_id":"evt-1","type":"SOS"}
        """);
    verify(incidents, never()).save(any());
  }

  @Test void unknownEventTypeRejected() {
    assertThrows(IllegalArgumentException.class, () ->
        service.process("smartcane/cane-01/events", """
            {"device_id":"cane-01","event_id":"evt-3","type":"USER_FALL"}
            """));
  }
}
