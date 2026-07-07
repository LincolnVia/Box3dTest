#pragma once
#include <box3d/math_functions.h>
#include <box3d/types.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

inline Vector3 B3Vec3ToVector3(const b3Vec3& v)
{
    return Vector3{ v.x, v.y, v.z };
}

inline Vector3 B3PosToVector3(const b3Pos& p)
{
    return B3Vec3ToVector3(b3ToVec3(p));
}

inline b3Vec3 Vector3ToB3Vec3(const Vector3& v)
{
    return b3Vec3{ v.x, v.y, v.z };
}

inline void DrawRayDebug(b3Pos origin, b3Vec3 translation)
{
    DrawLine3D(B3PosToVector3(origin), B3PosToVector3(b3OffsetPos(origin, translation)), RED);
}

inline Vector3 B3WorldTransformPoint(const b3WorldTransform& t, b3Vec3 local)
{
    return B3PosToVector3(b3TransformWorldPoint(t, local));
}

inline void DrawBoxWireframe(const b3WorldTransform& t, b3Vec3 halfExtents, Color color)
{
    const Vector3 p000 = B3WorldTransformPoint(t, { -halfExtents.x, -halfExtents.y, -halfExtents.z });
    const Vector3 p001 = B3WorldTransformPoint(t, { -halfExtents.x, -halfExtents.y,  halfExtents.z });
    const Vector3 p010 = B3WorldTransformPoint(t, { -halfExtents.x,  halfExtents.y, -halfExtents.z });
    const Vector3 p011 = B3WorldTransformPoint(t, { -halfExtents.x,  halfExtents.y,  halfExtents.z });

    const Vector3 p100 = B3WorldTransformPoint(t, {  halfExtents.x, -halfExtents.y, -halfExtents.z });
    const Vector3 p101 = B3WorldTransformPoint(t, {  halfExtents.x, -halfExtents.y,  halfExtents.z });
    const Vector3 p110 = B3WorldTransformPoint(t, {  halfExtents.x,  halfExtents.y, -halfExtents.z });
    const Vector3 p111 = B3WorldTransformPoint(t, {  halfExtents.x,  halfExtents.y,  halfExtents.z });

    // Bottom face
    DrawLine3D(p000, p100, color);
    DrawLine3D(p100, p101, color);
    DrawLine3D(p101, p001, color);
    DrawLine3D(p001, p000, color);

    // Top face
    DrawLine3D(p010, p110, color);
    DrawLine3D(p110, p111, color);
    DrawLine3D(p111, p011, color);
    DrawLine3D(p011, p010, color);

    // Vertical edges
    DrawLine3D(p000, p010, color);
    DrawLine3D(p100, p110, color);
    DrawLine3D(p101, p111, color);
    DrawLine3D(p001, p011, color);
}

inline Quaternion B3QuatToQuaternion(const b3Quat& q)
{
    return Quaternion{ q.v.x, q.v.y, q.v.z, q.s };
}

inline Matrix B3WorldTransformToMatrix(const b3WorldTransform& t, b3Vec3 scale)
{
    Matrix S = MatrixScale(scale.x, scale.y, scale.z);
    Matrix R = QuaternionToMatrix(B3QuatToQuaternion(t.q));
    const b3Vec3 position = b3ToVec3(t.p);
    Matrix T = MatrixTranslate(position.x, position.y, position.z);

    return MatrixMultiply(S, MatrixMultiply(R, T));
}

inline void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture.id);

    // Vertex data transformation can be defined with the commented lines,
    // but in this example we calculate the transformed vertex data directly when calling rlVertex3f()
    //rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(2.0f, 2.0f, 2.0f);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            // Front Face
            rlNormal3f(0.0f, 0.0f, 1.0f);       // Normal Pointing Towards Viewer
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            // Back Face
            rlNormal3f(0.0f, 0.0f, - 1.0f);     // Normal Pointing Away From Viewer
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            // Top Face
            rlNormal3f(0.0f, 1.0f, 0.0f);       // Normal Pointing Up
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            // Bottom Face
            rlNormal3f(0.0f, - 1.0f, 0.0f);     // Normal Pointing Down
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            // Right face
            rlNormal3f(1.0f, 0.0f, 0.0f);       // Normal Pointing Right
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            // Left Face
            rlNormal3f( - 1.0f, 0.0f, 0.0f);    // Normal Pointing Left
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
        rlEnd();
    //rlPopMatrix();

    rlSetTexture(0);
}

