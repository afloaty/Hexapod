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
        h1, h2, h3 {
            color: #ddd;
        }
        .controls {
            margin: 30px auto;
            padding: 20px;
            background-color: #222;
            border-radius: 10px;
            box-shadow: 0 2px 5px rgba(255,255,255,0.1);
            max-width: 800px;
        }
        .button {
            width: 100%;
            padding: 12px 24px;
            margin: 0;
            box-sizing: border-box;
            font-size: 16px;
            border: none;
            border-radius: 5px;
            background-color: #4CAF50;
            color: white;
            cursor: pointer;
            transition: background-color 0.3s, box-shadow 0.3s;
            line-height: 1.2;
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
        }
        .posture-controls {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            margin-top: 20px;
            max-width: 300px;
        }
        .indicators-container {
            margin: 30px auto;
            padding: 20px;
            background-color: #222;
            border-radius: 10px;
            box-shadow: 0 2px 5px rgba(255,255,255,0.1);
            max-width: 600px;
        }
        .indicator {
            margin: 10px 0;
            font-weight: bold;
            color: #bbb;
        }
        .status-box {
            display: inline-block;
            padding: 5px 10px;
            border: 1px solid #fff;
            border-radius: 5px;
            margin-left: 10px;
        }
        .status-idle { color: yellow; }
        .status-success { color: green; }
        .status-failed { color: red; }
        .status-error { color: red; }
        .latency-excellent { color: green; }
        .latency-good { color: yellow; }
        .latency-fair { color: orange; }
        .latency-poor { color: red; }
        .latency-error { color: red; }
        input[type="range"] {
            -webkit-appearance: none;
            width: 150px;
            height: 10px;
            background: #ddd;
            border-radius: 5px;
            outline: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 20px;
            height: 20px;
            background: #4CAF50;
            border-radius: 50%;
            cursor: pointer;
        }
        .settings {
            margin-top: 20px;
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
        .control {
            display: flex;
            align-items: center;
        }
        .control label {
            width: 150px;
        }
        .control input[type="range"] {
            flex: 1;
            margin-right: 10px;
        }
        .control input[type="number"] {
            width: 60px;
        }
    </style>
</head>
<body>
    <h1>Hexapod Control Interface</h1>

    <!-- Movement and Speed Controls -->
    <div class="controls">
        <h2>Movement and Speed Control</h2>
        <p>Use W, A, S, D for movement, Q and E for rotation. Press Z for Sit and X for Stand. Adjust speed with slider or +, -, or R keys.</p>
        <div style="display: flex; justify-content: space-around; align-items: flex-start;">
            <div>
                <div class="movement-grid">
                    <button id="ccw" class="button" onclick="sendCommand('/rotate/ccw')">CCW<br>(Q)</button>
                    <button id="forward" class="button" onclick="sendCommand('/move/forward')">Forward<br>(W)</button>
                    <button id="cw" class="button" onclick="sendCommand('/rotate/cw')">CW<br>(E)</button>
                    <button id="left" class="button" onclick="sendCommand('/move/left')">Left<br>(A)</button>
                    <div></div>
                    <button id="right" class="button" onclick="sendCommand('/move/right')">Right<br>(D)</button>
                    <div></div>
                    <button id="backward" class="button" onclick="sendCommand('/move/backward')">Backward<br>(S)</button>
                    <div></div>
                </div>
                <div class="posture-controls">
                    <button id="sit" class="button" onclick="sendCommand('/sit')">Sit<br>(Z)</button>
                    <div></div>
                    <button id="stand" class="button" onclick="sendCommand('/stand')">Stand<br>(X)</button>
                </div>
            </div>
            <div class="settings">
                <div class="control">
                    <label for="speedSlider">Speed Multiplier</label>
                    <input type="range" id="speedSlider" min="0" max="2" step="0.1" value="1.0">
                    <input type="number" id="speedInput" min="0" max="2" step="0.1" value="1.0">
                </div>
                <div class="control">
                    <label for="groundClearanceSlider">Ground Clearance</label>
                    <input type="range" id="groundClearanceSlider" min="0" max="100" step="1" value="50">
                    <input type="number" id="groundClearanceInput" min="0" max="100" step="1" value="50">
                </div>
                <div class="control">
                    <label for="stepRadiusSlider">Step Radius</label>
                    <input type="range" id="stepRadiusSlider" min="0" max="200" step="1" value="100">
                    <input type="number" id="stepRadiusInput" min="0" max="200" step="1" value="100">
                </div>
            </div>
        </div>
    </div>

    <!-- Indicators -->
    <div class="indicators-container">
        <div class="indicator">
            Connection Status: <span id="connectionStatus" class="status-box status-idle">Idle</span>
        </div>
        <div class="indicator">
            Latency: <span id="latencyIndicator" class="status-box">Measuring...</span>
        </div>
    </div>

    <script>
        let activeButton = null;
        let isMoving = false;

        function sendCommand(endpoint) {
            return fetch(endpoint)
                .then(response => {
                    const statusSpan = document.getElementById('connectionStatus');
                    if (response.ok) {
                        statusSpan.innerText = 'Success';
                        statusSpan.className = 'status-box status-success';
                    } else {
                        statusSpan.innerText = 'Failed';
                        statusSpan.className = 'status-box status-failed';
                    }
                    if (endpoint === '/move/stop') {
                        fetch('/led/off');
                    } else if (endpoint !== '/led/on' && endpoint !== '/led/off') {
                        fetch('/led/on');
                    }
                    return response.text();
                })
                .catch(() => {
                    const statusSpan = document.getElementById('connectionStatus');
                    statusSpan.innerText = 'Error';
                    statusSpan.className = 'status-box status-error';
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
            const key = event.key.toLowerCase();
            if (['w', 'a', 's', 'd', 'q', 'e'].includes(key)) {
                if (!isMoving) {
                    isMoving = true;
                    switch (key) {
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
            } else if (key === 'z') {
                sendCommand('/sit');
                setActiveButton('sit');
                setTimeout(() => {
                    if (activeButton && activeButton.id === 'sit') {
                        activeButton.classList.remove('active');
                        activeButton = null;
                    }
                }, 300);
            } else if (key === 'x') {
                sendCommand('/stand');
                setActiveButton('stand');
                setTimeout(() => {
                    if (activeButton && activeButton.id === 'stand') {
                        activeButton.classList.remove('active');
                        activeButton = null;
                    }
                }, 300);
            } else if (key === '+' || key === '=') {
                sendCommand('/speed/increase').then(() => updateSpeedDisplay());
            } else if (key === '-' || key === '_') {
                sendCommand('/speed/decrease').then(() => updateSpeedDisplay());
            } else if (key === 'r') {
                sendCommand('/speed/reset').then(() => updateSpeedDisplay());
            }
        });

        document.addEventListener('keyup', () => {
            if (isMoving) {
                isMoving = false;
                stopMovement();
            }
        });

        // Movement button event listeners
        const movementButtons = document.querySelectorAll('.movement-grid .button');
        movementButtons.forEach(button => {
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

        // Posture button event listeners
        const postureButtons = document.querySelectorAll('.posture-controls .button');
        postureButtons.forEach(button => {
            button.addEventListener('click', () => {
                setActiveButton(button.id);
                setTimeout(() => {
                    if (activeButton && activeButton.id === button.id) {
                        activeButton.classList.remove('active');
                        activeButton = null;
                    }
                }, 300);
            });
        });

        // Speed control event listeners
        const speedSlider = document.getElementById('speedSlider');
        const speedInput = document.getElementById('speedInput');
        speedSlider.addEventListener('input', function() {
            speedInput.value = this.value;
        });
        speedSlider.addEventListener('change', function() {
            fetch(`/speed/set?value=${this.value}`)
                .then(() => updateSpeedDisplay());
        });
        speedInput.addEventListener('change', function() {
            let value = parseFloat(this.value);
            if (!isNaN(value)) {
                value = Math.min(Math.max(value, 0), 2);
                speedSlider.value = value;
                this.value = value.toFixed(1);
                fetch(`/speed/set?value=${value}`)
                    .then(() => updateSpeedDisplay());
            } else {
                this.value = speedSlider.value;
            }
        });

        // Ground Clearance control event listeners
        const groundClearanceSlider = document.getElementById('groundClearanceSlider');
        const groundClearanceInput = document.getElementById('groundClearanceInput');
        groundClearanceSlider.addEventListener('input', function() {
            groundClearanceInput.value = this.value;
        });
        groundClearanceSlider.addEventListener('change', function() {
            fetch(`/groundClearance/set?value=${this.value}`)
                .then(() => updateGroundClearanceDisplay());
        });
        groundClearanceInput.addEventListener('change', function() {
            let value = parseFloat(this.value);
            if (!isNaN(value)) {
                value = Math.min(Math.max(value, 0), 100);
                groundClearanceSlider.value = value;
                this.value = value.toFixed(0);
                fetch(`/groundClearance/set?value=${value}`)
                    .then(() => updateGroundClearanceDisplay());
            } else {
                this.value = groundClearanceSlider.value;
            }
        });

        // Step Radius control event listeners
        const stepRadiusSlider = document.getElementById('stepRadiusSlider');
        const stepRadiusInput = document.getElementById('stepRadiusInput');
        stepRadiusSlider.addEventListener('input', function() {
            stepRadiusInput.value = this.value;
        });
        stepRadiusSlider.addEventListener('change', function() {
            fetch(`/stepRadius/set?value=${this.value}`)
                .then(() => updateStepRadiusDisplay());
        });
        stepRadiusInput.addEventListener('change', function() {
            let value = parseFloat(this.value);
            if (!isNaN(value)) {
                value = Math.min(Math.max(value, 0), 200);
                stepRadiusSlider.value = value;
                this.value = value.toFixed(0);
                fetch(`/stepRadius/set?value=${value}`)
                    .then(() => updateStepRadiusDisplay());
            } else {
                this.value = stepRadiusSlider.value;
            }
        });

        // Function to update speed display
        function updateSpeedDisplay() {
            fetch('/speed/get')
                .then(response => response.text())
                .then(text => {
                    const value = parseFloat(text);
                    if (!isNaN(value)) {
                        speedSlider.value = value;
                        speedInput.value = value.toFixed(1);
                    }
                });
        }

        // Function to update ground clearance display
        function updateGroundClearanceDisplay() {
            fetch('/groundClearance/get')
                .then(response => response.text())
                .then(text => {
                    const value = parseFloat(text);
                    if (!isNaN(value)) {
                        groundClearanceSlider.value = value;
                        groundClearanceInput.value = value.toFixed(0);
                    }
                });
        }

        // Function to update step radius display
        function updateStepRadiusDisplay() {
            fetch('/stepRadius/get')
                .then(response => response.text())
                .then(text => {
                    const value = parseFloat(text);
                    if (!isNaN(value)) {
                        stepRadiusSlider.value = value;
                        stepRadiusInput.value = value.toFixed(0);
                    }
                });
        }

        // Latency measurement
        function measureLatency() {
            const startTime = Date.now();
            fetch('/ping?time=' + startTime)
                .then(response => response.text())
                .then(() => {
                    const endTime = Date.now();
                    const latency = endTime - startTime;
                    let status = '';
                    let statusClass = '';
                    if (latency < 100) {
                        status = 'Excellent';
                        statusClass = 'latency-excellent';
                    } else if (latency < 300) {
                        status = 'Good';
                        statusClass = 'latency-good';
                    } else if (latency < 1000) {
                        status = 'Fair';
                        statusClass = 'latency-fair';
                    } else {
                        status = 'Poor';
                        statusClass = 'latency-poor';
                    }
                    const latencySpan = document.getElementById('latencyIndicator');
                    latencySpan.innerText = `${latency}ms (${status})`;
                    latencySpan.className = `status-box ${statusClass}`;
                })
                .catch(() => {
                    const latencySpan = document.getElementById('latencyIndicator');
                    latencySpan.innerText = 'Error';
                    latencySpan.className = 'status-box latency-error';
                });
        }

        // Periodic updates
        setInterval(() => {
            measureLatency();
            updateSpeedDisplay();
            updateGroundClearanceDisplay();
            updateStepRadiusDisplay();
        }, 5000);

        // Initial updates
        measureLatency();
        updateSpeedDisplay();
        updateGroundClearanceDisplay();
        updateStepRadiusDisplay();
    </script>
</body>
</html>
)rawliteral";