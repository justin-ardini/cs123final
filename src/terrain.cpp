#include "terrain.h"
using std::string;
using std::cout;
using std::endl;

// Change this to change the level at which terrain changes from grass to rock, rock to ice
#define TERRAIN_HEIGHT 1.8f

Terrain::Terrain() {
    mRoughness = 5;
    mDecay = 3;
    mScale = 0;
    mDepth = 8;
    mIncreasing = true;
    mSize = pow(2, mDepth) + 1;
    int terrain_size = mSize * mSize;
    terrain = new float3[terrain_size];
    normalmap = new float3[terrain_size];
}


Terrain::~Terrain() {
    delete[] terrain;
    delete[] normalmap;
}


/**
  adds uniform variables for terrain shader stuff, you need to fill this in for the terrain shader
  **/
void Terrain::updateTerrainShaderParameters(QGLShaderProgram *shader) {
    shader->setUniformValue("region1ColorMap", g_regions[0].texture);
    shader->setUniformValue("region2ColorMap", g_regions[1].texture);
    shader->setUniformValue("region3ColorMap", g_regions[2].texture);
    shader->setUniformValue("region4ColorMap", g_regions[3].texture);
    shader->setUniformValue("region1Min", g_regions[0].min);
    shader->setUniformValue("region2Min", g_regions[1].min);
    shader->setUniformValue("region3Min", g_regions[2].min);
    shader->setUniformValue("region4Min", g_regions[3].min);
    shader->setUniformValue("region1Max", g_regions[0].max);
    shader->setUniformValue("region2Max", g_regions[1].max);
    shader->setUniformValue("region3Max", g_regions[2].max);
    shader->setUniformValue("region4Max", g_regions[3].max);
    shader->setUniformValue("cubeMap", 0);
}


float3 * Terrain::getTerrain() {
    return terrain;
}

GLint Terrain::getTerrainSize() {
    return mSize * mSize;
}

float3 * Terrain::getNormalMap() {
    return normalmap;
}

void Terrain::setTextures(GLuint textures[4]) {
    g_regions[0].texture = textures[0];
    g_regions[1].texture = textures[1];
    g_regions[2].texture = textures[2];
    g_regions[3].texture = textures[3];
}


//stuff for tiling of textures, ignore it
bool Terrain::isMultiple(int index) {
    float val = (float)index/((mSize-1)/HEIGHTMAP_TILING_FACTOR);
    if (val - (int)val == 0){
        return true;
    }
    return false;
}

float2 Terrain::wrap(float2 val){
    return float2(val.row - (int)val.row, val.col - (int)val.col);
}


