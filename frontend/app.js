/**
 * Smart Cane IoT - PTIT | Frontend Application Logic
 * Developer: Đạt (PTIT 2026)
 * Conforming to MQTT Contract: docs/MQTT_CONTRACT.md
 */

// Định nghĩa vị trí mặc định: Học viện Công nghệ Bưu chính Viễn thông (PTIT - Hà Đông)
const DEFAULT_COORDS = {
  lat: 20.9806,
  lng: 105.7876,
  name: "Học viện Công nghệ Bưu chính Viễn thông (PTIT) - Hà Đông, Hà Nội"
};

// Trạng thái hệ thống (State)
const state = {
  telemetry: {
    device_id: "cane-01",
    timestamp: new Date().toISOString(),
    distance_cm: 85.0,
    water_detected: false,
    water_adc: 340,
    light_level: 2850,
    led_on: false,
    latitude: DEFAULT_COORDS.lat,
    longitude: DEFAULT_COORDS.lng,
    gps_valid: true,
    battery_voltage: 4.12,
    battery_percent: 94,
    status: "NORMAL"
  },
  safety: {
    sos: false,
    fall_suspected: false,
    fall_alert: false,
    obstacle_level: "WARNING", // SAFE (>150), WARNING (50-150), DANGER (<50)
    is_online: true
  },
  geofence: {
    center: [DEFAULT_COORDS.lat, DEFAULT_COORDS.lng],
    radius: 350, // 350 mét bán kính an toàn
    enabled: true,
    is_outside: false
  },
  config: {
    sound_enabled: true,
    auto_refresh: true,
    geofence_visible: true,
    path_visible: true
  },
  path_history: [
    [DEFAULT_COORDS.lat - 0.0008, DEFAULT_COORDS.lng - 0.0006],
    [DEFAULT_COORDS.lat - 0.0004, DEFAULT_COORDS.lng - 0.0003],
    [DEFAULT_COORDS.lat, DEFAULT_COORDS.lng]
  ],
  events: []
};

// Map & Audio handles
let map = null;
let caneMarker = null;
let geofenceCircle = null;
let pathPolyline = null;
let audioCtx = null;
let activeFilter = 'all';

// ============================================================================
// KHỞI TẠO BẢN ĐỒ LEAFLET
// ============================================================================
function initMap() {
  const mapElement = document.getElementById('map-container');
  if (!mapElement) return;

  if (typeof L === 'undefined') {
    mapElement.innerHTML = `<div style="padding: 20px; text-align: center; color: #94a3b8;">
      Không tải được thư viện Leaflet (vui lòng kiểm tra kết nối mạng).
    </div>`;
    return;
  }

  // Khởi tạo Leaflet Map
  map = L.map('map-container', {
    zoomControl: true,
    attributionControl: true
  }).setView([state.telemetry.latitude, state.telemetry.longitude], 16);

  // Lớp bản đồ nền OpenStreetMap (Dark CartoDB style hoặc chuẩn OSM)
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors | PTIT Smart Cane'
  }).addTo(map);

  // Tạo Custom Marker Icon cho Gậy
  const customCaneIcon = L.divIcon({
    className: 'custom-cane-marker',
    html: `
      <div style="
        position: relative;
        width: 34px;
        height: 34px;
        background: #06b6d4;
        border: 3px solid #ffffff;
        border-radius: 50%;
        display: flex;
        align-items: center;
        justify-content: center;
        font-size: 16px;
        color: white;
        box-shadow: 0 0 15px #06b6d4;
      ">
        🦯
        <div style="
          position: absolute;
          inset: -8px;
          border-radius: 50%;
          border: 2px solid rgba(6, 182, 212, 0.6);
          animation: pulse-dot 1.5s infinite;
        "></div>
      </div>
    `,
    iconSize: [34, 34],
    iconAnchor: [17, 17]
  });

  caneMarker = L.marker([state.telemetry.latitude, state.telemetry.longitude], {
    icon: customCaneIcon
  }).addTo(map);

  updateMarkerPopup();

  // Hàng rào địa lý an toàn (Geofence Circle)
  geofenceCircle = L.circle(state.geofence.center, {
    color: '#10b981',
    fillColor: '#10b981',
    fillOpacity: 0.12,
    radius: state.geofence.radius,
    dashArray: '6, 8',
    weight: 2
  }).addTo(map);

  // Đường vết di chuyển (Breadcrumbs trail)
  pathPolyline = L.polyline(state.path_history, {
    color: '#06b6d4',
    weight: 3,
    opacity: 0.7,
    dashArray: '4, 6'
  }).addTo(map);
}

