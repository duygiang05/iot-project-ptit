package vn.edu.ptit.smartcane.realtime;

import org.springframework.stereotype.Component;
import org.springframework.transaction.event.TransactionPhase;
import org.springframework.transaction.event.TransactionalEventListener;

@Component
public class RealtimeBridge {
    private final RealtimeHub hub;
    public RealtimeBridge(RealtimeHub hub) { this.hub = hub; }

    @TransactionalEventListener(phase = TransactionPhase.AFTER_COMMIT)
    public void onSaved(RealtimeSaved event) {
        hub.broadcast(event.deviceId(), event.type(), event.data());
    }
}
