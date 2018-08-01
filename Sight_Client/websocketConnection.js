/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
var output;
var canvas;
var canvasH264;
var jpegImg;
var wsUri	= "ws://localhost:9002/";
var reader	= new FileReader();
var ctx;
var imgdata;
var stop 	= false;
var mouseDownFlag = false;
// Enables/disables JPEG compression. 
// JPEG compression should be enabled/disabled in the server in cBroadcastServer.h
var jpegCompression = false;
var h264Compression = true;
var noCompression   = false;
var playerH264; 

//var imageheight = 512;
//var imagewidth	= 512;

function canvasResize ()
{
console.log (canvas.width + ", " + canvas.height);
}

function addMyListeners (c)
{
    // Adding mouse events
	// Some of invoked functions are declared in peripherals.js
	// For every new UI widget, its corresponding function should 
	// be declared in peripherals.js
	c.addEventListener( "contextmenu", preventDefaultHandler,	true );
	c.addEventListener( "mousedown", 	mouseDownHandler,	true );
	c.addEventListener( "mouseup",	mouseUpHandler, 	true );
	c.addEventListener( "mousemove", 	mouseMoveHandler, 	true );
	c.addEventListener( "mousewheel", 	mouseWheelHandler, 	true );
	c.addEventListener( "mousewheel", 	mouseWheelHandler, 	true );
	c.addEventListener( "mouseout", 	mouseOutHandler, 	true );

    c.addEventListener( "resize", 	canvasResize, 	false );
	document.addEventListener("keypress", 	keyDownHandler, 	false );
    //document.addEventListener("keyup", 	keyUpHandler, 		false );
}

function createMainCanvasAndContext ()
{
    canvas = document.createElement('canvas');
    canvas.className = "canvas";
    canvas.width = 1920;
    canvas.height = 1080;
    canvas.style = "-moz-transform: scale(-1, 1); -webkit-transform: scale(1, -1); -o-transform: scale(1, -1); transform: scale(1, -1);"        
    document.getElementById('main').appendChild(canvas);
    ctx = canvas.getContext('2d');     
}

function init() 
{ 
	// cheking for websocket support
	if (window.WebSocket) {
		connectAndCallbacks();
	}
	else
	{
		console.log ("This browser does not support Websocket.");
	}
	//canvas = document.getElementById('canvas');
    if (jpegCompression)
    {
        jpegImg	= new Image (); // jpeg IMAGE
        jpegImg.addEventListener('load', loadPixels, true );    
        createMainCanvasAndContext();
        addMyListeners (canvas);
    }
	else if (h264Compression)
	{
		playerH264 = new Player({
        webgl: "auto",
		useWorker: true,
		workerFile: "Broadway/Player/Decoder.js"
		});
        //canvas = playerH264.canvas;
        document.getElementById('main').appendChild (playerH264.canvas).className = "canvas";
        addMyListeners (playerH264.canvas);
	}
    else if (noCompression) // no compression
    {
        createMainCanvasAndContext ();
        addMyListeners (canvas);
        imgdata = ctx.getImageData(0,0,canvas.width,canvas.height);
    }
}

function loadPixels ()
{
	ctx.drawImage(jpegImg, 0, 0);
   
//	console.log ("Load pixels");
}

function onPictureDecodedHandler() 
{
    console.log ("decoded");
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
		// removing the (blank) header, keep pixels
		var img = dataUrl.split(',')[1];
		jpegImg.src = 'data:image/jpg;base64,' + img;
	}
	else if (h264Compression)
	{
		var frame = new Uint8Array(reader.result);
        	//console.log (frame);
		playerH264.decode (frame);
	}
    else if (noCompression)
	{
        console.log("No compression");
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
		else if (h264Compression || noCompression)
		{
            reader.readAsArrayBuffer(blob);
		}
        
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


