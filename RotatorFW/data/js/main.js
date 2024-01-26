"use strict";

var debug = true; // FALSE by default, disables "Trying to reconnect message;"

// the outer part of the compass that rotates
var rose = document.getElementById("rose");
var azimuthIndicator = document.getElementById("azimuthIndicator");
var targetIndicator = document.getElementById("targetIndicator");

var compassContainer = document.querySelector(".compass");

// elements that ouput our position
var currentAzimuth = document.getElementById("currentAzimuth");
var targetAzimuth = document.getElementById("targetAzimuth");
var rotationSpeed = document.getElementById("rotationSpeed");

// info popup elements, push buttons that open popups
var popup = document.getElementById("popup");
var popupContents = document.getElementById("popup-contents");
var popupInners = document.querySelectorAll(".popup__inner");
var btnsPopup = document.querySelectorAll(".btn-popup");

// buttons at the bottom of the screen
var btnLockOrientation = document.getElementById(
  "btn-lock-orientation"
);
var btnNightmode = document.getElementById("btn-nightmode");
var btnInfo = document.getElementById("btn-info");

// if we have shown the heading unavailable warning yet
var warningHeadingShown = false;

// switches keeping track of our current app state
var isOrientationLockable = false;
var isOrientationLocked = false;
var isNightMode = false;

// the orientation of the device on app load
var defaultOrientation;

// browser agnostic orientation
function getBrowserOrientation() {
  var orientation;
  if (screen.orientation && screen.orientation.type) {
    orientation = screen.orientation.type;
  } else {
    orientation =
      screen.orientation ||
      screen.mozOrientation ||
      screen.msOrientation;
  }

  /*
    'portait-primary':      for (screen width < screen height, e.g. phone, phablet, small tablet)
                                device is in 'normal' orientation
                            for (screen width > screen height, e.g. large tablet, laptop)
                                device has been turned 90deg clockwise from normal
 
    'portait-secondary':    for (screen width < screen height)
                                device has been turned 180deg from normal
                            for (screen width > screen height)
                                device has been turned 90deg anti-clockwise (or 270deg clockwise) from normal
 
    'landscape-primary':    for (screen width < screen height)
                                device has been turned 90deg clockwise from normal
                            for (screen width > screen height)
                                device is in 'normal' orientation
 
    'landscape-secondary':  for (screen width < screen height)
                                device has been turned 90deg anti-clockwise (or 270deg clockwise) from normal
                            for (screen width > screen height)
                                device has been turned 180deg from normal
    */

  return orientation;
}

// browser agnostic orientation unlock
function browserUnlockOrientation() {
  if (screen.orientation && screen.orientation.unlock) {
    screen.orientation.unlock();
  } else if (screen.unlockOrientation) {
    screen.unlockOrientation();
  } else if (screen.mozUnlockOrientation) {
    screen.mozUnlockOrientation();
  } else if (screen.msUnlockOrientation) {
    screen.msUnlockOrientation();
  }
}

// browser agnostic document.fullscreenElement
function getBrowserFullscreenElement() {
  if (typeof document.fullscreenElement !== "undefined") {
    return document.fullscreenElement;
  } else if (typeof document.webkitFullscreenElement !== "undefined") {
    return document.webkitFullscreenElement;
  } else if (typeof document.mozFullScreenElement !== "undefined") {
    return document.mozFullScreenElement;
  } else if (typeof document.msFullscreenElement !== "undefined") {
    return document.msFullscreenElement;
  }
}

// browser agnostic document.documentElement.requestFullscreen
function browserRequestFullscreen() {
  if (document.documentElement.requestFullscreen) {
    document.documentElement.requestFullscreen();
  } else if (document.documentElement.webkitRequestFullscreen) {
    document.documentElement.webkitRequestFullscreen();
  } else if (document.documentElement.mozRequestFullScreen) {
    document.documentElement.mozRequestFullScreen();
  } else if (document.documentElement.msRequestFullscreen) {
    document.documentElement.msRequestFullscreen();
  }
}

