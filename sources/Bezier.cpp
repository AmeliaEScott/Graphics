#include "Bezier.h"
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Bezier::Load(int height, int width, std::function<glm::vec3 (int, int)> func)
{
    glGenBuffers(1, &m_vertexBufferObject);
    glGenBuffers(1, &m_indexBufferObject);

    m_indexSize = (unsigned int) (6 * (height - 1) * (width - 1));

    Vertex vertices[height * width];
    int indexes[m_indexSize];
    int index = 0;
    int i = 0;
    for(int x = 0; x < height; x++){
        for(int y = 0; y < width; y++){
            index = (x * width) + y;
            vertices[index].pos = func(x, y);
            vertices[index].normal = glm::vec3(0.0, 0.0, 0.0);
            vertices[index].uv = glm::vec3(x, y, 1.0);
            if(y < height - 1 && x < width - 1){
                indexes[6 * i] = index;
                indexes[6 * i + 1] = index + 1;
                indexes[6 * i + 2] = index + width + 1;
                indexes[6 * i + 3] = index;
                indexes[6 * i + 4] = index + width + 1;
                indexes[6 * i + 5] = index + width;
                i++;
            }
        }
    }

    for (int f = 0; f < m_indexSize / 3; ++f)
    {
        int i = indexes[3 * f + 0];
        int j = indexes[3 * f + 1];
        int k = indexes[3 * f + 2];
        glm::vec3 p1 = vertices[i].pos;
        glm::vec3 p2 = vertices[j].pos;
        glm::vec3 p3 = vertices[k].pos;
        glm::vec3 a = p1 - p3;
        glm::vec3 b = p2 - p3;
        glm::vec3 c = glm::cross(a, b);
        vertices[i].normal += c;
        vertices[j].normal += c;
        vertices[k].normal += c;
    }

    for (int i = 0; i < (height * width); ++i)
    {
        vertices[i].normal = glm::normalize(vertices[i].normal);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, height * width * sizeof(Vertex), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexSize * sizeof(int), indexes, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}