function getBrowserSize() {
    var actualWidth = window.innerWidth ||
                      document.documentElement.clientWidth ||
                      document.body.clientWidth ||
                      document.body.offsetWidth;
    var actualHeight = window.innerHeight ||
                      document.documentElement.clientHeight ||
                      document.body.clientHeight ||
                      document.body.offsetHeight;
    return { "width": actualWidth, "height" : actualHeight };
}

function getContext(canvas) {
    var settings = { preserveDrawingBuffer: true };
    gl = canvas.getContext("webgl", settings) ||
         canvas.getContext("experimental-webgl", settings);
    return gl;
}

function physicalSizeRatio() {
    var div = document.createElement("div");
    div.style.width = "1mm";
    div.style.height = "1mm";
    var body = document.getElementsByTagName("body")[0];
    body.appendChild(div);
    var physicalWidth = document.defaultView.getComputedStyle(div, null).getPropertyValue('width');
    var physicalHeight = document.defaultView.getComputedStyle(div, null).getPropertyValue('height');
    body.removeChild(div);
    return {
        'width' : parseFloat(physicalWidth),
        'height' : parseFloat(physicalHeight)
    };
}

window.onload = function () {
    var DEBUG = 0;
    var canvas;
    var socket = new WebSocket("ws://" + host + ":" + port);
    socket.binaryType = "arraybuffer";
    var CONNECT_SERIAL = 666;
    var gl;
    var startTime = new Date();
    // There is no way to get proper vsync since we have no idea when the real
    // swap happens under the hood. What we can do is to delay the response for
    // the eglSwapBuffer call, i.e. block the client for the given number of
    // milliseconds on each swap.
    var SWAP_DELAY = 16; //${swap_delay};
    var contextData = { }; // context -> { shaderMap, programMap, ... }
    var currentContext = 0;
    var binaryDataBuffer = new Uint8Array(0);
    var currentWindowId = "";
    var windowData = {};
    var currentZIndex = 1;
    var textDecoder;
    if (typeof TextDecoder !== 'undefined') {
        textDecoder = new TextDecoder("utf8");
    } else {
    textDecoder = {
            "decode" : function (buffer)
            {
                var string = String.fromCharCode.apply(String, buffer);
                return string;
            }
        };
    }

    var sendObject = function (obj) { socket.send(JSON.stringify(obj)); }

    var connect = function () {
        var size = getBrowserSize();
        var width = size.width;
        var height = size.height;
        var physicalSize = physicalSizeRatio();

        var object = { "type" : "connect",
            "width" : width, "height" : height,
            "physicalWidth" : width / physicalSize.width,
            "physicalHeight" : height / physicalSize.height
        };
        sendObject(object);
    };

    var sendResponse = function (id, value) {
        if (DEBUG)
            console.log("Response to " + id + " = " + value);
        sendObject({ "type": "gl_response", "id": id, "value": value });
    };

    var sendResize = function (width, height, physicalWidth, physicalHeight) {
        if (DEBUG)
            console.log("Resizing canvas to " + width + " x " + height);
        var object = { "type": "canvas_resize",
            "width": width, "height": height,
            "physicalWidth" : physicalWidth, "physicalHeight" : physicalHeight
        };
        sendObject(socket);
    };

    var createLoadingCanvas = function(name, x, y, width, height) {
        var canvas = document.createElement("canvas");
        canvas.id = "loading_" + name;
        canvas.style.position = "absolute";
        canvas.style.left = x + "px";
        canvas.style.top = y + "px";
        canvas.style.width = width + "px";
        canvas.style.height = height + "px";
        canvas.style.zIndex = currentZIndex++;
        canvas.style.background = "black";
        canvas.width = width;
        canvas.height = height;
        var body = document.getElementsByTagName("body")[0];
        body.appendChild(canvas);

        var gl = canvas.getContext("webgl");

        var loadingVertexShaderSource =
            "attribute vec2 a_position;"+
            "void main() {"+
            "    gl_Position = vec4(a_position, 0, 1);"+
            "}";
        var loadingFragmentShaderSource = // https://www.shadertoy.com/view/XdBXzd
            "#define SMOOTH(r) (mix(1.0, 0.0, smoothstep(0.9,1.0, r)))\n"+
            "#define M_PI 3.1415926535897932384626433832795\n"+
            "precision mediump float;\n"+
            "uniform float u_time;\n"+
            "uniform vec2 u_size;\n"+
            "float movingRing(vec2 uv, vec2 center, float r1, float r2)\n"+
            "{\n"+
            "    vec2 d = uv - center;\n"+
            "    float r = sqrt( dot( d, d ) );\n"+
            "    d = normalize(d);\n"+
            "    float theta = -atan(d.y,d.x);\n"+
            "    theta  = mod(-u_time+0.5*(1.0+theta/M_PI), 1.0);\n"+
            "    theta -= max(theta - 1.0 + 1e-2, 0.0) * 1e2;\n"+
            "    return theta*(SMOOTH(r/r2)-SMOOTH(r/r1));\n"+
            "}\n"+
            "void main()"+
            "{\n"+
            "    vec2 uv = gl_FragCoord.xy;\n"+
            "    float ring = movingRing(uv, vec2(u_size.x/2.0,u_size.y/2.0), 20.0, 30.0);\n"+
            "    gl_FragColor = vec4( 0.1 + 0.9*ring );\n"+
            "}";

        var vertexShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vertexShader, loadingVertexShaderSource);
        gl.compileShader(vertexShader);

        var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(fragmentShader, loadingFragmentShaderSource);
        gl.compileShader(fragmentShader);

        var program = gl.createProgram();
        gl.attachShader(program, vertexShader);
        gl.attachShader(program, fragmentShader);
        gl.linkProgram(program);
        gl.useProgram(program);

        // look up where the vertex data needs to go.
        var positionLocation = gl.getAttribLocation(program, "a_position");
        var timeLocation = gl.getUniformLocation(program, "u_time");
        var sizeLocation = gl.getUniformLocation(program, "u_size");

        // Create a buffer and put a single clipspace rectangle in
        // it (2 triangles)
        var buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(
            gl.ARRAY_BUFFER,
            new Float32Array([
                -1.0, -1.0,
                 1.0, -1.0,
                -1.0,  1.0,
                -1.0,  1.0,
                 1.0, -1.0,
                 1.0,  1.0]),
            gl.STATIC_DRAW);
        gl.enableVertexAttribArray(positionLocation);
        gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);
        gl.uniform2fv(sizeLocation, new Float32Array([canvas.width, canvas.height]));
        var time = 0.0;

        function draw() {
            if (canvas) {
                gl.uniform1f(timeLocation, time);
                time += 0.01;
                gl.drawArrays(gl.TRIANGLES, 0, 6);
                setTimeout(draw, 16);
            }
        }
        draw();
        return canvas;
    };

    var createCanvas = function (name, x, y, width, height, title) {
        var canvas = document.createElement("canvas");
        canvas.id = name;
        canvas.style.position = "absolute";
        canvas.style.left = x + "px";
        canvas.style.top = y + "px";
        canvas.style.width = width + "px";
        canvas.style.height = height + "px";
        canvas.style.zIndex = currentZIndex++;
        canvas.width = width;
        canvas.height = height;
        var body = document.getElementsByTagName("body")[0];
        body.appendChild(canvas);

        var qtButtons = 0;
        var sendMouseEvent = function (buttons, localX, localY, globalX, globalY, name) {
            var object = { "type": "mouse",
                "buttons": buttons,
                "localX": localX, "localY": localY, "globalX" : globalX, "globalY" : globalY,
                "time" : new Date().getTime(),
                "name" : name
            };
            sendObject(object);
        };

        var mapButton = function (b) {
            var qtb = 1;
            if (b === 1)
                qtb = 4;
            else if (b === 2)
                qtb = 2;
            return qtb;
        };

        canvas.onmousedown = function (event) {
            qtButtons |= mapButton(event.button);
            sendMouseEvent(qtButtons, event.layerX, event.layerY, event.clientX, event.clientY,
                           name);
        };

        canvas.onmousemove = function (event) {
            sendMouseEvent(qtButtons, event.layerX, event.layerY, event.clientX, event.clientY,
                           name);
        };

        canvas.onmouseup = function (event) {
            qtButtons &= ~mapButton(event.button);
            sendMouseEvent(qtButtons, event.layerX, event.layerY, event.clientX, event.clientY,
                           name);
        };

        function handleMouseWheel(event) {
            var deltaY = 0;
            if (!event)
                deltaY = window.event;
            if (event.deltaY)
                deltaY = event.deltaY;
            else if (event.detail)
                deltaY = event.detail * 40;
            if (deltaY) {
                var object = { "type" : "wheel",
                    "localX" : event.layerX, "localY" : event.layerY,
                    "globalX" : event.clientX, "globalY" : event.clientY,
                    "deltaX" : event.deltaX, "deltaY" : deltaY, "deltaZ" : event.deltaZ,
                    "time" : new Date().getTime(),
                    "name" : name
                };
                sendObject(object);
            }
            if (event.preventDefault)
                event.preventDefault();
            event.returnValue = false;
        }

        if ("onmousewheel" in canvas)
            canvas.onmousewheel = handleMouseWheel;
        else
            canvas.addEventListener('DOMMouseScroll', handleMouseWheel, false);

        function handleTouch(event) {
            var object = {};
            object["type"] = "touch";
            object["name"] = name;
            object["time"] = new Date().getTime();
            object["event"] = event.type;
            object["changedTouches"] = [];
            for (var i = 0; i < event.targetTouches.length; ++i) {
                var changedTouch = event.targetTouches[i];
                var touch = {};
                touch["clientX"] = changedTouch.clientX;
                touch["clientY"] = changedTouch.clientY;
                touch["force"] = changedTouch.force;
                touch["identifier"] = changedTouch.identifier;
                touch["pageX"] = changedTouch.pageX;
                touch["pageY"] = changedTouch.pageY;
                touch["radiousX"] = changedTouch.radiousX;
                touch["radiousY"] = changedTouch.radiousY;
                touch["rotatingAngle"] = changedTouch.rotatingAngle;
                touch["screenX"] = changedTouch.screenX;
                touch["screenY"] = changedTouch.screenY;
                touch["normalPositionX"] = changedTouch.screenX / screen.width;
                touch["normalPositionY"] = changedTouch.screenY / screen.height;
                object.changedTouches.push(touch);
            }
            sendObject(object);

            if (event.preventDefault && event.cancelable)
                event.preventDefault();
            event.returnValue = false;
        }

        canvas.addEventListener("touchstart", handleTouch, false);
        canvas.addEventListener("touchend", handleTouch, false);
        canvas.addEventListener("touchcancel", handleTouch, false);
        canvas.addEventListener("touchmove", handleTouch, false);

        canvas.oncontextmenu = function (event) {
            event.preventDefault();
        };


        var gl = getContext(canvas);
        gl.clear([ gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT]);
        var data = windowData[name] = {
            "canvas" : canvas,
            "gl" : gl,
            "loadingCanvas" : createLoadingCanvas(name, x, y, width, height)
        };

        var defaultValuesObject = { 'type' : 'default_context_parameters', 'name' : name,
            '7939' : "GL_OES_element_index_uint GL_OES_standard_derivatives " + // GL_EXTENSIONS
                     "GL_OES_depth_texture GL_OES_packed_depth_stencil" };
        [
//            gl.ACTIVE_TEXTURE,
//            gl.ALIASED_LINE_WIDTH_RANGE,
//            gl.ALIASED_POINT_SIZE_RANGE,
//            gl.ALPHA_BITS,
            gl.BLEND,
//            gl.BLEND_COLOR,
//            gl.BLEND_DST_ALPHA,
//            gl.BLEND_DST_RGB,
//            gl.BLEND_EQUATION,
//            gl.BLEND_EQUATION_ALPHA,
//            gl.BLEND_EQUATION_RGB,
//            gl.BLEND_SRC_ALPHA,
//            gl.BLEND_SRC_RGB,
//            gl.BLUE_BITS,
//            gl.COLOR_CLEAR_VALUE,
//            gl.COLOR_WRITEMASK,
//            gl.COMPRESSED_TEXTURE_FORMATS,
//            gl.CULL_FACE,
//            gl.CULL_FACE_MODE,
//            gl.DEPTH_BITS,
//            gl.DEPTH_CLEAR_VALUE,
//            gl.DEPTH_FUNC,
//            gl.DEPTH_RANGE,
            gl.DEPTH_TEST,
//            gl.DEPTH_WRITEMASK,
//            gl.DITHER,
//            gl.FRONT_FACE,
//            gl.GENERATE_MIPMAP_HINT,
//            gl.GREEN_BITS,
//            gl.IMPLEMENTATION_COLOR_READ_FORMAT,
//            gl.IMPLEMENTATION_COLOR_READ_TYPE,
//            gl.LINE_WIDTH,
//            gl.MAX_COMBINED_TEXTURE_IMAGE_UNITS,
//            gl.MAX_CUBE_MAP_TEXTURE_SIZE,
//            gl.MAX_FRAGMENT_UNIFORM_VECTORS,
//            gl.MAX_RENDERBUFFER_SIZE,
//            gl.MAX_TEXTURE_IMAGE_UNITS,
            gl.MAX_TEXTURE_SIZE,
//            gl.MAX_VARYING_VECTORS,
            gl.MAX_VERTEX_ATTRIBS,
//            gl.MAX_VERTEX_TEXTURE_IMAGE_UNITS,
//            gl.MAX_VERTEX_UNIFORM_VECTORS,
//            gl.MAX_VIEWPORT_DIMS,
//            gl.PACK_ALIGNMENT,
//            gl.POLYGON_OFFSET_FACTOR,
//            gl.POLYGON_OFFSET_FILL,
//            gl.POLYGON_OFFSET_UNITS,
//            gl.RED_BITS,
            gl.RENDERER,
//            gl.SAMPLE_BUFFERS,
//            gl.SAMPLE_COVERAGE_INVERT,
//            gl.SAMPLE_COVERAGE_VALUE,
//            gl.SAMPLES,
//            gl.SCISSOR_BOX,
            gl.SCISSOR_TEST,
//            gl.SHADING_LANGUAGE_VERSION,
//            gl.STENCIL_BACK_FAIL,
//            gl.STENCIL_BACK_FUNC,
//            gl.STENCIL_BACK_PASS_DEPTH_FAIL,
//            gl.STENCIL_BACK_PASS_DEPTH_PASS,
//            gl.STENCIL_BACK_REF,
//            gl.STENCIL_BACK_VALUE_MASK,
//            gl.STENCIL_BACK_WRITEMASK,
//            gl.STENCIL_BITS,
//            gl.STENCIL_CLEAR_VALUE,
//            gl.STENCIL_FAIL,
//            gl.STENCIL_FUNC,
//            gl.STENCIL_PASS_DEPTH_FAIL,
//            gl.STENCIL_PASS_DEPTH_PASS,
//            gl.STENCIL_REF,
            gl.STENCIL_TEST,
//            gl.STENCIL_VALUE_MASK,
//            gl.STENCIL_WRITEMASK,
//            gl.SUBPIXEL_BITS,
            gl.UNPACK_ALIGNMENT,
//            gl.UNPACK_COLORSPACE_CONVERSION_WEBGL,
//            gl.UNPACK_FLIP_Y_WEBGL,
//            gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL,
            gl.VENDOR,
            gl.VERSION,
            gl.VIEWPORT
        ].forEach(function (value) {
            defaultValuesObject[value] = gl.getParameter(value);
        });
        sendObject(defaultValuesObject);

        gl._attachShader = gl.attachShader;
        gl.attachShader = function(program, shader) {
            var d = contextData[currentContext];
            gl._attachShader(d.programMap[program], d.shaderMap[shader].shader);
        };

        gl._bindAttribLocation = gl.bindAttribLocation;
        gl.bindAttribLocation = function(program, index, name) {
            var d = contextData[currentContext];
            gl._bindAttribLocation(d.programMap[program], index, name);
        };

        gl._bindBuffer = gl.bindBuffer;
        gl.bindBuffer = function(target, buffer) {
            var d = contextData[currentContext];
            gl._bindBuffer(target, buffer ? d.bufferMap[buffer] : null);
        };

        gl._bindFramebuffer = gl.bindFramebuffer;
        gl.bindFramebuffer = function(target, framebuffer) {
            var d = contextData[currentContext];
            gl._bindFramebuffer(target, framebuffer ? d.framebufferMap[framebuffer] : null);
        };

        gl._bindRenderbuffer = gl.bindRenderbuffer;
        gl.bindRenderbuffer = function(target, renderbuffer) {
            var d = contextData[currentContext];
            gl._bindRenderbuffer(target, renderbuffer ? d.renderbufferMap[renderbuffer] : null);
            d.boundRenderbuffer = renderbuffer;
        };

        gl._bindTexture = gl.bindTexture;
        gl.bindTexture = function(target, texture) {
            gl._bindTexture(target, texture ? mapTexture(currentContext, texture) : null);
        };

        gl._bufferData = gl.bufferData;
        gl.bufferData = function(target, usage, size, data) {
            gl._bufferData(target, data.length === 0 ? size : data, usage);
        };

        gl._clearColor = gl.clearColor;
        gl.clearColor = function (red, green, blue, alpha) {
            gl._clearColor(red, green, blue, alpha);
        }

        gl.clearDepthf = function(depth) {
            gl.clearDepth(depth);
        };

        gl._compileShader = gl.compileShader;
        gl.compileShader = function(remoteShader) {
            var d = contextData[currentContext];
            gl._compileShader(d.shaderMap[remoteShader].shader);
        };

        gl._createProgram = gl.createProgram;
        gl.createProgram = function() {
            var d = contextData[currentContext];
            var remoteProgram = d.nextProgramId++;
            var localProgram = gl._createProgram();
            d.programMap[remoteProgram] = localProgram;
            return remoteProgram;
        };

        gl._createShader = gl.createShader;
        gl.createShader = function(type) {
            var d = contextData[currentContext];
            var remoteShader = d.nextShaderId++;
            var localShader = gl._createShader(type);
            d.shaderMap[remoteShader] = { };
            d.shaderMap[remoteShader].shader = localShader;
            d.shaderMap[remoteShader].source = "";
            return remoteShader;
        };

        gl.deleteBuffers = function(n) {
            var d = contextData[currentContext];
            for (var i = 0; i < n; ++i)
                gl.deleteBuffer(d.bufferMap[arguments[1 +i]]);
        };

        gl._deleteFramebuffers = gl.deleteFramebuffers;
        gl.deleteFramebuffers = function(n) {
            var d = contextData[currentContext];
            for (var i = 0; i < n; ++i)
                gl.deleteFramebuffer(d.framebufferMap[arguments[1 + i]]);
        };

        gl._deleteProgram = gl.deleteProgram;
        gl.deleteProgram = function(program) {
            var d = contextData[currentContext];
            gl._deleteProgram(d.programMap[program]);
        };

        gl._deleteRenderbuffers = gl.deleteRenderbuffers;
        gl.deleteRenderbuffers = function() {
            var d = contextData[currentContext];
            for (var i in arguments)
                gl.deleteRenderbuffer(d.renderbufferMap[arguments[i]]);
        };

        gl._deleteShader = gl.deleteShader;
        gl.deleteShader = function(remoteShader) {
            var d = contextData[currentContext];
            gl._deleteShader(d.shaderMap[remoteShader].shader);
        };

        gl.deleteTextures = function(n) {
            for (var i = 0; i < n; ++i)
                gl.deleteTexture(mapTexture(currentContext, arguments[1 + i]));
        };

        gl._drawElements = gl.drawElements;
        gl.drawElements = function(mode, count, type, n, indices, offset) {
            if (!arguments[3].length)
                gl._drawElements(mode, count, type, offset);
            else
                console.error("fixme: Client-side drawElements not supported");
        };

        gl._framebufferRenderbuffer = gl.framebufferRenderbuffer;
        gl.framebufferRenderbuffer = function(target, attachment, renderbuffertarget, renderbuffer)
        {
            var d = contextData[currentContext];
            // With packed depth-stencil Quick tries to attach the same renderbuffer to both
            // the depth and stencil attachment points. WebGL does not allow this. Instead,
            // we need to attach to the DEPTH_STENCIL attachment point.
            if (d.renderbufferFormat[d.boundRenderbuffer] === gl.DEPTH_STENCIL) {
                if (attachment === gl.STENCIL_ATTACHMENT)
                    attachment = gl.DEPTH_STENCIL_ATTACHMENT;
            }
            gl._framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                        d.renderbufferMap[renderbuffer]);
        };

        gl.genBuffers = function(n) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < n; ++i) {
                var remoteBuf = d.nextBufferId++
                var localBuf = gl.createBuffer();
                data.push(remoteBuf);
                d.bufferMap[remoteBuf] = localBuf;
            }
            return data;
        };

        gl.genFramebuffers = function(n) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < n; ++i) {
                var remoteFramebuffer = d.nextFramebufferId++;
                var localFramebuffer = gl.createFramebuffer();
                d.framebufferMap[remoteFramebuffer] = localFramebuffer;
                data.push(remoteFramebuffer);
            }
            return data;
        };

        gl._genRenderbuffers = gl.genRenderbuffers;
        gl.genRenderbuffers = function(n) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < n; ++i) {
                var remoteRenderBuffer = d.nextRenderBufferId++;
                var localRenderBuffer = gl.createRenderbuffer();
                d.renderbufferMap[remoteRenderBuffer] = localRenderBuffer;
                data.push(remoteRenderBuffer);
            }
            return data;
        };

        gl.getAttachedShaders = function(program, maxCount) {
            var d = contextData[currentContext];
            var shaders = d.attachedShaderMap[program];
            var data = [];
            for (var shader in shaders) {
                for (var remoteShaderId in shaderMap) {
                    if (shaderMap[remoteShaderId].shader === shader)
                        data.push(remoteShaderId);
                }
            }
            return data;
        };

        gl._getAttribLocation = gl.getAttribLocation;
        gl.getAttribLocation = function(program, name) {
            if (typeof program === "object")
                return gl._getAttribLocation(program, name);
            var d = contextData[currentContext];
            return gl._getAttribLocation(d.programMap[program], name);
        };

        gl.getBooleanv = gl.getParameter;
        gl.getIntegerv = gl.getParameter;

        gl.getProgramiv = function(program, pname) {
            var d = contextData[currentContext];
            if (pname === 0x8B84) // INFO_LOG_LENGTH
                return gl._getProgramInfoLog(d.programMap[program]).length;
            else
                return gl.getProgramParameter(d.programMap[program], pname);
        };

        gl._getShaderInfoLog = gl.getShaderInfoLog;
        gl.getShaderInfoLog = function(shader) {
            if (typeof shader === "object")
                return gl._getShaderInfoLog(shader);
            var d = contextData[currentContext];
            return gl._getShaderInfoLog(d.shaderMap[shader].shader);
        };

        gl.getShaderiv = function(shader, pname) {
            var d = contextData[currentContext];
            var p;
            if (pname === 0x8B88)
                return d.shaderMap[shader].source.length;
            else
                return gl.getShaderParameter(d.shaderMap[shader].shader, pname);
        };

        gl._getShaderSource = gl.getShaderSource;
        gl.getShaderSource = function(remoteShader) {
            var d = contextData[currentContext];
            return gl._getShaderSource(d.shaderMap[remoteShader].shader);
        };

        gl.getString = function(pname) {
            var result = "";
            return gl.getParameter(pname);
        };

        gl._getProgramInfoLog = gl.getProgramInfoLog;
        gl.getProgramInfoLog = function(remoteProgram) {
            var d = contextData[currentContext];
            var localProgram = d.programMap[remoteProgram];
            return gl._getProgramInfoLog(localProgram);
        };

        gl._getUniformLocation = gl.getUniformLocation;
        gl.getUniformLocation = function(program, name) {
            if (typeof program === "object") {
                return gl._getUniformLocation(program, name);
            } else {
                var d = contextData[currentContext];
                var p = gl._getUniformLocation(d.programMap[program], name);
                var location = -1;
                if (p) {
                    location = d.nextLocation++;
                    d.uniformLocationMap[location] = p;
                }
                return location;
            }
        };

        gl.genTextures = function(n) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < n; ++i) {
                var remoteTexture = d.nextTextureId++;
                var localTexture = gl.createTexture();
                d.textureMap[remoteTexture] = localTexture;
                data.push(remoteTexture);
            }
            return data;
        };

        gl._framebufferTexture2D = gl.framebufferTexture2D;
        gl.framebufferTexture2D = function(target, attachment, texTarget, texture, level) {
            var d = contextData[currentContext];
            gl._framebufferTexture2D(target, attachment, texTarget, d.textureMap[texture], level);
        };

        gl._isRenderbuffer = gl.isRenderbuffer;
        gl.isRenderbuffer = function(renderbuffer) {
            if (typeof renderbuffer === "object")
                return gl._isRenderBuffer(renderbuffer);
            var d = contextData[currentContext];
            return gl._isRenderbuffer(d.renderbufferMap[renderbuffer]);
        }

        gl._linkProgram = gl.linkProgram;
        gl.linkProgram = function(program) {
            var d = contextData[currentContext];
            gl._linkProgram(d.programMap[program]);
        };

        gl._renderbufferStorage = gl.renderbufferStorage;
        gl.renderbufferStorage = function(target, internalFormat, width, height) {
            var d = contextData[currentContext];
            if (internalFormat === 0x88F0) // GL_DEPTH24_STENCIL8_OES
                internalformat = 0x84F9; // GL_DEPTH_STENCIL_OES
            d.renderbufferFormat[d.boundRenderbuffer] = internalFormat;
            gl._renderbufferStorage(target, internalFormat, width, height);
        };

        gl._texImage2D = gl.texImage2D;
        gl.texImage2D = function(target, level, internalFormat, width, height, border, format, type,
                                 data) {
            var dataSize = data ? data.byteLength : 0;
            var pixels;
            if (data === null || dataSize === 0)
                pixels = null;
            else if (type === gl.UNSIGNED_BYTE)
                pixels = data;
            else if (type === g.UNSIGNED_SHORT_5_6_5 || type === UNSIGNED_SHORT_4_4_4_4 ||
                       type === UNSIGNED_SHORT_5_5_5_1)
                pixels = new DataView(new Uint16Array(data));
            else
                console.error("gl.texImage2D: Unsupported type");
            gl._texImage2D(target, level, internalFormat, width, height, border, format, type,
                           pixels);
        };

        gl._texSubImage2D = gl.texSubImage2D;
        gl.texSubImage2D = function(target, level, xoffset, yoffset, width, height, format, type, data) {
            var dataSize = data ? data.byteLength : 0;
            var pixels;
            if (data === null || dataSize === 0)
                pixels = null;
            if (type === gl.UNSIGNED_BYTE)
                pixels = data;
            else if (type === gl.UNSIGNED_SHORT_5_6_5 || type === gl.UNSIGNED_SHORT_4_4_4_4 ||
                     type === gl.UNSIGNED_SHORT_5_5_5_1 || type === ext.HALF_FLOAT_OES)
                pixels = new Uint16Array(pixels);
            else if (type === gl.FLOAT)
                pixels = new Float32Array(pixels);
            else
                console.error("gl.texSubImage2D: Unsupported type");
            gl._texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        };

        gl._shaderSource = gl.shaderSource;
        gl.shaderSource = function(shader, count) {
            var d = contextData[currentContext];
            d.shaderMap[shader].source = "";
            for (var i = 0; i < count; ++i)
                d.shaderMap[shader].source += arguments[2 + i] + "\n";
            gl._shaderSource(d.shaderMap[shader].shader, d.shaderMap[shader].source);
        };

        gl._uniform1f = gl.uniform1f;
        gl.uniform1f = function(location, x) {
            var d = contextData[currentContext];
            gl._uniform1f(d.uniformLocationMap[location], x);
        };

        gl._uniform1fv = gl.uniform1fv;
        gl.uniform1fv = function(location, count) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < count; ++i)
                data.push(arguments[i + 2]);
            gl._uniform1fv(d.uniformLocationMap[location], data);
        };

        gl._uniform2fv = gl.uniform2fv;
        gl.uniform2fv = function(location, count) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < count * 2; ++i)
                data.push(arguments[i + 2]);
            gl._uniform2fv(d.uniformLocationMap[location], data);
        };

        gl._uniform3fv = gl.uniform3fv;
        gl.uniform3fv = function(location, count) {
            var d = contextData[context];
            var data = [];
            for (var i = 0; i < count * 3; ++i)
                data.push(arguments[i + 2]);
            gl._uniform3fv(d.uniformLocationMap[location], data);
        };

        gl._uniform1i = gl.uniform1i;
        gl.uniform1i = function(location, value) {
            var d = contextData[currentContext];
            gl._uniform1i(d.uniformLocationMap[location], value);
        };

        gl._uniform4fv = gl.uniform4fv;
        gl.uniform4fv = function(location, count) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < count * 4; ++i)
                data.push(arguments[i + 2]);
            gl._uniform4fv(d.uniformLocationMap[location], data);
        };

        gl._uniformMatrix3fv = gl.uniformMatrix3fv;
        gl.uniformMatrix3fv = function(location, count, transpose) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < count * 9; ++i)
                data.push(arguments[i + 3]);
            gl._uniformMatrix3fv(d.uniformLocationMap[location], transpose, data);
        };

        gl._uniformMatrix4fv = gl.uniformMatrix4fv;
        gl.uniformMatrix4fv = function(location, count, transpose) {
            var d = contextData[currentContext];
            var data = [];
            for (var i = 0; i < count * 16; ++i)
                data.push(arguments[i + 3]);
            gl._uniformMatrix4fv(d.uniformLocationMap[location], transpose, data);
        };

        gl._useProgram = gl.useProgram;
        gl.useProgram = function(program) {
            var d = contextData[currentContext];
            gl._useProgram(program !== 0 ? d.programMap[program] : null);
        };

        gl._vertexAttrib1fv = gl.vertexAttrib1fv;
        gl.vertexAttrib1fv = function (index, v0) {
            var values = new Float32Array([v0]);
            gl._vertexAttrib1fv(index, values);
        }

        gl._vertexAttrib2fv = gl.vertexAttrib2fv;
        gl.vertexAttrib2fv = function (index, v0, v1) {
            var values = new Float32Array([v0, v1]);
            gl._vertexAttrib2fv(index, values);
        }

        gl._vertexAttrib3fv = gl.vertexAttrib3fv;
        gl.vertexAttrib3fv = function (index, v0, v1, v2) {
            var values = new Float32Array([v0, v1, v2]);
            gl._vertexAttrib3fv(index, values);
        }

        gl._drawArrays = gl.drawArrays;
        gl.drawArrays = function (mode, first, count/*, size*/) {
            var d = contextData[currentContext];
            var subDataParts = [];
            var bufferSize = 0;
            var offset = 0;
            if (!d.drawArrayBuf)
                d.drawArrayBuf = gl.createBuffer();
            gl._bindBuffer(gl.ARRAY_BUFFER, d.drawArrayBuf);
            for (var i = 4; i < arguments.length; i += 6) {
                var subData = {};
                subData["index"] = arguments[i + 0];
                subData["size"] = arguments[i + 1];
                subData["type"] = arguments[i + 2];
                subData["normalized"] = arguments[i + 3];
                subData["stride"] = arguments[i + 4];
                subData["offset"] = 0;
                subData["data"] = arguments[i + 5];
                subDataParts.push(subData);
                bufferSize += subData.data.length;
            }
            gl._bufferData(gl.ARRAY_BUFFER, bufferSize, gl.STATIC_DRAW);
            for (var part in subDataParts) {
                gl.bufferSubData(gl.ARRAY_BUFFER, offset, subDataParts[part].data);
                gl._vertexAttribPointer(subDataParts[part].index,
                                        subDataParts[part].size,
                                        subDataParts[part].type,
                                        subDataParts[part].normalized,
                                        subDataParts[part].stride,
                                        offset);
                offset += subDataParts[part].data.length;
            }
            gl._drawArrays(mode, first, count);
        }

        gl._vertexAttribPointer = gl.vertexAttribPointer;
        gl.vertexAttribPointer = function (index, size, type, normalized, stride, pointer) {
            gl._vertexAttribPointer(index, size, type, normalized, stride, pointer);
        }


    }

    var commandsNeedingResponse = [
        "checkFramebufferStatus",
        "createProgram",
        "createShader",
        "genBuffers",
        "genFramebuffers",
        "genRenderbuffers",
        "genTextures",
        "getAttachedShaders",
        "getAttribLocation",
        "getBooleanv",
        "getError",
        "getFramebufferAttachmentParameteriv",
        "getIntegerv",
        "getParameter",
        "getProgramInfoLog",
        "getProgramiv",
        "getRenderbufferParameteriv",
        "getShaderiv",
        "getShaderPrecisionFormat",
        "getString",
        "getTexParameterfv",
        "getTexParameteriv",
        "getUniformfv",
        "getUniformLocation",
        "getUniformiv",
        "getVertexAttribfv",
        "getVertexAttribiv",
        "getShaderSource",
        "getShaderInfoLog",
        "isRenderbuffer"
    ];

    var ensureContextData = function (context) {
        if (!(context in contextData)) {
            contextData[context] = {
                shaderMap: { },
                programMap: { },
                textureMap: { },
                framebufferMap: { },
                renderbufferMap: { },
                renderbufferFormat: { },
                boundRenderbuffer: 0,
                bufferMap: { },
                uniformLocationMap: { },
                nextLocation: 1,
                nextBufferId: 1,
                nextProgramId: 1,
                nextShaderId: 1,
                nextFramebufferId: 1,
                nextRenderBufferId: 1,
                nextTextureId: 1,
                pendingBinary: [],
                drawArrayBuf: null,
                drawArrayBufSize: 0,
                glCommands: [],
                attribData: []
            };
        }
    };

    var mapTexture = function (context, localId) {
        var d = contextData[context];
        if (localId in d.textureMap)
            return d.textureMap[localId];
        // ### do we need sharing?
        console.error("Texture " + localId + " is not found in context " + context);
        return 0;
    };

    var execGL = function (context) {
        var d = contextData[context];
        if (DEBUG)
            console.log("executing " + d.glCommands.length + " commands");
        while (d.glCommands.length) {
            var obj = d.glCommands.shift();
            if (DEBUG)
                console.log("Calling: gl." + obj.function, obj.parameters);
            var response = gl[obj.function].apply(gl, obj.parameters);
            if (response !== undefined)
                sendResponse(obj.id, response);
        }
    };

    var injectGL = function (context, funcName, parameters) {
        contextData[context].glCommands.push({ "function": funcName, "parameters": parameters });
    };

    var handleBinaryMessage = function (event) {
        var appendBuffer = function(buffer1, buffer2) {
          var tmp = new Uint8Array(buffer1.byteLength + buffer2.byteLength);
          tmp.set(new Uint8Array(buffer1), 0);
          tmp.set(new Uint8Array(buffer2), buffer1.byteLength);
          return tmp.buffer;
        };

        var buffer = appendBuffer(binaryDataBuffer, event.data);
        var view = new DataView(buffer);
        if (view.getUint32(buffer.byteLength - 4) !== 0xbaadf00d) {
            binaryDataBuffer = buffer;
            return;
        }
        binaryDataBuffer = new ArrayBuffer(0);

        var offset = 0;
        var obj = { "parameters" : [] };
        obj["id"] = view.getUint32(offset);
        offset += 4;
        obj["functionNameSize"] = view.getUint32(offset);
        offset += 4;
        obj["function"] = textDecoder.decode(new Uint8Array(buffer, offset, obj.functionNameSize));
        offset += obj.functionNameSize;
        obj["parameterCount"] = view.getUint32(offset);
        offset += 4;
        for (var i = 0; i < obj.parameterCount; ++i) {
            var character = view.getUint8(offset);
            offset += 1;
            var parameterType = String.fromCharCode(character);
            if (parameterType === 'i') {
                obj.parameters.push(view.getInt32(offset));
                offset += 4;
            } else if (parameterType === 'u') {
                obj.parameters.push(view.getUint32(offset));
                offset += 4;
            } else if (parameterType === 'd') {
                obj.parameters.push(view.getFloat64(offset));
                offset += 8;
            } else if (parameterType === 'b') {
                obj.parameters.push(view.getUint8(offset) === 1);
                offset += 1;
                break;
            } else if (parameterType === 's') {
                var stringSize = view.getUint32(offset);
                offset += 4;
                var string = textDecoder.decode(new Uint8Array(buffer, offset, stringSize));
                obj.parameters.push(string);
                offset += stringSize;
            } else if (parameterType === 'x') {
                var dataSize = view.getUint32(offset);
                offset += 4;
                var data = new Uint8Array(buffer, offset, dataSize);
                var bytesRead = data.byteLength;
                if (bytesRead !== dataSize)
                    console.error("invalid data");
                obj.parameters.push(data);
                offset += dataSize;
            } else if (parameterType === 'n') {
                obj.parameters.push(null);
            }
        }
        var magic = view.getUint32(offset);
        if (magic !== 0xbaadf00d)
            console.error('Invalid magic');
        offset += 4;
        if (offset !== buffer.byteLength)
            console.error("Invalid buffer")

        if (!("function" in obj)) {
            console.error("Function not found");
        } else if (obj.function === "makeCurrent") {
            var winId = obj.parameters[3];
            if (winId in windowData) {
                canvas = windowData[winId].canvas;
                canvas.width = canvas.style.width = obj.parameters[1];
                canvas.height = canvas.style.height = obj.parameters[2];
                gl = windowData[winId].gl;
                currentWindowId = winId;
                currentContext = obj.parameters[0];
                if (DEBUG)
                    console.log("Current context is now " + currentContext);
                if (currentContext)
                    ensureContextData(currentContext);
            }
        } else if (obj.function === "swapBuffers") {
            var data =  windowData[currentWindowId];
            if (data.loadingCanvas) {
                var body = document.getElementsByTagName("body")[0];
                body.removeChild(data.loadingCanvas);
                data.loadingCanvas = undefined;
            }

            if (DEBUG)
                var t0 = performance.now();
            execGL(currentContext);
            if (startTime) {
                console.log((new Date() - startTime) + "ms to first frame.")
                startTime = undefined;
            }
            var frameTime = performance.now() - t0;
            if (DEBUG)
                console.log("Swap time: " + frameTime + " ms.");
            setTimeout((function () { sendResponse(obj.id, 1); }),
                       Math.max(SWAP_DELAY - frameTime, 0));
            // have preserved swap and now we need to clear for real
        } else {
            handleGlesMessage(obj);
        }
    }

    var handleGlesMessage = function (obj) {
        // A GLES call. Unfortunately WebGL swaps when the control gets back to
        // the event loop. This is totally retarded. So we queue the commands up
        // and issue them when receiving a function that needs a response. This
        // does not solve the problem and still needs preserved swap to be safe
        // since eglSwapBuffers is not the only command that expects a response,
        // but is more efficient than issuing everything from here.
        var d = contextData[currentContext];
        if (d)
            d.glCommands.push(obj);
        for (var i in commandsNeedingResponse)
            if (commandsNeedingResponse[i] === obj.function) {
                execGL(currentContext);
                break;
        }
    };

    socket.onopen = function (event) {
        console.log("Socket Open");
        (function(){
            var doCheck = true;
            var check = function(){
                var size = getBrowserSize();
                var width = size.width;
                var height = size.height;
                var physicalSize = physicalSizeRatio();

                var object = { "type" : "canvas_resize",
                    "width" : width, "height" : height,
                    "physicalWidth" : width / physicalSize.width,
                    "physicalHeight" : height / physicalSize.height
                };
                sendObject(object);
            };
            window.addEventListener("resize",(function(){
                if(doCheck){
                    check();
                    doCheck = false;
                    setTimeout((function(){
                        doCheck = true;
                        check();
                    }), 1000)
                }
            }));
        })();
        connect();
    };
    socket.onclose = function (event) {
        console.log("Socket Closed (" + event.code + "): " + event.reason);
    };
    socket.onerror = function (error) {
        console.log("Socket error: " + error.toString());
    };
    socket.onmessage = function (event) {
        if (event.data instanceof ArrayBuffer) {
            handleBinaryMessage(event);
            return;
        }
        var obj;
        try {
            obj = JSON.parse(event.data);
        } catch (e) {
            console.error("Failed to parse " + event.data + ": " + e.toString());
            return;
        }
        if (!("type" in obj)) {
            console.error("Message type not found");
        } else if (obj.type === "create_canvas") {
            createCanvas(obj.winId, obj.x, obj.y, obj.width, obj.height, obj.title);
            if (obj.title && obj.title.length)
                document.title = obj.title;
        } else if (obj.type === "destroy_canvas") {
            var canvas = document.getElementById(obj.winId);
            var body = document.getElementsByTagName("body")[0];
            body.removeChild(canvas);
        } else if (obj.type === "clipboard_updated") {
            // Opens a new window/tab and shows the current remote clipboard. There is no way to
            // copy some text to the local clipboard without user interaction.
            window.open("/clipboard", "clipboard");
        } else if (obj.type === "open_url") {
            window.open(obj.url);
        } else if (obj.type === "change_title") {
            document.title = obj.text;
        } else if (obj.type === "connect") {
            var sysinfo = obj;
            delete sysinfo["type"];
            console.log(sysinfo);
        } else {
            console.error("Unknown message type");
        }
    };

    var setupInput = function () {
        var keyHandler = function (event) {
            var object = { "type" : event.type,
                "char" : event.char,
                "key" : event.key,
                "which" : event.which,
                "location" : event.location,
                "repeat" : event.repeat,
                "locale" : event.locale,
                "ctrlKey" : event.ctrlKey, "shiftKey" : event.shiftKey, "altKey" : event.altKey,
                "metaKey" : event.metaKey,
                "string" : String.fromCharCode(event.wich ||
                                               event.keyCode),
                "keyCode" : event.keyCode, "charCode" : event.charCode,
                "time" : new Date().getTime(),
            };
            sendObject(object);
        }

        document.addEventListener('keypress', keyHandler, true);
        document.addEventListener('keydown', keyHandler, true);
        document.addEventListener('keyup', keyHandler, true);
    };
    setupInput();
};
