/* global mathBox */

function rangeTrans(obj, range) {
	obj.position.x /= obj.position.x > 0 ? range[0][1] : -range[0][0];
	obj.position.y /= obj.position.y > 0 ? range[1][1] : -range[1][0];
	obj.position.z /= obj.position.z > 0 ? range[2][1] : -range[2][0];
}

function getSphere(x, y, z, radius) {
	if (radius == undefined) radius = 0.1;
	
	var vp = window.mathbox.viewport();
	
	radius *= vp.transform.getMaxScaleOnAxis();
	
	var material = new THREE.MeshBasicMaterial({ color: 0x0000ff });
	var geometry = new THREE.SphereGeometry(radius, 16, 16); // Radius size, number of vertical segments, number of horizontal rings.
	var sphere = new THREE.Mesh(geometry, material); // Create a mesh based on the specified geometry (cube) and material (blue skin).
	
	sphere.position.x = x;
	sphere.position.y = y;
	sphere.position.z = z;
	vp.to(sphere.position);
	return sphere;
}

function addScene(scene) {
	console.log(scene);
	
	for (var i in scene.spheres) {
		var s = scene.spheres[i];
		mathbox.add(getSphere(s.x, s.y, s.z, s.radius));
	}
	
	//mathbox.add(getSphere(2, 0, 2));
}

var FPSControls = function (camera, domElement, options) {
  this.element = domElement;
  this.camera = camera;

  this.options = tQuery.extend(options, {
	position: [-1, 1, 1],
	rotation: [-Math.PI/2+0.5, 0.4, 0],
    //lookAt: [0, 0, 0],
    speed: 2,
	fov: 70,
  });

  this.init();
  this.start();
  this.update();
};

FPSControls.prototype = {

  init: function () {
    this.width = this.element && this.element.offsetWidth,
    this.height = this.element && this.element.offsetHeight;
    this.speed = this.options.speed;
	this.fov = this.options.fov;

	this.clock = new THREE.Clock();
	this.damping = 0.95;
	this.dampingVector = new THREE.Vector3(this.damping, this.damping, this.damping);
	this.velocity = new THREE.Vector3(0, 0, -1);
	this.velocityScaled = new THREE.Vector3();
	this.dtVector = new THREE.Vector3();
	this.camera.position.set.apply(this.camera.position, this.options.position || []);
	this.rotVector = new THREE.Vector3();
    this.rotVector.set.apply(this.rotVector, this.options.rotation || []);
  	this.lookAt = new THREE.Vector3();
    this.lookAt.set.apply(this.lookAt, this.options.lookAt || []);
  	this.keys = {};
  },

  start: function () {
    var that = this;
	
    this._mouseDown = function (event) {
      that.width = that.element && that.element.offsetWidth,
      that.height = that.element && that.element.offsetHeight;

      that.drag = true;
      that.lastHover = that.origin = { x: event.pageX, y: event.pageY };

      event.preventDefault();
    };

    this._mouseUp = function () {
      that.drag = false;
    };

    this._mouseMove = function (event) {
      if (that.drag) {
        var relative = { x: event.pageX - that.origin.x, y: event.pageY - that.origin.y },
            delta = { x: event.pageX - that.lastHover.x, y: event.pageY - that.lastHover.y };
        that.lastHover = { x: event.pageX, y: event.pageY };
        that.moved(that.origin, relative, delta);
      }
    };
	
	this._keyDown = function (event) {
		that.keys[event.keyCode] = true;
		//console.log(event.keyCode);
	}
	this._keyUp = function (event) {
		that.keys[event.keyCode] = false;
	}

    if (this.element) {
      this.element.addEventListener('mousedown', this._mouseDown, false);
      this.element.addEventListener('keydown', this._keyDown, false);
      this.element.addEventListener('keyup', this._keyUp, false);
      document.addEventListener('mouseup', this._mouseUp, false);
      document.addEventListener('mousemove', this._mouseMove, false);
    }
  },

  stop: function () {
    if (this.element) {
      this.element.removeEventListener('mousedown', this._mouseDown);
      this.element.removeEventListener('keydown', this._keyDown);
      this.element.removeEventListener('keyup', this._keyUp);
      document.removeEventListener('mouseup', this._mouseUp);
      document.removeEventListener('mousemove', this._mouseMove);
    }
  },

  moved: function (origin, relative, delta) {
    //this.emit('change');
	var scale = 0.1*Math.PI/180;
	//this.camera.rotation.y -= delta.x*scale;
	//this.camera.rotation.x -= delta.y*scale;
	this.rotVector.x += delta.x*scale;
	this.rotVector.y += delta.y*scale;
  },

  update: function () {
	
	var dt = this.clock.getDelta();
	this.dtVector.set(dt, dt, dt);
	
	this.camera.fov = this.fov;
	
	var W = 87, A = 65, S = 83, D = 68, Q = 81, E = 69;
	var speed = 0.1;
	this.velocity.z += (
		(this.keys[W] ? -speed : 0) +
		(this.keys[S] ? speed : 0)
	)
	this.velocity.x += (
		(this.keys[A] ? -speed : 0) +
		(this.keys[D] ? speed : 0)
	)
	this.velocity.y += (
		(this.keys[Q] ? -speed : 0) +
		(this.keys[E] ? speed : 0)
	)
	
	this.velocity.multiplySelf(this.dampingVector);
	this.velocityScaled.copy(this.velocity);
	this.velocityScaled.multiplySelf(this.dtVector);
	
	this.camera.translateX(this.velocityScaled.x);
	this.camera.translateY(this.velocityScaled.y);
	this.camera.translateZ(this.velocityScaled.z);
	
	
	this.phi = Math.PI/2 + this.rotVector.y;
	this.theta = this.rotVector.x;
	
	this.lookAt.x = this.camera.position.x + 100 * Math.sin( this.phi ) * Math.cos( this.theta );
	this.lookAt.y = this.camera.position.y + 100 * Math.cos( this.phi );
	this.lookAt.z = this.camera.position.z + 100 * Math.sin( this.phi ) * Math.sin( this.theta );
	
	//this.lookAt.set(0, Math.sin(this.ry), -Math.cos(this.ry));
	//this.lookAt.set(Math.sin(this.rx), Math.cos(this.rx), 0);
	//this.lookAt.set(Math.sin(this.rx), Math.cos(this.rx)+Math.sin(this.ry), -Math.cos(this.ry));
	//this.lookAt.set(0, 0, -1);
	//this.lookAt.normalize();
	//console.log(this.lookAt, this.camera.position);
	//this.lookAt.addSelf(this.camera.position);
	this.camera.lookAt(this.lookAt)
	
	//this.camera.lookAt(new THREE.Vector3(0, 0, 0))
	  /*
    this.camera.position.x = this.position.x;
    this.camera.position.y = this.position.y;
    this.camera.position.z = this.position.z;

    if (this.camera.position.addSelf) {
      this.camera.position.addSelf(this.lookAt);
    }
    else {
      this.camera.position.add(this.lookAt);
    }
    this.camera.lookAt(this.lookAt);
	*/
  }//,

};

