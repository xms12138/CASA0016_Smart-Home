const MQTT_HOST = "mqtt.cetools.org";
const MQTT_PORT = 1884;
const MQTT_PATH = "/mqtt";
const MQTT_TOPIC = "student/MUJI/HZH";

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

const MAX_POINTS = 50;
const labels = [];

const tempData = [];
const humData = [];
const co2Data = [];
const soundData = [];

const envCtx = document.getElementById("env-chart").getContext("2d");
const envChart = new Chart(envCtx, {
  type: "line",
  data: {
    labels,
    datasets: [
      {
        label: "Temperature (°C)",
        data: tempData,
        borderWidth: 1.5,
        tension: 0.25,
      },
      {
        label: "Humidity (%)",
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

function setMqttStatus(connected) {
  if (connected) {
    mqttStatusText.textContent = "MQTT Connected";
    mqttStatusPill.querySelector(".dot").className = "dot dot--connected";
  } else {
    mqttStatusText.textContent = "MQTT Disconnected";
    mqttStatusPill.querySelector(".dot").className = "dot dot--disconnected";
  }
}

function setPresence(presence) {
  if (presence) {
    presenceText.textContent = "Human: YES";
    presencePill.querySelector(".dot").className = "dot dot--connected";
  } else {
    presenceText.textContent = "Human: NO";
    presencePill.querySelector(".dot").className = "dot dot--unknown";
  }
}

function updateEnvironment(payload) {
  tempValue.textContent =
    payload.temperature != null ? payload.temperature.toFixed(1) : "--";
  humValue.textContent =
    payload.humidity != null ? payload.humidity.toFixed(1) : "--";
  co2Value.textContent = payload.co2 != null ? payload.co2.toFixed(0) : "--";
  lightValue.textContent = payload.light ?? "--";
  soundValue.textContent = payload.sound_level ?? "--";

  if (payload.co2 == null || isNaN(payload.co2)) {
    co2StatusText.textContent = "Waiting...";
  } else if (payload.co2 < 800) {
    co2StatusText.textContent = "Good";
  } else if (payload.co2 < 1500) {
    co2StatusText.textContent = "Vent recommended";
  } else {
    co2StatusText.textContent = "Vent required";
  }

  const level = Math.max(0, Math.min(200, Number(payload.sound_level) || 0));
  const percent = (level / 200) * 100;
  soundBarFill.style.width = percent + "%";
}

function updateDeviceCards(payload) {
  const fanOn = !!payload.fan;
  fanState.textContent = fanOn ? "ON" : "OFF";
  fanSource.textContent = `source: ${payload.fan_source || "--"}`;
  fanCard.classList.remove("ok", "warn", "danger");
  fanCard.classList.add(fanOn ? "ok" : "warn");

  const fire = !!payload.fire;
  fireState.textContent = fire ? "FIRE!" : "SAFE";
  fireSubtext.textContent = fire ? "Fire detected" : "No fire";
  fireCard.classList.remove("ok", "warn", "danger");
  fireCard.classList.add(fire ? "danger" : "ok");

  const curtain = payload.curtain || "unknown";
  curtainState.textContent = curtain.toUpperCase();
  let curtainTip = "";
  if (curtain === "open") curtainTip = "Open";
  else if (curtain === "close") curtainTip = "Closed";
  else if (curtain === "unchanged") curtainTip = "No change";
  else curtainTip = "Pending";
  curtainSubtext.textContent = curtainTip;
  curtainCard.classList.remove("ok", "warn", "danger");
  curtainCard.classList.add(
    curtain === "open" ? "ok" : curtain === "close" ? "warn" : "warn"
  );

  const buzzer = !!payload.buzzer;
  buzzerState.textContent = buzzer ? "ALARM" : "OFF";
  buzzerSubtext.textContent = buzzer ? "Active" : "No alarm";
  buzzerCard.classList.remove("ok", "warn", "danger");
  buzzerCard.classList.add(buzzer ? "danger" : "ok");
}

function updateRawJson(payload) {
  rawJsonPre.textContent = JSON.stringify(payload, null, 2);
}

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
    setMqttStatus(false);
    setTimeout(connectMqtt, 2000);
  };

  client.onMessageArrived = (message) => {
    try {
      const payloadStr = message.payloadString;
      const data = JSON.parse(payloadStr);

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
    useSSL: false,
    timeout: 5,
    onSuccess: () => {
      setMqttStatus(true);
      client.subscribe(MQTT_TOPIC, { qos: 0 });
    },
    onFailure: () => {
      setMqttStatus(false);
      setTimeout(connectMqtt, 3000);
    },
  });
}

window.addEventListener("load", () => {
  setMqttStatus(false);
  connectMqtt();
});