function updateMarkerPopup() {
  if (!caneMarker) return;
  const t = state.telemetry;
  const statusColor = state.safety.sos ? '#ef4444' : (t.water_detected ? '#06b6d4' : '#10b981');
  const statusText = state.safety.sos ? 'KHẨN CẤP (SOS)' : (t.water_detected ? 'CẢNH BÁO NƯỚC' : 'HOẠT ĐỘNG BÌNH THƯỜNG');

  const popupContent = `
    <div style="font-family: system-ui; min-width: 200px; color: #1e293b;">
      <h4 style="margin: 0 0 6px 0; font-size: 14px; color: #0f172a; border-bottom: 1px solid #e2e8f0; padding-bottom: 4px;">
        🦯 Smart Cane [${t.device_id}]
      </h4>
      <div style="font-size: 12px; margin-bottom: 4px;">
        <strong>Trạng thái:</strong> <span style="color: ${statusColor}; font-weight: bold;">${statusText}</span>
      </div>
      <div style="font-size: 12px; margin-bottom: 4px;">
        <strong>Tọa độ:</strong> ${t.latitude.toFixed(5)}, ${t.longitude.toFixed(5)}
      </div>
      <div style="font-size: 12px; margin-bottom: 4px;">
        <strong>Khoảng cách:</strong> ${t.distance_cm} cm
      </div>
      <div style="font-size: 11px; color: #64748b; margin-top: 6px;">
        📍 PTIT Hà Đông (Đang cập nhật thời gian thực)
      </div>
    </div>
  `;
  caneMarker.bindPopup(popupContent);
}

// Kiểm tra khoảng cách Haversine (mét) giữa 2 tọa độ
function calculateDistanceMeters(lat1, lon1, lat2, lon2) {
  const R = 6371e3; // Bán kính Trái Đất (mét)
  const phi1 = (lat1 * Math.PI) / 180;
  const phi2 = (lat2 * Math.PI) / 180;
  const deltaPhi = ((lat2 - lat1) * Math.PI) / 180;
  const deltaLambda = ((lon2 - lon1) * Math.PI) / 180;

  const a =
    Math.sin(deltaPhi / 2) * Math.sin(deltaPhi / 2) +
    Math.cos(phi1) * Math.cos(phi2) * Math.sin(deltaLambda / 2) * Math.sin(deltaLambda / 2);
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));

  return R * c;
}

// ============================================================================
// HỆ THỐNG ÂM THANH CẢNH BÁO (WEB AUDIO API)
// ============================================================================
function playAlarmBeep(type = 'danger') {
  if (!state.config.sound_enabled) return;

  try {
    if (!audioCtx) {
      audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    }
    if (audioCtx.state === 'suspended') {
      audioCtx.resume();
    }

    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();

    osc.connect(gain);
    gain.connect(audioCtx.destination);

    if (type === 'danger') {
      osc.type = 'sawtooth';
      osc.frequency.setValueAtTime(880, audioCtx.currentTime); // A5
      osc.frequency.setValueAtTime(440, audioCtx.currentTime + 0.15); // A4
      gain.gain.setValueAtTime(0.15, audioCtx.currentTime);
      gain.gain.exponentialRampToValueAtTime(0.01, audioCtx.currentTime + 0.35);
      osc.start();
      osc.stop(audioCtx.currentTime + 0.35);
    } else if (type === 'water') {
      osc.type = 'sine';
      osc.frequency.setValueAtTime(600, audioCtx.currentTime);
      osc.frequency.exponentialRampToValueAtTime(300, audioCtx.currentTime + 0.25);
      gain.gain.setValueAtTime(0.12, audioCtx.currentTime);
      gain.gain.exponentialRampToValueAtTime(0.01, audioCtx.currentTime + 0.25);
      osc.start();
      osc.stop(audioCtx.currentTime + 0.25);
    } else {
      osc.type = 'triangle';
      osc.frequency.setValueAtTime(523.25, audioCtx.currentTime); // C5
      gain.gain.setValueAtTime(0.08, audioCtx.currentTime);
      gain.gain.exponentialRampToValueAtTime(0.01, audioCtx.currentTime + 0.15);
      osc.start();
      osc.stop(audioCtx.currentTime + 0.15);
    }
  } catch (err) {
    console.warn("Audio Context init pending user gesture:", err);
  }
}

