// Placeholder values, not real device readings.
const mock={device_id:'cane-01',distance_cm:83.5,water_detected:false,sos:false,latitude:21.0285,longitude:105.8542};
document.getElementById('device').textContent=mock.device_id;
document.getElementById('distance').textContent=mock.distance_cm+' cm';
document.getElementById('water').textContent=mock.water_detected?'Có nước':'Khô';
document.getElementById('sos').textContent=mock.sos?'CẢNH BÁO':'Bình thường';
if(typeof L!=='undefined') { const map=L.map('map').setView([mock.latitude,mock.longitude],13); L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',{attribution:'&copy; OpenStreetMap contributors'}).addTo(map); L.marker([mock.latitude,mock.longitude]).addTo(map).bindPopup('Vị trí GIẢ LẬP — cane-01'); }
else document.getElementById('map').textContent='Không tải được Leaflet (kiểm tra Internet).';
