let canvasWidth = 500;
let canvasHeight = 500;
let spriteSize = 20;
let foodSize = 10;
let spriteX, spriteY, foodX, foodY;
let echolocation = false;
let echolocationRadius = 0;
let speed = 10;
let score = 0;
let foodVisible = false;

let isPressed = false;

// Serial
let pHtmlMsg;
let serialOptions = { baudRate: 115200  };
let serial;

// 2nd dimension of perlin noise - controls the horizontal position
let yNoise = 0.0; // 2nd dimension of perlin noise
let backgroundSound;
let echoSFX;

function preload() {
  soundFormats('mp3');
  backgroundSound = loadSound('ocean-waves.mp3');
  echoSFX = loadSound('radar.mp3');
}

function setup() {
  createCanvas(canvasWidth, canvasHeight);
  
  // Setup Web Serial using serial.js
  serial = new Serial();
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  // If we have previously approved ports, attempt to connect with them
  serial.autoConnectAndOpenPreviouslyApprovedPort(serialOptions);

  // Add in a lil <p> element to provide messages. This is optional
  pHtmlMsg = createP("Click anywhere on this page to open the serial connection dialog");
  pHtmlMsg.style('color', 'deeppink');
  
  reset();
  backgroundSound.loop();
}

function draw() {
  background(239, 226, 200);
  
  /** beach background setup**/
  fill(118, 170, 206);
  stroke("white");
  strokeWeight(2);
  // We are going to draw a polygon out of the wave points
  beginShape();

  let xNoise = 0; 

  // Iterate over horizontal pixels
  for (let x = 0; x <= width; x += 10) {
    // Calculate a y value according to noise, map to

    let y = map(noise(xNoise, yNoise), 0, 1, 10, 110);

    // Set the vertex
    vertex(x, y);
    // Increment x dimension for noise
    xNoise += 0.05;
  }
  // increment y dimension for noise
  yNoise += 0.01;
  vertex(width, height);
  vertex(0, height);
  endShape(CLOSE);

  // Draw food only if it is visible
  if (foodVisible) {
    fill(255, 0, 0);
    noStroke();
    ellipse(foodX, foodY, foodSize, foodSize);
  }
    
  // check input
  if (isPressed) {
    echolocate();
  } 

  // Draw sprite
  fill(0, 0, 255);
  noStroke();
  circle(spriteX, spriteY, spriteSize);
  
  // Check if the sprite is close to the food
  let distance = dist(spriteX, spriteY, foodX, foodY);
  if (distance < spriteSize / 2 + foodSize / 2) {
    foodFound();
    foodVisible = false;
  }

  // Draw echolocation effect
  if (echolocation) {
    noFill();
    stroke(255);
    ellipse(spriteX, spriteY, echolocationRadius * 2, echolocationRadius * 2);
    echolocationRadius += 5; // Adjust speed of animation
    if (echolocationRadius > canvasWidth) {
      echolocation = false;
      echolocationRadius = 0;
      foodVisible = true;
    }
  }
  
  // check sprite bounds
  if (spriteX > width- 2*spriteSize) {
    spriteX = width;
  } else if (spriteX < 0) {
    spriteX = 0;
  }
  
  if (spriteY > height- 2*spriteSize) {
    spriteY = height;
  } else if (spriteY < 0) {
    spriteY = 0;
  } 
  
  drawScore();
}

// function keyPressed() {
//   if (keyCode === ENTER) {
//     echolocate();
//   } else if (keyCode === LEFT_ARROW) {
//     spriteX -= speed;
//   } else if (keyCode === RIGHT_ARROW) {
//     spriteX += speed;
//   } else if (keyCode === UP_ARROW) {
//     spriteY -= speed;
//   } else if (keyCode === DOWN_ARROW) {
//     spriteY += speed;
//   }
// }

function echolocate() {
  echolocation = true;
  echoSFX.play();
  
  for (let i = 0; i < canvasWidth; i += 10) {
    if (dist(i, 0, spriteX, spriteY) < echolocationRadius && dist(i, 0, foodX, foodY) < echolocationRadius) {
      foodFound();
      break;
    }
  }
}

function foodFound() {
  score++;
  resetFood();
}

function resetFood() {
  foodX = random(foodSize, canvasWidth - foodSize);
  foodY = random(foodSize, canvasHeight - foodSize);
}

function reset() {
  spriteX = random(spriteSize, canvasWidth - spriteSize);
  spriteY = random(spriteSize, canvasHeight - spriteSize);
  resetFood();
  score = 0;
}

function drawScore() {
  fill(0);
  noStroke();
  text(`Score: ${score}`, 10, 20);
}

/**
 * Callback function by serial.js when there is an error on web serial
 * 
 * @param {} eventSender 
 */
 function onSerialErrorOccurred(eventSender, error) {
  console.log("onSerialErrorOccurred", error);
  pHtmlMsg.html(error);
}

/**
 * Callback function by serial.js when web serial connection is opened
 * 
 * @param {} eventSender 
 */
function onSerialConnectionOpened(eventSender) {
  console.log("onSerialConnectionOpened");
  pHtmlMsg.html("Serial connection opened successfully");
}

/**
 * Callback function by serial.js when web serial connection is closed
 * 
 * @param {} eventSender 
 */
function onSerialConnectionClosed(eventSender) {
  console.log("onSerialConnectionClosed");
  pHtmlMsg.html("onSerialConnectionClosed");
}

/**
 * Callback function serial.js when new web serial data is received
 * 
 * @param {*} eventSender 
 * @param {String} newData new data received over serial
 */
function onSerialDataReceived(eventSender, newData) {
  let data = split(newData, ",");
  let xData = parseInt(data[0]);
  let yData = parseInt(data[1]);
  
  spriteX = map(xData, 0, 1023, 0, width);
  spriteY = map(yData, 0, 1023, height, 0);
  
  if (parseInt(data[2]) == 0) {
    isPressed = true;
  } else {
    isPressed = false;
  }
  
  // console.log("onSerialDataReceived", newData);
  pHtmlMsg.html("onSerialDataReceived: " + xData + ", y: " + yData + ", button: " + isPressed);
}

/**
 * Called automatically by the browser through p5.js when mouse clicked
 */
function mouseClicked() {
  if (!serial.isOpen()) {
    serial.connectAndOpen(null, serialOptions);
  }
}
