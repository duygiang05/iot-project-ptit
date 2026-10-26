package vn.edu.ptit.smartcane.api;
import org.springframework.data.domain.PageRequest;
import org.springframework.web.bind.annotation.*;
import vn.edu.ptit.smartcane.repo.*;
import vn.edu.ptit.smartcane.model.*;
import java.util.List;
@RestController @RequestMapping("/api/devices")
@CrossOrigin(origins={"http://localhost:5500","http://127.0.0.1:5500"})
public class ReadApi {
 private final TelemetryRepository telemetry;private final IncidentRepository incidents;
 public ReadApi(TelemetryRepository t,IncidentRepository i){telemetry=t;incidents=i;}
 @GetMapping("/{id}/telemetry/latest") public List<TelemetryRecord> telemetry(@PathVariable String id){return telemetry.findByDeviceIdOrderByReceivedAtDesc(id,PageRequest.of(0,1));}
 @GetMapping("/{id}/incidents") public List<IncidentRecord> incidents(@PathVariable String id,@RequestParam(defaultValue="20") int limit){return incidents.findByDeviceIdOrderByReceivedAtDesc(id,PageRequest.of(0,Math.max(1,Math.min(limit,100))));}
}
