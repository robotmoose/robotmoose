var url_path="";

function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_dependencies()
{
	load_js(url_path+"js/three/loaders/OBJLoader.js");
	load_js(url_path+"js/three/loaders/STLLoader.js");
	load_js(url_path+"js/three/loaders/STLLoader.js");
	load_js(url_path+"js/three/controls/OrbitControls.js");
};

(function(){load_dependencies()})();

function cube_t(scene)
{
	var myself=this;
	myself.scene=scene;

	myself.mesh=new THREE.Mesh(new THREE.CubeGeometry(1,1,1),
		new THREE.MeshPhongMaterial({color:0xffffff}));
	myself.mesh.position.set(0,0,0);
	myself.mesh.rotation.set(0,0,0);
	myself.mesh.scale.set(1,1,1);
	myself.position=myself.mesh.position;
	myself.rotation=myself.mesh.rotation;
	myself.scale=myself.mesh.scale;
	myself.scene.add(myself.mesh);

	myself.set_color=function(color)
	{
		myself.mesh.material.color.setHex(color);
	};

	myself.destroy=function()
	{
		myself.scene.remove(myself.mesh);
		myself.mesh=null;
		myself.scene=null;
		myself.position=null;
		myself.scale=null;
	};
};

function light_t(scene,intensity,position)
{
	var myself=this;
	myself.scene=scene;

	myself.light=new THREE.SpotLight(0xffffff,intensity);
	myself.light.position.copy(position);
	myself.light.target.position.set(0,0,0);
	myself.light.castShadow=true;

	var size=1000; // hack!
	myself.light.shadowMapHeight=512;
	myself.light.shadowMapWidth=512;
	myself.light.shadowCameraNear=size/2;
	myself.light.shadowCameraFar=size*3;
	myself.light.shadowCameraFov=60;
	myself.light.shadowCameraVisible=true; // debug

	myself.position=myself.light.position;
//	if (position) myself.position.copy(position);
	myself.scene.add(myself.light);

	myself.set_color=function(color)
	{
		myself.mesh.material.color.setHex(color);
	};

	myself.destroy=function()
	{
		myself.scene.remove(myself.light);
		myself.light=null;
		myself.scene=null;
		myself.position=null;
	};
};

//need to fix parents in this class...they do not cleanup themselves...
function model_t(scene)
{
	var myself=this;
	myself.loaded=false;
	myself.scene=scene;
	myself.mesh=null;
	myself.position=new THREE.Vector3(0,0,0);
	myself.rotation=new THREE.Vector3(0,0,0);
	myself.scale=new THREE.Vector3(1,1,1);

	myself.set_color=function(color)
	{
		if(myself.loaded&&myself.mesh)
		{
			myself.mesh.traverse(function(child){if(child instanceof THREE.Mesh)
				child.material.color.setHex(color);});
		}
		else
		{
			setTimeout(function(){myself.set_color(color);},10);
		}
	};

	myself.destroy=function()
	{
		if(myself.loaded)
		{
			myself.scene.remove(myself.mesh);
			myself.loaded=false;
			myself.mesh=null;
			myself.scene=null;
			myself.position=null;
			myself.rotation=null;
			myself.scale=null;
		}
		else
		{
			setTimeout(myself.destroy,10);
		}
	};

	myself.set_parent=function(parent)
	{
		if(parent)
		{
			if(myself.loaded&&myself.mesh)
			{
				try
				{
					parent.mesh.add(myself.mesh);
					myself.scene.remove(myself.mesh);
					myself.scene=parent;
				}
				catch(e)
				{
					setTimeout(function(){myself.set_parent(parent);},10);
				}

			}
			else
			{
				setTimeout(function(){myself.set_parent(parent);},10);
			}
		}
	};
};

