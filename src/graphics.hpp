#pragma once
#include "Engine/global.hpp"

#include <box3d/math_functions.h>
#include <box3d/types.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

inline Vector3 B3Vec3ToVector3(const b3Vec3 &v) {
  return Vector3{v.x, v.y, v.z};
}

inline Vector3 B3PosToVector3(const b3Pos &p) {
  return B3Vec3ToVector3(b3ToVec3(p));
}

inline b3Vec3 Vector3ToB3Vec3(const Vector3 &v) {
  return b3Vec3{v.x, v.y, v.z};
}

inline void DrawRayDebug(b3Pos origin, b3Vec3 translation) {
  DrawLine3D(B3PosToVector3(origin),
             B3PosToVector3(b3OffsetPos(origin, translation)), RED);
}

inline Vector3 B3WorldTransformPoint(const b3WorldTransform &t, b3Vec3 local) {
  return B3PosToVector3(b3TransformWorldPoint(t, local));
}

inline void DrawBoxWireframe(const b3WorldTransform &t, b3Vec3 halfExtents,
                             Color color) {
  const Vector3 p000 = B3WorldTransformPoint(
      t, {-halfExtents.x, -halfExtents.y, -halfExtents.z});
  const Vector3 p001 =
      B3WorldTransformPoint(t, {-halfExtents.x, -halfExtents.y, halfExtents.z});
  const Vector3 p010 =
      B3WorldTransformPoint(t, {-halfExtents.x, halfExtents.y, -halfExtents.z});
  const Vector3 p011 =
      B3WorldTransformPoint(t, {-halfExtents.x, halfExtents.y, halfExtents.z});

  const Vector3 p100 =
      B3WorldTransformPoint(t, {halfExtents.x, -halfExtents.y, -halfExtents.z});
  const Vector3 p101 =
      B3WorldTransformPoint(t, {halfExtents.x, -halfExtents.y, halfExtents.z});
  const Vector3 p110 =
      B3WorldTransformPoint(t, {halfExtents.x, halfExtents.y, -halfExtents.z});
  const Vector3 p111 =
      B3WorldTransformPoint(t, {halfExtents.x, halfExtents.y, halfExtents.z});

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

inline Quaternion B3QuatToQuaternion(const b3Quat &q) {
  return Quaternion{q.v.x, q.v.y, q.v.z, q.s};
}

inline Matrix B3WorldTransformToMatrix(const b3WorldTransform &t,
                                       b3Vec3 scale) {
  Matrix S = MatrixScale(scale.x, scale.y, scale.z);
  Matrix R = QuaternionToMatrix(B3QuatToQuaternion(t.q));
  const b3Vec3 position = b3ToVec3(t.p);
  Matrix T = MatrixTranslate(position.x, position.y, position.z);

  return MatrixMultiply(S, MatrixMultiply(R, T));
}

inline void DrawCubeTexture(Texture2D texture, Vector3 position, float width,
                            float height, float length, Color color) {
  float x = position.x;
  float y = position.y;
  float z = position.z;

  // Set desired texture to be enabled while drawing following vertex data
  rlSetTexture(texture.id);

  // Vertex data transformation can be defined with the commented lines,
  // but in this example we calculate the transformed vertex data directly when
  // calling rlVertex3f()
  // rlPushMatrix();
  // NOTE: Transformation is applied in inverse order (scale -> rotate ->
  // translate)
  // rlTranslatef(2.0f, 0.0f, 0.0f);
  // rlRotatef(45, 0, 1, 0);
  // rlScalef(2.0f, 2.0f, 2.0f);

  rlBegin(RL_QUADS);
  rlColor4ub(color.r, color.g, color.b, color.a);
  // Front Face
  rlNormal3f(0.0f, 0.0f, 1.0f); // Normal Pointing Towards Viewer
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x - width / 2, y - height / 2,
             z + length / 2); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x + width / 2, y - height / 2,
             z + length / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x + width / 2, y + height / 2,
             z + length / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x - width / 2, y + height / 2,
             z + length / 2); // Top Left Of The Texture and Quad
  // Back Face
  rlNormal3f(0.0f, 0.0f, -1.0f); // Normal Pointing Away From Viewer
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x - width / 2, y - height / 2,
             z - length / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x - width / 2, y + height / 2,
             z - length / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x + width / 2, y + height / 2,
             z - length / 2); // Top Left Of The Texture and Quad
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x + width / 2, y - height / 2,
             z - length / 2); // Bottom Left Of The Texture and Quad
  // Top Face
  rlNormal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x - width / 2, y + height / 2,
             z - length / 2); // Top Left Of The Texture and Quad
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x - width / 2, y + height / 2,
             z + length / 2); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x + width / 2, y + height / 2,
             z + length / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x + width / 2, y + height / 2,
             z - length / 2); // Top Right Of The Texture and Quad
  // Bottom Face
  rlNormal3f(0.0f, -1.0f, 0.0f); // Normal Pointing Down
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x - width / 2, y - height / 2,
             z - length / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x + width / 2, y - height / 2,
             z - length / 2); // Top Left Of The Texture and Quad
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x + width / 2, y - height / 2,
             z + length / 2); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x - width / 2, y - height / 2,
             z + length / 2); // Bottom Right Of The Texture and Quad
  // Right face
  rlNormal3f(1.0f, 0.0f, 0.0f); // Normal Pointing Right
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x + width / 2, y - height / 2,
             z - length / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x + width / 2, y + height / 2,
             z - length / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x + width / 2, y + height / 2,
             z + length / 2); // Top Left Of The Texture and Quad
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x + width / 2, y - height / 2,
             z + length / 2); // Bottom Left Of The Texture and Quad
  // Left Face
  rlNormal3f(-1.0f, 0.0f, 0.0f); // Normal Pointing Left
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x - width / 2, y - height / 2,
             z - length / 2); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x - width / 2, y - height / 2,
             z + length / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x - width / 2, y + height / 2,
             z + length / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x - width / 2, y + height / 2,
             z - length / 2); // Top Left Of The Texture and Quad
  rlEnd();
  // rlPopMatrix();

  rlSetTexture(0);
}

