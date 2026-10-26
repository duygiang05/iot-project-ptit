package vn.edu.ptit.smartcane.service;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.core.JsonProcessingException;
import org.springframework.context.ApplicationEventPublisher;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import vn.edu.ptit.smartcane.model.*;
import vn.edu.ptit.smartcane.repo.*;
import vn.edu.ptit.smartcane.realtime.RealtimeSaved;
import java.time.Instant;
import java.util.Set;

@Service
public class MessageService {
 private static final Set<String> EVENT_TYPES=Set.of("SOS","CANE_FALL_SUSPECTED","CANE_FALL_ALERT","CANCEL");
 private final TelemetryRepository telemetry; private final IncidentRepository incidents;
 private final ObjectMapper json;
 private final ApplicationEventPublisher publisher;
 public MessageService(TelemetryRepository t,IncidentRepository i,ObjectMapper j,ApplicationEventPublisher p){telemetry=t;incidents=i;json=j;publisher=p;}
 static String stringRequired(JsonNode node,String key){
   JsonNode v=node.get(key);
   if(v==null || !v.isTextual() || v.asText().isBlank() || v.asText().length()>120)throw new IllegalArgumentException("Invalid field: "+key);
   return v.asText();
 }
 static Instant optionalTimestamp(JsonNode node){
   JsonNode v=node.get("timestamp");
   if(v==null || v.isNull()) return null;
   if(!v.isTextual())throw new IllegalArgumentException("timestamp must be ISO-8601 string");
   try{return Instant.parse(v.asText());}catch(Exception e){throw new IllegalArgumentException("Invalid timestamp");}
 }
 static Double optionalDouble(JsonNode n,String k,Double min, Double max){
   JsonNode v=n.get(k);if(v==null||v.isNull())return null;
   if(!v.isNumber()||!Double.isFinite(v.asDouble())||v.asDouble()<min||v.asDouble()>max)throw new IllegalArgumentException("Invalid "+k);
   return v.asDouble();
 }
 static Boolean optionalBool(JsonNode n,String k){
   JsonNode v=n.get(k);if(v==null||v.isNull())return null;
   if(!v.isBoolean())throw new IllegalArgumentException("Invalid "+k);
   return v.asBoolean();
 }
 static Integer optionalInt(JsonNode n,String k){
   JsonNode v=n.get(k);if(v==null||v.isNull())return null;
   if(!v.canConvertToInt()||v.asInt()<0||v.asInt()>4095)throw new IllegalArgumentException("Invalid "+k);
   return v.asInt();
 }
 private JsonNode parse(String payload){try{JsonNode n=json.readTree(payload);if(!n.isObject())throw new IllegalArgumentException("Expected JSON object");return n;}catch(JsonProcessingException ex){throw new IllegalArgumentException("Malformed JSON",ex);}}
 private String validateTopic(JsonNode n,String topic,String suffix){
   String id=stringRequired(n,"device_id");
   if(!id.matches("[A-Za-z0-9_-]{1,64}")||!topic.equals("smartcane/"+id+"/"+suffix))throw new IllegalArgumentException("device_id and topic mismatch");
   return id;
 }
 @Transactional
 public void process(String topic,String payload){
  JsonNode n=parse(payload);
  if(topic.endsWith("/telemetry")){
   String id=validateTopic(n,topic,"telemetry");
   TelemetryRecord t=new TelemetryRecord(id,Instant.now());
   t.deviceTimestamp=optionalTimestamp(n);
   t.distanceCm=optionalDouble(n,"distance_cm",0.0,1000.0);
   t.latitude=optionalDouble(n,"latitude",-90.0,90.0);t.longitude=optionalDouble(n,"longitude",-180.0,180.0);
   t.gpsValid=optionalBool(n,"gps_valid");t.waterDetected=optionalBool(n,"water_detected");
   t.ledOn=optionalBool(n,"led_on");t.lightLevel=optionalInt(n,"light_level");
   t.batteryVoltage=optionalDouble(n,"battery_voltage",0.0,5.0);
   if(Boolean.TRUE.equals(t.gpsValid)&&(t.latitude==null||t.longitude==null))throw new IllegalArgumentException("gps_valid=true requires coordinates");
   telemetry.save(t);
   publisher.publishEvent(new RealtimeSaved(id,"telemetry",t));
  }else if(topic.endsWith("/events")){
   String id=validateTopic(n,topic,"events");
   String eventId=stringRequired(n,"event_id");String type=stringRequired(n,"type");
   if(!EVENT_TYPES.contains(type))throw new IllegalArgumentException("Unknown event type");
   if(incidents.existsByEventId(eventId))return;
   IncidentRecord e=new IncidentRecord(eventId,id,type,Instant.now());e.deviceTimestamp=optionalTimestamp(n);
   e.latitude=optionalDouble(n,"latitude",-90.0,90.0);e.longitude=optionalDouble(n,"longitude",-180.0,180.0);e.gpsValid=optionalBool(n,"gps_valid");
   if(Boolean.TRUE.equals(e.gpsValid)&&(e.latitude==null||e.longitude==null))throw new IllegalArgumentException("gps_valid=true requires coordinates");
   incidents.save(e);
   publisher.publishEvent(new RealtimeSaved(id,"incident",e));
   // Telegram notification / durable outbox belongs to a separate integration task.
  }else throw new IllegalArgumentException("Unsupported topic");
 }
}
