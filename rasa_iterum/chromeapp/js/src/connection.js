/**
 This file handles serial communication with the 
 tabula_rasa Arduino firmware.
 
 

 Mike Moss & Orion Lawlor, 2015-12, Public Domain
*/

function connection_t(div,on_message,on_disconnect)
{
	var _this=this;
	_this.config="";
	_this.status_message=on_message;
	_this.on_disconnect=on_disconnect;
	_this.port_name="not connected yet";
	
	_this.connection_invalid="yes, totally invalid";
	_this.connection=_this.connection_invalid; // serial api writes an int
	
	// Are there other serial JS apis?
	_this.serial_api=chrome.serial;
}


// Callback from GUI
connection_t.prototype.gui_connect=function(port_name)
{
	var _this=this;
	_this.port_name=port_name;
	_this.status_message("Connecting to "+port_name);
	_this.serial_api.connect(port_name, {bitrate: 57600}, 
		function(connectionInfo) {
			_this.connection=connectionInfo.connectionId;
			_this.status_message("Connected to "+port_name);
			
			_this.serial_api.onReceive.addListener(
				function(info) { _this.serial_onReceive(info); }
			);
			_this.serial_api.onReceiveError.addListener(
				function(info) { _this.serial_onReceiveError(info); }
			);
			
			_this.version_check();
		}
	);
	
}

// Callback from serial API: incoming data
connection_t.prototype.serial_onReceive=function(info) 
{
	var _this=this;
	_this.status_message("Serial data arrived: "+info.data.byteLength+" bytes");
	
	// parse?
	var buffer=info.data; // ArrayBuffer
	var arr=new Uint8Array(buffer);
	for (var i=0;i<arr.length;i++) {
		var v=arr[i];
		var c=String.fromCharCode(v);
		_this.status_message("   \t"+v+"  \t"+c);
	}
	
}

// Callback from serial API: error on serial port
connection_t.prototype.serial_onReceiveError=function(info) 
{
	var _this=this;
	_this.status_message("Serial error: "+info.error);
	_this.gui_disconnect(_this.port_name);
}

connection_t.prototype.version_check=function()
{
	var _this=this;
	
}



// Callback from GUI
connection_t.prototype.gui_disconnect=function(port_name)
{
	var _this=this;
	if (_this.connection!==_this.connection_invalid) {
		_this.status_message("Disconnecting from "+port_name);
		chrome.serial.disconnect(_this.connection,
			function() {
				if (chrome.runtime.lastError)
					_this.status_message("Error disconnecting from "+port_name);
				_this.status_message("Disconnected from "+port_name);
			}
		);
		_this.connection=_this.connection_invalid;
	}
}

connection_t.prototype.connected=function()
{
	var _this=this;
	return _this.connection!==_this.connection_invalid;
}

