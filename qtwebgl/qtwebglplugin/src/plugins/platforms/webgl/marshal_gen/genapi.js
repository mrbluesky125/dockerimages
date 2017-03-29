var gxp = require("./glxmlparser.js");
var getopt = require("node-getopt");
var fs = require("fs");

function runCode(code, glfunc) {
    return eval(code);
}

var substRe = new RegExp("%{[^%]+?}", "g");

function subst(s, f, basicSubsts, codeSnippets) {
    var joinParams = function (strGenFunc) {
        var r = "";
        for (var i = 0; i < f.params.length; ++i) {
            r += strGenFunc(f.params[i]);
            if (i < f.params.length - 1)
                r += ", ";
        }
        return r;
    };
    return s.replace(substRe, function (match) {
        var key = match.substring(2, match.length - 1);
        if (f) {
            if (key === "RET")
                return f.ret;
            if (key === "NAME")
                return f.name;
            if (key === "NAMEWOGL")
                return f.name.substring(2);
            if (key === "PARAMS")
                return joinParams(function (param) { return param.type + " " + param.name; });
            if (key === "PARAMTYPES")
                return joinParams(function (param) { return param.type; });
            if (key === "PARAMNAMES")
                return joinParams(function (param) { return param.name; });
            if (key === "FUNCMAJOR")
                return f.apiversion.split('.')[0];
            if (key === "FUNCMINOR")
                return f.apiversion.split('.')[1];
        }
        if (key.indexOf("RUN ") === 0) {
            var codeKey = key.substr(4);
            if (codeKey in codeSnippets)
                return runCode(codeSnippets[codeKey], f);
        }
        if (basicSubsts && key in basicSubsts)
            return basicSubsts[key];
        return match;
    });
}

function printSection(section, substFunc) {
    section.split('\n').forEach(function (line) {
        if (line.trimLeft().substr(0, 2) == "%#")
            return;
        if (substFunc)
            line = substFunc(line);
        process.stdout.write(line);
        process.stdout.write('\n');
    });
}

function main() {
    var opt = getopt.create([
        ["h", "help", "Display this help." ],
        ["v", "version", "Show version." ],
        ["f", "file=FILENAME", "Input XML: gl.xml (default) or egl.xml"],
        ["", "api=API", "API to use: gl, gles2 (default) or egl."],
        ["", "apiversion=VERSION", "API version, for example 1.4, 2.0 (default) or 4.3."],
        ["", "profile=PROFILE", "Profile: core (default) or compatibility. Ignored for APIs without profiles."],
        ["", "extensions=FILENAME", "Name of file with extensions to be included, one per line."]
    ]);
    opt.setHelp(
        "Usage: node genapi.js [OPTIONS] binding_description_file\n" +
        "OpenGL bindings generator.\n\n" +
        "[[OPTIONS]]\n"
    );
    var opts = opt.bindHelp().parseSystem();
    if (opts.options.version) {
        console.log("genapi.js v0.1");
        return;
    }
    if (!opts.argv.length) {
        console.error("No description file given.");
        process.exit(1);
    }

    var descfile = opts.argv[0];
    var xmlfile = opts.options.file || "gl.xml";
    var api = opts.options.api || "gles2";
    var apiversion = opts.options.apiversion || "2.0";
    var profile = opts.options.profile || "core";
    var extensions = [];
    if (opts.options.extensions) {
        var lines = [];
        try {
            lines = fs.readFileSync(opts.options.extensions, "utf8").split('\n');
        } catch (e) {
            console.error("Failed to read extensions file: " + e.toString());
            process.exit(1);
        }
        lines.forEach(function (ext) {
            var extName = ext.trim();
            if (extName.length)
                extensions.push(extName);
        });
    }
    var desc;
    try {
        desc = fs.readFileSync(descfile, "utf8");
    } catch (e) {
        console.error("Failed to read description file: " + e.toString());
        process.exit(1);
    }

    var sections = [], a = [], sectType;
    var addSect = function () {
        if (a.length) {
            sections.push({ "type": sectType, "content": a.join('\n') });
            a = [];
        }
    };
    desc.split('\n').forEach(function (line) {
        if (line.trimLeft().substr(0, 2) === "%#")
            return;
        if (line.substr(0, 2) == "%%") {
            addSect();
            sectType = line.substr(2).trim();
            return;
        }
        a.push(line.trimRight());
    });
    addSect();

    gxp.parse(xmlfile, api, apiversion, profile, extensions, function (glfunclist) {
        var codeSnippets = { };
        var basicSubsts = { "PROFILE": profile,
                            "APIMAJOR": apiversion.split('.')[0],
                            "APIMINOR": apiversion.split('.')[1] };
        sections.forEach(function (section) {
            if (!section.type)
                return;
            if (section.type === "copy") {
                printSection(section.content, function (s) {
                    return subst(s, null, basicSubsts, codeSnippets);
                });
            } else if (section.type === "for-each-function") {
                glfunclist.forEach(function (glfunc) {
                    printSection(section.content, function (s) {
                        return subst(s, glfunc, basicSubsts, codeSnippets);
                    });
                });
            } else if (section.type.indexOf("code ") === 0) {
                var key = section.type.substr(5);
                codeSnippets[key] = section.content;
            } else {
                console.error("\nUnrecognized section type: " + section.type);
                process.exit(1);
            }
        });
    });
}

main();