// browser agnostic document.documentElement.exitFullscreen
function browserExitFullscreen() {
  if (document.exitFullscreen) {
    document.exitFullscreen();
  } else if (document.webkitExitFullscreen) {
    document.webkitExitFullscreen();
  } else if (document.mozCancelFullScreen) {
    document.mozCancelFullScreen();
  } else if (document.msExitFullscreen) {
    document.msExitFullscreen();
  }
}

function showHeadingWarning() {
  if (!warningHeadingShown) {
    popupOpen("noorientation");
    warningHeadingShown = true;
  }
}

function onFullscreenChange() {
  if (isOrientationLockable && getBrowserFullscreenElement()) {
    if (screen.orientation && screen.orientation.lock) {
      screen.orientation
        .lock(getBrowserOrientation())
        .then(function () { })
        .catch(function () { });
    }
  } else {
    lockOrientationRequest(false);
  }
}

function toggleOrientationLockable(lockable) {
  isOrientationLockable = lockable;

  if (isOrientationLockable) {
    btnLockOrientation.classList.remove("btn--hide");

    btnNightmode.classList.add("column-25");
    btnNightmode.classList.remove("column-33");
    btnInfo.classList.add("column-25");
    btnInfo.classList.remove("column-33");
  } else {
    btnLockOrientation.classList.add("btn--hide");

    btnNightmode.classList.add("column-33");
    btnNightmode.classList.remove("column-25");
    btnInfo.classList.add("column-33");
    btnInfo.classList.remove("column-25");
  }
}

function checkLockable() {
  if (screen.orientation && screen.orientation.lock) {
    screen.orientation
      .lock(getBrowserOrientation())
      .then(function () {
        toggleOrientationLockable(true);
        browserUnlockOrientation();
      })
      .catch(function (event) {
        if (event.code === 18) {
          // The page needs to be fullscreen in order to call lockOrientation(), but is lockable
          toggleOrientationLockable(true);
          browserUnlockOrientation(); //needed as chrome was locking orientation (even if not in fullscreen, bug??)
        } else {
          // lockOrientation() is not available on this device (or other error)
          toggleOrientationLockable(false);
        }
      });
  } else {
    toggleOrientationLockable(false);
  }
}

function lockOrientationRequest(doLock) {
  if (isOrientationLockable) {
    if (doLock) {
      browserRequestFullscreen();
      lockOrientation(true);
    } else {
      browserUnlockOrientation();
      browserExitFullscreen();
      lockOrientation(false);
    }
  }
}

function lockOrientation(locked) {
  if (locked) {
    btnLockOrientation.classList.add("active");
  } else {
    btnLockOrientation.classList.remove("active");
  }

  isOrientationLocked = locked;
}

function toggleOrientationLock() {
  if (isOrientationLockable) {
    lockOrientationRequest(!isOrientationLocked);
  }
}

function setNightmode(on) {
  if (on) {
    btnNightmode.classList.add("active");
  } else {
    btnNightmode.classList.remove("active");
  }

  window.setTimeout(function () {
    if (on) {
      document.documentElement.classList.add("nightmode");
    } else {
      document.documentElement.classList.remove("nightmode");
    }
  }, 1);

  isNightMode = on;
}

function toggleNightmode() {
  setNightmode(!isNightMode);
}

function popupOpenFromClick(event) {
  popupOpen(event.currentTarget.dataset.name);
}

function popupOpen(name) {
  var i;
  for (i = 0; i < popupInners.length; i++) {
    popupInners[i].classList.add("popup__inner--hide");
  }
  document
    .getElementById("popup-inner-" + name)
    .classList.remove("popup__inner--hide");

  popup.classList.add("popup--show");
}

function popupClose() {
  popup.classList.remove("popup--show");
}

function popupContentsClick(event) {
  event.stopPropagation();
}

function decimalToSexagesimal(decimal, type) {
  var degrees = decimal | 0;
  var fraction = Math.abs(decimal - degrees);
  var minutes = (fraction * 60) | 0;
  var seconds = (fraction * 3600 - minutes * 60) | 0;

  var direction = "";
  var positive = degrees > 0;
  degrees = Math.abs(degrees);
  switch (type) {
    case "lat":
      direction = positive ? "N" : "S";
      break;
    case "lng":
      direction = positive ? "E" : "W";
      break;
  }

  return degrees + "° " + minutes + "' " + seconds + '" ' + direction;
}

