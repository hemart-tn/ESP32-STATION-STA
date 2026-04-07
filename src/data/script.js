// ================= WEBSOCKET =================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// ================= GRAPH =================
let canvas, ctx;
let dataPoints = [];

// ================= LOAD =================
window.addEventListener('load', onload);

function onload() {
    initWebSocket();

    // ambil canvas setelah halaman siap
    canvas = document.getElementById("chart");
    ctx = canvas.getContext("2d");

    canvas.width = 300;
    canvas.height = 200;
}

// ================= WEBSOCKET INIT =================
function initWebSocket() {
    console.log('Opening WebSocket...');
    websocket = new WebSocket(gateway);

    websocket.onopen = () => console.log("Connected");

    websocket.onclose = () => {
        console.log("Disconnected");
        setTimeout(initWebSocket, 2000);
    };

    websocket.onmessage = onMessage;
}

// ================= TERIMA DATA SENSOR =================
function onMessage(event) {

    let value = parseInt(event.data);

    // update teks jarak
    let d = document.getElementById('distance');
    d.innerHTML = value;

    // animasi angka
    d.style.transform = "scale(1.2)";
    setTimeout(() => {
        d.style.transform = "scale(1)";
    }, 150);

    // simpan data grafik
    dataPoints.push(value);
    if (dataPoints.length > 30) {
        dataPoints.shift();
    }

    drawGraph();
}

// ================= GAMBAR GRAFIK =================
function drawGraph() {

    if (!ctx) return;

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    ctx.beginPath();
    ctx.strokeStyle = "#00ffff";
    ctx.lineWidth = 2;

    let max = 100; // batas jarak (bisa kamu ubah)

    for (let i = 0; i < dataPoints.length; i++) {

        let x = (i / 30) * canvas.width;
        let y = canvas.height - (dataPoints[i] / max * canvas.height);

        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
    }

    ctx.stroke();
}

// ================= RGB CONTROL =================
function updateRGB() {

    const r = document.getElementById("sliderRed").value;
    const g = document.getElementById("sliderGreen").value;
    const b = document.getElementById("sliderBlue").value;

    // update angka
    document.getElementById("valueRed").value = r;
    document.getElementById("valueGreen").value = g;
    document.getElementById("valueBlue").value = b;

    // ambil bar
    let redBar = document.getElementById("barRed");
    let greenBar = document.getElementById("barGreen");
    let blueBar = document.getElementById("barBlue");

    // panjang bar
    redBar.style.width = (r / 255 * 100) + "%";
    greenBar.style.width = (g / 255 * 100) + "%";
    blueBar.style.width = (b / 255 * 100) + "%";

    // efek terang gelap
    redBar.style.filter = `brightness(${0.3 + r/255})`;
    greenBar.style.filter = `brightness(${0.3 + g/255})`;
    blueBar.style.filter = `brightness(${0.3 + b/255})`;

    // kirim ke ESP32
    let msg = `RGB:${r},${g},${b}`;

    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(msg);
    }
}

// ================= EVENT SLIDER =================
["sliderRed", "sliderGreen", "sliderBlue"].forEach(id => {
    document.getElementById(id).addEventListener("input", updateRGB);
});

// ================= INIT =================
updateRGB();