/**
    The main drawing method which will be called 30 frames per second. All of the tesselation is done for you.
    You need to worry about texturing the terrain here.
**/
void Terrain::render() {
    //float time = mIncrement++ / (float)mFPS;

    // Clear the color and depth buffers to the current glClearColor
    glMatrixMode(GL_MODELVIEW);

    // Push a new matrix onto the stack for modelling transformations
    glPushMatrix();

    /*
    glColor3f(0.5f, 0.5f, 0.6f);
    GLfloat ambientLight[] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0, 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat position[] = { 10, 0, 10, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    */

    float unitIncrement = HEIGHTMAP_TILING_FACTOR/(mSize-1);
    glBegin(GL_QUADS);
    for (int row = 0; row < mSize-1;row++){
        for (int column= 0; column < mSize-1;column++){
            float3 tlv = terrain[row*mSize + column];
            float3 tln = normalmap[row*mSize + column];
            float3 blv = terrain[(row+1)*mSize + column];
            float3 bln = normalmap[(row+1)*mSize + column];
            float3 brv = terrain[(row+1)*mSize + column+1];
            float3 brn = normalmap[(row+1)*mSize + column+1];
            float3 trv = terrain[row*mSize + column+1];
            float3 trn = normalmap[row*mSize + column+1];

            float2 tlc = wrap(float2(row*unitIncrement,column*unitIncrement));
            float2 blc = wrap(float2((row+1)*unitIncrement,column*unitIncrement));
            float2 brc = wrap(float2((row+1)*unitIncrement,(column+1)*unitIncrement));
            float2 trc = wrap(float2(row*unitIncrement,(column+1)*unitIncrement));

            if (isMultiple(column+1)){
                trc = float2(trc.row,1.0);
                brc = float2(brc.row,1.0);
            }
            if (isMultiple(row+1)){
                blc = float2(1.0,blc.col);
                brc = float2(1.0,brc.col);
            }

            glTexCoord2d(tlc.col, tlc.row);
            glNormal3f(tln.x, tln.y, tln.z);
            glVertex3f(tlv.x, tlv.y, tlv.z);

            glTexCoord2d(blc.col, blc.row);
            glNormal3f(bln.x, bln.y, bln.z);
            glVertex3f(blv.x, blv.y, blv.z);

            glTexCoord2d(brc.col, brc.row);
            glNormal3f(brn.x, brn.y, brn.z);
            glVertex3f(brv.x, brv.y, brv.z);


            glTexCoord2d(trc.col, trc.row);
            glNormal3f(trn.x, trn.y, trn.z);
            glVertex3f(trv.x, trv.y, trv.z);
        }

    }
    glEnd();
    // Discard the modelling transformations (leaving only camera settings)
    glPopMatrix();
}


/**
  takes in a coordinate, (row, col),
  outputs -1 if the coordinate is not valid,
  the corresponding index in an array otherwise, give the width of
  each row, mSize
  **/
GLint Terrain::coordinateToIndex(float2 c) {
    if (c.row < 0 || c.row >= mSize || c.col < 0 || c.col >= mSize){
        return -1;
    }
    return c.row*mSize+c.col;
}
/**
  initializes the height map's corner values, fills the map
  **/
void Terrain::populateTerrain(float3 tl, float3 tr, float3 bl, float3 br) {
    terrain[0] = tl;
    terrain[mSize-1] = tr;
    terrain[(mSize-1)*mSize] = bl;
    terrain[(mSize*mSize-1)] = br;
    for (int i = 0; i < mDepth; i++){
        int numincrements = pow(2,i);
        int gsize = mSize/numincrements;
        if (gsize == mSize){
            gsize --;
        }
        for (int row = 0; row <numincrements;row++){
            for (int col = 0; col < numincrements;col++){
                float2 curtl(gsize*row,gsize*col);
                float2 curbr(curtl.row+gsize,curtl.col+gsize);
                fillSquare(curtl, curbr, i);
            }
        }
        for (int row = 0; row <numincrements;row++){
            for (int col = 0; col < numincrements;col++){
                float2 curtl(gsize*row,gsize*col);
                float2 curbr(curtl.row+gsize,curtl.col+gsize);
                fillAllDiamonds(curtl, curbr, i);
            }
        }
    }
    float maxHeight = 0;
    float minHeight = 0;
    for (int row = 0; row < mSize;row++){
        for (int col = 0; col < mSize; col++){
            float rowDiff = row - 0.5 * mSize;
            float colDiff = col - 0.5 * mSize;
            terrain[coordinateToIndex(float2(row,col))].z += 0.00022 * ((rowDiff * rowDiff) + (colDiff * colDiff));
            float curHeight = terrain[coordinateToIndex(float2(row,col))].z;
            if (curHeight < minHeight){
                minHeight = curHeight;
            }
            else if (curHeight > maxHeight){
                maxHeight = curHeight;
            }
        }
    }
    float rangeIncrement = (maxHeight - minHeight) / 4.0;

    g_regions[0] = TerrainRegion(minHeight - TERRAIN_HEIGHT,
                                 minHeight - TERRAIN_HEIGHT + rangeIncrement, 0, "/course/cs123/pub/lab07/textures/dirt.JPG");
    g_regions[1] = TerrainRegion(minHeight - TERRAIN_HEIGHT + rangeIncrement,
                                 minHeight - TERRAIN_HEIGHT + rangeIncrement * 2, 0, "/course/cs123/pub/lab07/textures/grass.JPG");
    g_regions[2] = TerrainRegion(minHeight - TERRAIN_HEIGHT + rangeIncrement * 2,
                                 minHeight - TERRAIN_HEIGHT + rangeIncrement * 3, 0, "/course/cs123/pub/lab07/textures/rock.JPG");
    g_regions[3] = TerrainRegion(minHeight - TERRAIN_HEIGHT + rangeIncrement * 3,
                                 minHeight - TERRAIN_HEIGHT + rangeIncrement * 4, 0, "/course/cs123/pub/lab07/textures/snow.JPG");

    /* for(int location = 0; location < (mSize * mSize); location++){
        if(terrain[location].z <= SEA_LEVEL){
            terrain[location].z = SEA_LEVEL;
        }
    } */
}

