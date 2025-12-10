// ============= MQTT 配置（你可以根据实际情况改端口和 path） =============
const MQTT_HOST = "mqtt.cetools.org"; // broker 地址
const MQTT_PORT = 1884; // 改成 1884
const MQTT_PATH = "/mqtt"; // 常见 WebSocket path，先试这个
const MQTT_TOPIC = "student/MUJI/HZH";

// ============= 一些 DOM 元素引用 =============
const mqttStatusText = document.getElementById("mqtt-status-text");
const mqttStatusPill = document.getElementById("mqtt-status-pill");

const presencePill = document.getElementById("presence-pill");
const presenceText = document.getElementById("presence-text");

const tempValue = document.getElementById("temp-value");
const humValue = document.getElementById("hum-value");
const co2Value = document.getElementById("co2-value");
const co2StatusText = document.getElementById("co2-status-text");
const lightValue = document.getElementById("light-value");
const soundValue = document.getElementById("sound-value");
const soundBarFill = document.getElementById("sound-bar-fill");

const fanCard = document.getElementById("fan-card");
const fanState = document.getElementById("fan-state");
const fanSource = document.getElementById("fan-source");

const fireCard = document.getElementById("fire-card");
const fireState = document.getElementById("fire-state");
const fireSubtext = document.getElementById("fire-subtext");

const curtainCard = document.getElementById("curtain-card");
const curtainState = document.getElementById("curtain-state");
const curtainSubtext = document.getElementById("curtain-subtext");

const buzzerCard = document.getElementById("buzzer-card");
const buzzerState = document.getElementById("buzzer-state");
const buzzerSubtext = document.getElementById("buzzer-subtext");

const rawJsonPre = document.getElementById("raw-json");

// ============= Chart.js 数据缓存 =============
const MAX_POINTS = 50;
const labels = [];

const tempData = [];
const humData = [];
const co2Data = [];
const soundData = [];

// 创建环境折线图（温度 / 湿度 / CO2）
const envCtx = document.getElementById("env-chart").getContext("2d");
const envChart = new Chart(envCtx, {
  type: "line",
  data: {
    labels,
    datasets: [
      {
        label: "温度 (°C)",
        data: tempData,
        borderWidth: 1.5,
        tension: 0.25,
      },
      {
        label: "湿度 (%)",
        data: humData,
        borderWidth: 1.5,
        tension: 0.25,
      },
      {
        label: "CO₂ (ppm)",
        data: co2Data,
        borderWidth: 1.5,
        tension: 0.25,
      },
    ],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: { labels: { color: "#e5e7eb", font: { size: 11 } } },
    },
    scales: {
      x: {
        ticks: { color: "#9ca3af", maxRotation: 0 },
        grid: { color: "rgba(55,65,81,0.4)" },
      },
      y: {
        ticks: { color: "#9ca3af" },
        grid: { color: "rgba(55,65,81,0.4)" },
      },
    },
  },
});

// 创建声音折线图
const soundCtx = document.getElementById("sound-chart").getContext("2d");
const soundChart = new Chart(soundCtx, {
  type: "line",
  data: {
    labels,
    datasets: [
      {
        label: "Sound Level",
        data: soundData,
        borderWidth: 1.5,
        tension: 0.25,
      },
    ],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: { labels: { color: "#e5e7eb", font: { size: 11 } } },
    },
    scales: {
      x: {
        ticks: { color: "#9ca3af", maxRotation: 0 },
        grid: { color: "rgba(55,65,81,0.4)" },
      },
      y: {
        ticks: { color: "#9ca3af" },
        grid: { color: "rgba(55,65,81,0.4)" },
      },
    },
  },
});

// 更新图表数据
function pushDataAndTrim(arr, value) {
  arr.push(value);
  if (arr.length > MAX_POINTS) {
    arr.shift();
  }
}

function updateCharts(payload) {
  const now = new Date();
  const label = now.toLocaleTimeString("zh-CN", {
    hour12: false,
    minute: "2-digit",
    second: "2-digit",
  });
  labels.push(label);
  if (labels.length > MAX_POINTS) labels.shift();

  pushDataAndTrim(tempData, payload.temperature);
  pushDataAndTrim(humData, payload.humidity);
  pushDataAndTrim(co2Data, payload.co2);
  pushDataAndTrim(soundData, payload.sound_level);

  envChart.update();
  soundChart.update();
}

// ============= UI 状态更新函数 =============

// 更新 MQTT 连接状态
function setMqttStatus(connected) {
  if (connected) {
    mqttStatusText.textContent = "MQTT 已连接";
    mqttStatusPill.querySelector(".dot").className = "dot dot--connected";
  } else {
    mqttStatusText.textContent = "MQTT 未连接";
    mqttStatusPill.querySelector(".dot").className = "dot dot--disconnected";
  }
}

// 更新人体存在状态
function setPresence(presence) {
  if (presence) {
    presenceText.textContent = "Human: YES";
    presencePill.querySelector(".dot").className = "dot dot--connected";
  } else {
    presenceText.textContent = "Human: NO";
    presencePill.querySelector(".dot").className = "dot dot--unknown";
  }
}