// ============================================================================
// CẬP NHẬT GIAO DIỆN (UI RENDER)
// ============================================================================
function updateUI() {
  const t = state.telemetry;
  const s = state.safety;

  // 1. Header & Online/Offline
  const statusDot = document.getElementById('status-dot');
  const statusText = document.getElementById('status-text');
  const lastSync = document.getElementById('last-sync-time');

  if (s.is_online) {
    if (s.sos || s.fall_alert) {
      statusDot.className = 'status-dot danger';
      statusText.textContent = 'BÁO ĐỘNG';
    } else {
      statusDot.className = 'status-dot pulsing';
      statusText.textContent = 'TRỰC TUYẾN';
    }
  } else {
    statusDot.className = 'status-dot offline';
    statusText.textContent = 'MẤT KẾT NỐI (LWT)';
  }

  const now = new Date();
  lastSync.textContent = now.toLocaleTimeString('vi-VN');

  // 2. Banner Cảnh Báo Khẩn Cấp (Emergency Banner)
  const emergencyBanner = document.getElementById('emergency-banner');
  const emergencyTitle = document.getElementById('emergency-title');
  const emergencyDesc = document.getElementById('emergency-desc');

  if (s.sos) {
    emergencyBanner.classList.add('active');
    emergencyTitle.textContent = '🚨 BÁO ĐỘNG KHẨN CẤP: NÚT SOS ĐÃ NHẤN!';
    emergencyDesc.textContent = `Người dùng đã kích hoạt nút SOS khẩn cấp tại tọa độ (${t.latitude.toFixed(5)}, ${t.longitude.toFixed(5)}). Hệ thống đã tự động gửi tin nhắn Telegram tới người thân!`;
    playAlarmBeep('danger');
  } else if (s.fall_alert) {
    emergencyBanner.classList.add('active');
    emergencyTitle.textContent = '⚠️ CẢNH BÁO: GẬY BỊ NGÃ / RƠI ĐỔ!';
    emergencyDesc.textContent = 'Cảm biến MPU6050 phát hiện gậy nghiêng đổ bất thường trong hơn 10 giây mà không được hủy. Vui lòng kiểm tra ngay!';
    playAlarmBeep('danger');
  } else if (t.water_detected) {
    emergencyBanner.classList.add('active');
    emergencyBanner.style.background = 'linear-gradient(135deg, #0284c7 0%, #0369a1 100%)';
    emergencyTitle.textContent = '💧 CẢNH BÁO MÔI TRƯỜNG: TIẾP XÚC NƯỚC!';
    emergencyDesc.textContent = `Đầu dò gậy đã chạm mực nước sâu / vũng nước (ADC = ${t.water_adc}). Rung cảnh báo trên thân gậy đang được kích hoạt!`;
    playAlarmBeep('water');
  } else {
    emergencyBanner.classList.remove('active');
    emergencyBanner.style.background = '';
  }

  // 3. Card 1: Khoảng Cách Vật Cản
  const valDistance = document.getElementById('val-distance');
  const barDistance = document.getElementById('bar-distance');
  const badgeDistance = document.getElementById('badge-distance');
  const noteDistance = document.getElementById('note-distance');

  valDistance.textContent = t.distance_cm.toFixed(1);
  const distPercent = Math.min(100, Math.max(5, (t.distance_cm / 200) * 100));
  barDistance.style.width = `${distPercent}%`;

  if (t.distance_cm < 50) {
    s.obstacle_level = "DANGER";
    barDistance.className = "progress-bar progress-danger";
    badgeDistance.className = "state-badge danger";
    badgeDistance.textContent = "NGUY HIỂM (<50cm)";
    noteDistance.textContent = "Mô-tơ rung LIÊN TỤC (Mức 3)";
  } else if (t.distance_cm <= 150) {
    s.obstacle_level = "WARNING";
    barDistance.className = "progress-bar progress-warning";
    badgeDistance.className = "state-badge warning";
    badgeDistance.textContent = "CẢNH BÁO (50-150cm)";
    noteDistance.textContent = "Mô-tơ rung NGẮT QUÃNG (Mức 2)";
  } else {
    s.obstacle_level = "SAFE";
    barDistance.className = "progress-bar progress-safe";
    badgeDistance.className = "state-badge safe";
    badgeDistance.textContent = "AN TOÀN (>150cm)";
    noteDistance.textContent = "Mô-tơ KHÔNG RUNG (Mức 1)";
  }

  // 4. Card 2: Cảm biến Nước (Water Sensor - Module của Đạt)
  const valWater = document.getElementById('val-water');
  const valWaterAdc = document.getElementById('val-water-adc');
  const badgeWater = document.getElementById('badge-water');
  const barWater = document.getElementById('bar-water');

  valWaterAdc.textContent = `${t.water_adc} / 4095`;
  const waterPercent = Math.min(100, (t.water_adc / 4095) * 100);
  barWater.style.width = `${waterPercent}%`;

  if (t.water_detected) {
    valWater.textContent = "CHẠM NƯỚC";
    valWater.style.color = "#38bdf8";
    badgeWater.className = "state-badge danger";
    badgeWater.textContent = "⚠️ PHÁT HIỆN NƯỚC";
    barWater.className = "progress-bar progress-danger";
  } else {
    valWater.textContent = "KHÔ RÁO";
    valWater.style.color = "#f8fafc";
    badgeWater.className = "state-badge safe";
    badgeWater.textContent = "✓ AN TOÀN";
    barWater.className = "progress-bar progress-cyan";
  }

  // 5. Card 3: Ánh Sáng & Đèn LED (LDR & LED - Module của Đạt)
  const valLight = document.getElementById('val-light');
  const valLightPercent = document.getElementById('val-light-percent');
  const badgeLed = document.getElementById('badge-led');
  const barLight = document.getElementById('bar-light');
  const ledIndicator = document.getElementById('led-indicator-text');

  valLight.textContent = t.light_level;
  const lightPct = Math.round((t.light_level / 4095) * 100);
  valLightPercent.textContent = `(${lightPct}%)`;
  barLight.style.width = `${lightPct}%`;

  if (t.led_on) {
    badgeLed.className = "state-badge warning";
    badgeLed.innerHTML = "💡 ĐANG BẬT (TỐI)";
    ledIndicator.textContent = "LED sáng tự động (Trời tối)";
    ledIndicator.style.color = "#f59e0b";
  } else {
    badgeLed.className = "state-badge safe";
    badgeLed.innerHTML = "🌙 ĐANG TẮT (SÁNG)";
    ledIndicator.textContent = "LED tắt (Đủ sáng)";
    ledIndicator.style.color = "#94a3b8";
  }

  // 6. Card 4: Gia tốc MPU6050 & SOS
  const valSafety = document.getElementById('val-safety');
  const badgeSafety = document.getElementById('badge-safety');
  const noteSafety = document.getElementById('note-safety');

  if (s.sos) {
    valSafety.textContent = "SOS ACTIVE";
    valSafety.style.color = "#ef4444";
    badgeSafety.className = "state-badge danger";
    badgeSafety.textContent = "KHẨN CẤP";
    noteSafety.textContent = "Nút bấm SOS đang giữ";
  } else if (s.fall_alert) {
    valSafety.textContent = "GẬY ĐỔ";
    valSafety.style.color = "#f59e0b";
    badgeSafety.className = "state-badge warning";
    badgeSafety.textContent = "CẢNH BÁO NGÃ";
    noteSafety.textContent = "Còi đếm ngược đã kích hoạt";
  } else {
    valSafety.textContent = "ỔN ĐỊNH";
    valSafety.style.color = "#f8fafc";
    badgeSafety.className = "state-badge safe";
    badgeSafety.textContent = "BÌNH THƯỜNG";
    noteSafety.textContent = "Góc nghiêng gậy thẳng";
  }

  // 7. Card 5: Mức Pin 18650
  const valBattery = document.getElementById('val-battery');
  const valBatteryVolt = document.getElementById('val-battery-volt');
  const barBattery = document.getElementById('bar-battery');

  valBattery.textContent = `${t.battery_percent}%`;
  valBatteryVolt.textContent = `${t.battery_voltage.toFixed(2)} V`;
  barBattery.style.width = `${t.battery_percent}%`;

  if (t.battery_percent < 20) {
    barBattery.className = "progress-bar progress-danger";
  } else if (t.battery_percent < 50) {
    barBattery.className = "progress-bar progress-warning";
  } else {
    barBattery.className = "progress-bar progress-safe";
  }

  // 8. Cập nhật vị trí bản đồ Leaflet & Geofence
  if (map && caneMarker) {
    const newLatLng = [t.latitude, t.longitude];
    caneMarker.setLatLng(newLatLng);
    updateMarkerPopup();

    // Kiểm tra Hàng rào ảo Geofence
    const distFromCenter = calculateDistanceMeters(
      state.geofence.center[0],
      state.geofence.center[1],
      t.latitude,
      t.longitude
    );

    const badgeGeofence = document.getElementById('badge-geofence');
    const geofenceDistText = document.getElementById('geofence-dist-text');

    if (distFromCenter > state.geofence.radius) {
      state.geofence.is_outside = true;
      geofenceCircle.setStyle({ color: '#ef4444', fillColor: '#ef4444' });
      badgeGeofence.className = 'state-badge danger';
      badgeGeofence.textContent = '⚠️ NGOÀI VÙNG';
      geofenceDistText.textContent = `Cách tâm: ${Math.round(distFromCenter)}m (Vượt ngưỡng ${state.geofence.radius}m)`;
    } else {
      state.geofence.is_outside = false;
      geofenceCircle.setStyle({ color: '#10b981', fillColor: '#10b981' });
      badgeGeofence.className = 'state-badge safe';
      badgeGeofence.textContent = '✓ TRONG VÙNG AN TOÀN';
      geofenceDistText.textContent = `Cách tâm: ${Math.round(distFromCenter)}m / ${state.geofence.radius}m`;
    }

    // Cập nhật đường lịch sử vết di chuyển
    const lastPoint = state.path_history[state.path_history.length - 1];
    if (!lastPoint || lastPoint[0] !== t.latitude || lastPoint[1] !== t.longitude) {
      state.path_history.push(newLatLng);
      if (state.path_history.length > 20) state.path_history.shift();
      pathPolyline.setLatLngs(state.path_history);
    }
  }

  // Cập nhật JSON Output Box
  const jsonBox = document.getElementById('json-preview');
  if (jsonBox) {
    const exportTelemetry = {
      device_id: t.device_id,
      timestamp: t.timestamp,
      distance_cm: t.distance_cm,
      water_detected: t.water_detected,
      water_adc: t.water_adc,
      light_level: t.light_level,
      led_on: t.led_on,
      latitude: t.latitude,
      longitude: t.longitude,
      gps_valid: t.gps_valid,
      battery_voltage: t.battery_voltage,
      status: s.sos ? "SOS_ALERT" : (t.water_detected ? "WATER_WARNING" : t.status)
    };
    jsonBox.textContent = JSON.stringify(exportTelemetry, null, 2);
  }
}

