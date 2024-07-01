#pragma once
#include <iostream>
#include <vector>


class ObjLoader
{
public:

    struct Vertex
    {
        float Position[3];
    };

    struct TextureCoord
    {
        float UV[2];
    };

    struct VertexNormal
    {
        float Normal[3];
    };



    struct Model
    {
        std::vector<Vertex> Vertices;
        std::vector<TextureCoord> UVs;
        std::vector<VertexNormal> VertexNormals;

        std::vector<uint16_t> VertexIndices;
        std::vector<uint16_t> UVIndices;
        std::vector<uint16_t> VNormalIndices;

    };

public:
    ObjLoader() = default;
    ~ObjLoader() = default;

    static Model LoadFromFile(const std::string& filepath);

};

