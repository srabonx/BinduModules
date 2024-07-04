#include "../Include/ObjLoader.h"
#include <cstring>
#include <fstream>
#include <iostream>

ObjLoader::Model ObjLoader::LoadFromFile(const std::string& filepath)
{

    Model model;

    // Open the file
    FILE* file;

    fopen_s(&file, filepath.c_str(), "r");

    if (file == nullptr)
    {
        std::cerr << "File cannot be opened!";
        return model;
    }

    // Read the file until the end
    while (true)
    {
        char lineHeader[128];

        int res = fscanf_s(file, "%s", lineHeader, static_cast<unsigned int>(_countof(lineHeader)));

        if (res == EOF)
            break;

        if (strcmp(lineHeader, "v") == 0)
        {
            Vertex vertex;
            fscanf_s(file, "%f %f %f\n", &vertex.Position[0], &vertex.Position[1], &vertex.Position[2]);
            model.Vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            TextureCoord texCoord;
            fscanf_s(file, "%f %f\n", &texCoord.UV[0], &texCoord.UV[1]);
            model.UVs.push_back(texCoord);
        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            VertexNormal vNormal;
            fscanf_s(file, "%f %f %f\n", &vNormal.Normal[0], &vNormal.Normal[1], &vNormal.Normal[2]);
            model.VertexNormals.push_back(vNormal);
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            std::uint16_t vertexIndices[4], uvIndices[4], vertexNormal[4];


            int matches = fscanf_s(file, "%hu/%hu/%hu %hu/%hu/%hu %hu/%hu/%hu %hu/%hu/%hu\n",
                &vertexIndices[0], &uvIndices[0], &vertexNormal[0],
                &vertexIndices[1], &uvIndices[1], &vertexNormal[1],
                &vertexIndices[2], &uvIndices[2], &vertexNormal[2],
                &vertexIndices[3], &uvIndices[3], &vertexNormal[3]);


            if (matches != 9 && matches != 12)
            {
                std::cerr << "Faces cannot be read by this parser!" << std::endl;
                return model;
            }

            // - 1 because unlike c++ obj file starts indexing from 1
            model.VertexIndices.push_back(vertexIndices[0] - 1);
            model.VertexIndices.push_back(vertexIndices[1] - 1);
            model.VertexIndices.push_back(vertexIndices[2] - 1);

            model.UVIndices.push_back(uvIndices[0] - 1);
            model.UVIndices.push_back(uvIndices[1] - 1);
            model.UVIndices.push_back(uvIndices[2] - 1);

            model.VNormalIndices.push_back(vertexNormal[0] - 1);
            model.VNormalIndices.push_back(vertexNormal[1] - 1);
            model.VNormalIndices.push_back(vertexNormal[2] - 1);

            if (matches == 12)
            {
                model.VertexIndices.push_back(vertexIndices[0] - 1);
                model.VertexIndices.push_back(vertexIndices[2] - 1);
                model.VertexIndices.push_back(vertexIndices[3] - 1);

                model.UVIndices.push_back(uvIndices[0] - 1);
                model.UVIndices.push_back(uvIndices[2] - 1);
                model.UVIndices.push_back(uvIndices[3] - 1);

                model.VNormalIndices.push_back(vertexNormal[0] - 1);
                model.VNormalIndices.push_back(vertexNormal[2] - 1);
                model.VNormalIndices.push_back(vertexNormal[3] - 1);

            }

        }
    }

    return model;
}