/**
  populates all the normals...
  **/
void Terrain::populateNormals() {
    for (int row = 0; row < mSize; row++){
        for (int column = 0; column < mSize ; column++){
            float3 surround[8];
            GLint numVecs = getSurroundingVectors(row, column, surround);
            float3 normals[8];
            for (int i = 0; i < 8; i++){
                normals[i] = findnormal(surround[i], surround[(i+1)%8]);
            }
            normalmap[row*mSize+column] = averageNormal(normals, numVecs).getNormalized();
        }
    }

    /* for(int location = 0; location < (mSize * mSize); location++){
        if(terrain[location].z <= SEA_LEVEL){
            normalmap[location] = float3(0.0f, 0.0f, 1.0f);
        }
    } */
}

/**
    Finds all vectors from vertex at grid coordinate (i,j) to surrounding vectors.
    These vectors are stored in vecs.
    Returns a the number of resulting vectors that aren't(0,0,0)(edge cases)
    **/
GLint Terrain::getSurroundingVectors(int row , int column, float3* vecs) {
    //ordering: left, top, right, bottom
    float2 curCoord(row,column);
    float3 curVert = terrain[coordinateToIndex(curCoord)];
    GLint numVecs = 0;
    // the left vector
    float2 coords[8];
    GLint indices[8];
    coords[0] = float2(row, column-1);
    coords[1] = float2(row+1, column-1);
    coords[2] = float2(row+1, column);
    coords[3] = float2(row+1, column+1);
    coords[4] = float2(row,column+1);
    coords[5] = float2(row-1, column+1);
    coords[6] = float2(row-1, column);
    coords[7] = float2(row-1, column-1);
    for (int i = 0; i < 8; i++){
        indices[i] = coordinateToIndex(coords[i]);
    }
    for (int i = 0; i < 8; i++){
        if (indices[i] == -1){
            vecs[i] = float3(0,0,0);
        }
        else{
            float3 otherVert = terrain[indices[i]];
            vecs[i] = otherVert - curVert;
            numVecs++;
        }
    }
    return numVecs;
}

/**
  findnormal: Given two vectors, finds the vector perpendicular to both
      **/
float3 Terrain::findnormal(float3 vec1, float3 vec2) {
    float3 toreturn = (vec1.cross(vec2));
    if (toreturn.x != 0 || toreturn.y != 0 || toreturn.z != 0){
        return toreturn.getNormalized();
    }
    return toreturn;
}
/**
  averageNormal: give some number of normals, numNorm,
      finds the average normal. If a normal is unspecified it will be
      0,0,0
      **/
float3 Terrain::averageNormal (float3 * n, int numNorm){
    float3 toreturn = n[0] + n[1] + n[2] + n[3] + n[4] + n[5] + n[6] + n[7];
    if (numNorm == 8){
        toreturn/=numNorm;
    }
    else{
        toreturn /= (numNorm-1);
    }
    toreturn = toreturn.getNormalized();
    return toreturn;
}

