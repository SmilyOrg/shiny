var exec = require('sync-exec');
var shell_exec = require('shell_exec');
var fs = require('fs');
var isThere = require("is-there");
var chalk = require("chalk");
var log = jake.logger.log;

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
var protobufInclude = "include/google/protobuf/";
var protobufIncludeHeader = protobufInclude + "message.h";
var protobufProtocDir = "tools/protobuf/";
var protobufProtoc = protobufProtocDir + "protoc.exe";
var protobufOutput = protobufBuild + "Debug/";
var libDir = "lib/";

desc("Creates protobuf project");
file(protobufSolution, [], function() {
	log("Creating protobuf " + VisualStudioVersion +" project...");
	
	jake.mkdirP(protobufBuild);
	process.chdir(root + protobufBuild);
	logexec("cmake -G \"" + VisualStudioVersion + "\" ..");
	
	process.chdir(root);
})

desc("Builds protobuf solution");
task("protobuf-build", [protobufSolution], function() {
	var msbuild64 = process.env.ProgramFiles + "/MSBuild/12.0/bin/MSBuild.exe";
	var msbuild86 = process.env["ProgramFiles(x86)"] + "/MSBuild/12.0/bin/MSBuild.exe";
	var msbuild =
		isThere(msbuild64) ? msbuild64 :
		isThere(msbuild86) ? msbuild86 :
		"";
	
	if (msbuild == "") fail("msbuild not found");
	
	//log("Setting up VS environment")
	//log(exec("cmd /K \"C:/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/Tools/VsDevCmd.bat\""))
	
	// Build solution
	log("Building protobuf...");
	process.chdir(root + protobufBuild);
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
file(protobufInclude, [], function() {
	jake.Task["protobuf-build"].invoke();
	
	// Extract and copy
	log("Extracting includes...")
	process.chdir(root + protobufBuild)
	logexec("extract_includes.bat")
	
	jake.mkdirP(root + protobufInclude)
	jake.cpR(protobufInclude, root + protobufInclude)
	
	process.chdir(root)
})

desc("Ensures that protoc is available");
file(protobufProtoc, [], function () {
	jake.Task["protobuf-build"].invoke();
	
	jake.mkdirP(root + libDir)
	var files = [
		"libprotobuf.lib",
		"libprotobuf-lite.lib",
		"libprotoc.lib"
	].forEach(function(file) {
		jake.cpR(root + protobufOutput + file, root + libDir + file);
	})
	
	jake.mkdirP(root + protobufProtocDir)
	jake.cpR(root + protobufOutput + "protoc.exe", root + protobufProtoc);
});

task("protobuf-include", [protobufInclude]);
task("protoc", [protobufProtoc]);

desc("Compile protobuf messages to C++ source")
task("protobuf", ["protobuf-include", "protoc"], function() {
	// exec tools/protobuf/protoc.exe --cpp_out=src/ proto/renderRequest.proto
});