inline void DrawCubeTextureV(Texture2D texture, Vector3 position, Vector3 scale,
                             Color color) {
  float x = position.x;
  float y = position.y;
  float z = position.z;

  // Set desired texture to be enabled while drawing following vertex data
  rlSetTexture(texture.id);

  // Vertex data transformation can be defined with the commented lines,
  // but in this example we calculate the transformed vertex data directly when
  // calling rlVertex3f()
  // rlPushMatrix();
  // NOTE: Transformation is applied in inverse order (scale -> rotate ->
  // translate)
  // rlTranslatef(2.0f, 0.0f, 0.0f);
  // rlRotatef(45, 0, 1, 0);
  // rlScalef(2.0f, 2.0f, 2.0f);

  rlBegin(RL_QUADS);
  rlColor4ub(color.r, color.g, color.b, color.a);
  // Front Face
  rlNormal3f(0.0f, 0.0f, 1.0f); // Normal Pointing Towards Viewer
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x - scale.x / 2, y - scale.y / 2,
             z + scale.z / 2); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x + scale.x / 2, y - scale.y / 2,
             z + scale.z / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x + scale.x / 2, y + scale.y / 2,
             z + scale.z / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x - scale.x / 2, y + scale.y / 2,
             z + scale.z / 2); // Top Left Of The Texture and Quad
  // Back Face
  rlNormal3f(0.0f, 0.0f, -1.0f); // Normal Pointing Away From Viewer
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x - scale.x / 2, y - scale.y / 2,
             z - scale.z / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x - scale.x / 2, y + scale.y / 2,
             z - scale.z / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x + scale.x / 2, y + scale.y / 2,
             z - scale.z / 2); // Top Left Of The Texture and Quad
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x + scale.x / 2, y - scale.y / 2,
             z - scale.z / 2); // Bottom Left Of The Texture and Quad
  // Top Face
  rlNormal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x - scale.x / 2, y + scale.y / 2,
             z - scale.z / 2); // Top Left Of The Texture and Quad
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x - scale.x / 2, y + scale.y / 2,
             z + scale.z / 2); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x + scale.x / 2, y + scale.y / 2,
             z + scale.z / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x + scale.x / 2, y + scale.y / 2,
             z - scale.z / 2); // Top Right Of The Texture and Quad
  // Bottom Face
  rlNormal3f(0.0f, -1.0f, 0.0f); // Normal Pointing Down
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x - scale.x / 2, y - scale.y / 2,
             z - scale.z / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x + scale.x / 2, y - scale.y / 2,
             z - scale.z / 2); // Top Left Of The Texture and Quad
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x + scale.x / 2, y - scale.y / 2,
             z + scale.z / 2); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x - scale.x / 2, y - scale.y / 2,
             z + scale.z / 2); // Bottom Right Of The Texture and Quad
  // Right face
  rlNormal3f(1.0f, 0.0f, 0.0f); // Normal Pointing Right
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x + scale.x / 2, y - scale.y / 2,
             z - scale.z / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x + scale.x / 2, y + scale.y / 2,
             z - scale.z / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x + scale.x / 2, y + scale.y / 2,
             z + scale.z / 2); // Top Left Of The Texture and Quad
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x + scale.x / 2, y - scale.y / 2,
             z + scale.z / 2); // Bottom Left Of The Texture and Quad
  // Left Face
  rlNormal3f(-1.0f, 0.0f, 0.0f); // Normal Pointing Left
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(x - scale.x / 2, y - scale.y / 2,
             z - scale.z / 2); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(1.0f, 0.0f);
  rlVertex3f(x - scale.x / 2, y - scale.y / 2,
             z + scale.z / 2); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(1.0f, 1.0f);
  rlVertex3f(x - scale.x / 2, y + scale.y / 2,
             z + scale.z / 2); // Top Right Of The Texture and Quad
  rlTexCoord2f(0.0f, 1.0f);
  rlVertex3f(x - scale.x / 2, y + scale.y / 2,
             z - scale.z / 2); // Top Left Of The Texture and Quad
  rlEnd();
  // rlPopMatrix();

  rlSetTexture(0);
}

