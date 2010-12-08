#ifndef TERRAIN_H
#define TERRAIN_H

#include "common.h"
#include <string>
#include <QGLWidget>
#include <QGLShader>
#include <QFile>

//for terrain shader
struct TerrainRegion
{
    float min;
    float max;
    GLuint texture;
    std::string filename;
    TerrainRegion() {
        min = 0;
        max = 0;
        texture = 0;
        filename = "";
    }

    TerrainRegion(float mn, float mx, GLuint tex, std::string fn) {
        min = mn;
        max = mx;
        texture = tex;
        filename = fn;
    }
};

class Terrain
{
public:
    Terrain();
    ~Terrain();

    float3 * getTerrain();
    GLint getTerrainSize();
    float3 * getNormalMap();

    //for texturing
    GLuint loadTexture(const QFile &file);
    void setTextures(GLuint textures[4]);

    //for terrain and normals
    GLint coordinateToIndex(float2 c);
    void populateTerrain(float3 tl, float3 tr, float3 bl, float3 br);
    GLint getSurroundingVectors(int i , int j, float3 * surround);
    float3 findnormal(float3 vec1, float3 vec2);
    float3 averageNormal (float3* normals, int numNorm);
    void fillDiamond(float2 ptof, int dist,float2 xy, int depth);
    void fillAllDiamonds(float2 tl, float2 br, int depth);
    void fillSquare(float2 tlg, float2 brg, int depth);
    double getPerturb(int cur_depth);
    void populateNormals();
    void updateTerrainShaderParameters(QGLShaderProgram *shader);
    void render();

    float2 wrap(float2 val);
    bool isMultiple(int val);

    GLuint bumpmap;
private:
    //terrain stuff
    float3 * terrain;
    float3 * normalmap;
    GLint mDepth;
    GLfloat mDecay;
    GLint mSize;
    GLfloat mRoughness;
    GLfloat mScale;
    bool mIncreasing;

    static const int       TERRAIN_REGIONS_COUNT = 4;
    static const float     HEIGHTMAP_TILING_FACTOR = 4;

    //tiling factor must divide 2^depth
    TerrainRegion g_regions[TERRAIN_REGIONS_COUNT];

    float offsetX;
    float offsetY;
};

#endif // TERRAIN_H