inline void DrawCubeTextureV(Texture2D texture, Vector3 position, Vector3 scale, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture.id);

    // Vertex data transformation can be defined with the commented lines,
    // but in this example we calculate the transformed vertex data directly when calling rlVertex3f()
    //rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(2.0f, 2.0f, 2.0f);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            // Front Face
            rlNormal3f(0.0f, 0.0f, 1.0f);       // Normal Pointing Towards Viewer
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - scale.x/2, y - scale.y/2, z + scale.z/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + scale.x/2, y - scale.y/2, z + scale.z/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + scale.x/2, y + scale.y/2, z + scale.z/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - scale.x/2, y + scale.y/2, z + scale.z/2);  // Top Left Of The Texture and Quad
            // Back Face
            rlNormal3f(0.0f, 0.0f, - 1.0f);     // Normal Pointing Away From Viewer
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - scale.x/2, y - scale.y/2, z - scale.z/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - scale.x/2, y + scale.y/2, z - scale.z/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + scale.x/2, y + scale.y/2, z - scale.z/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + scale.x/2, y - scale.y/2, z - scale.z/2);  // Bottom Left Of The Texture and Quad
            // Top Face
            rlNormal3f(0.0f, 1.0f, 0.0f);       // Normal Pointing Up
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - scale.x/2, y + scale.y/2, z - scale.z/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - scale.x/2, y + scale.y/2, z + scale.z/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + scale.x/2, y + scale.y/2, z + scale.z/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + scale.x/2, y + scale.y/2, z - scale.z/2);  // Top Right Of The Texture and Quad
            // Bottom Face
            rlNormal3f(0.0f, - 1.0f, 0.0f);     // Normal Pointing Down
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - scale.x/2, y - scale.y/2, z - scale.z/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + scale.x/2, y - scale.y/2, z - scale.z/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + scale.x/2, y - scale.y/2, z + scale.z/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - scale.x/2, y - scale.y/2, z + scale.z/2);  // Bottom Right Of The Texture and Quad
            // Right face
            rlNormal3f(1.0f, 0.0f, 0.0f);       // Normal Pointing Right
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + scale.x/2, y - scale.y/2, z - scale.z/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + scale.x/2, y + scale.y/2, z - scale.z/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + scale.x/2, y + scale.y/2, z + scale.z/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + scale.x/2, y - scale.y/2, z + scale.z/2);  // Bottom Left Of The Texture and Quad
            // Left Face
            rlNormal3f( - 1.0f, 0.0f, 0.0f);    // Normal Pointing Left
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - scale.x/2, y - scale.y/2, z - scale.z/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - scale.x/2, y - scale.y/2, z + scale.z/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - scale.x/2, y + scale.y/2, z + scale.z/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - scale.x/2, y + scale.y/2, z - scale.z/2);  // Top Left Of The Texture and Quad
        rlEnd();
    //rlPopMatrix();

    rlSetTexture(0);
}

inline void b3_DrawCube(b3Pos position, b3Vec3 scale, Color color)
{
    DrawCubeV(B3PosToVector3(position), B3Vec3ToVector3(scale), color);
}
inline void b3_DrawModel(Model model, Matrix transform)
{
    for (int i = 0; i < model.meshCount; ++i)
    {
        Mesh mesh = model.meshes[i];
        Material material = model.materials[model.meshMaterial[i]];
        DrawMesh(mesh, material, transform);
    }
}

inline void b3_DrawCubeTex(Texture2D texture, b3Pos position, b3Vec3 scale, Color color)
{
    DrawCubeTextureV(texture, B3PosToVector3(position), B3Vec3ToVector3(scale) * 2, color);
}
