var fs = require("fs");
var xml2js = require("xml2js");

(function () {
    var makeVersion = function (v) {
        var a = v.split('.');
        return {
            major: a[0],
            minor: a[1],
            sameOrNewerThan: function (other) {
                return this.major > other.major
                    || (this.major == other.major && this.minor >= other.minor);
            }
        };
    };

    exports.parse = function (filename, api, version, profile, extensions, callback) {
        var wantedFunctions = { };
        var functions = [];

        var forAllCommands = function (base, func) {
            if (!base)
                return;
            base.forEach(function (elem) {
                if (elem.$ && elem.$.profile && profile !== elem.$.profile)
                    return;
                if (elem.command)
                    elem.command.forEach(func);
            });
        };

        var parseFeatures = function (registry) {
            registry.feature.forEach(function (feature) {
                if (feature.$.api !== api || !version.sameOrNewerThan(makeVersion(feature.$.number)))
                    return;
                forAllCommands(feature.require, function (reqCommand) {
                    wantedFunctions[reqCommand.$.name] = feature.$.number;
                });
                forAllCommands(feature.remove, function (remCommand) {
                    delete wantedFunctions[remCommand.$.name];
                });
            });
        };

        var parseExtensions = function (registry) {
            registry.extensions.forEach(function (exts) {
                exts.extension.forEach(function (ext) {
                    if (/*ext.$.supported !== api ||*/ !(ext.$.name in extensions))
                        return;
                    if (ext.require)
                        forAllCommands(ext.require, function (extCommand) {
                            wantedFunctions[extCommand.$.name] = "ext";
                        });
                });
            });
        };

        var parseCommands = function (registry) {
            registry.commands.forEach(function (commands) {
                commands.command.forEach(function (command) {
                    // <param>const <ptype>GLchar</ptype> *const*<name>strings</name></param>
                    // parses to { _: "const *const*", ptype: "GLchar", ... } which is not
                    // quite good for our purposes but for now can be worked around by taking
                    // the first word (const), then the type and then the rest.
                    var typeToString = function (protoOrParam) {
                        var type = "";
                        var p = [];
                        if (protoOrParam._)
                            p = protoOrParam._.split(' ');
                        if (protoOrParam.ptype) {
                            if (p.length)
                                type = p[0] + ' ';
                            type += protoOrParam.ptype[0];
                            type += p.slice(1).join(' ');
                        } else {
                            type = p.join(' ');
                        }
                        return type.trim();
                    };
                    var proto = command.proto[0];
                    var funcName = proto.name[0];
                    if (!(funcName in wantedFunctions))
                        return;
                    var params = [];
                    if (command.param)
                        command.param.forEach(function (param) {
                            var len;
                            if (param.$ && param.$.len)
                                len = param.$.len.trim();
                            var paramInfo = {
                                "type": typeToString(param),
                                "name": param.name[0],
                            };
                            if (param.ptype)
                                paramInfo["ptype"] = param.ptype[0];
                            if (len) {
                                if (len === "COMPSIZE()")
                                    paramInfo["nullterm"] = true;
                                else if (len.substr(0, 9) == "COMPSIZE(") {
                                    paramInfo["depends"] = len.substring(9, len.length - 1).split(',');
                                } else {
                                    paramInfo["count"] = parseInt(len, 10);
                                };
                            }
                            params.push(paramInfo);
                        });
                    functions.push({
                        "ret": typeToString(proto),
                        "name": proto.name[0],
                        "params": params,
                        "apiversion": wantedFunctions[funcName]
                    });
                });
            });
            functions.sort(function (a, b) {
                var compare = function (x, y) { return x > y ? 1 : x < y ? -1 : 0; };
                return compare(a.apiversion, b.apiversion) || compare(a.name, b.name);
            });
        };

        version = makeVersion(version);
        profile = profile || "core";
        var extTab = { };
        if (extensions)
            for (var i in extensions)
                extTab[extensions[i]] = 1;
        extensions = extTab;
        var parser = new xml2js.Parser();
        fs.readFile(filename, function (err, data) {
            if (err)
                throw err;
            parser.parseString(data, function (err, result) {
                if (err)
                    throw err;

                parseFeatures(result.registry);
                parseExtensions(result.registry);
                parseCommands(result.registry);

                if (callback)
                    callback(functions);
            })
        });
    };

    exports.printFuncs = function (filename, api, version, profile, extensions) {
        var print = function (funcs) {
            funcs.forEach(function (f) {
                var decl = f.ret + " " + f.name + "(";
                for (var i = 0; i < f.params.length; ++i) {
                    decl += f.params[i].type + " " + f.params[i].name;
                    // if (f.params[i].nullterm)
                    //     decl += " NULLTERM";
                    // if (f.params[i].depends)
                    //     decl += " DEPENDS:" + f.params[i].depends.join(' ');
                    // if (f.params[i].count)
                    //     decl += " COUNT:" + f.params[i].count;
                    // if (f.params[i].ptype)
                    //     decl += " PTYPE: " + f.params[i].ptype;
                    if (i < f.params.length - 1)
                        decl += ", ";
                }
                decl += ")";
                console.log(decl);
            });
        };
        exports.parse(filename, api, version, profile, extensions, print);
    };
})();
