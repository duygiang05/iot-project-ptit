package vn.edu.ptit.smartcane.repo;
import vn.edu.ptit.smartcane.model.TelemetryRecord;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.domain.Pageable;
import java.util.List;
public interface TelemetryRepository extends JpaRepository<TelemetryRecord,Long> {
 List<TelemetryRecord> findByDeviceIdOrderByReceivedAtDesc(String deviceId,Pageable page);
}
