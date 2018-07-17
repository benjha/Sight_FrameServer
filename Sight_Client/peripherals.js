/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

// Keyboard Input message is according to  the RFB Protocol
// http://tools.ietf.org/html/rfc6143 Section 7.5.4. KeyEvent

Peripherals = function() {};

$prototype = Peripherals.prototype;

$prototype.init = function(websocket)
{
	var $this 		= this;
   // this.stream 	= new CompositeStream();
	this.buttonMask = 0;
	this.websocket	= websocket; 
}

var doKeyEvent = function doKeyEvent($this, key, downFlag) {

    var event = new CompositeStream();

    event.appendBytes(4);     // type (u8 4)
    event.appendBytes(downFlag);
    event.appendBytes(0,0);   // padding

    event.appendUint32(key);

	//console.log (key);
	console.log (event.chunks[3]);
    sendBytes($this, event.consume(event.length));
}

// For most ordinary keys, the keysym is the same as the corresponding
// ASCII value. 
// Keys as defined by <X11/keysymdef.h> in the X Window System.
// TODO: Check out (special) key values for Windows

var getKey = function getKey(e) {
    var n = e.which || e.keyCode;
    var val;

   console.log(n);

    // Partial switch of remaining keys. May require adjustment.
    switch (n) {
        case 8:
            return 0xff08;      // backspace
        case 9:
            return 0xff09;      // tab
        case 13:
            return 0xff0d;      // return
        case 16:
            return 0xffe1;      // shift
        case 17:
            return 0xffe3;      // ctrl
        case 46:
            return 0xffff;      // delete
        case 219:
            return 91;          // left square bracket
        case 220:
            return 92;          // backslash
        case 221:
            return 93;          // right square bracket
        case 222:
            return 39;          // quote
        case 187:
            return 43;          // plus
        case 188:
            return 44;          // comma
        case 189:
            return 45;          // minus
        case 190:
            return 46;          // period
        case 191:
            return 47;          // slash
        case 32:
            return 32;          // space
        case 186:
            return 58;          // colon
    }

    return n;
}

$prototype.keyDownHandler = function keyDownHandler($this, e) {
    var key = getKey(e);
    doKeyEvent(this, key, true);
}

$prototype.keyUpHandler = function keyUpHandler($this, e) {
    var key = getKey(e);
    doKeyEvent(this, key, false);
}

// Send bytes on the connection socket.
var sendBytes = function sendBytes($this, buf) 
{
	if ($this.websocket.readyState == WebSocket.OPEN )
	{
		$this.websocket.send (buf);
	}
}

// Mouse Input message is according to the RFB protocol
// http://tools.ietf.org/html/rfc6143 Section 7.5.5. PointerEvent

var doMouseEvent = function ($this, e) {
    var event = new CompositeStream();

	//console.log(event);

    event.appendBytes(5);     // type (u8 5)
    event.appendBytes($this.buttonMask);

    // position
    event.appendUint16( e.offsetX == undefined ? e.layerX : e.offsetX );
    event.appendUint16( e.offsetY == undefined ? e.layerY : e.offsetY );

    //console.log (event.chunks[2])
    sendBytes($this, event.consume(event.length));
	
}

$prototype.mouseMoveHandler = function($this, e) {
    doMouseEvent($this, e);
}

$prototype.mouseDownHandler = function($this, e) {

    // left click
    if (e.which == 1) 
	{
        $this.buttonMask ^= 1;
    } 
	// middle click
	else if (e.which == 2)
	{
		$this.buttonMask ^= (1<<1);
	}
    // right click
	else if (e.which == 3) 
	{

        $this.buttonMask ^= (1<<2);
    }
	
    doMouseEvent($this, e);
}

$prototype.mouseUpHandler = function($this, e) {
    // left click    
	if (e.which == 1) 
	{
        $this.buttonMask ^= 1;
    }
	// middle click
	else if (e.which == 2 )
	{
		$this.buttonMask ^= (1<<1);
	} 
    // right click
	else if (e.which == 3) 
	{
        $this.buttonMask ^= (1<<2);
    }
    doMouseEvent($this, e);
}

// mousewheel up and down is represented by a press and release of buttons
// 4 and 5 respectively.
$prototype.mouseWheelHandler = function($this, e) {
    if(e.wheelDelta > 0) {
        $this.buttonMask |= (1<<3);
        doMouseEvent($this, e);
        $this.buttonMask ^= (1<<3);
        doMouseEvent($this, e);
    } else {
        $this.buttonMask |= (1<<4);
        doMouseEvent($this, e);
        $this.buttonMask ^= (1<<4);
        doMouseEvent($this, e);
    }
}

$prototype.mouseOutHandler = function ($this, e)
{
	$this.buttonMask = 0;
}
//
//=======================================================================================
//

var preventDefaultHandler = function(e) {
    e.preventDefault();
    return false;
}

//
//=======================================================================================
//
var mouseDownHandler = function(e) 
{
    peripherals.mouseDownHandler	(peripherals, e);
	e.preventDefault();
   
    return false;
}

var mouseUpHandler = function(e) 
{
	e.preventDefault();  
    peripherals.mouseUpHandler		(peripherals, e);  
    return false;
}

var mouseWheelHandler = function(e) 
{
    e.preventDefault();
    peripherals.mouseWheelHandler(peripherals, e);
}

var mouseMoveHandler = function(e) 
{
	e.preventDefault();
   	peripherals.mouseMoveHandler(peripherals, e);

}

var mouseOutHandler = function(e) 
{
	e.preventDefault();
	peripherals.mouseOutHandler(peripherals, e);

}
//
//=======================================================================================
//
var keyDownHandler = function(e) {
    peripherals.keyDownHandler(peripherals, e);
    e.preventDefault();
    return false;
}

var keyUpHandler = function(e) {
    peripherals.keyUpHandler(peripherals, e);
    e.preventDefault();
    return false;
}



