function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_dependencies()
{
	load_js("js/three/three.min.js");

	setTimeout(
		function()
		{
			load_js("js/three/loaders/OBJLoader.js");
			load_js("js/three/loaders/STLLoader.js");
		},100);
};

(function(){load_dependencies()})();

function cube_t(scene)
{
	var myself=this;
	myself.scene=scene;

	myself.mesh=new THREE.Mesh(new THREE.CubeGeometry(1,1,1),
		new THREE.MeshPhongMaterial({color:0xffffff}));
	myself.mesh.receiveShadow=true;
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

	myself.set_shadow=function(on)
	{
		myself.mesh.castShadow=on;
		myself.mesh.receiveShadow=on;
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

function light_t(scene)
{
	var myself=this;
	myself.scene=scene;

	myself.light=new THREE.SpotLight(0xffffffff,1);
	myself.light.castShadow=true;
	myself.light.position.set(0,0,0);
	myself.position=myself.light.position;
	myself.scene.add(myself.light);

	myself.set_color=function(color)
	{
		myself.mesh.material.color.setHex(color);
	};

	myself.set_shadow=function(on)
	{
		myself.mesh.castShadow=on;
	};

	myself.destroy=function()
	{
		myself.scene.remove(myself.light);
		myself.light=null;
		myself.scene=null;
		myself.position=null;
	};
};

function model_t(scene)
{
	var myself=this;
	myself.loaded=false;
	myself.scene=scene;
	myself.mesh=null;
	myself.position=new THREE.Vector3(0,0,0);
	myself.rotation=new THREE.Vector3(0,0,0);
	myself.scale=new THREE.Vector3(1,1,1);

	myself.set_shadow=function(on)
	{
		if(myself.loaded&&myself.mesh)
		{
			//myself.mesh.castShadow=on;
			//myself.mesh.receiveShadow=on;
		}
		else
		{
			setTimeout(function(){myself.set_shadow(on);},10);
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
};

function renderer_t()
{
	var myself=this;
	myself.div=null;
	myself.width=320;
	myself.height=240;

	myself.clock=0;
	myself.dt=0;
	myself.viewport=null;
	myself.scene=null;
	myself.camera=null;

	myself.create=function(div)
	{
		myself.div=div;
		myself.div.innerHTML="";

		window.addEventListener("resize",myself.set_size,false);

		myself.clock=new THREE.Clock();

		myself.viewport=new THREE.WebGLRenderer();
		myself.viewport.setSize(myself.width,myself.height);
		myself.viewport.shadowMapEnabled=true;
		myself.viewport.shadowMapSoft=true;
		myself.viewport.shadowMapWidth=2048;
		myself.viewport.shadowMapHeight=2048;
		myself.div.appendChild(myself.viewport.domElement);

		myself.scene=new THREE.Scene();
		myself.scene.matrixAutoUpdate=true;

		myself.camera=new THREE.PerspectiveCamera(45,myself.width/myself.height,1,20000);
		myself.camera.position.y=32;

		myself.loop();
	};

	myself.destroy=function()
	{
		myself.div=null;
		myself.viewport=null;
		myself.scene=null;
		myself.camera=null;
	};

	myself.set_size=function(width,height)
	{
		if(width)
			myself.width=width;

		if(height)
			myself.height=height;

		myself.camera.aspect=myself.width/myself.height;
		myself.camera.updateProjectionMatrix();
		myself.viewport.setSize(myself.width,myself.height);
	};

	myself.loop=function()
	{
		myself.dt=myself.clock.getDelta();
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
						object.castShadow=child.castShadow=true;
						object.receiveShadow=child.receiveShadow=true;
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

	myself.create_light=function()
	{
		return new light_t(myself.scene);
	};
}