#ifndef MODEL_OBJ_H
#define MODEL_OBJ_H

#include "glad/glad.h"
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class ModelOBJ {
private:
    struct VertexIndex {
        int v = 0;
        int vt = 0;
        int vn = 0;
    };

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    GLsizei vertexCount = 0;
    std::vector<float> vertexData;

    static int resolveIndex(int idx, size_t size) {
        if (idx > 0) return idx - 1;
        if (idx < 0) return static_cast<int>(size) + idx;
        return -1;
    }

    static VertexIndex parseVertexIndex(const std::string& token) {
        VertexIndex idx;
        size_t firstSlash = token.find('/');
        if (firstSlash == std::string::npos) {
            if (!token.empty()) {
                idx.v = std::stoi(token);
            }
        } else {
            std::string vStr = token.substr(0, firstSlash);
            if (!vStr.empty()) {
                idx.v = std::stoi(vStr);
            }
            size_t secondSlash = token.find('/', firstSlash + 1);
            if (secondSlash == std::string::npos) {
                std::string vtStr = token.substr(firstSlash + 1);
                if (!vtStr.empty()) {
                    idx.vt = std::stoi(vtStr);
                }
            } else {
                if (secondSlash > firstSlash + 1) {
                    std::string vtStr = token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
                    if (!vtStr.empty()) {
                        idx.vt = std::stoi(vtStr);
                    }
                }
                if (secondSlash + 1 < token.size()) {
                    std::string vnStr = token.substr(secondSlash + 1);
                    if (!vnStr.empty()) {
                        idx.vn = std::stoi(vnStr);
                    }
                }
            }
        }
        return idx;
    }

public:
    ModelOBJ() : VAO(0), VBO(0), vertexCount(0) {}

    ModelOBJ(const std::string& path) : VAO(0), VBO(0), vertexCount(0) {
        load(path);
    }

    ~ModelOBJ() {
        cleanup();
    }

    // Move constructor
    ModelOBJ(ModelOBJ&& other) noexcept
        : VAO(other.VAO), VBO(other.VBO), vertexCount(other.vertexCount), vertexData(std::move(other.vertexData)) {
        other.VAO = 0;
        other.VBO = 0;
        other.vertexCount = 0;
    }

    // Move assignment
    ModelOBJ& operator=(ModelOBJ&& other) noexcept {
        if (this != &other) {
            cleanup();
            VAO = other.VAO;
            VBO = other.VBO;
            vertexCount = other.vertexCount;
            vertexData = std::move(other.vertexData);
            other.VAO = 0;
            other.VBO = 0;
            other.vertexCount = 0;
        }
        return *this;
    }

    // Deshabilitar copia para prevenir liberación prematura de VAO/VBO
    ModelOBJ(const ModelOBJ&) = delete;
    ModelOBJ& operator=(const ModelOBJ&) = delete;

    void cleanup() {
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
            VBO = 0;
        }
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
        vertexCount = 0;
        vertexData.clear();
    }

    bool load(const std::string& path) {
        cleanup();

        std::ifstream file(path);
        std::string actualPath = path;

        // Fallbacks inteligentes de rutas (para variaciones de mayúsculas o nombres de archivo)
        if (!file.is_open()) {
            std::vector<std::string> fallbacks;
            if (path.rfind("Modelos/", 0) == 0) {
                fallbacks.push_back("modelos/" + path.substr(8));
            } else if (path.rfind("modelos/", 0) == 0) {
                fallbacks.push_back("Modelos/" + path.substr(8));
            }
            if (path.find("chasis.obj") != std::string::npos || path.find("Base_simon.obj") != std::string::npos || path.find("base.obj") != std::string::npos) {
                fallbacks.push_back("Modelos/base.obj");
                fallbacks.push_back("modelos/base.obj");
                fallbacks.push_back("Modelos/Base_simon.obj");
                fallbacks.push_back("modelos/Base_simon.obj");
                fallbacks.push_back("Modelos/chasis.obj");
                fallbacks.push_back("modelos/chasis.obj");
            }
            for (const auto& fb : fallbacks) {
                file.open(fb);
                if (file.is_open()) {
                    actualPath = fb;
                    break;
                }
            }
        }

        if (!file.is_open()) {
            std::cerr << "[ModelOBJ] Error: No se pudo abrir el archivo OBJ: " << path << std::endl;
            return false;
        }

        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec2> temp_texCoords;
        std::vector<glm::vec3> temp_normals;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            if (line[0] == 'v') {
                if (line.size() > 1 && line[1] == ' ') {
                    // Posición: v x y z
                    std::stringstream ss(line.substr(2));
                    glm::vec3 pos;
                    ss >> pos.x >> pos.y >> pos.z;
                    temp_positions.push_back(pos);
                } else if (line.size() > 2 && line[1] == 't' && line[2] == ' ') {
                    // Coordenadas UV: vt u v (directas, para que el mapeo coincida exactamente con la imagen cargada por Texture.h)
                    std::stringstream ss(line.substr(3));
                    glm::vec2 uv;
                    ss >> uv.x >> uv.y;
                    temp_texCoords.push_back(uv);
                } else if (line.size() > 2 && line[1] == 'n' && line[2] == ' ') {
                    // Normales: vn nx ny nz
                    std::stringstream ss(line.substr(3));
                    glm::vec3 norm;
                    ss >> norm.x >> norm.y >> norm.z;
                    temp_normals.push_back(norm);
                }
            } else if (line[0] == 'f' && line.size() > 1 && line[1] == ' ') {
                // Caras: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 [v4/vt4/vn4 ...]
                std::stringstream ss(line.substr(2));
                std::string token;
                std::vector<VertexIndex> faceVertices;
                while (ss >> token) {
                    faceVertices.push_back(parseVertexIndex(token));
                }

                if (faceVertices.size() < 3) continue;

                // Descomposición en abanico (fan triangulation): (0, i, i+1)
                // Soportando triángulos y quads convertidos a triángulos
                for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
                    VertexIndex triVerts[3] = { faceVertices[0], faceVertices[i], faceVertices[i + 1] };
                    for (int k = 0; k < 3; ++k) {
                        const VertexIndex& vi = triVerts[k];

                        int pIdx = resolveIndex(vi.v, temp_positions.size());
                        glm::vec3 pos = (pIdx >= 0 && pIdx < static_cast<int>(temp_positions.size()))
                                            ? temp_positions[pIdx]
                                            : glm::vec3(0.0f);

                        int nIdx = resolveIndex(vi.vn, temp_normals.size());
                        glm::vec3 norm = (nIdx >= 0 && nIdx < static_cast<int>(temp_normals.size()))
                                             ? temp_normals[nIdx]
                                             : glm::vec3(0.0f, 1.0f, 0.0f);

                        int tIdx = resolveIndex(vi.vt, temp_texCoords.size());
                        glm::vec2 uv = (tIdx >= 0 && tIdx < static_cast<int>(temp_texCoords.size()))
                                           ? temp_texCoords[tIdx]
                                           : glm::vec2(0.0f);

                        // Layout intercalado (stride = 8 floats): Pos (3) + Normal (3) + UV (2)
                        vertexData.push_back(pos.x);
                        vertexData.push_back(pos.y);
                        vertexData.push_back(pos.z);

                        vertexData.push_back(norm.x);
                        vertexData.push_back(norm.y);
                        vertexData.push_back(norm.z);

                        vertexData.push_back(uv.x);
                        vertexData.push_back(uv.y);
                    }
                }
            }
        }
        file.close();

        vertexCount = static_cast<GLsizei>(vertexData.size() / 8);
        std::cout << "[ModelOBJ] Modelo cargado: " << actualPath 
                  << " (" << vertexCount << " vertices, " 
                  << (vertexCount / 3) << " triangulos)" << std::endl;

        // Configurar VAO y VBO si GLAD ya inicializó las funciones OpenGL
        if (glGenVertexArrays != nullptr) {
            setupBuffers();
        }

        return true;
    }

    void setupBuffers() {
        if (vertexData.empty()) return;

        if (VAO == 0) glGenVertexArrays(1, &VAO);
        if (VBO == 0) glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

        GLsizei stride = 8 * sizeof(float);

        // location = 0: aPos (3 floats)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        // location = 1: aNormal (3 floats)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // location = 2: aTexCoords (2 floats)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void draw() const {
        if (VAO == 0 && !vertexData.empty() && glGenVertexArrays != nullptr) {
            const_cast<ModelOBJ*>(this)->setupBuffers();
        }
        if (VAO != 0 && vertexCount > 0) {
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
            glBindVertexArray(0);
        }
    }

    unsigned int getVAO() const { return VAO; }
    unsigned int getVBO() const { return VBO; }
    GLsizei getVertexCount() const { return vertexCount; }
};

#endif // MODEL_OBJ_H