if (screen.width > screen.height) {
  defaultOrientation = "landscape";
} else {
  defaultOrientation = "portrait";
}

document.addEventListener("fullscreenchange", onFullscreenChange);
document.addEventListener("webkitfullscreenchange", onFullscreenChange);
document.addEventListener("mozfullscreenchange", onFullscreenChange);
document.addEventListener("MSFullscreenChange", onFullscreenChange);

btnLockOrientation.addEventListener("click", toggleOrientationLock);
btnNightmode.addEventListener("click", toggleNightmode);

var i;
for (i = 0; i < btnsPopup.length; i++) {
  btnsPopup[i].addEventListener("click", popupOpenFromClick);
}

popup.addEventListener("click", popupClose);
popupContents.addEventListener("click", popupContentsClick);

setNightmode(true);
checkLockable();

const gateway = `ws://${window.location.hostname}/ws`;
//const gateway = `ws://192.168.1.249/ws`;

let socket = new WebSocket(gateway);
let messageTimeout;

function resetMessageTimeout() {
  clearTimeout(messageTimeout);
  messageTimeout = setTimeout(() => {
    console.log("Connection timeout, closing and reconnecting...");
    if (debug == false) {
      popupOpen("lostconnction");
    }

    socket.close();
    setupWebSocket();
  }, 5000);
}

function setupWebSocket() {
  resetMessageTimeout();
  socket = new WebSocket(gateway);

  socket.onopen = () => {
    console.log("WebSocket connection opened.");
    popupClose();
    resetMessageTimeout();
  };

  socket.onmessage = (event) => {
    //console.log("Got new data: " + event.data);
    const data = JSON.parse(event.data);
    if (
      data.current_position !== undefined &&
      data.target_position !== undefined
    ) {
      const { current_position, target_position, current_speed } = data;

      currentAzimuth.textContent = current_position + "°";
      targetAzimuth.textContent = target_position + "°";
      rotationSpeed.textContent = current_speed + "°";

      rotateIndicator(azimuthIndicator, current_position);
      if (!mousepressed)
        rotateIndicator(targetIndicator, target_position);
    }
    resetMessageTimeout();
  };
}
setupWebSocket();

function rotateIndicator(indicator, angle) {
  const indicatorAngle = `rotate(${angle}deg)`;

  if (typeof indicator.style.transform !== "undefined") {
    indicator.style.transform = "rotateZ(" + angle + "deg)";
  } else if (
    typeof azimuthIndicator.style.webkitTransform !== "undefined"
  ) {
    indicator.style.webkitTransform = "rotateZ(" + angle + "deg)";
  }
}

function calculateAzimuth(event) {
  var boundingBox = compassContainer.getBoundingClientRect();
  var centerX = boundingBox.left + boundingBox.width / 2;
  var centerY = boundingBox.top + boundingBox.height / 2;

  var angle = Math.atan2(
    event.clientY - centerY,
    event.clientX - centerX
  );
  var azimuth = (angle * (180 / Math.PI) + 360 + 90) % 360;
  return azimuth;
}

function handleMove(event) {
  if (
    (event.buttons === 1 && event.type === "mousemove") ||
    event.type === "click" ||
    event.type === "touchstart" ||
    event.type === "touchmove"
  ) {
    var azimuth = calculateAzimuth(event);
    if (event.type != "mousemove") {
      const message = {
        target_position: parseInt(azimuth, 10),
      };
      socket.send(JSON.stringify(message));
    } else rotateIndicator(targetIndicator, azimuth);
  }
}

compassContainer.addEventListener("click", handleMove);
compassContainer.addEventListener("mousemove", handleMove);

var mousepressed;
document.addEventListener("mousedown", function (event) {
  if (event.button === 0) {
    mousepressed = true;
  }
});

document.addEventListener("mouseup", function (event) {
  if (event.button === 0) {
    mousepressed = false;
  }
});

const darkModeToggle = document.getElementById("darkModeToggle");