function renderer_t(div,setup_func,loop_func)
{
	var myself=this;

	myself.div=div;
	myself.div.innerHTML="";

	myself.width=320;
	myself.height=240;

	myself.user_setup=setup_func;
	myself.user_loop=loop_func;

	myself.clock=0;
	myself.dt=0;
	myself.viewport=null;
	myself.scene=null;
	myself.camera=null;
	myself.controls=null;

	myself.setup=function()
	{
		if(myself.div&&myself.user_setup&&myself.user_loop)
		{
			window.addEventListener("resize",myself.resize,false);

			myself.clock=new THREE.Clock();

			myself.viewport=new THREE.WebGLRenderer({antialias:true});
			myself.viewport.setSize(myself.width,myself.height);
			myself.viewport.setClearColor(new THREE.Color(0xaaccff));
			myself.div.appendChild(myself.viewport.domElement);

			myself.scene=new THREE.Scene();
			myself.scene.matrixAutoUpdate=true;

			myself.camera=new THREE.PerspectiveCamera(45,myself.width/myself.height,1,20000);

			myself.controls=new THREE.OrbitControls(myself.camera,myself.div);
			myself.controls.movementSpeed=100;
			myself.controls.lookSpeed=0.2;
			myself.controls.center.set(0,64,0);
			myself.controls.object.position.set(0,100,250);

			myself.user_setup();
			myself.loop();

			return true;
		}

		return false;
	};

	myself.destroy=function()
	{
		myself.div=null;
		myself.viewport=null;
		myself.scene=null;
		myself.camera=null;
		myself.controls=null;
	};

	myself.set_size=function(width,height)
	{
		if(width)
			myself.width=width;

		if(height)
			myself.height=height;

		myself.resize();
	};

	myself.resize=function()
	{
		myself.camera.aspect=myself.width/myself.height;
		myself.camera.updateProjectionMatrix();
		myself.viewport.setSize(myself.width,myself.height);
	};

	myself.loop=function()
	{
		myself.dt=myself.clock.getDelta();
		myself.user_loop(myself.dt);
		myself.controls.update(myself.dt);
		requestAnimationFrame(myself.loop);
		myself.viewport.render(myself.scene,myself.camera);
	};

	myself.load_texture=function(filename)
	{
		return THREE.ImageUtils.loadTexture(filename);
	};

	myself.load_obj=function(filename,texture)
	{
		var model=new model_t(myself.scene);
		var loader=new THREE.OBJLoader();

		loader.addEventListener("load",function(event)
		{
			var object=event.content;

			if(texture)
				object.traverse(function(child){
					if(child instanceof THREE.Mesh)
					{
						THREE.GeometryUtils.center(child.geometry);
						child.geometry.computeFaceNormals();
						child.material.map=texture;
					}
					});

			object.position=model.position;
			object.rotation=model.rotation;
			object.scale=model.scale;

			model.scene.add(object);
			model.mesh=object;
			model.position=model.mesh.position;
			model.rotation=model.mesh.rotation;
			model.scale=model.mesh.scale;
			model.loaded=true;
		});

		loader.load(filename);

		return model;
	};

	myself.create_cube=function()
	{
		return new cube_t(myself.scene);
	};

	myself.create_light=function(intensity,position)
	{
		return new light_t(myself.scene,intensity,position);
	};

	myself.create_grid=function(size,width,height)
	{
		var plane_geometry=new THREE.PlaneGeometry(size*width,size*height,size,size);
		plane_geometry.normalsNeedUpdate=true;
		var plane_material=new THREE.MeshBasicMaterial({color:0xd8eef4,depthWrite:false,
				side:THREE.DoubleSide});
		var plane=new THREE.Mesh(plane_geometry,plane_material);
		plane.rotation.set(3*Math.PI/2,0,0);
		myself.scene.add(plane);

		var line_geometry=new THREE.Geometry();
		var line_material=new THREE.LineBasicMaterial({color:0x0488c8,linewidth:1.5});

		for(var xx=0;xx<=width;++xx)
		{
			line_geometry.vertices.push(new THREE.Vector3(-size*width/2,-size*height/2.0+xx*size,0));
			line_geometry.vertices.push(new THREE.Vector3(size*width/2,-size*height/2.0+xx*size,0));
		}

		for(var yy=0;yy<=height;++yy)
		{
			line_geometry.vertices.push(new THREE.Vector3(-size*width/2.0+yy*size,-size*height/2,0));
			line_geometry.vertices.push(new THREE.Vector3(-size*width/2.0+yy*size,size*height/2,0));
		}

		var line=new THREE.Line(line_geometry,line_material,THREE.LinePieces);
		plane.add(line);

		return plane;
	};
}
