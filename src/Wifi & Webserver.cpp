#include <Arduino.h>
#include <header.h>

// HTML page definition
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hexapod Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 40px;
            background-color: black;
            color: white;
        }
        h1, h2 {
            color: #ddd;
        }
        .controls {
            margin: 30px auto;
            padding: 20px;
            background-color: #222;
            border-radius: 10px;
            box-shadow: 0 2px 5px rgba(255,255,255,0.1);
            max-width: 600px;
        }
        .button {
            padding: 12px 24px;
            margin: 5px;
            font-size: 16px;
            border: none;
            border-radius: 5px;
            background-color: #4CAF50;
            color: white;
            cursor: pointer;
            transition: background-color 0.3s, box-shadow 0.3s;
        }
        .button:hover {
            background-color: #45a049;
        }
        .button.active {
            box-shadow: 0 0 10px #fff;
            background-color: #2196F3;
        }
        .movement-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            margin-top: 20px;
            max-width: 300px;
            margin-left: auto;
            margin-right: auto;
        }
        .indicator {
            margin-top: 20px;
            font-weight: bold;
            color: #bbb;
        }
    </style>
</head>
<body>
    <h1>Hexapod Control Interface</h1>

    <!-- Movement Controls -->
    <div class="controls">
        <h2>Movement Control</h2>
        <p>Use W, A, S, D for movement, Q and E for rotation. The hexapod stops when no keys are pressed.</p>
        <div class="movement-grid">
            <button id="ccw" class="button" onclick="sendCommand('/rotate/ccw')">CCW (Q)</button>
            <button id="forward" class="button" onclick="sendCommand('/move/forward')">Forward (W)</button>
            <button id="cw" class="button" onclick="sendCommand('/rotate/cw')">CW (E)</button>
            <button id="left" class="button" onclick="sendCommand('/move/left')">Left (A)</button>
            <div></div>
            <button id="right" class="button" onclick="sendCommand('/move/right')">Right (D)</button>
            <div></div>
            <button id="backward" class="button" onclick="sendCommand('/move/backward')">Backward (S)</button>
            <div></div>
        </div>
        <button id="sit" class="button" onclick="sendCommand('/sit')">Sit</button>
        <button id="stand" class="button" onclick="sendCommand('/stand')">Stand</button>
    </div>

    <!-- Indicators -->
    <div class="indicator" id="connectionStatus">Connection Status: Idle</div>
    <div class="indicator" id="latencyIndicator">Latency: Measuring...</div>

    <script>
        let activeButton = null;
        let isMoving = false;

        function sendCommand(endpoint) {
            fetch(endpoint)
                .then(response => {
                    document.getElementById('connectionStatus').innerText = 
                        response.ok ? 'Connection Status: Success' : 'Connection Status: Failed';
                    if (endpoint === '/move/stop') {
                        fetch('/led/off');
                    } else if (endpoint !== '/led/on' && endpoint !== '/led/off') {
                        fetch('/led/on');
                    }
                })
                .catch(() => {
                    document.getElementById('connectionStatus').innerText = 'Connection Status: Error';
                });
        }

        function setActiveButton(buttonId) {
            if (activeButton) {
                activeButton.classList.remove('active');
            }
            activeButton = document.getElementById(buttonId);
            if (activeButton) {
                activeButton.classList.add('active');
            }
        }

        function stopMovement() {
            console.log("Stopping movement");
            sendCommand('/move/stop');
            if (activeButton) {
                activeButton.classList.remove('active');
                activeButton = null;
            }
        }

        // Keyboard event listeners
        document.addEventListener('keydown', (event) => {
            if (!isMoving) {
                isMoving = true;
                switch (event.key.toLowerCase()) {
                    case 'w':
                        sendCommand('/move/forward');
                        setActiveButton('forward');
                        break;
                    case 's':
                        sendCommand('/move/backward');
                        setActiveButton('backward');
                        break;
                    case 'a':
                        sendCommand('/move/left');
                        setActiveButton('left');
                        break;
                    case 'd':
                        sendCommand('/move/right');
                        setActiveButton('right');
                        break;
                    case 'q':
                        sendCommand('/rotate/ccw');
                        setActiveButton('ccw');
                        break;
                    case 'e':
                        sendCommand('/rotate/cw');
                        setActiveButton('cw');
                        break;
                }
            }
        });

        document.addEventListener('keyup', () => {
            isMoving = false;
            stopMovement();
        });

        // Button event listeners
        const buttons = document.querySelectorAll('.button');
        buttons.forEach(button => {
            button.addEventListener('mousedown', () => {
                const endpoint = button.getAttribute('onclick').match(/'([^']+)'/)[1];
                sendCommand(endpoint);
                setActiveButton(button.id);
            });
            button.addEventListener('mouseup', stopMovement);
            button.addEventListener('mouseleave', () => {
                if (activeButton && activeButton.id === button.id) {
                    stopMovement();
                }
            });
        });

        // Latency measurement
        function measureLatency() {
            const startTime = Date.now();
            fetch('/ping?time=' + startTime)
                .then(response => response.text())
                .then(() => {
                    const endTime = Date.now();
                    const latency = endTime - startTime;
                    let status = '';
                    if (latency < 100) status = 'Excellent';
                    else if (latency < 300) status = 'Good';
                    else if (latency < 1000) status = 'Fair';
                    else status = 'Poor';
                    document.getElementById('latencyIndicator').innerText = `Latency: ${latency}ms (${status})`;
                })
                .catch(() => {
                    document.getElementById('latencyIndicator').innerText = 'Latency: Error';
                });
        }

        setInterval(measureLatency, 5000);
        measureLatency();
    </script>
</body>
</html>
)rawliteral";