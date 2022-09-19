#include "Program.h"

#include <iostream>
#include <cassert>

#include "GLSL.h"

using namespace std;

Program::Program(string vert_shader, string frag_shader, const vector<string>& attributes, const vector<string>& uniforms, string compute_shader="") :
	vShaderName(""),
	cShaderName(""),
	fShaderName(""),
	pid(0),
	verbose(true)
{
	setShaderNames(vert_shader, frag_shader, compute_shader);
	setVerbose(true);
	init();
	addAttributes(attributes);
	addUniforms(uniforms);
	setVerbose(false);
}

Program::~Program()
{
	
}

void Program::setShaderNames(const string &v, const string &f, const string &c)
{
	vShaderName = v;
	fShaderName = f;
	cShaderName = c;
}

bool Program::init()
{
	GLint rc;
	
	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shader sources
	const char *vshader = GLSL::textFileRead(vShaderName.c_str());
	const char *fshader = GLSL::textFileRead(fShaderName.c_str());
	glShaderSource(VS, 1, &vshader, NULL);
	glShaderSource(FS, 1, &fshader, NULL);
	
	// Compile vertex shader
	glCompileShader(VS);
	glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
	if(!rc) {
		if(isVerbose()) {
			GLSL::printShaderInfoLog(VS);
			cout << "Error compiling vertex shader " << vShaderName << endl;
		}
		return false;
	}
	
	// Compile fragment shader
	glCompileShader(FS);
	glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
	if(!rc) {
		if(isVerbose()) {
			GLSL::printShaderInfoLog(FS);
			cout << "Error compiling fragment shader " << fShaderName << endl;
		}
		return false;
	}

	if (cShaderName != "") {
		GLuint CS = glCreateShader(GL_COMPUTE_SHADER);
		const char *cshader = GLSL::textFileRead(cShaderName.c_str());
		glShaderSource(CS, 1, &cshader, NULL);

		// Compile vertex shader
		glCompileShader(CS);
		glGetShaderiv(CS, GL_COMPILE_STATUS, &rc);
		if(!rc) {
			if(isVerbose()) {
				GLSL::printShaderInfoLog(CS);
				cout << "Error compiling compute shader " << cShaderName << endl;
			}
			return false;
		}
	}
	
	// Create the program and link
	pid = glCreateProgram();
	glAttachShader(pid, VS);
	glAttachShader(pid, FS);
	if (cShaderName != "") {
		glAttachShader(pid, CS);
	}
	glLinkProgram(pid);
	glGetProgramiv(pid, GL_LINK_STATUS, &rc);
	if(!rc) {
		if(isVerbose()) {
			GLSL::printProgramInfoLog(pid);
			cout << "Error linking shaders " << vShaderName << " and " << fShaderName << endl;
		}
		return false;
	}
	
	GLSL::checkError(GET_FILE_LINE);
	return true;
}

void Program::bind()
{
	glUseProgram(pid);
}

void Program::unbind()
{
	glUseProgram(0);
}

void Program::addAttribute(const string &name)
{
	attributes[name] = glGetAttribLocation(pid, name.c_str());
}

void Program::addAttributes(const vector<string>& names) {
	for (auto name : names) {
		addAttribute(name);
	}
}

void Program::addUniform(const string &name)
{
	uniforms[name] = glGetUniformLocation(pid, name.c_str());
}

void Program::addUniforms(const vector<string>& names) {
	for (auto name : names) {
		addUniform(name);
	}
}

GLint Program::getAttribute(const string &name) const
{
	map<string,GLint>::const_iterator attribute = attributes.find(name.c_str());
	if(attribute == attributes.end()) {
		if(isVerbose()) {
			cout << name << " is not an attribute variable" << endl;
		}
		return -1;
	}
	return attribute->second;
}

GLint Program::getUniform(const string &name) const
{
	map<string,GLint>::const_iterator uniform = uniforms.find(name.c_str());
	if(uniform == uniforms.end()) {
		if(isVerbose()) {
			cout << name << " is not a uniform variable" << endl;
		}
		return -1;
	}
	return uniform->second;
}
