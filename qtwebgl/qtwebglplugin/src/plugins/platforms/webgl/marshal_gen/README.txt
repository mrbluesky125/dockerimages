genapi.js parses the official Khronos XML specifications (EGL up to
1.5, OpenGL up to 4.3, ES up to 3.1) and outputs content defined by a
binding description file for a selected API.

Requires the node modules xml2js and node-getopt.

Run with --help for options to select the API.

For example

node genapi.js --api=gles2 --apiversion=3.1 test.glbind

with test.glbind containing

%%copy
class Stuff {
%%for-each-function
    %{RET} %{NAME}(%{PARAMS});
%%copy
};

%%for-each-function
%{RET} Stuff::%{NAME}(%{PARAMS})
{
}

results in the output

class Stuff {
    void glActiveTexture(GLenum texture);
    ...
};

void Stuff::glActiveTexture(GLenum texture)
{
}

...

where ... stands for all functions available in OpenGL ES 3.1.

********************************************************************************

Section types:

%%copy                  This section is printed in the output once.

The following basic substitutions are performed:
%{MAJOR}                API major version
%{MINOR}                API minor version
%{PROFILE}              API profile in lowercase ("core" or "compatibility")
%{RUN <snippet_name>}   The result of evaluating the Javascript code snippet with the given name



%%for-each-function     Printed for each function in the API.

In addition to the basic substitutions the following ones are supported too:

%{RET}                  Return type of the function
%{NAME}                 Function name
%{NAMEWOGL}             Function name without gl prefix
%{PARAMS}               Full parameter list with comma separated type - name pairs
%{PARAMTYPES}           Comma separated list of the parameter types
%{PARAMNAMES}           Comma separate list of the parameter names
%{FUNCMAJOR}            Lowest API major version that supports the function
%{FUNCMINOR}            Lowest API minor version that supports the function



%%code <snippet_name>   Not included in the output. Contains Javascript code.

Such code is executed each time a %{RUN} substitution is encountered.

When invoked from a for-each-function section, the variable 'glfunc' contains the
description of the current function:

ret          Return type
name         Function name
apiversion   Lowest API version that supports the function
params       Array of parameters where each element is an object of the following:
      type      Parameter type
      name      Parameter name
      ptype     API-specific type or undefined for a standard C type.
                For example a GLsizei *length parameter results in
                type being "GLsizei *", name being "length" and ptype being "GLsizei",
                while a const void *data parameter results in
                type being "const void *", name being "data" and ptype being undefined.
      nullterm  When present, indicates that the parameter is a null-terminated string.
                (e.g. the 'name' parameter of glGetUniformLocation)
      count     When present, contains the number of elements in the data pointed by the parameter.
                (e.g. 8 in case of glUniformMatrix2x4fv)
      depends   When present, it is an array of names of other parameters on which the
                size of the actual data depends.
                (e.g. ["format", "type", "width", "height"] for glReadPixels)