// 更新环境数据
function updateEnvironment(payload) {
  // 温度、湿度、CO2、光照、声音
  tempValue.textContent =
    payload.temperature != null ? payload.temperature.toFixed(1) : "--";
  humValue.textContent =
    payload.humidity != null ? payload.humidity.toFixed(1) : "--";
  co2Value.textContent = payload.co2 != null ? payload.co2.toFixed(0) : "--";
  lightValue.textContent = payload.light ?? "--";
  soundValue.textContent = payload.sound_level ?? "--";

  // CO2 状态简单提示
  if (payload.co2 == null || isNaN(payload.co2)) {
    co2StatusText.textContent = "等待数据...";
  } else if (payload.co2 < 800) {
    co2StatusText.textContent = "空气质量良好";
  } else if (payload.co2 < 1500) {
    co2StatusText.textContent = "略显浑浊，建议通风";
  } else {
    co2StatusText.textContent = "CO₂ 较高，通风很有必要";
  }

  // 声音进度条（用简单线性映射 0~200）
  const level = Math.max(0, Math.min(200, Number(payload.sound_level) || 0));
  const percent = (level / 200) * 100;
  soundBarFill.style.width = percent + "%";
}

// 更新设备状态块
function updateDeviceCards(payload) {
  // 风扇
  const fanOn = !!payload.fan;
  fanState.textContent = fanOn ? "ON" : "OFF";
  fanSource.textContent = `source: ${payload.fan_source || "--"}`;
  fanCard.classList.remove("ok", "warn", "danger");
  fanCard.classList.add(fanOn ? "ok" : "warn");

  // 火焰
  const fire = !!payload.fire;
  fireState.textContent = fire ? "FIRE!" : "SAFE";
  fireSubtext.textContent = fire ? "检测到火焰/异常热源" : "当前无火警";
  fireCard.classList.remove("ok", "warn", "danger");
  fireCard.classList.add(fire ? "danger" : "ok");

  // 窗帘
  const curtain = payload.curtain || "unknown";
  curtainState.textContent = curtain.toUpperCase();
  let curtainTip = "";
  if (curtain === "open") curtainTip = "室内有人 & 光线充足";
  else if (curtain === "close") curtainTip = "室内有人但光线不足";
  else if (curtain === "unchanged") curtainTip = "无人，窗帘保持上次状态";
  else curtainTip = "等待状态...";
  curtainSubtext.textContent = curtainTip;
  curtainCard.classList.remove("ok", "warn", "danger");
  curtainCard.classList.add(
    curtain === "open" ? "ok" : curtain === "close" ? "warn" : "warn"
  );

  // 蜂鸣器
  const buzzer = !!payload.buzzer;
  buzzerState.textContent = buzzer ? "ALARM" : "OFF";
  buzzerSubtext.textContent = buzzer ? "报警进行中（5s）" : "当前无声响报警";
  buzzerCard.classList.remove("ok", "warn", "danger");
  buzzerCard.classList.add(buzzer ? "danger" : "ok");
}

// 更新原始 JSON 显示
function updateRawJson(payload) {
  rawJsonPre.textContent = JSON.stringify(payload, null, 2);
}

// ============= MQTT 客户端逻辑 =============
let client = null;

function connectMqtt() {
  const clientId = "web_" + Math.random().toString(16).substr(2, 8);
  client = new Paho.MQTT.Client(
    MQTT_HOST,
    Number(MQTT_PORT),
    MQTT_PATH,
    clientId
  );

  client.onConnectionLost = (responseObject) => {
    console.warn("MQTT connection lost:", responseObject.errorMessage);
    setMqttStatus(false);
    setTimeout(connectMqtt, 2000); // 简单重连
  };

  client.onMessageArrived = (message) => {
    try {
      const payloadStr = message.payloadString;
      const data = JSON.parse(payloadStr);

      // 适配你发出来的 JSON 字段：
      // presence, co2, temperature, humidity, fan, fan_source,
      // light, curtain, fire, buzzer, sound_level
      const payload = {
        presence: !!data.presence,
        co2: Number(data.co2),
        temperature: Number(data.temperature),
        humidity: Number(data.humidity),
        fan: !!data.fan,
        fan_source: data.fan_source,
        light: Number(data.light),
        curtain: data.curtain,
        fire: !!data.fire,
        buzzer: !!data.buzzer,
        sound_level: Number(data.sound_level),
      };

      setPresence(payload.presence);
      updateEnvironment(payload);
      updateDeviceCards(payload);
      updateRawJson(data);
      updateCharts(payload);
    } catch (err) {
      console.error("Failed to parse MQTT message:", err);
    }
  };

  client.connect({
    useSSL: false, // 如果 broker 要求 wss，可以改成 true
    timeout: 5,
    onSuccess: () => {
      console.log("MQTT Connected");
      setMqttStatus(true);
      client.subscribe(MQTT_TOPIC, { qos: 0 });
    },
    onFailure: (err) => {
      console.error("MQTT connect failed:", err);
      setMqttStatus(false);
      setTimeout(connectMqtt, 3000);
    },
  });
}

// 页面加载完后启动 MQTT
window.addEventListener("load", () => {
  setMqttStatus(false);
  connectMqtt();
});