// ============================================================================
// HỆ THỐNG GHI LOG SỰ KIỆN (REAL-TIME EVENT LOG)
// ============================================================================
function addEventLog(type, title, description, category = 'info') {
  const event = {
    id: 'evt-' + Date.now().toString(36),
    time: new Date().toLocaleTimeString('vi-VN'),
    type,
    title,
    description,
    category
  };

  state.events.unshift(event);
  if (state.events.length > 50) state.events.pop();

  renderEventList();
}

function renderEventList() {
  const container = document.getElementById('event-list-container');
  if (!container) return;

  const filtered = activeFilter === 'all' 
    ? state.events 
    : state.events.filter(e => e.category === activeFilter);

  if (filtered.length === 0) {
    container.innerHTML = `
      <div style="padding: 30px; text-align: center; color: var(--text-dim); font-size: 0.85rem;">
        Chưa có sự kiện nào trong danh mục này.
      </div>
    `;
    return;
  }

  container.innerHTML = filtered.map(item => {
    let icon = 'ℹ️';
    if (item.category === 'danger') icon = '🚨';
    else if (item.category === 'warning') icon = '⚠️';
    else if (item.category === 'water') icon = '💧';
    else if (item.category === 'environment') icon = '💡';

    return `
      <div class="event-item">
        <div class="event-left">
          <div class="event-icon ${item.category}">${icon}</div>
          <div class="event-info">
            <strong>${item.title}</strong>
            <span>${item.description}</span>
          </div>
        </div>
        <div class="event-time">${item.time}</div>
      </div>
    `;
  }).join('');
}

