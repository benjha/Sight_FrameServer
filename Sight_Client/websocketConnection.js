/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */


var output;
var canvas;
var pixels;
var wsUri	= "ws://localhost:9002/";
var reader	= new FileReader();
var ctx;
var imgdata;
var stop 	= false;
var mouseDownFlag = false;
// Enables/disables JPEG compression. 
// JPEG compression should be enabled/disabled in the server in cBroadcastServer.h
var jpegCompression = true;

//var imageheight = 512;
//var imagewidth	= 512;

function canvasResize ()
{
console.log (canvas.width + ", " + canvas.height);
}

function init() 
{ 
	output = document.getElementById("output"); 

	// cheking for websocket support
	if (window.WebSocket) {
		connectAndCallbacks();
	}
	else
	{
		console.log ("This browser does not support Websocket.");
	}

	
	// adding mouse events
	canvas = document.getElementById('canvas');

	// Some of invoked functions are declared in peripherals.js
	// For every new UI widget, its corresponding function should 
	// be declared in peripherals.js
	canvas.addEventListener( "contextmenu", preventDefaultHandler,	true );
	canvas.addEventListener( "mousedown", 	mouseDownHandler,	true );
	canvas.addEventListener( "mouseup",	mouseUpHandler, 	true );
	canvas.addEventListener( "mousemove", 	mouseMoveHandler, 	true );
	canvas.addEventListener( "mousewheel", 	mouseWheelHandler, 	true );
	canvas.addEventListener( "mousewheel", 	mouseWheelHandler, 	true );
	canvas.addEventListener( "mouseout", 	mouseOutHandler, 	true );

	canvas.addEventListener( "resize", 	canvasResize, 	false );

	document.addEventListener("keypress", 	keyDownHandler, 	false );
	//document.addEventListener("keyup", 	keyUpHandler, 		false );

	ctx 	= canvas.getContext('2d');
	imgdata = ctx.getImageData(0,0,canvas.width,canvas.height);

	pixels	= new Image ();
	pixels.addEventListener('load', loadPixels, true );
}

function loadPixels ()
{
	ctx.drawImage(pixels, 0, 0);
//	console.log ("Load pixels");
}

function connectAndCallbacks ()
{
	websocket		= new WebSocket(wsUri); 
	websocket.onopen 	= function(evt) { onOpen	(evt)	}; 
	websocket.onclose 	= function(evt) { onClose	(evt)	}; 
	websocket.onmessage	= function(evt) { onMessage	(evt)	}; 
	websocket.onerror 	= function(evt) { onError	(evt)	};

	// sets websocket's binary messages as ArrayBuffer type
	//websocket.binaryType = "arraybuffer";	
	// sets websocket's binary messages as Blob type by default is Blob type
	//websocket.binaryType = "blob";
	
	reader.onload		= function(evt) { readBlob		(evt) };
	reader.onloadend	= function(evt) { nextBlob		(evt) };
	reader.onerror		= function(evt) { fileReaderError 	(evt) };

	peripherals		= new Peripherals ();
	peripherals.init(websocket);
}


// connection has been opened
function onOpen(evt)
{
	
}

function fileReaderError (e)
{
	console.error("FileReader error. Code " + event.target.error.code);
}

// this method is launched when FileReader ends loading the blob
function nextBlob (e)
{
	if (!stop)
	{
		//var event = new CompositeStream();
		//event.appendBytes(0);     // type (u8 0)  0 identifies an event of type MESSAGE 
		//event.appendBytes ("NXTFR");
		//websocket.send (event.consume(event.length));
		websocket.send ("NXTFR");
	}
	//console.log (stop);
}

function Uint8ToString(u8a){
  var CHUNK_SZ = 0x8000;
  var c = [];
  for (var i=0; i < u8a.length; i+=CHUNK_SZ) {
    c.push(String.fromCharCode.apply(null, u8a.subarray(i, i+CHUNK_SZ)));
  }
  return c.join("");
}


// readBlob is called when reader.readAsBinaryString(blob); from onMessage method occurs
function readBlob (e)
{
	var i,j=0;

	if (jpegCompression)
	{
		var dataUrl = reader.result;
		// removing the header, keep pixels
		var img = dataUrl.split(',')[1];
		pixels.src = 'data:image/jpg;base64,' + img;
	}
	else
	{
		// comment this when using jpeg compression
		var img = new Uint8Array(reader.result);
	
		// comment this when using jpeg compression
	 	for(i=0;i<imgdata.data.length;i+=4)
		{
			imgdata.data[i]		= img[j  ] ; //.charCodeAt(j);
			imgdata.data[i+1] 	= img[j+1] ; //reader.result.charCodeAt(j+1);
			imgdata.data[i+2] 	= img[j+2] ; //reader.result.charCodeAt(j+2);
			imgdata.data[i+3] 	= 255;
			j+=3;
		}
		// comment this when using jpeg compression
		ctx.putImageData(imgdata,0,0);
	}
}

// message from the server
function onMessage(e)
{

	if (typeof e.data == "string")
	{
		console.log ("String msg: ", e, e.data);
	}
	else if (e.data instanceof Blob)
	{
		var blob = e.data;
		
		if (jpegCompression)
		{
			reader.readAsDataURL(blob);
		}
		else
		{
			reader.readAsArrayBuffer(blob);
		}		
	}
	else if (e.data instanceof ArrayBuffer)
	{
		var img = new Uint8Array(e.data); 
		console.log("Array msg:", + img[63] );
	}

}

// if there is an error
function onError (e)
{
	console.log("Websocket Error: ", e);
	// Custom functions for handling errors
	// handleErrors (e);
}

// when closing
function onClose (e)
{

	console.log ("Connection closed", e);
	
}

function startingConnection()
{
	alert('Streaming ON...');

	websocket.send ("STVIS");
	stop = false;
}

function closeConnection ( )
{
	 if (websocket.readyState === WebSocket.OPEN) 
	{
		websocket.close ();
		stop = true;
	}
}

function captureFrame ()
{
	alert('Frame Saved!.');
	websocket.send ("SAVE ");
}
    
function closingConnection()
{
    	alert('Streaming OFF...');
	websocket.send ("END  ");
	stop = true;


	
	//websocket.close ();
}


window.addEventListener("load", init, false);  


