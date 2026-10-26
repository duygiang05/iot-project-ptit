package vn.edu.ptit.smartcane.model;
import jakarta.persistence.*;
import java.time.Instant;
@Entity @Table(name="telemetry", indexes={@Index(name="idx_telemetry_device_received",columnList="deviceId,receivedAt")})
public class TelemetryRecord {
 @Id @GeneratedValue(strategy=GenerationType.IDENTITY) public Long id;
 @Column(nullable=false) public String deviceId;
 @Column(nullable=false) public Instant receivedAt;
 public Instant deviceTimestamp;
 public Double distanceCm, latitude, longitude, batteryVoltage;
 public Boolean gpsValid, waterDetected, ledOn;
 public Integer lightLevel;
 protected TelemetryRecord() {}
 public TelemetryRecord(String d,Instant r){deviceId=d;receivedAt=r;}
}
