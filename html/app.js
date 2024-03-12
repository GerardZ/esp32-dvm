
// make socket global...
socket = null;
socketLastMessage = Date.now();
timer = null;

debugHost = '192.168.0.207' // we need this foor debugging websockets, it makes sure we connect to the ESP32

window.onload = function () {
    Init();
}

function Init() {

    Init7Seg("7seg", 5);
    ConnectWs();
    setValue("7seg", 999, 4);
    //setDigit("7seg", 1, 1);
    //setDigit("7seg",2, 2);
    //setDigit("7seg",3, 3, true);
    //setDigit("7seg",0, 9);

    InitWsTimer();
}

function ConnectWs() {
    var host = location.host;

    CloseWs();

    if (location.host.startsWith('127.0.0')) { host = 'ws://' + debugHost + '/ws'; }
    else { host = 'ws://' + host + '/ws'; }

    console.debug("Host is: " + host);

    if (!(socket) || (socket.readystate != 'CONNECTING')) {
        socket = new WebSocket(host);
    }




    //if (!host.startsWith('127.0.0')) { socket = new WebSocket('ws://' + host + '/ws'); }
    //else { socket = new WebSocket('ws://' + debugHost + '/ws'); }
    socketLastMessage = Date.now();

    socket.onmessage = function (event) {
        //console.debug(event.data);
        HandleWsMessage(event);
        socketLastMessage = Date.now();
    }
    InitWsTimer();
}

function CloseWs() {
    if (socket) {
        socket.onclose = function () { };
        socket.close();
    }
    socket = null;
}

function InitWsTimer() {
    const interval = 5000;
    if (timer) {
        clearInterval(timer);
    }
    timer = setInterval(CheckWs, interval);
}

function CheckWs() {
    if ((socketLastMessage + 5000) < Date.now()) {
        console.debug("Trying to restore WebSocket connection.");
        ConnectWs();
    }
}

function button(id) {
    console.log('Button: ' + id + ' was pressed. ')
}

function toggle() {
    socket.send("toggle");
    console.debug("toggle");
}


function HandleWsMessage(event) {
    let message = JSON.parse(event.data);

    if ("value" in message) {
        setValue("7seg", message["value"], 5);
        SetBar(message["value"]);
    }
}

function SetBar(value) {
    line1 = document.getElementById("valueLine1");
    line1.setAttribute("x2", value * 400 / 20000);
    line2 = document.getElementById("valueLine1");
    line2.setAttribute("x2", value * 400 / 20000);

}


// ********* 7 Segment ***********
function Init7Seg(parentId, numdigits, scale = 800) {

    clockDotOffset = 0;
    width = numdigits * 13 * scale / 100 + clockDotOffset;
    vbWidth = numdigits * 13 + clockDotOffset;
    height = 18 * scale / 100;

    backGround = "#EEEEEE";
    segOn = "black";
    segOff = "#DDDDDD";
    suppressLeadingZero = true;

    svgString = '<div style="width:' + (width + 5) + 'px; height:' + (height + 8) + 'px; border: 4px solid ' + backGround + '; background-color:' + backGround + ';">\n';
    svgString += '<svg xmlns="http://www.w3.org/2000/svg" width="' + width + 'px" height="' + height + '" viewBox="0 0 ' + (vbWidth + 18) + ' 18">\n';
    svgString += '<g>';

    for (i = 0; i < numdigits; i++) {
        index = i;
        dpOffset = (numdigits - i - 1) * 13 + 2;

        svgString += '<g transform="translate(' + dpOffset + ',0) skewX(-5)" style="fill-rule:evenodd; stroke:' + backGround + '; stroke-width:0.3; stroke-opacity:1; stroke-linecap:butt; stroke-linejoin:miter;">\n';
        svgString += '<polygon points=" 1, 1  2, 0  8, 0   9, 1  8, 2  2, 2" fill="' + segOff + '" id="' + parentId + '_digit' + index + '_a"/>\n';
        svgString += '<polygon points=" 9, 1 10, 2 10, 8   9, 9  8, 8  8, 2" fill="' + segOff + '" id="' + parentId + '_digit' + index + '_b"/>\n';
        svgString += '<polygon points=" 9, 9 10,10 10,16   9,17  8,16  8,10" fill="' + segOff + '" id="' + parentId + '_digit' + index + '_c"/>\n';
        svgString += '<polygon points=" 9,17  8,18  2,18   1,17  2,16  8,16" fill="' + segOff + '" id="' + parentId + '_digit' + index + '_d"/>\n';
        svgString += '<polygon points=" 1,17  0,16  0,10   1, 9  2,10  2,16" fill="' + segOff + '" id="' + parentId + '_digit' + index + '_e"/>\n';
        svgString += '<polygon points=" 1, 9  0, 8  0, 2   1, 1  2, 2  2, 8" fill="' + segOff + '" id="' + parentId + '_digit' + index + '_f"/>\n';
        svgString += '<polygon points=" 1, 9  2, 8  8, 8   9, 9  8,10  2,10" fill="' + segOff + '" id="' + parentId + '_digit' + index + '_g"/>\n';
        svgString += '<circle cx="11" cy="17" r="1" fill="' + segOff + '" id="' + parentId + '_digit' + index + '_dot"/>\n';

        svgString += '</g>\n'
    }

    svgString += '</g>\n</svg>\n</div>\n';

    //console.debug(svgString);

    segParent = document.getElementById(parentId);

    // https://stackoverflow.com/questions/9723422/is-there-some-innerhtml-replacement-in-svg-xml
    // we need to parse the string first.
    var doc = new DOMParser().parseFromString(svgString, 'application/xml');
    segParent.appendChild(segParent.ownerDocument.importNode(doc.documentElement, true));
}

function setDigit(parentId, digit, value, dot = false) {
    segments = get7Segments(value);

    for (i = 0; i < 7; i++) {
        segId = parentId + '_digit' + digit + '_' + String.fromCharCode(97 + i);
        if ((segments >> i) % 2) { document.getElementById(segId).setAttribute("fill", segOn); }
        else { document.getElementById(segId).setAttribute("fill", segOff); }
    }

    if (dot) { document.getElementById(parentId + '_digit' + digit + '_dot').setAttribute("fill", segOn); }
    else { document.getElementById(parentId + '_digit' + digit + '_dot').setAttribute("fill", segOff); }
}

function get7Segments(value) {
    return [
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111, // 9
        0b01110111, // A
        0b01111100, // B
        0b00111001, // C
        0b01011110, // D
        0b01111001, // E
        0b01110001, // F
        0,          //   allOff 16
        0b01000000, // - minus 17
    ][value];
}

function setValue(parentId, value, numDigits) {
    digitOff = 16;
    digitMinus = 17;
    dot = false;


    stringVal = ('             ' + String(value))
    stringVal = stringVal.slice(stringVal.length - numDigits);
    //console.debug(stringVal);

    digitId = 0;
    for (n = 0; n < numDigits; n++) {
        digitValue = stringVal.slice(n, n + 1);
        if (digitValue == ".") { dot = true; }
        else {
            if (digitValue == '-') { setDigit(parentId, numDigits - 1 - digitId, digitMinus) }
            if (digitValue == ' ') { setDigit(parentId, numDigits - 1 - digitId, digitOff) }
            else { setDigit(parentId, numDigits - 1 - digitId, parseInt(digitValue), dot) }
            digitId++;
            dot = false;
        }
    }
}