// ============================================================================
// BỘ GIẢ LẬP KỊCH BẢN (SCENARIO SIMULATOR)
// ============================================================================
function applyScenario(scenarioId) {
  // Highlight nút được chọn
  document.querySelectorAll('.btn-sim-preset').forEach(btn => {
    btn.classList.toggle('active', btn.dataset.scenario === scenarioId);
  });

  const t = state.telemetry;
  const s = state.safety;

  switch (scenarioId) {
    case 'normal':
      t.distance_cm = 165.0;
      t.water_detected = false;
      t.water_adc = 310;
      t.light_level = 3200;
      t.led_on = false;
      t.latitude = DEFAULT_COORDS.lat;
      t.longitude = DEFAULT_COORDS.lng;
      s.sos = false;
      s.fall_alert = false;
      s.is_online = true;
      addEventLog('NORMAL', 'Kịch bản Bình Thường', 'Đường đi thông thoáng, khô ráo, pin đầy', 'info');
      break;

    case 'obstacle_danger':
      t.distance_cm = 32.5;
      t.water_detected = false;
      t.water_adc = 290;
      s.sos = false;
      s.fall_alert = false;
      s.is_online = true;
      addEventLog('OBSTACLE_DANGER', 'Vật Cản Rất Gần (<50cm)', 'Mô tơ kích hoạt rung liên tục mức 3 cảnh báo va chạm', 'danger');
      break;

    case 'water_warning':
      t.water_detected = true;
      t.water_adc = 2650;
      t.distance_cm = 85.0;
      s.sos = false;
      s.fall_alert = false;
      s.is_online = true;
      addEventLog('WATER_DETECTED', 'Phát Hiện Chạm Nước!', 'Đầu gậy tiếp xúc vũng nước sâu/ngập (ADC 2650)', 'water');
      break;

    case 'night_mode':
      t.light_level = 820;
      t.led_on = true;
      t.water_detected = false;
      t.water_adc = 280;
      s.sos = false;
      s.fall_alert = false;
      s.is_online = true;
      addEventLog('NIGHT_MODE', 'Trời Tối - Bật Đèn LED', 'LDR giảm dưới 1600 ADC -> Tự động bật đèn LED trắng an toàn', 'environment');
      break;

    case 'sos_emergency':
      s.sos = true;
      s.fall_alert = false;
      s.is_online = true;
      addEventLog('SOS_EVENT', 'BÁO ĐỘNG SOS KHẨN CẤP', 'Người khiếm thị nhấn giữ nút SOS 3 giây gọi trợ giúp', 'danger');
      break;

    case 'cane_fall':
      s.sos = false;
      s.fall_alert = true;
      s.is_online = true;
      addEventLog('CANE_FALL', 'Nghi Ngờ Gậy Bị Ngã / Đổ', 'Góc nghiêng MPU6050 vượt ngưỡng, còi đếm lùi kích hoạt', 'warning');
      break;

    case 'geofence_out':
      // Dịch chuyển ra ngoài bán kính 350m
      t.latitude = DEFAULT_COORDS.lat + 0.0055;
      t.longitude = DEFAULT_COORDS.lng + 0.0055;
      s.sos = false;
      s.fall_alert = false;
      s.is_online = true;
      addEventLog('GEOFENCE_ALERT', 'Rời Khỏi Vùng An Toàn', 'Vị trí gậy đã vượt quá bán kính bảo vệ 350m xung quanh PTIT', 'warning');
      if (map) {
        map.panTo([t.latitude, t.longitude]);
      }
      break;

    case 'offline':
      s.is_online = false;
      addEventLog('STATUS_OFFLINE', 'Mất Kết Nối Thiết Bị', 'Broker MQTT kích hoạt Last Will and Testament (LWT)', 'warning');
      break;
  }

  // Cập nhật lại các thanh trượt điều khiển thủ công
  syncSlidersFromState();
  updateUI();
}

