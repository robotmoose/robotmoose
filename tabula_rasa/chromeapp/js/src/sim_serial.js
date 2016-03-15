/*Fake a serial port to enable simulated robots
  Same interface as chrome.serial API 

   Arsh Chauhan
   03/14/2016
 */

/*
	TO DO: Make functions actually return data. This 'compiles' but 
	doesn't do anything. 
*/
 function sim_serial_t()
 {
 	// Add stuff to fake encoders here
 	var _this=this;

 	_this.onReceive={
 						addListener:function(action){ console.log ("In onReceive addListener");}
 					};
 	_this.onReceiveError={
 						addListener:function(action){ console.log ("In onReceiveError addListener");}
 					};
 }

//TO DO: Send faked data to stop connection from timing out
 sim_serial_t.prototype.connect = function(port, options, callback)
 {
 	console.log("In function sim_serial.connect");
 	console.log("port name: " + port);
 	for (key in options)
 	{
 		console.log(key +": " + options[key]);
 	}
 	callback(options);
 }

sim_serial_t.prototype.disconnect = function(connection, action)
 {
 	console.log("closing connection:" + connection);
 	//FIX ME: Handle runtime errors 
 }

 sim_serial_t.prototype.flush = function(connection, action)
 {
 	console.log("flushing connection:" + connection);

 }

 sim_serial_t.prototype.send = function(connection, buf,callback)
 {
 	console.log("Bytes Recieved: " + buf.length);
 	var sendInfo= {
 						bytesSent:buf.length
 				  }
 	callback(sendInfo);
 }