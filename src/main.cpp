#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>  // Thư viện xử lý thời gian

const char* ssid = "Lab_IoT";      // Thay bằng tên Wi-Fi của bạn
const char* password = "labiotpm4";  // Thay bằng mật khẩu Wi-Fi của bạn

ESP8266WebServer server(80);  // Tạo một server trên cổng 80

String servoAngle = "0";  // Biến để lưu góc servo
int lightSensorPin = A0;  // Pin cảm biến ánh sáng
int lightSensorValue = 0; // Giá trị cảm biến ánh sáng
const char* ntpServer = "time.windows.com";  // Máy chủ NTP
const long gmtOffset_sec = 7 * 3600;         // Múi giờ GMT+7
const int daylightOffset_sec = 0;           // Không có giờ mùa hè

String getFormattedTime() {
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo)) {
    return "Failed to obtain time";
  }
  char buffer[50];
  strftime(buffer, sizeof(buffer), "%H:%M:%S %d-%m-%y", &timeInfo);
  return String(buffer);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Cấu hình NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Trang chính hiển thị HTML với CSS và AJAX
  server.on("/", HTTP_GET, []() {
    String html = R"rawliteral(
       <meta charset="UTF-8">
       <meta name="viewport" content="width=device-width, initial-scale=1.0">
       <title>Tracker Solar System</title>
       <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
       <style>
           body {  background-image: url(https://special.nhandan.vn/phat-trien-nang-luong-tai-tao/assets/5pL7PcmSJV/cover-main-2669x1502.jpg);
            background-size: cover;
            background-position: center;
            color: green;
            display: flex;
            flex-direction: column;
            align-items: center;
            height: 100vh; }
           .container { width: 80%; max-width: 600px; background: rgba(255, 255, 255, 0.9); padding: 20px; border-radius: 10px; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3); text-align: center; margin-top: 20px; color: black; }
           canvas { width: 100%; height: 300px; }
       </style>
       <h1>Solar Light Intensity</h1>
       <div class="container">
           <div class="header">Solar Tracking System</div>
           <div class="card">
               <p><strong>Module light parameters:</strong></p>
               <p> <span id="angle" class="angle-display">0</span></p>
           </div>
           <div class="card">
               <p><strong>Date & Time:</strong></p>
               <p> <span id="time"></span></p>
           </div>
           <div class="card">
               <p><strong>IP Address:</strong> )rawliteral" + WiFi.localIP().toString() + R"rawliteral(</p>
           </div>
           <div class="card">
               <h2>Light Intensity Chart</h2>
               <canvas id="lightChart"></canvas>
           </div>
       </div>

       <script>
           let lightData = [];
           let timeData = [];

           function fetchData() {
               fetch('/light-data')
               .then(response => response.json())
               .then(data => {
                   lightData.push(data.light);
                   timeData.push(data.time);
                   if (lightData.length > 20) {
                       lightData.shift();
                       timeData.shift();
                   }
                   updateChart();
               });

               fetch('/servo-angle')
               .then(response => response.text())
               .then(data => {
                   document.getElementById('angle').innerText = data;
               });

               fetch('/current-time')
               .then(response => response.text())
               .then(data => {
                   document.getElementById('time').innerText = data;
               });
           }

           function updateChart() {
               myChart.data.labels = timeData;
               myChart.data.datasets[0].data = lightData;
               myChart.update();
           }

           setInterval(fetchData, 2000);  // Cập nhật mỗi 2000ms

           const ctx = document.getElementById('lightChart').getContext('2d');
           const myChart = new Chart(ctx, {
               type: 'line',
               data: {
                   labels: timeData,
                   datasets: [{
                       label: 'Light Intensity',
                       data: lightData,
                       borderColor: 'rgb(75, 192, 192)',
                       fill: false
                   }]
               },
               options: {
                   scales: {
                       x: { type: 'linear', position: 'bottom' },
                       y: { beginAtZero: true }
                   }
               }
           });
       </script>
    )rawliteral";
    server.send(200, "text/html", html);
  });

  // Endpoint để trả về dữ liệu cảm biến ánh sáng
  server.on("/light-data", HTTP_GET, []() {
    lightSensorValue = analogRead(lightSensorPin);  // Đọc giá trị cảm biến ánh sáng
    String time = getFormattedTime();
    String json = "{\"light\": " + String(lightSensorValue) + ", \"time\": \"" + time + "\"}";
    server.send(200, "application/json", json);
  });

  // Endpoint để trả về giá trị góc servo
  server.on("/servo-angle", HTTP_GET, []() {
    server.send(200, "text/plain", servoAngle);
  });

  // Endpoint để trả về thời gian hiện tại
  server.on("/current-time", HTTP_GET, []() {
    server.send(200, "text/plain", getFormattedTime());
  });

  server.begin();
}

void loop() {
  server.handleClient();  // Xử lý các yêu cầu web

  // Kiểm tra xem có dữ liệu mới từ Arduino qua Serial không
  if (Serial.available()) {
    servoAngle = Serial.readStringUntil('\n');  // Đọc dữ liệu từ Arduino
  }
}
