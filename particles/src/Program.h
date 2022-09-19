#pragma  once
#ifndef PROGRAM_H
#define PROGRAM_H

#include <map>
#include <string>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

/**
 * An OpenGL Program (vertex and fragment shaders)
 */
class Program
{
public:
	Program(std::string vert_shader, std::string frag_shader, const std::vector<std::string>& attributes, const std::vector<std::string>& uniforms, std::string compute_shader);
	virtual ~Program();
	
	void setVerbose(bool v) { verbose = v; }
	bool isVerbose() const { return verbose; }
	
	void setShaderNames(const std::string &v, const std::string &f, const std::string &c);
	virtual bool init();
	virtual void bind();
	virtual void unbind();

	void addAttribute(const std::string &name);
	void addAttributes(const std::vector<std::string> &names);
	void addUniform(const std::string &name);
	void addUniforms(const std::vector<std::string> &names);
	GLint getAttribute(const std::string &name) const;
	GLint getUniform(const std::string &name) const;
	
protected:
	std::string vShaderName;
	std::string fShaderName;
	std::string cShaderName
	
private:
	GLuint pid;
	std::map<std::string,GLint> attributes;
	std::map<std::string,GLint> uniforms;
	bool verbose;
};

#endif
