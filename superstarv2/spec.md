# Superstar v2 Protocol Specification
## Get Requests
### General Information
GET requests are read only.

### Traversal
You can traverse the JSON database stored in Superstar using get requests. Simply start with /superstar/ and follow with the path of the object, example:

If the current database was:

    {"foo":{"bar":[4,5,6,7]}}

To get the value of `foo.bar[2]`, you would type in `://URL/superstar/foo/bar/2` which would yield `6`.

Typing in `://URL/superstar/foo/bar` would yield `[4,5,6,7]`.

## POST Requests
### Introduction
All POST requests to any path starting with `/superstar/` are considered to be JSON-RPC requests. [See JSONRPC.org for details.](http://www.jsonrpc.org/specification)

### Error codes
Superstar v2 only has one special error code, -32000. This error is for authentication failures (message is "Not authorized").
### Methods
#### Get
Exactly the same as doing an HTTP GET request (AKA, gets a value from the database).

Example:
If the current database was: `{"foo":{"bar":[4,5,6,7]}}`
To get the value of `foo.bar[2]`, make a POST request to `://URL/superstar/` with data:

    {"jsonrpc":"2.0","method":"get","params":{"path":"/foo/bar/2"},"id":null}

This will return: 

    {"jsonrpc":"2.0","result":6,"id":null}

#### Set
Sets a value of the database. Return result will be true on success.

All write based commands have two additional parameters:
* auth - The HMAC-SHA256 of the opts argument (this is an optional argument).
* opts - A STRING of a JSON object. This argument is a STRING encoded JSON object because JSON serialization is not standard. Thus, taking the HMAC-SHA256 of an object serialized on two different platforms may produce different hashes.

In this case, opts has a single member value. The member value contains the value of which to set the path.

Note, if there is a value at the given path, that value will be overwritten. Also, if the requested path does not exist, it will be created.

See Authentication for details on the authentication.

Example:
If the database had a password of "123" on `/` and the current database was:

    {"bar":123}

Sending:

    {"params":{"path":"/","opts":"{\"value\":{\"foo\":{\"bar\":[4,5,6,7]}}}","auth":"823c0e122e4de0acdf4526c807ca93987368651ee54f474f8200b6f9161eb930"},"jsonrpc":"2.0","method":"set","id":null}

Yields:

    {"jsonrpc":"2.0","result":true,"id":null}

#### Sub
This method returns an array consisting of all the subkeys in a given path.

Examples:

If the current database was:

    {"foo":{"bar":[4,5,6,7]}}

Sending:

    {"jsonrpc":"2.0","method":"sub","params":{"path":"/"},"id":null}

Yields:

    {"jsonrpc":"2.0","result":["foo"],"id":null}

The new value of the database would be: 

    {"foo":{"bar":[4,5,6,7]}}

Using sub on arrays yields the integer keys (mimicking Javascript).
Sending:

    {"jsonrpc":"2.0","method":"sub","params":{"path":"/foo/bar"},"id":null}

Yields:

    {"jsonrpc":"2.0","result":[0,1,2,3],"id":null}

#### Push
Pushes a value onto a database path. Return result will be true on success.

All write based commands have two additional parameters:
* auth - The HMAC-SHA256 of the opts argument (this is an optional argument).
* opts - A STRING of a JSON object. This argument is a STRING encoded JSON object because JSON serialization is not standard. Thus, taking the HMAC-SHA256 of an object serialized on two different platforms may produce different hashes.

In this case, opts has two members: value and length.

The member value contains the value of which to set the path.
The member length is the max length of the array at the the given path after the value has been pushed. All arrays on Superstar have a max length of 1000.

Note, if the value at the given path was not an array, it will be erased with a new array with the given pushed value.

See Authentication for details on the authentication.

Example:
If the database had a password of "123" on `/` and the current database was:

    {"foo":{"bar":[4,5,6,7]}}

Sending:

    {"params":{"path":"/foo/bar","opts":"{\"value\":8,\"length\":3}","auth":"978aa81ebab04388534df74c818f886f09f29a23bde3acd5e130f58d190e4673"},"jsonrpc":"2.0","method":"push","id":null}

Yields:

    {"jsonrpc":"2.0","result":true,"id":null}

The new value of the database would be: 

    {"foo":{"bar":[6,7,8]}}

#### Change Auth
Changes an authentication code for a given path. Result is a boolean value indicating whether the authentication code was changed or successfully.

All write based commands have two additional parameters:
* auth - The HMAC-SHA256 of the opts argument (this is an optional argument).
* opts - A STRING of a JSON object. This argument is a STRING encoded JSON object because JSON serialization is not standard. Thus, taking the HMAC-SHA256 of an object serialized on two different platforms may produce different hashes.

In this case, opts has a single member value. The member value contains the value of which to set the authentication code for the given path. Note that authentication codes can only be changed, not created, with this command. Valid authentication codes must be of length 8 or greater and can only contain ASCII values between 33 and 126 (no white space or non-printable characters).

See Authentication for details on the authentication.

Example:
If the database had a password of "123" on `/foo`:


Sending:

    {"params":{"path":"/foo","opts":"{\"value\":\"imaprettykitty\"}","auth":"05b0c72924e9a017afc07a17d8d492e377cbc4f1b2ee66658605fd4105838ae6"},"jsonrpc":"2.0","method":"change_auth","id":null}

Yields:

    {"jsonrpc":"2.0","result":true,"id":null}


#### Authentication
Superstar authentication uses HMAC-SHA256, where the key is the password for the given path and the value that is being hashed is the concatenation of the path and the opts string. The path used is stripped of all trailing and leading slashes. The path also has all multiple slashes replaced with a single slash.

Auth codes are kept in the auth file. Authorization in Superstar are recursive and top down. Having the password to `/` gives access to everything at and below `/`. Having the password to /foo gives access to everything at and below `/foo`, but not at `/` and not `/bar`.

Example:
To authenticate a request with path `/foo/bar/` and opts value `{\"value\":null}` the value would be: `foo/bar{\"value\":null}`


### Appendix A: Curl Examples
Set "/" to `{"foo":{"bar":[4,5,6,7]}}` using auth "123":

    curl 127.0.0.1:8081/superstar/ --data '{"params":{"path":"/","opts":"{\"value\":{\"foo\":{\"bar\":[4,5,6,7]}}}","auth":"823c0e122e4de0acdf4526c807ca93987368651ee54f474f8200b6f9161eb930"},"jsonrpc":"2.0","method":"set","id":null}'

Get `/foo/bar/2`:
  
    curl 127.0.0.1:8081/superstar/ --data '{"jsonrpc":"2.0","method":"get","params":{"path":"/foo/bar/2"},"id":null}'

Sub `/`:
  
    curl 127.0.0.1:8081/superstar/ --data '{"jsonrpc":"2.0","method":"sub","params":{"path":"/"},"id":null}'

Push "8" onto array `/foo/bar` and resize to "3" using auth "123":

    curl 127.0.0.1:8081/superstar/ --data '{"params":{"path":"/foo/bar","opts":"{\"value\":8,\"length\":3}","auth":"978aa81ebab04388534df74c818f886f09f29a23bde3acd5e130f58d190e4673"},"jsonrpc":"2.0","method":"push","id":null}'

Note, post data in curl can be loaded from a file. To use the contents of test.json, use:

    curl 127.0.0.1:8081/superstar/ --data '@test.json'


### Appendix B - Python API
To use the python api, use the python module superstar.py:

    import superstar

Create a superstar instance:

    ss=superstar_t("127.0.0.1:8081")

The API uses a callback interface, so create some callbacks:

    def getprint(result):
    	print(result)
    
    def subprint(result):
    	print(result)

Make all necessary requests:

    # get: path,success_callback(result),error_callback(error_obj)
    ss.get("/blarg",getprint)
    
    # set: path,value,auth,success_callback(result),error_callback(error_obj)
    ss.set("/blarg",4,"123")
    
    # sub: path,success_callback(result),error_callback(error_obj)
    ss.sub("/",subprint)
    
    # push: path,value,length,auth,success_callback(result),error_callback(error_obj)
    ss.push("/blarg2",2,3,"123")
  
    # Finally, flush to batch send the request and receive the responses:
    ss.flush()


### Appendix C - Javascript API
Make sure to include superstar.js.

Create requests:

    //get: path,success_callback(result),error_callback(error_obj)
    superstar.get("/blarg",function(result){console.log(result);});
    
    //set: path,value,auth,success_callback(result),error_callback(error_obj)
    superstar.set("/blarg",4,"123");
    
    //sub: path,success_callback(result),error_callback(error_obj)
    superstar.sub("/",function(result){console.log(result);});
    
    //push: path,value,length,auth,success_callback(result),error_callback(error_obj)
    superstar.push("/blarg2",2,3,"123");
    
    // Finally, flush to batch send the request and receive the responses:
    superstar.flush();