function syncSlidersFromState() {
  const t = state.telemetry;
  const s = state.safety;

  const sliderDist = document.getElementById('slider-dist');
  const sliderWater = document.getElementById('slider-water');
  const sliderLight = document.getElementById('slider-light');
  const chkWater = document.getElementById('chk-water');
  const chkSos = document.getElementById('chk-sos');
  const chkLed = document.getElementById('chk-led');

  if (sliderDist) sliderDist.value = t.distance_cm;
  if (sliderWater) sliderWater.value = t.water_adc;
  if (sliderLight) sliderLight.value = t.light_level;
  if (chkWater) chkWater.checked = t.water_detected;
  if (chkSos) chkSos.checked = s.sos;
  if (chkLed) chkLed.checked = t.led_on;

  document.getElementById('lbl-slider-dist').textContent = `${t.distance_cm} cm`;
  document.getElementById('lbl-slider-water').textContent = `${t.water_adc} ADC`;
  document.getElementById('lbl-slider-light').textContent = `${t.light_level} ADC`;
}

// ============================================================================
// LẮNG NGHE SỰ KIỆN GIAO DIỆN (EVENT LISTENERS)
// ============================================================================
function setupEventListeners() {
  // Preset buttons
  document.querySelectorAll('.btn-sim-preset').forEach(btn => {
    btn.addEventListener('click', () => {
      applyScenario(btn.dataset.scenario);
    });
  });

  // Slider Khoảng cách
  const sliderDist = document.getElementById('slider-dist');
  if (sliderDist) {
    sliderDist.addEventListener('input', (e) => {
      const val = parseFloat(e.target.value);
      state.telemetry.distance_cm = val;
      document.getElementById('lbl-slider-dist').textContent = `${val.toFixed(1)} cm`;
      updateUI();
    });
  }

  // Slider Cảm biến Nước ADC
  const sliderWater = document.getElementById('slider-water');
  if (sliderWater) {
    sliderWater.addEventListener('input', (e) => {
      const val = parseInt(e.target.value);
      state.telemetry.water_adc = val;
      state.telemetry.water_detected = val >= 1500;
      document.getElementById('lbl-slider-water').textContent = `${val} ADC`;
      document.getElementById('chk-water').checked = state.telemetry.water_detected;
      updateUI();
    });
  }

  // Slider Cảm biến Ánh Sáng ADC
  const sliderLight = document.getElementById('slider-light');
  if (sliderLight) {
    sliderLight.addEventListener('input', (e) => {
      const val = parseInt(e.target.value);
      state.telemetry.light_level = val;
      // Auto Hysteresis simulation
      if (val < 1600) {
        state.telemetry.led_on = true;
      } else if (val > 2100) {
        state.telemetry.led_on = false;
      }
      document.getElementById('lbl-slider-light').textContent = `${val} ADC`;
      document.getElementById('chk-led').checked = state.telemetry.led_on;
      updateUI();
    });
  }

  // Checkboxes thủ công
  const chkWater = document.getElementById('chk-water');
  if (chkWater) {
    chkWater.addEventListener('change', (e) => {
      state.telemetry.water_detected = e.target.checked;
      state.telemetry.water_adc = e.target.checked ? 2800 : 350;
      document.getElementById('slider-water').value = state.telemetry.water_adc;
      document.getElementById('lbl-slider-water').textContent = `${state.telemetry.water_adc} ADC`;
      if (e.target.checked) {
        addEventLog('WATER_MANUAL', 'Bật cảnh báo nước thủ công', 'Người dùng gạt công tắc nước', 'water');
      }
      updateUI();
    });
  }

  const chkSos = document.getElementById('chk-sos');
  if (chkSos) {
    chkSos.addEventListener('change', (e) => {
      state.safety.sos = e.target.checked;
      if (e.target.checked) {
        addEventLog('SOS_MANUAL', 'Bật SOS thủ công', 'Gạt công tắc SOS khẩn cấp', 'danger');
      }
      updateUI();
    });
  }

  const chkLed = document.getElementById('chk-led');
  if (chkLed) {
    chkLed.addEventListener('change', (e) => {
      state.telemetry.led_on = e.target.checked;
      addEventLog('LED_TOGGLE', 'Điều khiển LED thủ công', e.target.checked ? 'Bật LED ban đêm' : 'Tắt LED ban đêm', 'environment');
      updateUI();
    });
  }

  // Tắt cảnh báo khẩn cấp từ banner
  const btnDismissAlert = document.getElementById('btn-dismiss-alert');
  if (btnDismissAlert) {
    btnDismissAlert.addEventListener('click', () => {
      state.safety.sos = false;
      state.safety.fall_alert = false;
      state.telemetry.water_detected = false;
      state.telemetry.water_adc = 300;
      addEventLog('ALERT_DISMISSED', 'Đã tắt còi cảnh báo', 'Người giám sát đã xác nhận tình huống', 'info');
      syncSlidersFromState();
      updateUI();
    });
  }

  // Map controls
  const btnCenterMap = document.getElementById('btn-center-map');
  if (btnCenterMap) {
    btnCenterMap.addEventListener('click', () => {
      if (map) {
        map.setView([state.telemetry.latitude, state.telemetry.longitude], 17);
      }
    });
  }

  const btnToggleGeofence = document.getElementById('btn-toggle-geofence');
  if (btnToggleGeofence) {
    btnToggleGeofence.addEventListener('click', () => {
      state.config.geofence_visible = !state.config.geofence_visible;
      if (geofenceCircle && map) {
        if (state.config.geofence_visible) {
          map.addLayer(geofenceCircle);
          btnToggleGeofence.classList.add('active');
        } else {
          map.removeLayer(geofenceCircle);
          btnToggleGeofence.classList.remove('active');
        }
      }
    });
  }

  const btnClearPath = document.getElementById('btn-clear-path');
  if (btnClearPath) {
    btnClearPath.addEventListener('click', () => {
      state.path_history = [[state.telemetry.latitude, state.telemetry.longitude]];
      if (pathPolyline) {
        pathPolyline.setLatLngs(state.path_history);
      }
    });
  }

  // Filter Buttons
  document.querySelectorAll('.filter-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      activeFilter = btn.dataset.filter;
      renderEventList();
    });
  });

  // Export Events JSON
  const btnExportLog = document.getElementById('btn-export-log');
  if (btnExportLog) {
    btnExportLog.addEventListener('click', () => {
      const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(state.events, null, 2));
      const downloadAnchor = document.createElement('a');
      downloadAnchor.setAttribute("href", dataStr);
      downloadAnchor.setAttribute("download", `smartcane_events_${Date.now()}.json`);
      document.body.appendChild(downloadAnchor);
      downloadAnchor.click();
      downloadAnchor.remove();
    });
  }

  // Modal Giới thiệu & Hướng dẫn
  const btnOpenModal = document.getElementById('btn-open-modal');
  const btnCloseModal = document.getElementById('btn-close-modal');
  const modalOverlay = document.getElementById('modal-overlay');

  if (btnOpenModal && modalOverlay) {
    btnOpenModal.addEventListener('click', () => modalOverlay.classList.add('active'));
  }
  if (btnCloseModal && modalOverlay) {
    btnCloseModal.addEventListener('click', () => modalOverlay.classList.remove('active'));
  }
  if (modalOverlay) {
    modalOverlay.addEventListener('click', (e) => {
      if (e.target === modalOverlay) modalOverlay.classList.remove('active');
    });
  }

  // Toggle Âm thanh
  const btnToggleSound = document.getElementById('btn-toggle-sound');
  if (btnToggleSound) {
    btnToggleSound.addEventListener('click', () => {
      state.config.sound_enabled = !state.config.sound_enabled;
      btnToggleSound.textContent = state.config.sound_enabled ? '🔊 Âm Báo: BẬT' : '🔇 Âm Báo: TẮT';
      btnToggleSound.classList.toggle('active', state.config.sound_enabled);
    });
  }

  // Toggle Chế độ Backend (Đ03)
  const btnToggleBackend = document.getElementById('btn-toggle-backend');
  if (btnToggleBackend) {
    btnToggleBackend.addEventListener('click', () => {
      if (backendManager.isLive) {
        backendManager.disconnect();
      } else {
        const modal = document.getElementById('modal-overlay');
        if (modal) modal.classList.add('active');
        const inputUrl = document.getElementById('input-backend-url');
        if (inputUrl) inputUrl.focus();
      }
    });
  }

  // Kết nối Backend từ Modal
  const btnConnectBackend = document.getElementById('btn-connect-backend');
  if (btnConnectBackend) {
    btnConnectBackend.addEventListener('click', () => {
      const input = document.getElementById('input-backend-url');
      const url = input ? input.value.trim() : 'ws://localhost:8080/ws/telemetry';
      backendManager.connect(url);
    });
  }

  // Ngắt kết nối Backend từ Modal
  const btnDisconnectBackend = document.getElementById('btn-disconnect-backend');
  if (btnDisconnectBackend) {
    btnDisconnectBackend.addEventListener('click', () => {
      backendManager.disconnect();
    });
  }
}