inline void b3_DrawCube(b3Pos position, b3Vec3 scale, Color color) {
  DrawCubeV(B3PosToVector3(position), B3Vec3ToVector3(scale), color);
}
inline void b3_DrawModel(Model model, Matrix transform) {
  for (int i = 0; i < model.meshCount; ++i) {
    Mesh mesh = model.meshes[i];
    Material material = model.materials[model.meshMaterial[i]];
    DrawMesh(mesh, material, transform);
  }
}

inline void b3_DrawCubeTex(Texture2D texture, b3Pos position, b3Vec3 scale,
                           Color color) {
  DrawCubeTextureV(texture, B3PosToVector3(position),
                   B3Vec3ToVector3(scale) * 2, color);
}

inline TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama,
                                        int size, int format) {
  TextureCubemap cubemap = {0};

  rlDisableBackfaceCulling(); // Disable backface culling to render inside the
                              // cube

  // STEP 1: Setup framebuffer
  //------------------------------------------------------------------------------------------
  unsigned int rbo = rlLoadTextureDepth(size, size, true);
  cubemap.id = rlLoadTextureCubemap(0, size, format, 1);

  unsigned int fbo = rlLoadFramebuffer();
  rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER,
                      0);
  rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0,
                      RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

  // Check if framebuffer is complete with attachments (valid)
  if (rlFramebufferComplete(fbo))
    TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully",
             fbo);
  //------------------------------------------------------------------------------------------

  // STEP 2: Draw to framebuffer
  //------------------------------------------------------------------------------------------
  // NOTE: Shader is used to convert HDR equirectangular environment map to
  // cubemap equivalent (6 faces)
  rlEnableShader(shader.id);

  // Define projection matrix and send it to shader
  Matrix matFboProjection = MatrixPerspective(
      90.0 * DEG2RAD, 1.0, rlGetCullDistanceNear(), rlGetCullDistanceFar());
  rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION],
                     matFboProjection);

  // Define view matrix for every side of the cubemap
  Matrix fboViews[6] = {
      MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{1.0f, 0.0f, 0.0f},
                   Vector3{0.0f, -1.0f, 0.0f}),
      MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{-1.0f, 0.0f, 0.0f},
                   Vector3{0.0f, -1.0f, 0.0f}),
      MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 1.0f, 0.0f},
                   Vector3{0.0f, 0.0f, 1.0f}),
      MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, -1.0f, 0.0f},
                   Vector3{0.0f, 0.0f, -1.0f}),
      MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 0.0f, 1.0f},
                   Vector3{0.0f, -1.0f, 0.0f}),
      MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 0.0f, -1.0f},
                   Vector3{0.0f, -1.0f, 0.0f})};

  rlViewport(0, 0, size, size); // Set viewport to current fbo dimensions

  // Activate and enable texture for drawing to cubemap faces
  rlActiveTextureSlot(0);
  rlEnableTexture(panorama.id);

  for (int i = 0; i < 6; i++) {
    // Set the view matrix for the current cube face
    rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);

    // Select the current cubemap face attachment for the fbo
    // WARNING: This function by default enables->attach->disables fbo!!!
    rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0,
                        RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
    rlEnableFramebuffer(fbo);

    // Load and draw a cube, it uses the current enabled texture
    rlClearScreenBuffers();
    rlLoadDrawCube();

    // ALTERNATIVE: Try to use internal batch system to draw the cube instead of
    // rlLoadDrawCube for some reason this method does not work, maybe due to
    // cube triangles definition? normals pointing out?
    // TODO: Investigate this issue...
    // rlSetTexture(panorama.id); // WARNING: It must be called after enabling
    // current framebuffer if using internal batch system!
    // rlClearScreenBuffers();
    // DrawCubeV(Vector3Zero(), Vector3One(), WHITE);
    // rlDrawRenderBatchActive();
  }
  //------------------------------------------------------------------------------------------

  // STEP 3: Unload framebuffer and reset state
  //------------------------------------------------------------------------------------------
  rlDisableShader();        // Unbind shader
  rlDisableTexture();       // Unbind texture
  rlDisableFramebuffer();   // Unbind framebuffer
  rlUnloadFramebuffer(fbo); // Unload framebuffer (and automatically attached
                            // depth texture/renderbuffer)

  // Reset viewport dimensions to default
  rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
  rlEnableBackfaceCulling();
  //------------------------------------------------------------------------------------------

  cubemap.width = size;
  cubemap.height = size;
  cubemap.mipmaps = 1;
  cubemap.format = format;

  return cubemap;
}
inline Model createSkybox(Shader &shdrCubemap) {
  Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);

  Model skybox = LoadModelFromMesh(cube);
  skybox.materials[0].shader = LoadShader(
      TextFormat("resources/shaders/glsl%i/skybox.vs", GLSL_VERSION),
      TextFormat("resources/shaders/glsl%i/skybox.fs", GLSL_VERSION));

  int environmentMap = MATERIAL_MAP_CUBEMAP;
  SetShaderValue(
      skybox.materials[0].shader,
      GetShaderLocation(skybox.materials[0].shader, "environmentMap"),
      &environmentMap, SHADER_UNIFORM_INT);
  int doGamma = useHDR ? 1 : 0;
  SetShaderValue(skybox.materials[0].shader,
                 GetShaderLocation(skybox.materials[0].shader, "doGamma"),
                 &doGamma, SHADER_UNIFORM_INT);
  int vflipped = useHDR ? 1 : 0;
  SetShaderValue(skybox.materials[0].shader,
                 GetShaderLocation(skybox.materials[0].shader, "vflipped"),
                 &vflipped, SHADER_UNIFORM_INT);

  int equirectangularMap = 0;
  SetShaderValue(shdrCubemap,
                 GetShaderLocation(shdrCubemap, "equirectangularMap"),
                 &equirectangularMap, SHADER_UNIFORM_INT);

  char skyboxFileName[256] = {0};

  if (useHDR) {
    TextCopy(skyboxFileName, "resources/textures/skyboxes/skybox.hdr");

    // Load HDR panorama (sphere) texture
    Texture2D panorama = LoadTexture(skyboxFileName);

    // Generate cubemap (texture with 6 quads-cube-mapping) from panorama HDR
    // texture NOTE 1: New texture is generated rendering to texture, shader
    // calculates the sphere->cube coordinates mapping NOTE 2: It seems on some
    // Android devices WebGL, fbo does not properly support a FLOAT-based
    // attachment, despite texture can be successfully created.. so using
    // PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 instead of
    // PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = GenTextureCubemap(
        shdrCubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    UnloadTexture(
        panorama); // Texture not required anymore, cubemap already generated
  } else {
    // TODO: WARNING: On PLATFORM_WEB it requires a big amount of memory to
    // process input image and generate the required cubemap image to be passed
    // to rlLoadTextureCubemap()
    Image image = LoadImage("resources/skybox.png");
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture =
        LoadTextureCubemap(image, CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(image);
  }

  return skybox;
}

inline void DrawSkybox(Model &skybox) {
  rlDisableBackfaceCulling();
  rlDisableDepthMask();
  DrawModel(skybox, Vector3{0, 0, 0}, 1.0f, WHITE);
  rlEnableBackfaceCulling();
  rlEnableDepthMask();
}
