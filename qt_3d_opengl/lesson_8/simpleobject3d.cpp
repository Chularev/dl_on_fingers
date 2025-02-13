#include "simpleobject3d.h"
#include "material.h"

#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

SimpleObject3D::SimpleObject3D()
    : m_indexBuffer(QOpenGLBuffer::IndexBuffer),
      m_texture(0),
      m_scale(1.0f)
{

}

SimpleObject3D::SimpleObject3D(const QVector<VertexData> &vertData, const QVector<GLuint> &indexes, Material *material)
    : m_indexBuffer(QOpenGLBuffer::IndexBuffer),
      m_texture(0),
      m_scale(1.0f)
{
    init(vertData, indexes, material);
}

SimpleObject3D::~SimpleObject3D()
{
    if (m_vertexBuffer.isCreated())
        m_vertexBuffer.destroy();

    if (m_indexBuffer.isCreated())
        m_indexBuffer.destroy();

    if (m_texture != 0){
        if (m_texture->isCreated())
            m_texture->destroy();
    }
}

void SimpleObject3D::init(const QVector<VertexData> &vertData, const QVector<GLuint> &indexes, Material *material)
{
    if (m_vertexBuffer.isCreated())
        m_vertexBuffer.destroy();

    if (m_indexBuffer.isCreated())
        m_indexBuffer.destroy();

    if (m_texture != 0){
        if (m_texture->isCreated()) {
            delete m_texture;
            m_texture = 0;
        }
    }

    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(vertData.constData(), vertData.size() * sizeof(VertexData));
    m_vertexBuffer.release();

    m_indexBuffer.create();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(indexes.constData(), indexes.size() * sizeof (GLint));
    m_indexBuffer.release();

    m_material = material;
    if (material->getTextureExist())
    {
        m_texture = new QOpenGLTexture(material->diffuseMap().mirrored());

        m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
        m_texture->setMinificationFilter(QOpenGLTexture::Linear);

        m_texture->setWrapMode(QOpenGLTexture::Repeat);
    }

}

void SimpleObject3D::draw(QOpenGLShaderProgram *program, QOpenGLFunctions *functions)
{
    if (!m_vertexBuffer.isCreated() || ! m_indexBuffer.isCreated()) return;

    if (m_texture) {
        m_texture->bind(0);
        program->setUniformValue("u_texture",0);
    }
    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();
    modelMatrix.translate(m_translate);
    modelMatrix.rotate(m_rotate);
    modelMatrix.scale(m_scale);

    modelMatrix = m_globalTransform * modelMatrix;

    program->setUniformValue("u_modelMatrix", modelMatrix);

    m_vertexBuffer.bind();

    int offset = 0;
    int vertLoc = program->attributeLocation("a_position");
    program->enableAttributeArray(vertLoc);
    program->setAttributeBuffer(vertLoc, GL_FLOAT, offset, 3, sizeof (VertexData));

    offset += sizeof (QVector3D);
    int textLoc = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(textLoc);
    program->setAttributeBuffer(textLoc, GL_FLOAT, offset, 2, sizeof (VertexData));


    offset += sizeof (QVector2D);
    int normalLoc = program->attributeLocation("a_normal");
    program->enableAttributeArray(normalLoc);
    program->setAttributeBuffer(normalLoc, GL_FLOAT, offset, 3, sizeof (VertexData));

    m_indexBuffer.bind();
    functions->glDrawElements(GL_TRIANGLES, m_indexBuffer.size(), GL_UNSIGNED_INT,0);

    m_vertexBuffer.release();
    m_indexBuffer.release();

    if (m_texture)
        m_texture->release();

}

void SimpleObject3D::translate(const QVector3D &translateVector)
{
    m_translate += translateVector;
}

void SimpleObject3D::rotate(const QQuaternion &r)
{
    m_rotate = r * m_rotate;
}

void SimpleObject3D::scale(const float &s)
{
    m_scale *= s;
}

void SimpleObject3D::setGlobalTransform(QMatrix4x4 &g)
{
    m_globalTransform = g;
}
