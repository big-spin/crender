#include "custom-types.h"

#define SCENE_LOADER_IMPL
#include "scene-loader.h"

#define X11_IMPL
#include "x11.h"

#include "render.h"
#include "math-utils.h"

int width = 640;
int height = 480;

int main(int argc, char *argv[]) {
        if (argc < 2) {
                puts("No scene file provided\n");
                return 0;
        }

        OpenX11Window();

        FrameBuffer buffer = {
            .data=malloc(width * height * sizeof(uint32_t)),
            .width=(int)(width / RENDER_SCALE),
            .height=(int)(height / RENDER_SCALE),
        };

        float *depthBuffer = malloc((int)(width / RENDER_SCALE) * (int)(height / RENDER_SCALE) * sizeof(float));

        CreateXImage(&buffer, (int)(width / RENDER_SCALE), (int)(height / RENDER_SCALE));

        Object *scene = NULL;

        int sceneSize = LoadSceneFromFile(argv[1], &scene, NULL, 0);
        if (sceneSize == 0) return 1;

        Camera cam = {
            .pos = {0.0, 0.0, 0.0},
            .pitch = 0.0,
            .yaw = 0.0,
            .speed = 1.0,
        };

        Event ev;

        ev.quit = 0;
        ev.wireframeMode = 0;

        struct timespec deltaTimeClock;
        double deltaTime = 0.0f;

        clock_gettime(CLOCK_MONOTONIC, &deltaTimeClock);

        while (ev.quit != 1) {
                ClearBuffer(&buffer);
                ClearDepthBuffer(depthBuffer, (int)(width / RENDER_SCALE), (int)(height / RENDER_SCALE));

                for (int i = 0; i < sceneSize; i++) {
                        if (scene[i].hasFunction == 1) {
                                scene[i].func(&scene[i], &ev, &cam);
                        }

                        for (int j = 0; j < scene[i].triangleCount; j++) {
                                RenderTriangle(
                                        &scene[i].mesh[j], &scene[i],
                                        &cam, depthBuffer, &buffer,
                                        ev.wireframeMode, 
                                        (int)(width / RENDER_SCALE), (int)(height / RENDER_SCALE)
                                );
                        }
                }

                PresentBuffer(&buffer);

                int didWindowResize = X11Input(&cam, &ev, &width, &height);
                if (didWindowResize == 1) {
                        depthBuffer = realloc(
                                depthBuffer,
                                (int)(width / RENDER_SCALE) * (int)(height / RENDER_SCALE) * sizeof(float)
                        );

                        DestroyXImage();

                        buffer.data = malloc(
                                (int)(width / RENDER_SCALE) * (int)(height / RENDER_SCALE) * sizeof(uint32_t)
                        );
                        buffer.width = (int)(width / RENDER_SCALE);
                        buffer.height = (int)(height / RENDER_SCALE);

                        CreateXImage(&buffer, (int)(width / RENDER_SCALE), (int)(height / RENDER_SCALE));
                }

                Vec3 forward = RotateVec3AroundAxis(
                    (Vec3){0.0, 0.0, -cam.speed * deltaTime}, cam.yaw, Y_AXIS);
                Vec3 backward = RotateVec3AroundAxis(
                    (Vec3){0.0, 0.0, cam.speed * deltaTime}, cam.yaw, Y_AXIS);
                Vec3 right = RotateVec3AroundAxis(
                    (Vec3){cam.speed * deltaTime, 0.0, 0.0}, cam.yaw, Y_AXIS);
                Vec3 left = RotateVec3AroundAxis(
                    (Vec3){-cam.speed * deltaTime, 0.0, 0.0}, cam.yaw, Y_AXIS);

                if (ev.keys.w == 1)
                        cam.pos = AddVec3(cam.pos, forward);
                if (ev.keys.a == 1)
                        cam.pos = AddVec3(cam.pos, left);
                if (ev.keys.s == 1)
                        cam.pos = AddVec3(cam.pos, backward);
                if (ev.keys.d == 1)
                        cam.pos = AddVec3(cam.pos, right);

                if (ev.keys.space == 1)
                        cam.pos.y += cam.speed * deltaTime;
                if (ev.keys.shift == 1)
                        cam.pos.y -= cam.speed * deltaTime;
                
                if (ev.keys.ctrl == 1)
                        cam.speed = 2.0;
                if (ev.keys.ctrl == 0)
                        cam.speed = 1.0;

                struct timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);

                deltaTime =
                    (double)(now.tv_sec - deltaTimeClock.tv_sec) +
                    (double)(now.tv_nsec - deltaTimeClock.tv_nsec) / 1000000000;

                deltaTimeClock = now;

                printf("%ffps, %fms\n", (1 / deltaTime), deltaTime * 1000);
        }

        CloseX11Window();

        free(depthBuffer);

        for (int i = 0; i < sceneSize; i++) {
                free(scene[i].mesh);
                if (scene[i].hasTexture)
                        free(scene[i].tex.data);
        }
        free(scene);

        return 0;
}