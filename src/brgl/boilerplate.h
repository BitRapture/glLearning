#pragma once
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <string>
#include <memory>

namespace br
{
    std::string open_text_file(const char* _filePath);
}

namespace br::gl
{
    enum class Status
    {
        ERROR = 0,
        ERROR_SHADER_COMPILATION,
        OK
    };

    class ShaderProgram
    {
    public:
        void add_shader(const GLuint& _shaderID);
        const GLuint compile_shader(const GLenum& _shaderType, const char* _source);
        const Status link();
        const GLuint get_id() { return shaderProgramID; }
        const bool is_linked() { return isLinked; }
        void bind() { glUseProgram(shaderProgramID); }
        void unbind() { glUseProgram(0); }

    public:
        const GLint operator[](const char* _uniformName);

    private:
        GLuint shaderProgramID;
        std::vector<GLuint> shaderObjects;
        bool isLinked;

    public:
        ShaderProgram();
        ~ShaderProgram();
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;
    };
    const Status compile_shader(GLuint& _shaderID, const GLenum& _shaderType, const char* _source);
    std::string get_shader_status(const GLuint& _shaderID);
    std::string get_program_status(const GLuint& _programID);

    template <typename T>
    class BufferObject
    {
    public:
        void set_data(const std::vector<T>& _bufferData) { bufferData = std::move(_bufferData); }
        void set_attributes(const GLuint& _layout, const GLenum& _normalizedData = GL_FALSE)
        {
            layout = _layout;
            normalizedData = _normalizedData;
        }
        const size_t get_size() { return bufferData.size(); }
        const GLuint get_id() { return bufferID; }
        void bind() { glBindBuffer(bufferType, bufferID); }
        void unbind() { glBindBuffer(bufferType, 0); }

    public:
        T& operator[](const size_t& _index) { return bufferData[_index]; }

    private:
        void release() { if (bufferID == 0) return; glDeleteBuffers(1, &bufferID); bufferID = 0; }

    private:
        friend class VertexArrayObject;
        friend class BufferObjectHandle;
        std::vector<T> bufferData;
        GLuint bufferID;
        GLenum bufferType;
        GLuint layout;
        GLenum dataType;
        GLenum normalizedData;

    public:
        BufferObject()
        {
            dataType = GL_FLOAT;
            bufferType = GL_ARRAY_BUFFER;
            bufferID = 0;
            set_attributes(0);
        }
        BufferObject(const std::vector<T>& _bufferData, const GLenum& _dataType, const GLuint& _layout = 0, const GLenum& _bufferType = GL_ARRAY_BUFFER) 
        { 
            dataType = _dataType;
            bufferType = _bufferType;
            set_data(_bufferData);
            set_attributes(_layout);
            glGenBuffers(1, &bufferID);
        }
        void operator=(BufferObject<T>& _other)
        {
            if (this != &_other)
            {
                release();
                std::swap(bufferData, _other.bufferData);
                std::swap(bufferID, _other.bufferID);
                bufferType = _other.bufferType;
                dataType = _other.dataType;
                set_attributes(_other.layout, _other.normalizedData);
            }
        }
        ~BufferObject() { release(); }
        BufferObject(const BufferObject&) = delete;
        BufferObject& operator=(const BufferObject&) = delete;
    };

    class BufferObjectHandle
    {
    public:
        const GLuint get_id() { return bufferID; }
        template<typename T>
        void operator=(BufferObject<T>& _bufferObject)
        {
            release();
            std::swap(bufferID, _bufferObject.bufferID);
        }

    private:
        void release() { if (bufferID == 0) return; glDeleteBuffers(1, &bufferID); bufferID = 0; }

    private:
        GLuint bufferID;

    public:
        BufferObjectHandle() : bufferID{ 0 } { }
        ~BufferObjectHandle() { release(); }
        BufferObjectHandle(const BufferObjectHandle&) = delete;
        BufferObjectHandle& operator=(const BufferObjectHandle&) = delete;
    };

    const GLuint create_vertex_array_object();

    class VertexArrayObject
    {
    public:
        template<typename T>
        void link_buffer_object(const BufferObject<T>& _bufferObject, const GLuint& _sizePerVertex, const GLenum& _drawType = GL_STATIC_DRAW);
        template<typename T, typename V>
        void link_buffer_object(const BufferObject<T>& _bufferObject, const BufferObject<V>& _elementObject, const GLuint& _sizePerVertex, const GLenum& _drawType = GL_STATIC_DRAW);
        const GLuint get_id() { return vaoID; }
        void bind() { glBindVertexArray(vaoID); }
        void unbind() { glBindVertexArray(0); }
    
    private:
        GLuint vaoID;

    public:
        VertexArrayObject();
        ~VertexArrayObject();
        VertexArrayObject(const VertexArrayObject&) = delete;
        VertexArrayObject& operator=(const VertexArrayObject&) = delete;
    };

    const GLuint create_texture_2d(unsigned char* _textureData, const GLint& _width, const GLint& _height, const GLenum& _format);
};

namespace br::gl
{
    template<typename T>
    void VertexArrayObject::link_buffer_object(const BufferObject<T>& _bufferObject, const GLuint& _sizePerVertex, const GLenum& _drawType)
    {
        glBindVertexArray(vaoID);
        glBindBuffer(_bufferObject.bufferType, _bufferObject.bufferID);
        glBufferData(_bufferObject.bufferType, _bufferObject.bufferData.size() * sizeof(T), &_bufferObject.bufferData.front(), _drawType);
        glVertexAttribPointer(_bufferObject.layout, _sizePerVertex, _bufferObject.dataType, _bufferObject.normalizedData, 0, NULL);
        glEnableVertexAttribArray(_bufferObject.layout);
        glBindBuffer(_bufferObject.bufferType, 0);
        glBindVertexArray(0);
    };

    template<typename T, typename V>
    void VertexArrayObject::link_buffer_object(const BufferObject<T>& _bufferObject, const BufferObject<V>& _elementObject, const GLuint& _sizePerVertex, const GLenum& _drawType)
    {
        glBindVertexArray(vaoID);
        glBindBuffer(_bufferObject.bufferType, _bufferObject.bufferID);
        glBufferData(_bufferObject.bufferType, _bufferObject.bufferData.size() * sizeof(T), &_bufferObject.bufferData.front(), _drawType);
        glBindBuffer(_elementObject.bufferType, _elementObject.bufferID);
        glBufferData(_elementObject.bufferType, _elementObject.bufferData.size() * sizeof(V), &_elementObject.bufferData.front(), _drawType);
        glVertexAttribPointer(_bufferObject.layout, _sizePerVertex, _bufferObject.dataType, _bufferObject.normalizedData, 0, NULL);
        glEnableVertexAttribArray(_bufferObject.layout);
        glBindBuffer(_bufferObject.bufferType, 0);
        glBindVertexArray(0);
    };
}