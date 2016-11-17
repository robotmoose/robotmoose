
/**
 User Interface builder:
   Allows robot control code to create buttons and labels at runtime.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-09-15 (Public Domain)
*/


function UI_builder_t(div) {
	this.div=div;
	this.e_counter=0;

	// Elements is a hash indexed by the element's name+type:
	this.elements={};
	
	this.stop();
}


// Empty all DOM and stored objects
UI_builder_t.prototype.clean=function() {
	this.e_counter=0;
	this.elements={};
	while (this.div.firstChild) this.div.removeChild(this.div.firstChild);
}

UI_builder_t.prototype.run=function() {
	this.clean();

}
UI_builder_t.prototype.stop=function() {
	this.clean();

	this.div.appendChild(
		document.createTextNode("Run your Code to make buttons here!")
	);
}

// We're starting this state--delete old UI elements
UI_builder_t.prototype.start_state=function(state_name) {
	this.clean();
}

/**
 Build a new UI element of this name and type.
*/
UI_builder_t.prototype.create_element=function(name,type,opts) {
	opts=opts||{}; // allow empty opts
	//console.log("UI: Adding "+type+" named '"+name+"'");

	var e={};
	e.name=name;
	e.type=type;
	e.disabled=false;

	// Create appropriate DOM element
	switch (type) {
	case "button":
		e.dom=document.createElement("input");
		e.dom.className="btn btn-primary";
		e.dom.type="button";
		e.dom.value=name;
		if(this.e_counter>0)
			e.dom.style.marginTop=10;

		// Mouse click handling (edge trigger)
		e.oneshot=false;
		e.dom.onclick=function(evt) { e.oneshot=true; }

		// Mouse up/down tracking (level trigger)
		e.value=false;
		e.dom.onmousedown=function(evt) { e.value=true; }
		e.dom.onmouseleave=function(evt) { e.value=false; }
		e.dom.onmouseup=function(evt) { e.value=false; }
		break;

	case "checkbox":
		e.dom=document.createElement("input");
		e.dom.type="checkbox";
		e.label = document.createTextNode(" " + name); // Checkbox label
		break;

	case "slider":
		e.dom=document.createElement("input");
		e.dom.type="range";
		e.dom.min = opts.min;
		e.dom.max = opts.max;
		if(e.dom.min>e.dom.max)
			{
				e.dom.min = opts.max;
				e.dom.max = opts.min;
			}
		e.dom.defaultValue = opts.defaultValue;
		e.dom.step = opts.step;
		var value = e.dom.defaultValue;
		var text_node = document.createTextNode(name);
		text_node.appendData(" = "+value)
		e.dom.onchange=e.dom.oninput=function(evt) {
			value = e.dom.value;
			console.log("min:"+e.dom.min+"max:"+e.dom.max+"value: "+value);
			text_node.deleteData(0,text_node.length);
			text_node.appendData(name+" = "+value);
			
		}
		var div=document.createElement("div");
		div.appendChild(text_node);
		div.style.marginTop=-10;
		this.div.appendChild(div);
		break;
	case "spinner":
		e.dom=document.createElement("input");
		e.dom.type="number";
		e.dom.value="0.0";
		e.dom.style.width=100;
		e.label=document.createTextNode(" " + name);
		break;

	case "label":
		e.dom=document.createElement("div");
		e.dom.appendChild(document.createTextNode(name));
		if(this.e_counter>0)
			e.dom.style.marginTop=10;
		break;
	default:
		throw("Unknown UI element type "+type);
	};

	++this.e_counter;

	// Add generic styling options
	if (opts.title) e.dom.title=opts.title;

	// Add to UI panel
	this.div.appendChild(e.dom);
	if (e.label) this.div.appendChild(e.label); // If the element has a label
	if (opts.same_line!=true) {
		this.div.appendChild(document.createElement("br")); // newline
	}

	this.minimized = false;
	return e;
}

/**
 Create *or* query a UI element of this type.
*/
UI_builder_t.prototype.element=function(name,type,opts) {
	var nametype=name+"-"+type;
	var e=this.elements[nametype];
	if (!e) { // need to build element
		e=this.elements[nametype]=this.create_element(name,type,opts);
	}

	// Code checked it, so re-enable element
	if (e.disabled) {
		e.disabled=false;
		e.dom.disabled=false;
	}
	return e;
}


