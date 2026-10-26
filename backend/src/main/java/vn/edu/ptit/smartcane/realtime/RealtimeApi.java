package vn.edu.ptit.smartcane.realtime;

import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.servlet.mvc.method.annotation.SseEmitter;

@RestController
@RequestMapping("/api/devices")
@CrossOrigin(origins = {"http://localhost:5500", "http://127.0.0.1:5500"})
public class RealtimeApi {
    private final RealtimeHub hub;

    public RealtimeApi(RealtimeHub hub) { this.hub = hub; }

    @GetMapping(value = "/{id}/stream", produces = MediaType.TEXT_EVENT_STREAM_VALUE)
    public SseEmitter stream(@PathVariable String id) {
        if (!id.matches("[A-Za-z0-9_-]{1,64}")) {
            throw new org.springframework.web.server.ResponseStatusException(
                    org.springframework.http.HttpStatus.BAD_REQUEST, "Invalid device id");
        }
        return hub.subscribe(id);
    }
}