FPSControls.bind  = function (camera, domElement, options) {
  return new FPSControls(camera, domElement, options);
}


DomReady.ready(function() {
	  function flat(a, b, x, y) {
    return [a/3, 0.001, b/3];
  }
  
	var scene = new THREE.Scene();
	
	// MathBox boilerplate
	var mathbox = window.mathbox = mathBox({
		cameraControls: true,
		cursor:         true,
		controlClass:   FPSControls,
		elementResize:  true,
		fullscreen:     true,
		screenshot:     true,
		stats:          false,
		scale:          1,
		scene: 			scene,
	}).start();
	
	//rangeTrans(sphere, viewRange)
	
	/*
	var pointLight = new THREE.PointLight( 0xffffff, 1 );
	pointLight.position.x = 2;
	pointLight.position.y = 2;
	pointLight.position.z = 2;
	scene.add( pointLight );
	*/
	
	
	mathbox
		// Cartesian viewport
		.viewport({
			type: 'cartesian',
			range: [[-10, 10], [-10, 10], [-10, 10]],
			scale: [1, 1, 1],
		})
		.camera({
		})
		// Apply automatic 300ms fade in/out
		.transition(300)
		
		// Add XYZ axes
		.axis({
			id: 'x-axis',
			axis: 0,
			color: 0xa0a0a0,
			ticks: 5,
			lineWidth: 2,
			size: .05,
			labels: true,
		})
		.axis({
			id: 'y-axis',
			axis: 1,
			color: 0xa0a0a0,
			ticks: 5,
			lineWidth: 2,
			size: .05,
			labels: true,
			zero: false,
		})
		.axis({
			id: 'z-axis',
			axis: 2,
			color: 0xa0a0a0,
			ticks: 5,
			lineWidth: 2,
			size: .05,
			zero: false,
			labels: true,
		})
		// Grid
		.grid({
			id: 'grid-xz',
			axis: [0, 2],
		})
	
	mathbox.update()
	
	//*
	var camControls = new THREE.FirstPersonControls(mathbox.cameraProxy.controls.camera);
	camControls.lookSpeed = 0.4;
	camControls.movementSpeed = 20;
	camControls.noFly = true;
	camControls.lookVertical = true;
	camControls.constrainVertical = true;
	camControls.verticalMin = 1.0;
	camControls.verticalMax = 2.0;
	camControls.lon = -150;
	camControls.lat = 120;
	//*/

	addScene(window.mathboxScene);
	
	// Set up director
	var director = window.director = new MathBox.Director(mathbox, window.mathboxScript);
	director.forward()
	
	// Arrow controls
	// Controls for stand-alone
	/*
	window.addEventListener('touchstart', function (e) {
		director.forward();
	});
	*/
	window.addEventListener('keydown', function (e) {
		if (e.keyCode == 38 || e.keyCode == 37) director.back();
		else if (e.keyCode == 40 || e.keyCode == 39) director.forward();
		else {
			return;
		}
		console.log(director.step)
	});
	
	/*
	var clock = new THREE.Clock();
	render();
	function render() {
		var delta = clock.getDelta();
		camControls.update();
		//console.log(mathbox.cameraProxy.controls.camera.position);
		//webGLRenderer.clear();
		mathbox.update();
		// render using requestAnimationFrame
		requestAnimationFrame(render);
		//webGLRenderer.render(scene, camera)
	}
	*/
	
})