/**
  returns a random value to perturb a vertex by based on an inputed level of depth
  Feel free to modify this
  **/
double Terrain::getPerturb(int cur_depth) {
    double toreturn = mRoughness*pow(((double)cur_depth/mDepth), mDecay)*((rand()%200-100)/100.0);
    return toreturn;
}

/**
  Does the diamond step, fills the center of the square with a value
  **/
void Terrain::fillSquare(float2 tlg, float2 brg, int depth) {
    float3 tl = terrain[coordinateToIndex(tlg)];
    float3 br = terrain[coordinateToIndex(brg)];
    float2 trg(tlg.row,brg.col);
    float2 blg(brg.row,tlg.col);
    float3 tr = terrain[coordinateToIndex(trg)];
    float3 bl = terrain[coordinateToIndex(blg)];
    float2 midg((tlg.row+brg.row)/2, (tlg.col+brg.col)/2);
    float3 mid((br.x+bl.x+tr.x+tl.x)/4,(bl.y+tr.y+tl.y+br.y)/4,
               ((tl.z+tr.z+bl.z+br.z)/4)+getPerturb(mDepth-depth)) ;
    terrain[coordinateToIndex(midg)] = mid;
}

/**
  Does the square step, fills the sides centers of all diamonds surrounding a point with a value
  Creates squares
  **/
void Terrain::fillAllDiamonds(float2 tl, float2 br, int depth){
    float2 left((tl.row+br.row)/2,tl.col);
    float2 top(tl.row,(tl.col+br.col)/2 );
    float2 right((tl.row+br.row)/2,br.col);
    float2 bot( br.row,(tl.col+br.col)/2);
    float3 actualtl(terrain[coordinateToIndex(tl)]);
    float3 actualbr(terrain[coordinateToIndex(br)]);
    float2 leftxy(actualtl.x, (actualtl.y+actualbr.y)/2);
    float2 topxy((actualtl.x+actualbr.x)/2,actualtl.y);
    float2 rightxy(actualbr.x, (actualtl.y+actualbr.y)/2);
    float2 botxy((actualtl.x+actualbr.x)/2, actualbr.y);
    int dist = (br.row-tl.row)/2;
    fillDiamond(left, dist, leftxy, depth);
    fillDiamond(top, dist, topxy, depth);
    fillDiamond(right, dist, rightxy, depth);
    fillDiamond(bot, dist, botxy, depth);
}

/**
  Helper method for fillAllDiamonds, fills the point for an individual diamond
  **/
void Terrain::fillDiamond(float2 ptof, int dist,float2 xy, int depth){
    float2 left(ptof.row-dist, ptof.col);
    float2 top(ptof.row, ptof.col-dist);
    float2 right(ptof.row+dist, ptof.col);
    float2 bot(ptof.row, ptof.col+dist);
    int leftindex = coordinateToIndex(left);
    int topindex = coordinateToIndex(top);
    int rightindex = coordinateToIndex(right);
    int botindex = coordinateToIndex(bot);
    int num_add= 0;
    float3 diamond(xy.x, xy.y,0);
    if (leftindex != -1){
        diamond.z += terrain[leftindex].z;
        num_add++;
    }
    if (topindex != -1){
        diamond.z += terrain[topindex].z;
        num_add++;
    }
    if (rightindex != -1){
        diamond.z += terrain[rightindex].z;
        num_add++;
    }
    if (botindex != -1){
        diamond.z += terrain[botindex].z;
        num_add++;
    }
    diamond.z /= num_add;
    diamond.z += getPerturb(mDepth-depth);

    terrain[coordinateToIndex(ptof)] = diamond;
}

GLuint Terrain::getTextureInt(int i){
    if(i < 0 || i > 3){
        return g_regions[0].texture;
    }
    return g_regions[i].texture;
}
