package vn.edu.ptit.smartcane.repo;

import java.time.Instant;
import java.util.List;

import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

import vn.edu.ptit.smartcane.model.TelemetryRecord;

public interface TelemetryRepository extends JpaRepository<TelemetryRecord, Long> {
    List<TelemetryRecord> findByDeviceIdOrderByReceivedAtDesc(String deviceId, Pageable pageable);

    List<TelemetryRecord> findByDeviceIdAndReceivedAtBetweenOrderByReceivedAtDesc(
            String deviceId, Instant from, Instant to, Pageable pageable);
}
