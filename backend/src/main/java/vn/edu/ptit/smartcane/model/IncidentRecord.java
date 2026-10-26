package vn.edu.ptit.smartcane.model;
import jakarta.persistence.*;
import java.time.Instant;
@Entity @Table(name="incident",uniqueConstraints=@UniqueConstraint(name="uk_event_id",columnNames="eventId"))
public class IncidentRecord {
 @Id @GeneratedValue(strategy=GenerationType.IDENTITY) public Long id;
 @Column(nullable=false,unique=true) public String eventId;
 @Column(nullable=false) public String deviceId;
 @Column(nullable=false) public String type;
 @Column(nullable=false) public Instant receivedAt;
 public Instant deviceTimestamp;
 public Double latitude,longitude;
 public Boolean gpsValid;
 protected IncidentRecord() {}
 public IncidentRecord(String e,String d,String t,Instant r){eventId=e;deviceId=d;type=t;receivedAt=r;}
}
