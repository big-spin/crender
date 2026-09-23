# crender

A simple software renderer written in C with a minimal set of dependecies.

It can render most OBJ (wavefront) models as well as their respective textures (only in .ppm format for now)
with diffuse shading.

## Scenes

Scene files (ending in .scene) describe a scene through a set of commands that are interpreted by the program during scene loading.

There are currently 6 commands available:

`LOAD <path-to-obj-file>`

This loads and renders a new model and also assigns it an index in the scene which is used by other commands to acess the object later. The indexes are based on load order, first file loaded gets index 0, seconds one gets index 1, etc ...

`MOVE <id> x/y/z`

Moves an object specified by `id` to a new position.

`ROTATE <id> x/y/z`

Rotates an object specified by `id`

`SCALE <id> x/y/z`

Scales an object specified by `id`

`ASSIGN <id> <function-name>`

Assigns a function to an object specified by `id`

This function must be defined in [main.c](src/main.c) and obey the following structure:

```
void foo(Object *obj, Event *ev, Camera *cam) {
        ...
}
```

With these functions you can move a specific object based on keyboard input provided by the Event argument and move
the camera as well.

To then assign it to an object, the scene loader must be aware of the function, so you need to create a
NameFunctionPair array, pretty much a dictionary that links a string function name key to the actual function in
[main.c](src/main.c) and you need to then pass that array to the scene loader like so:

```
NameFunctionPair functions[1] = {
        {"foo", foo},
};

// number of custom functions defined
int functionCount = 1;

LoadSceneFromFile( ... , functions, functionCount)
```

`TEXTURE <id> <path-to-ppm-file>`

Will load a texture into memory and assign it to an object specified through it's id.

### Example Scene

`example.scene`
```
LOAD data/car01/car.obj
MOVE 0 -6/-1/-6
ROTATE 0 0/90/0
SCALE 0 0.5/1/2
ASSIGN 0 spinCar
TEXTURE 0 data/car01/car.ppm
```

`main.c`
```
void SpinCar(Object *obj, Event *ev, Camera *cam) {
        obj->rotation.y += 1;
}

int main(...) {
        ...
        NameFunctionPair functions[1] {
                {"spinCar", SpinCar},
        };

        int sceneSize = LoadSceneFromFile( ... , functions, 1);
        ...
}
```

## License
`crender` is free and licensed under the [BSD 3-Clause License](LICENSE).