// ============================================================================
// QUẢN LÝ KẾT NỐI BACKEND THẬT (Đ03 - WebSocket & REST Polling)
// ============================================================================
const backendManager = {
  ws: null,
  isLive: false,
  pollingTimer: null,
  targetUrl: 'ws://localhost:8080/ws/telemetry',

  updateBadge(status, text, color) {
    const dot = document.getElementById('backend-status-dot');
    const label = document.getElementById('backend-status-text');
    const msg = document.getElementById('backend-conn-status-msg');

    if (dot) dot.style.background = color;
    if (label) label.textContent = text;
    if (msg) {
      msg.textContent = `• Trạng thái: ${status}`;
      msg.style.color = color;
    }
  },

  connect(url) {
    this.disconnect();
    this.targetUrl = url || this.targetUrl;
    this.updateBadge('Đang thử kết nối tới ' + this.targetUrl + '...', 'Đang kết nối...', '#38bdf8');

    if (this.targetUrl.startsWith('ws://') || this.targetUrl.startsWith('wss://')) {
      try {
        this.ws = new WebSocket(this.targetUrl);

        this.ws.onopen = () => {
          this.isLive = true;
          this.updateBadge('Đã kết nối trực tiếp với Backend Spring Boot (' + this.targetUrl + ')', 'Trực Tuyến (Live)', '#10b981');
          addEventLog('BACKEND_CONNECTED', 'Kết nối Backend thành công', `Đã mở kênh WebSocket nhận dữ liệu thời gian thực từ ESP32`, 'info');
        };

        this.ws.onmessage = (event) => {
          try {
            const data = JSON.parse(event.data);
            this.handleIncomingData(data);
          } catch (e) {
            console.warn('[Backend] Lỗi parse JSON:', e);
          }
        };

        this.ws.onerror = (err) => {
          console.warn('[Backend] Lỗi WebSocket:', err);
          this.updateBadge('Không thể kết nối tới Backend (Kiểm tra xem Spring Boot đã chạy chưa)', 'Mô Phỏng (Mock)', '#eab308');
        };

        this.ws.onclose = () => {
          if (this.isLive) {
            this.isLive = false;
            this.updateBadge('Mất kết nối Backend. Tự động chuyển về Chế độ Giả lập (Mock)', 'Mô Phỏng (Mock)', '#eab308');
            addEventLog('BACKEND_DISCONNECTED', 'Mất kết nối Backend', 'Đã chuyển về chế độ giả lập nội bộ (Standalone Mock)', 'warning');
          }
        };
      } catch (err) {
        this.updateBadge('Lỗi khởi tạo WebSocket: ' + err.message, 'Mô Phỏng (Mock)', '#ef4444');
      }
    } else {
      // Chế độ HTTP REST Polling
      this.isLive = true;
      this.updateBadge('Đang nhận dữ liệu qua REST Polling (' + this.targetUrl + ')', 'REST Polling', '#10b981');
      addEventLog('BACKEND_REST', 'Kích hoạt REST Polling', `Chu kỳ lấy mẫu 2000ms qua REST API: ${this.targetUrl}`, 'info');

      this.pollingTimer = setInterval(async () => {
        try {
          const res = await fetch(`${this.targetUrl}/api/v1/telemetry`);
          if (res.ok) {
            const json = await res.json();
            this.handleIncomingData(json);
          }
        } catch (e) {
          console.warn('[Backend] Lỗi polling REST API:', e);
        }
      }, 2000);
    }
  },

  disconnect() {
    if (this.ws) {
      try { this.ws.close(); } catch(e){}
      this.ws = null;
    }
    if (this.pollingTimer) {
      clearInterval(this.pollingTimer);
      this.pollingTimer = null;
    }
    this.isLive = false;
    this.updateBadge('Đang hoạt động ở chế độ Giả lập nội bộ (Standalone Mock)', 'Mô Phỏng (Mock)', '#eab308');
  },

  handleIncomingData(payload) {
    if (!payload || typeof payload !== 'object') return;
    const t = state.telemetry;
    const s = state.safety;

    // Cập nhật các trường theo docs/MQTT_CONTRACT.md
    if (payload.device_id) t.device_id = payload.device_id;
    if (payload.distance_cm !== undefined && payload.distance_cm !== null) {
      t.distance_cm = parseFloat(payload.distance_cm);
    }
    if (payload.water_detected !== undefined) {
      t.water_detected = Boolean(payload.water_detected);
    }
    if (payload.water_adc !== undefined) {
      t.water_adc = parseInt(payload.water_adc);
    }
    if (payload.light_level !== undefined) {
      t.light_level = parseInt(payload.light_level);
    }
    if (payload.led_on !== undefined) {
      t.led_on = Boolean(payload.led_on);
    }
    if (payload.latitude !== undefined && payload.latitude !== null) {
      t.latitude = parseFloat(payload.latitude);
    }
    if (payload.longitude !== undefined && payload.longitude !== null) {
      t.longitude = parseFloat(payload.longitude);
    }
    if (payload.gps_valid !== undefined) {
      t.gps_valid = Boolean(payload.gps_valid);
    }
    if (payload.battery_percent !== undefined) {
      t.battery_percent = parseInt(payload.battery_percent);
    }
    if (payload.battery_voltage !== undefined) {
      t.battery_voltage = parseFloat(payload.battery_voltage);
    }
    if (payload.sos !== undefined) {
      s.sos = Boolean(payload.sos);
    }
    if (payload.fall_alert !== undefined) {
      s.fall_alert = Boolean(payload.fall_alert);
    }
    if (payload.status) {
      t.status = payload.status;
    }

    t.timestamp = payload.timestamp || new Date().toISOString();
    s.is_online = true;

    syncSlidersFromState();
    updateUI();
  }
};

// ============================================================================
// KHỞI ĐỘNG ỨNG DỤNG
// ============================================================================
window.addEventListener('DOMContentLoaded', () => {
  initMap();
  setupEventListeners();

  // Thêm một số log khởi động ban đầu
  addEventLog('SYSTEM_BOOT', 'Hệ thống Smart Cane sẵn sàng', 'Đã khởi tạo bảng điều khiển IoT Dashboard (PTIT)', 'info');
  addEventLog('GEOFENCE_INIT', 'Kích hoạt hàng rào an toàn', 'Bán kính bảo vệ 350m tại khuôn viên PTIT Hà Đông', 'info');

  syncSlidersFromState();
  updateUI();

  // Mô phỏng dao động nhẹ thời gian thực khi ở chế độ Mock
  setInterval(() => {
    state.telemetry.timestamp = new Date().toISOString();
    if (!backendManager.isLive && state.safety.is_online) {
      const jitter = (Math.random() - 0.5) * 2;
      state.telemetry.distance_cm = Math.max(15, Math.min(220, state.telemetry.distance_cm + jitter * 0.5));
      updateUI();
    }
  }, 2500);
});
