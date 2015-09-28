/*global jake*/
/*global task*/
/*global file*/
/*global fail*/
/*global desc*/
/*global rule*/
/*global directory*/
/*global complete*/

task("default", ["run"])

var exec = require('sync-exec');
var shell_exec = require('shell_exec');
var fs = require('fs');
var isThere = require("is-there");
var chalk = require("chalk");
var log = jake.logger.log;

//var VisualStudioVersionNumber = 14;
//var VisualStudioVersion = "Visual Studio 14 2015";

var VisualStudioVersionNumber = 12;
var VisualStudioVersion = "Visual Studio 12 2013";

function logexec(cmd) {
	var result = exec(cmd);
	if (result.status !== 0) {
		log(result.stdout);
		log(chalk.red(result.stderr));
		fail("Command failed: " + cmd);
	}
	return result;
}

var root = process.cwd() + "/";

var protobufSrc = "src/vendor/protobuf/";
var protobufCMake = protobufSrc + "cmake/";
var protobufBuild = protobufCMake + "build/";
var protobufSolution = protobufBuild + "protobuf.sln";
var protobufTest = protobufBuild + "Debug/lite-test.exe";
var protobufIncludeParent = "include/google/";
var protobufInclude = protobufIncludeParent + "protobuf/";
var protobufIncludeHeader = protobufInclude + "message.h";
var protobufProtocDir = "tools/protobuf/";
var protobufProtoc = protobufProtocDir + "protoc.exe";
var protobufOutput = protobufBuild + "Debug/";
var protobufLibs = [
	"libprotobuf.lib",
	"libprotobuf-lite.lib",
	"libprotoc.lib"
]

var protoSourceDir = "proto/"
var protoCompiledDir = "src/proto/";

var msbuild = "";

var buildDir = "build/";
var libDir = "lib/";

function ensureMSBuild() {
	if (msbuild != "") return;
	var suffix = "/MSBuild/" + VisualStudioVersionNumber + ".0/bin/MSBuild.exe";
	var msbuild64 = process.env.ProgramFiles + suffix;
	var msbuild86 = process.env["ProgramFiles(x86)"] + suffix;
	msbuild =
		isThere(msbuild64) ? msbuild64 :
		isThere(msbuild86) ? msbuild86 :
		"MSBUILD_NOT_FOUND"
	if (msbuild == "MSBUILD_NOT_FOUND") fail("msbuild not found at " + msbuild64 + " or " + msbuild86);
}

desc("Cleans the generated files")
task("clean", [], function() {
	jake.rmRf(root + protobufInclude);
	jake.rmRf(root + protobufBuild);
	jake.rmRf(root + protobufProtocDir);
	jake.rmRf(root + "src/proto/");
	jake.rmRf(root + "bin/");
	protobufLibs.forEach(function(file) {
		jake.rmRf(root + libDir + file);
	})
	jake.rmRf(root + buildDir);
})

desc("Creates protobuf project");
file(protobufSolution, [], function() {
	log("Creating protobuf " + VisualStudioVersion +" project...");
	
	jake.mkdirP(protobufBuild);
	process.chdir(root + protobufBuild);
	logexec("cmake -G \"" + VisualStudioVersion + "\" ..");
	
	process.chdir(root);
})

desc("Builds protobuf solution");
file(protobufTest, [protobufSolution], function() {
	ensureMSBuild();
	
	//log("Setting up VS environment")
	//log(exec("cmd /K \"C:/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/Tools/VsDevCmd.bat\""))
	
	// Build solution
	log("Building protobuf...");
	process.chdir(root + protobufBuild);
	exec("\"" + msbuild + "\" /m protobuf.sln")
	logexec("\"" + msbuild + "\" /m protobuf.sln")
	
	// Run tests
	log("Running tests...")
	process.chdir(root + protobufOutput)
	//logexec("tests.exe")
	var testResult = logexec("lite-test.exe");
	if (testResult.status != 0 || testResult.stdout != "PASS\r\n") fail("protobuf lite-test failed (" + testResult.status + "): " + testResult.stdout + ", " + testResult.stderr);
	
	process.chdir(root)
})

desc("Ensures that protobuf includes are available")
file(protobufInclude, [protobufTest], function() {
	//jake.Task["protobuf-build"].invoke();
	
	
	
	// Extract and copy
	log("Extracting includes...")
	process.chdir(root + protobufBuild)
	logexec("extract_includes.bat")
	
	jake.mkdirP(root + protobufIncludeParent)
	jake.cpR(protobufInclude, root + protobufIncludeParent)
	
	process.chdir(root)
})

desc("Ensures that protoc is available");
file(protobufProtoc, [protobufTest], function () {
	//jake.Task["protobuf-build"].invoke();
	
	log("Copying protobuf libs and tools..." + isThere(protobufProtoc))
	
	jake.mkdirP(root + libDir)
	protobufLibs.forEach(function(file) {
		jake.cpR(root + protobufOutput + file, root + libDir + file);
	})
	
	jake.mkdirP(root + protobufProtocDir)
	jake.cpR(root + protobufOutput + "protoc.exe", root + protobufProtoc);
});

task("protobuf-include", [protobufInclude]);
task("protoc", [protobufProtoc]);

directory(protoCompiledDir);

rule(".pb.h", function(name) {
	return name.replace(new RegExp("^" + protoCompiledDir), protoSourceDir).replace(/\.pb\.h$/, ".proto");
}, function () {
	log(this.source)
	logexec("\".\\" + protobufProtoc + "\"" +
		" --proto_path=" + protoSourceDir +
		" --cpp_out=" + protoCompiledDir +
		" " + this.source
	);
});

desc("Compile protobuf messages to C++ source")
task("protobuf", [
	protoCompiledDir, protobufProtoc,
	protoCompiledDir + "renderRequest.pb.h"
]);

desc("Make project with CMake")
task("cmake", [], function() {
	jake.mkdirP(root + buildDir)
	process.chdir(root + buildDir)
	logexec("cmake -G \"" + VisualStudioVersion + "\" ..")
	process.chdir(root)
})

desc("Build project")
task("build", ["protobuf", "cmake"], function() {
	ensureMSBuild();
	
	process.chdir(root + buildDir)
	logexec("\"" + msbuild + "\" shiny.sln /t:shiny")
	process.chdir(root)
})

desc("Run shiny")
task("run", ["build"], { async: true }, function() {
	var ex = jake.createExec(["build\\Debug\\shiny.exe"]);
	
	ex.addListener("cmdStart", function(cmd) {
		//log(cmd);
	});
	ex.addListener("cmdEnd", function(cmd) {
		complete();
	});
	
	ex.addListener("stdout", function(chunk) {
		log(chunk.toString());
	});
	ex.addListener("stderr", function(chunk) {
		log(chalk.red(chunk.toString()));
	});
	
	ex.addListener("error", function (msg, code) {
		if (code == 127) {
			log("Unable to find executable")
		} else {
			fail("Fatal error: " + msg, code);
		}
	});
	
	ex.run();
})
