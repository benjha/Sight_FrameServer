/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
var output;
var canvas;
var canvasH264;
var jpegImg;
var wsUri	= "ws://localhost:9003/";
var reader	= new FileReader();
var ctx;
var imgdata;
var stop 	= false;
var mouseDownFlag = false;
// Enables/disables JPEG compression. 
// JPEG compression should be enabled/disabled in the server in cBroadcastServer.h
var jpegCompression = true; 
var h264Compression = false;
var h264MP4Compression = false;
var noCompression   = false;
var playerH264; // from Broadway.cs 
// when using h264MP4Compression:
var mediaSource={};
var video;
var bufArray;
var arraySize = 0;
var sourceBuffer={};
var timeAtLastFrame = -1; // used for performance metrics
// canvas resolution:
//var width = 1920;
//var height = 1088;
var width = 3840;
var height = 2160;
function canvasResize ()
{
    //console.log (canvas.width + ", " + canvas.height);
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
    canvas.width = width;
    canvas.height = height-8;
    canvas.style = "-moz-transform: scale(-1, 1); -webkit-transform: scale(1, -1); -o-transform: scale(1, -1); transform: scale(1, -1);";        
    document.getElementById('main').appendChild(canvas);
    ctx = canvas.getContext('2d');     
}

function initVideoh264mp4 ()
{
    mediaSource = new MediaSource();
    video = document.createElement('video');
    video.id = "video";
    video.width = width;
    video.height = height;
    video.autoplay = false;
    //video.className = "canvas";
    video.src = window.URL.createObjectURL(mediaSource);
    bufArray = new Array ();
    document.getElementById('main').appendChild(video);    
    // for sight is baseline level 4.1 mbps 32 fps 30
    // https://cconcolato.github.io/media-mime-support/
    // http://blog.mediacoderhq.com/h264-profiles-and-levels/
    //var mimecodec = 'video/mp4; codecs="avc1.420028"';
    var mimecodec = 'video/mp4;codecs="avc1.64001E"';// 'video/mp4; codecs="avc1.42E01E"';
    //var mimecodec = 'video/mp4; codecs="avc1.42E01E"';
    // 0x64=100 "High Profile"; 0x00 No constraints; 0x1F=31 "Level 3.1"
    //var mimecodec = 'video/mp4; codecs="avc1.64001F"';
    mediaSource.addEventListener('sourceopen', function() {
        //console.log("sourceOpen...");
        // get a source buffer to contain video data this we'll receive from the server
        //console.log (that.video.canPlayType(mimecodec));
        sourceBuffer = mediaSource.addSourceBuffer(mimecodec);
    });
    mediaSource.addEventListener('webkitsourceopen', function() {
        //console.log("webkitsourceopen...");
        // get a source buffer to contain video data this we'll receive from the server
        sourceBuffer = mediaSource.addSourceBuffer(mimecodec);
        //that.sourceBuffer = that.mediaSource.addSourceBuffer('video/mp4;codecs="avc1.64001E"');
    });
    // this seems not to work yet
    sourceBuffer.updateend = function(evt) { nextH264MP4Frame (evt) };
    
    addMyListeners(video);
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
        webgl: true,
		useWorker: true,
		workerFile: "Broadway/Player/Decoder.js"
		});
        //canvas = playerH264.canvas;
        document.getElementById('main').appendChild (playerH264.canvas).className = "canvas";
        playerH264.canvas.style = "-moz-transform: scale(-1, 1); -webkit-transform: scale(1, -1); -o-transform: scale(1, -1); transform: scale(1, -1);";  
        addMyListeners (playerH264.canvas);
	}
	else if (h264MP4Compression)
	{
		initVideoh264mp4 ();
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

function connectAndCallbacks ()
{
	websocket		= new WebSocket(wsUri); 
	websocket.onopen 	= function(evt) { onOpen	(evt)	}; 
	websocket.onclose 	= function(evt) { onClose	(evt)	}; 
	websocket.onmessage	= function(evt) { onMessage	(evt)	}; 
	websocket.onerror 	= function(evt) { onError	(evt)	};

	// sets websocket's binary messages as ArrayBuffer type
    // need to check if arraybuffer works for Broadway.cs
    if (h264MP4Compression || h264Compression)
        websocket.binaryType = "arraybuffer";
    else
	// sets websocket's binary messages as Blob type by default is Blob type
	   websocket.binaryType = "blob";
	
	reader.onload		= function(evt) { readBlob		(evt) };
	reader.onloadend	= function(evt) { nextBlob		(evt) };
	reader.onerror		= function(evt) { fileReaderError 	(evt) };

	peripherals		= new Peripherals ();
	peripherals.init(websocket);
}

// websocket connection has been opened
function onOpen(evt)
{
    if(h264MP4Compression)
    {
 // https://stackoverflow.com/a/40238567
    var playPromise = video.play();
    if ( playPromise !== undefined) {
        console.log("Got play promise; waiting for fulfilment...");
        playPromise.then(function() 
        {
            console.log("Play promise fulfilled! Starting playback.");
            video.connected = true;
            //vframe.width = that.width;
            //vframe.height = that.height;
            video.currentTime = 0;
        }).catch(function(error) {
            console.log("Failed to start playback: "+error);
        });
        }        
    }
}

function fileReaderError (e)
{
	console.error("FileReader error. Code " + event.target.error.code);
}
// method called when sourceBuffer update ends
function nextH264MP4Frame (e)
{
    console.log ("updateend");
    //websocket.send ("NXTFR");
}

// this method is launched when FileReader ends loading the blob
function nextBlob (e)
{
	if (!stop)
    {
        if (jpegCompression || noCompression )
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
// readBlob is called when reader.readAsBinaryString(blob)
// from onMessage method occurs
function readBlob (e)
{
	var i,j=0;
	if (jpegCompression)
	{
        //console.log ("jpegCompression");
		var dataUrl = reader.result;
		// removing the (blank) header, keep pixels
		var img = dataUrl.split(',')[1];
		jpegImg.src = 'data:image/jpg;base64,' + img;
	}
	else if (noCompression)
	{
        //console.log("No compression");
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
// Video decoding function when using h264 and Broadway.cs
function decodeH264  (frame)
{
    var frame = new Uint8Array(frame.data);
    //console.log (decodeH264);
	playerH264.decode (frame);            
}
// Video decoding  function when using h264 and MP4 wrap
function decodeH264MP4 (frame)
{
    var frame = new Uint8Array(frame.data);
    bufArray.push(frame);
    arraySize+=frame.length;
    
    if (!sourceBuffer.updating)
    {
        //var myBufferedRange = sourceBuffer.buffered;
        //console.log (myBufferedRange.start(0));
        //console.log (myBufferedRange.end(0));
        //console.log("decoding, video paused " + video.paused);
        var streamBuffer = new Uint8Array(arraySize);
        var i=0;
        var nchunks=0;
        while (bufArray.length > 0)
        {
            var b = bufArray.shift();
            //console.log (i);
            streamBuffer.set(b, i);
            i += b.length;
            nchunks+=1;
        }
        websocket.send("NXTFR");

        arraySize = 0;
        // Add the received data to the source buffer
        sourceBuffer.appendBuffer(streamBuffer);
        var tnow = performance.now();
        var logmsg=" ";// = 'Frame: ' + this.frame+ '(in '+nchunks+' chunks)';
        if (timeAtLastFrame >= 0)
        {
            var dt = Math.round(tnow - timeAtLastFrame);
            logmsg += '; dt = ' + dt + 'ms (' + 1000/dt + ' fps)' ;
            logmsg += '\n' + Array(Math.round(dt/10)).join('*');
        }
        timeAtLastFrame = tnow;        
        console.log(logmsg);    
    }
    // variable used for perfomance metrics
    //frame++;
    if (video.paused)
    {
        video.play();
    }
    //TODO: Figure out a smarter way to manage the frame size (i.e. cache it?)
    //video.width = video.videoWidth;
    //video.height = video.videoHeight;    
}

// message from the server
function onMessage(e)
{
    if (typeof e.data == "string")
    {
        console.log ("String msg: ", e, e.data);
        return;
    }
    if (h264MP4Compression)
        decodeH264MP4 (e);
    else if (h264Compression)
        decodeH264 (e);
    else
    {
        if (e.data instanceof Blob)
        {
            var blob = e.data;
            if (jpegCompression)
                reader.readAsDataURL(blob);
            else if ( noCompression )
                reader.readAsArrayBuffer(blob);
       }
	}
}
// if there is an error
function onError (e)
{
    if (h264MP4Compression)
        video.connected = false;
	console.log("Websocket Error: ", e);
	// Custom functions for handling errors
	// handleErrors (e);
}
// when closing
function onClose (e)
{
	console.log ("Connection closed", e);
    if (h264MP4Compression)
    {
        video.connected = false;
        sourceBuffer.remove(0, 10000000);        
    }
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


