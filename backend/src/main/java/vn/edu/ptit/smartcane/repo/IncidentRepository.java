package vn.edu.ptit.smartcane.repo;
import vn.edu.ptit.smartcane.model.IncidentRecord;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.domain.Pageable;
import java.util.List;
public interface IncidentRepository extends JpaRepository<IncidentRecord,Long> {
 boolean existsByEventId(String eventId);
 List<IncidentRecord> findByDeviceIdOrderByReceivedAtDesc(String deviceId,Pageable page);
}
