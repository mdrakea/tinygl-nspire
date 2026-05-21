#include <stddef.h>
#include <GLES/gl.h>

void tinygl_es11_compile_coverage(void)
{
    static const GLfixed identity[16] = {
        TGL_FIX_ONE, 0, 0, 0,
        0, TGL_FIX_ONE, 0, 0,
        0, 0, TGL_FIX_ONE, 0,
        0, 0, 0, TGL_FIX_ONE
    };
    static GLfixed vertices[9] = {
        0, 0, 0,
        TGL_FIX_ONE, 0, 0,
        0, TGL_FIX_ONE, 0
    };
    static GLfixed normals[9] = {
        0, 0, TGL_FIX_ONE,
        0, 0, TGL_FIX_ONE,
        0, 0, TGL_FIX_ONE
    };
    static GLubyte colors[12] = {
        255, 0, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255
    };
    static GLushort indices[3] = {0, 1, 2};
    GLfixed light_pos[4] = {0, 0, TGL_I(4), 0};
    GLfixed material[4] = {TGL_FIX_ONE, TGL_FIX_HALF, 0, TGL_FIX_ONE};
    GLuint buffers[2];
    GLint size = 0;
    GLenum err;

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixx(identity);
    glTranslatex(0, 0, -TGL_I(4));
    glRotatex(TGL_I(30), TGL_FIX_ONE, 0, 0);
    glScalex(TGL_FIX_ONE, TGL_FIX_ONE, TGL_FIX_ONE);
    glFrustumx(-TGL_FIX_ONE, TGL_FIX_ONE, -TGL_FIX_ONE, TGL_FIX_ONE,
               TGL_FIX_ONE, TGL_I(10));
    glOrthox(-TGL_FIX_ONE, TGL_FIX_ONE, -TGL_FIX_ONE, TGL_FIX_ONE,
             TGL_FIX_ONE, TGL_I(10));

    glLightxv(GL_LIGHT0, GL_POSITION, light_pos);
    glMaterialxv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FIXED, 0, vertices);
    glNormalPointer(GL_FIXED, 0, normals);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, indices);

    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexPointer(3, GL_FIXED, 0, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, NULL);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDeleteBuffers(2, buffers);

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    err = glGetError();
    (void)err;
    (void)size;
}
