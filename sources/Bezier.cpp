#include "Bezier.h"
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <stdlib.h>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>


void Bezier::Load(float startu, float endu, int usteps, float startv, float endv, int vsteps, std::function<glm::vec3 (float, float)> func)
{
    glGenBuffers(1, &m_vertexBufferObject);
    glGenBuffers(1, &m_indexBufferObject);

    m_indexSize = (unsigned int) (6 * (usteps - 1) * (vsteps - 1));

    float ustepsize = (endu - startu) / (usteps - 1);
    float vstepsize = (endv - startv) / (vsteps - 1);

    Vertex vertices[usteps * vsteps];
    int indexes[m_indexSize];
    int index = 0;
    int i = 0;
    for(int x = 0; x < usteps; x++){
        for(int y = 0; y < vsteps; y++){
            index = (x * usteps) + y;
            float u = (x * ustepsize) + startu;
            float v = (y * vstepsize) + startv;
            vertices[index].pos = func(u, v);
            vertices[index].normal = glm::vec3(0.0, 0.0, 0.0);
            vertices[index].uv = glm::vec3(u, v, 1.0);
            if(y < vsteps - 1 && x < usteps - 1){
                indexes[6 * i] = index;
                indexes[6 * i + 1] = index + 1;
                indexes[6 * i + 2] = index + usteps + 1;
                indexes[6 * i + 3] = index;
                indexes[6 * i + 4] = index + usteps + 1;
                indexes[6 * i + 5] = index + usteps;
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

    for (int i = 0; i < (usteps * vsteps); ++i)
    {
        vertices[i].normal = glm::normalize(vertices[i].normal);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, usteps * vsteps * sizeof(Vertex), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexSize * sizeof(int), indexes, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // This code is just for generating the texture.
    // I realize it's a mess. I'm sorry. At least it makes pretty polka dots.

    srand(time(NULL));

    int rb = 255;
    int gb = 105;
    int bb = 180;

    for(int x = 0; x < TEXTURE_WIDTH / SQUARE_SIZE; x++){
        for(int y = 0; y < TEXTURE_HEIGHT / SQUARE_SIZE; y++){
            int r, g, b;
            if((x + y) % 2 == 0){
            //if(rand() % 5 == 0){
                r = rand() % 255;
                g = rand() % 255;
                b = rand() % 255;
            }else{
                r = rb;
                g = gb;
                b = bb;
            }
            for(int u = 0; u < SQUARE_SIZE; u++){
                for(int v = 0; v < SQUARE_SIZE; v++){
                    int i, j, radius;
                    radius = SQUARE_SIZE / 2;
                    i = v - radius;
                    j = u - radius;
                    //printf("(x, y) = (%d, %d), (u, v) = (%d, %d), (i, j) = (%d, %d)\n", x, y, u, v, i, j);
                    if(i * i + j * j < radius * radius) {
                        texture[x * SQUARE_SIZE + u][y * SQUARE_SIZE + v][0] = (GLubyte) r;
                        texture[x * SQUARE_SIZE + u][y * SQUARE_SIZE + v][1] = (GLubyte) g;
                        texture[x * SQUARE_SIZE + u][y * SQUARE_SIZE + v][2] = (GLubyte) b;
                    }else{
                        texture[x * SQUARE_SIZE + u][y * SQUARE_SIZE + v][0] = (GLubyte) rb;
                        texture[x * SQUARE_SIZE + u][y * SQUARE_SIZE + v][1] = (GLubyte) gb;
                        texture[x * SQUARE_SIZE + u][y * SQUARE_SIZE + v][2] = (GLubyte) bb;
                    }
                }
            }
        }
    }


    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

    glEnable(GL_TEXTURE_2D);
}

void Bezier::Draw() {
    //glActiveTexture(GL_TEXTURE0);
    Object::Draw();
}

void Bezier::Bind(){
    Object::Bind();
    glBindTexture(GL_TEXTURE_2D, textureHandle);
}

void Bezier::UnBind() {
    Object::UnBind();
    glBindTexture(GL_TEXTURE_2D, 0);
}